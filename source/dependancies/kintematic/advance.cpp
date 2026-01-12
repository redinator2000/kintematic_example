#include "advance.hpp"
#include "raycast.hpp"
#include "minkowski.hpp"
#include <algorithm>
#include <ranges>

namespace kint
{
struct Slide_Return
{
    Rational2D clipped_velocity;
    Rational2D next_velocity;
};
Slide_Return slide(Rational2D vel, const Impact & impact)
{
    Rational2D fh = vel * impact.t;
    Rational2D sh = vel * (1 - impact.t);
    Rational2D edge = Rational2D(impact.edge.node);
    Rational2D sh_rotated = dot(sh, edge) * edge;
    i2d::ntype edge_ls = dot(impact.edge.node, impact.edge.node);
    Rational2D nv = (sh_rotated / edge_ls).reduced();
    return {.clipped_velocity = Rational2D(fh + nv).reduced(),
            .next_velocity = nv};
};
bool beneath_line(Shape_Line line, i2d point)
{
    if(gcf::cross(line.node, point - line.position) < 0)
        return false;
    return true;
}
bool in_line_domain(Shape_Line line, i2d point)
{
    if(gcf::dot(line.node, point - line.position) <= 0)
        return false;
    if(gcf::dot(line.node, point - line.node_absolute()) >= 0)
        return false;
    return true;
}
std::vector<Impact> impact_occlusion_filter(std::span<const Impact> impacts)
{
    if (impacts.size() < 2)
        return std::vector<Impact>(impacts.begin(), impacts.end());

    std::vector<Impact> filtered;
    filtered.reserve(impacts.size());

    for(const auto & im : impacts)
    {
        bool beneath_any =
        std::ranges::any_of(impacts, [&](auto const& other)
        {
            if (&other == &im)
                return false;

            return beneath_line(other.edge, im.edge.position)
                    && beneath_line(other.edge, im.edge.node_absolute())
                    && (in_line_domain(other.edge, im.edge.position)
                        || in_line_domain(other.edge, im.edge.node_absolute())) //if neither points are in_line_domain, its probably two edges sharing a tip
                    && gcf::cross(im.edge.node, other.edge.node) != 0;
        });

        if(!beneath_any)
            filtered.push_back(im);
    }

    return filtered;
}
struct Clip_Return
{
    i2d clipped_velocity;
    std::optional<i2d> next_velocity;
    std::vector<Impact> impacts;
};
Clip_Return sr_to_cr(const Slide_Return & sr, std::vector<Impact> && impacts)
{
    return {trunc(sr.clipped_velocity), trunc(sr.next_velocity), std::move(impacts)};
}
Clip_Return clip_and_slide(Shape_Point rect, const Minkowski_Set & mset)
{
    std::vector<Impact> impacts_closest = raycast_Minkowski_Set(rect.position, rect.position + rect.velocity, mset);

    std::vector<Impact> impacts_filtered = impact_occlusion_filter(impacts_closest);

    if(impacts_filtered.empty())
        return {rect.velocity, std::nullopt,
                std::move(impacts_filtered)};

    if(impacts_filtered.size() == 1)
        return sr_to_cr(slide(Rational2D(rect.velocity), impacts_filtered[0]),  std::move(impacts_filtered));

    i2d e0 = impacts_filtered[0].edge.node;
    bool all_parallel = true;

    for(size_t i = 1; i < impacts_filtered.size(); i++)
    {
        if(gcf::cross(e0, impacts_filtered[i].edge.node) != 0)
        {
            all_parallel = false;
            break;
        }
    }

    if(!all_parallel)
    {
        // True corner: multiple non-parallel edges hit in their interior
        return {trunc(Rational2D(rect.velocity) * impacts_filtered[0].t),
                i2d{0, 0},
                std::move(impacts_filtered)};
    }

    // All edges parallel, slide along any of them
    return sr_to_cr(slide(Rational2D(rect.velocity), impacts_filtered[0]), std::move(impacts_filtered));
}
std::vector<Impact> move_and_slide(Shape_Rectangle & rect, const Minkowski_Set & mset, bool better_next_velocity)
{
    Clip_Return clipped = clip_and_slide(shape_position_point(rect), mset);
    rect.position += clipped.clipped_velocity;
    if(clipped.next_velocity)
        rect.velocity = *clipped.next_velocity;
    else if(better_next_velocity)
    {
        Clip_Return nclipped = clip_and_slide(shape_position_point(rect), mset);
        rect.velocity = nclipped.clipped_velocity;
    }
    return clipped.impacts;
}
}
