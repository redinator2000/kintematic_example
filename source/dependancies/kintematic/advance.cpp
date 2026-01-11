#include "advance.hpp"
#include "raycast.hpp"
#include "minkowski.hpp"
#include <algorithm>
#include <ranges>

namespace kint
{
Rational2D slide(Rational2D vel, const Impact & impact)
{
    Rational2D fh = vel * impact.t;
    Rational2D sh = vel * (1 - impact.t);
    Rational2D edge = Rational2D(impact.edge.node);
    Rational2D sh_rotated = dot(sh, edge) * edge;
    i2d::ntype edge_ls = dot(impact.edge.node, impact.edge.node);
    return Rational2D(fh + sh_rotated / edge_ls).reduced();
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

i2d clip_and_slide(Shape_Point rect, const Minkowski_Set & mset)
{
    i2d A = rect.position;
    i2d B = rect.position + rect.velocity;

    std::vector<Impact> impacts;

    const auto find_best_impact = [&](const auto & shape, size_t /*shape_id*/)
    {
        if (auto ni = raycast_unmoving(A, B, shape))
        {
            if (!impacts.empty() && ni->t < impacts[0].t)
                impacts.clear();

            if (impacts.empty() || ni->t == impacts[0].t)
                impacts.push_back(*ni);
        }
    };

    for (size_t i = 0; i < mset.rects.size(); i++)
        find_best_impact(mset.rects[i], mset.rect_id[i]);
    for (size_t i = 0; i < mset.polys.size(); i++)
        find_best_impact(mset.polys[i], mset.poly_id[i]);

    std::vector<Impact> impacts_filtered = impact_occlusion_filter(std::span(impacts.begin(), impacts.end()));

    if (impacts_filtered.empty())
        return rect.velocity;

    if (impacts_filtered.size() == 1)
        return trunc(slide(Rational2D(rect.velocity), impacts_filtered[0]));

    i2d e0 = impacts_filtered[0].edge.node;
    bool all_parallel = true;

    for (size_t i = 1; i < impacts_filtered.size(); i++)
    {
        if(gcf::cross(e0, impacts_filtered[i].edge.node) != 0)
        {
            all_parallel = false;
            break;
        }
    }

    if (!all_parallel)
    {
        // True corner: multiple non-parallel edges hit in their interior
        return i2d{0, 0};
    }

    // All edges parallel → slide along any of them
    return trunc(slide(Rational2D(rect.velocity), impacts_filtered[0]));
}

}
