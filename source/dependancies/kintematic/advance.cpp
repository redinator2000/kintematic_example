#include "advance.hpp"
#include "raycast.hpp"
#include "minkowski.hpp"
#include "collision.hpp"
#include <algorithm>
#include <ranges>
#include <cassert>

namespace kint
{
std::optional<i2d> Platformer_Properties::sticky_slope_reflected() const
{
    if(!sticky_slope)
        return std::nullopt;
    return *sticky_slope - 2 * gcf::dot(*sticky_slope, down_direction) * down_direction;
}
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
    if(r.reduced().den != 1)
        r = trunc(r);
    else if(r > 1)
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
Rational2D slide_exact(i2d pos, Rational2D vel, const Impact & impact)
{
    Rational2D fh = vel * impact.t;
    assert(Rational2D(pos) + fh == impact.position);
    Rational2D sh = (vel * (1 - impact.t)).reduced();
    Rational2D edge = Rational2D(impact.edge.node);
    Rational2D sh_rotated = gcf::dot(sh, edge) * edge;
    i2d::ntype edge_ls = gcf::dot(impact.edge.node, impact.edge.node);
    Rational2D nv = (sh_rotated / edge_ls).reduced();
    Rational2D cv = (fh + nv).reduced();
    return cv;
};
i2d slide(i2d pos, Rational2D vel, const Impact & impact)
{
    return slide_trunc(pos, slide_exact(pos, vel, impact), impact);
}
std::vector<Impact_ID> impact_occlusion_filter(std::span<const Impact_ID> impacts)
{
    if(impacts.size() < 2)
        return std::vector<Impact_ID>(impacts.begin(), impacts.end());

    std::vector<Impact_ID> filtered;
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
        {
            filtered.push_back(im);
        }
    }

    return filtered;
}
i2d clip_velocity_conditional_slide(Shape_Point rect, const Minkowski_Set & mset, std::vector<Impact_ID> * impacts_out, auto slide_condition)
{
    std::vector<Impact_ID> impacts_closest = raycast_Minkowski_Set(rect.position, rect.position + rect.velocity, mset);

    std::vector<Impact_ID> impacts_filtered = impact_occlusion_filter(impacts_closest);

    if(impacts_filtered.empty())
        return rect.velocity;

    bool any_nonparallel = impacts_filtered.size() > 1 && [&]()
    {
        i2d e0 = impacts_filtered[0].edge.node;
        for(size_t i = 1; i < impacts_filtered.size(); i++)
        {
            if(gcf::cross(e0, impacts_filtered[i].edge.node) != 0)
            {
                return true;
            }
        }
        return false;
    }();

    i2d clipped_vel;
    if(any_nonparallel || !slide_condition(impacts_filtered[0]))
    {
        // True corner: multiple non-parallel edges hit in their interior
        clipped_vel = trunc(Rational2D(rect.velocity) * impacts_filtered[0].t);
    }
    else
        clipped_vel = slide(rect.position, Rational2D(rect.velocity), impacts_filtered[0]);

    if(impacts_out)
        std::move(impacts_filtered.begin(), impacts_filtered.end(), std::back_inserter(*impacts_out));

    return clipped_vel;
}
i2d clip_velocity(Shape_Point rect, const Minkowski_Set & mset, std::vector<Impact_ID> * impacts_out)
{
    return clip_velocity_conditional_slide(rect, mset, impacts_out, [](const Impact_ID &){ return false; });
}
i2d::ntype length_axis_aligned(i2d v)
{
    assert(v.x == 0 || v.y == 0);
    return std::max(std::abs(v.x), std::abs(v.y));
}
i2d normalized_axis_aligned(i2d v)
{
    assert(v.x == 0 || v.y == 0);
    if(v.x > 0)
        return {1, 0};
    else if(v.x < 0)
        return {-1, 0};
    else if(v.y > 0)
        return {0, 1};
    else if(v.y < 0)
        return {0, -1};
    return {0, 0};
}
std::vector<Impact_ID> move_and_slide(Shape_Rectangle & rect, const Minkowski_Set & mset, i2d::ntype max_escape_distance, std::optional<Platformer_Properties> platformer_properties)
{
    std::vector<Impact_ID> all_impacts;

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
    };
    const auto escape_collision = [&](const auto & shape, size_t shape_id)
    {
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
            all_impacts.emplace_back(*best_escape_impact, shape_id);
            if(!collides_Minkowski_Set(rect.position + *best_escape, mset).any_collision())
                rect.position += *best_escape;
        }
    };
    for(const auto & s : cmsr.rect_collisions)
        drag_collision(mset.rects[s]);
    for(const auto & s : cmsr.poly_collisions)
        drag_collision(mset.polys[s]);

    if(max_escape_distance)
    {
        for(const auto & s : cmsr.rect_collisions)
            escape_collision(mset.rects[s], mset.rect_id[s]);
        for(const auto & s : cmsr.poly_collisions)
            escape_collision(mset.polys[s], mset.poly_id[s]);
    }

    i2d old_vel = rect.velocity;

    std::optional<i2d> shallow_slope_reflected = std::nullopt;
    if(platformer_properties)
        shallow_slope_reflected = platformer_properties->sticky_slope_reflected();

    size_t old_all_impacts_size;
    do
    {
        old_all_impacts_size = all_impacts.size();
        rect.velocity = clip_velocity_conditional_slide(shape_position_point(rect), mset, &all_impacts,
            [&](const Impact_ID & impact)
        {
            if(!shallow_slope_reflected)
                return true;
            return gcf::cross(*platformer_properties->sticky_slope, impact.edge.node) > 0 ||
                   gcf::cross(impact.edge.node, *shallow_slope_reflected) > 0;
        });
    }
    while(old_all_impacts_size < all_impacts.size());

    rect.position += rect.velocity;

    if(platformer_properties && platformer_properties->step_height)
    {
        i2d flat_dir = normalized_axis_aligned({-platformer_properties->down_direction.y, platformer_properties->down_direction.x});
        i2d::ntype old_flat_vel = gcf::dot(old_vel, flat_dir);
        i2d::ntype clipped_flat_vel = gcf::dot(rect.velocity, flat_dir);
        if(std::abs(clipped_flat_vel) < std::abs(old_flat_vel))
        {
            Shape_Point stepper = shape_position_point(rect);
            stepper.velocity = -platformer_properties->down_direction * *platformer_properties->step_height;
            i2d up_movement = clip_velocity(stepper, mset, nullptr);
            stepper.position += up_movement;
            i2d flat_vel = flat_dir * (old_flat_vel - clipped_flat_vel);
            stepper.velocity = flat_vel;
            stepper.position += clip_velocity(stepper, mset, nullptr);
            stepper.velocity = -up_movement;
            stepper.position += clip_velocity(stepper, mset, nullptr);
            i2d stepper_change = stepper.position - rect.position;
            if(std::abs(stepper_change.x) > 0)
            {
                rect.position = stepper.position;
                rect.velocity = old_flat_vel * flat_dir;
            }
        }
    }

    return all_impacts;
}
}
