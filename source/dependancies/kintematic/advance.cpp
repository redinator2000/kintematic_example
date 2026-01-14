#include "advance.hpp"
#include "raycast.hpp"
#include "minkowski.hpp"
#include "collision.hpp"
#include <algorithm>
#include <ranges>
#include <cassert>

namespace kint
{
bool beneath_line(Shape_Line line, i2d point) // true if on line
{
    if(gcf::cross(line.node, point - line.position) < 0)
        return false;
    return true;
}
bool in_line_domain(Shape_Line line, i2d point) // false if on edge
{
    if(gcf::dot(line.node, point - line.position) <= 0)
        return false;
    if(gcf::dot(line.node, point - line.node_absolute()) >= 0)
        return false;
    return true;
}
Rational shrunk(Rational r)
{
    assert(r != 0);
    Rational old_r = r;
    if(r > 1)
        r -= 1;
    else if(r < -1)
        r += 1;
    else
        r = 0;
    assert(abs(r) < abs(old_r));
    return r;
}
bool shrunk_checker(Rational2D a, Rational2D b)
{
    return (abs(a.x) < abs(b.x) && abs(a.y) <= abs(b.y)) ||
           (abs(a.x) <= abs(b.x) && abs(a.y) < abs(b.y));
}
i2d slide_trunc(i2d pos, Rational2D ideal_vel, const Impact & impact)
{
    const auto & clear = [pos, impact](auto v) -> bool
    {
        decltype(v) A = (decltype(v))impact.edge.node;
        decltype(v) B = v + (decltype(v))(pos - impact.edge.position);
        auto c = gcf::cross(A, B);
        bool bl = c <= 0;
        return bl;
    };
    Rational2D reducer = ideal_vel;
    while(gcf::dot(reducer, reducer) >= 1)
    {
        assert(clear(reducer));
        if(reducer.x == 0 || reducer.y == 0)
            return trunc(reducer);
        if(Rational2D(trunc(reducer)) == reducer)
            return trunc(reducer);
        Rational2D old_r = reducer;
        i2d t = trunc(reducer);
        if(clear(t))
            return t;
        Rational2D rx = Rational2D(shrunk(reducer.x), reducer.y);
        Rational2D ry = Rational2D(reducer.x, shrunk(reducer.y));
        assert(shrunk_checker(rx, reducer));
        assert(shrunk_checker(ry, reducer));
        bool rx_clear = clear(rx);
        bool ry_clear = clear(ry);
        if(rx_clear && ry_clear) // i don't think this should happen
        {
            if(gcf::dot(rx - reducer, rx - reducer) < gcf::dot(ry - reducer, ry - reducer))
                reducer = rx;
            else
                reducer = ry;
        }
        else if(rx_clear)
            reducer = rx;
        else if(ry_clear)
            reducer = ry;
        else
            return i2d{0, 0}; // i don't think this should happen
        assert(shrunk_checker(reducer, old_r));
    }
    return i2d{0, 0};
}
struct Slide_Return
{
    i2d clipped_velocity;
    i2d next_velocity;
};
Slide_Return slide(i2d pos, Rational2D vel, const Impact & impact)
{
    Rational2D fh = vel * impact.t;
    assert(Rational2D(pos) + fh == impact.position);
    Rational2D sh = (vel * (1 - impact.t)).reduced();
    Rational2D edge = Rational2D(impact.edge.node);
    Rational2D sh_rotated = gcf::dot(sh, edge) * edge;
    i2d::ntype edge_ls = gcf::dot(impact.edge.node, impact.edge.node);
    Rational2D nv = (sh_rotated / edge_ls).reduced();
    Rational2D cv = (fh + nv).reduced();
    return {.clipped_velocity = slide_trunc(pos, cv, impact),
            .next_velocity = trunc(cv - fh)};
};
std::vector<Impact> impact_occlusion_filter(std::span<const Impact> impacts)
{
    if(impacts.size() < 2)
        return std::vector<Impact>(impacts.begin(), impacts.end());

    std::vector<Impact> filtered;
    filtered.reserve(impacts.size());

    for(const auto & im : impacts)
    {
        bool beneath_any =
        std::ranges::any_of(impacts, [&](const auto & other)
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
    return {sr.clipped_velocity, sr.next_velocity, std::move(impacts)};
}
Clip_Return clip_and_slide(Shape_Point rect, const Minkowski_Set & mset)
{
    std::vector<Impact> impacts_closest = raycast_Minkowski_Set(rect.position, rect.position + rect.velocity, mset);

    std::vector<Impact> impacts_filtered = impact_occlusion_filter(impacts_closest);

    if(impacts_filtered.empty())
        return {rect.velocity, std::nullopt,
                std::move(impacts_filtered)};

    if(impacts_filtered.size() == 1)
        return sr_to_cr(slide(rect.position, Rational2D(rect.velocity), impacts_filtered[0]),  std::move(impacts_filtered));

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
    return sr_to_cr(slide(rect.position, Rational2D(rect.velocity), impacts_filtered[0]), std::move(impacts_filtered));
}
i2d::ntype length_axis_aligned(i2d v)
{
    assert(v.x == 0 || v.y == 0);
    return std::max(std::abs(v.x), std::abs(v.y));
}
std::vector<Impact> move_and_slide(Shape_Rectangle & rect, const Minkowski_Set & mset, bool better_next_velocity, i2d::ntype max_escape_distance)
{
    std::vector<Impact> all_impacts;

    collides_Minkowski_Set_return cmsr = collides_Minkowski_Set(rect.position, mset);
    const auto drag_collision = [&](const auto & shape)
    {
        i2d m = shape.velocity;
        i2d::ntype mm = dot(m, m);
        if(mm)
        {
            i2d::ntype current_flow = dot(m, rect.velocity);
            if(current_flow < mm)
            {
                rect.velocity += ((mm - current_flow) * m) / mm;
            }
        }
        constexpr std::array<i2d, 4> escape_vectors = {i2d{0, 1}, i2d{0, -1}, i2d{-1, 0}, i2d{1, 0}};
        std::optional<i2d> best_escape = std::nullopt;
        std::optional<Impact> best_escape_impact = std::nullopt;
        for(const auto & ev : escape_vectors)
        {
            i2d evl = ev * max_escape_distance;
            std::optional<Impact> impact = raycast_unmoving(rect.position + evl, rect.position, shape);
            if(impact)
            {
                i2d bump = trunc(Rational2D(evl) * (1 - impact->t)); //todo: do the opposite of trunc, where it always gets a bit longer
                if(!best_escape || length_axis_aligned(bump) < length_axis_aligned(*best_escape))
                {
                    best_escape = bump;
                    best_escape_impact = impact;
                }
            }
        }
        assert(bool(best_escape) == bool(best_escape_impact));
        if(best_escape)
        {
            all_impacts.push_back(*best_escape_impact);
            if(!collides_Minkowski_Set(rect.position + *best_escape, mset).any_collision())
                rect.position += *best_escape;
        }

    };
    for(const auto & s : cmsr.rect_collisions)
        drag_collision(mset.rects[s]);
    for(const auto & s : cmsr.poly_collisions)
        drag_collision(mset.polys[s]);

    std::optional<i2d> nv = std::nullopt;
    Clip_Return clipped;
    do
    {
        clipped = clip_and_slide(shape_position_point(rect), mset);
        rect.velocity = clipped.clipped_velocity;
        if(clipped.next_velocity)
            nv = clipped.next_velocity;
        std::move(clipped.impacts.begin(), clipped.impacts.end(), std::back_inserter(all_impacts));
    }
    while(clipped.next_velocity);

    rect.position += rect.velocity;

    /*if(better_next_velocity)
    {
        Clip_Return nclipped = clip_and_slide(shape_position_point(rect), mset);
        rect.velocity = nclipped.clipped_velocity;
    }
    else*/ if(nv)
        rect.velocity = *nv;

    return all_impacts;
}
}
