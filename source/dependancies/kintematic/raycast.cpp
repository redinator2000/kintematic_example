#include "raycast.hpp"
#include "collision.hpp"
#include "minkowski.hpp"
#include <algorithm>
#include <cassert>

namespace kint
{
std::optional<Impact> raycast_unmoving(i2d A, i2d B, const Shape_Variant & variant)
{
    return std::visit([&](auto shape){ return raycast_unmoving(A, B, shape); }, variant);
}
std::optional<Impact> raycast_unmoving(i2d A, i2d B, Shape_Point p)
{
    i2d P = p.position;
    i2d AB = B - A;
    i2d AP = P - A;

    // Check collinearity: cross(AP, AB) == 0
    if(cross(AP, AB) != 0)
        return std::nullopt;

    // Compute t = dot(AP, AB) / dot(AB, AB)
    i2d::ntype denom = dot(AB, AB);
    if(denom == 0)
        return std::nullopt; // A == B, degenerate segment

    Rational t{ dot(AP, AB), denom };

    // Check if intersection lies on segment AB
    if(t < Rational{0} || t > Rational{1})
        return std::nullopt;

    // Build the impact point
    Rational2D hit = Rational2D{A} + t * Rational2D{AB};

    Shape_Line edge;
    edge.position = P;
    edge.node = i2d{-AB.y, AB.x};
    edge.velocity = p.velocity;

    return Impact{hit, t, edge};
}
std::optional<Impact> raycast_unmoving(i2d A, i2d B, Shape_Line line)
{
    i2d C = line.position;
    i2d D = line.node_absolute();

    i2d AB = B - A;
    i2d AC = C - A;
    i2d AD = D - A;
    i2d CD = line.node;
    i2d CA = -AC;
    i2d CB = B - C;

    i2d::ntype o1 = cross(AB, AC);
    i2d::ntype o2 = cross(AB, AD);
    i2d::ntype o3 = cross(CD, CA);
    i2d::ntype o4 = cross(CD, CB);

    if(line.one_way && o3 < 0)
        return std::nullopt;

    if(((o1 > 0 && o2 < 0) || (o1 < 0 && o2 > 0)) &&
       ((o3 > 0 && o4 < 0) || (o3 < 0 && o4 > 0)))
    {
        Rational t = Rational{o3, cross(AB, CD)};
        return Impact{Rational2D{A} + t * Rational2D{AB},
                      t,
                      line};
    }

    return std::nullopt;
}
std::optional<Impact> raycast_unmoving(i2d A, i2d B, Shape_Rectangle rect) // Liang–Barsky
{
    gcf::intV2D<long int, 64> whatever = gcf::intV2D<long int, 64>(gcf::intV2D<long int, 1>(A));

    i2d d = B - A; // direction

    // Rectangle bounds
    i2d min = rect.position;
    i2d max = rect.position + rect.dimensions;

    // Liang–Barsky parameters
    i2d::ntype p[4] = { -d.x,  d.x, -d.y,  d.y };
    i2d::ntype q[4] = { A.x - min.x,
                        max.x - A.x,
                        A.y - min.y,
                        max.y - A.y };

    Rational t_enter{0};
    Rational t_exit{1};

    int hit_edge = -1; // 0=left,1=right,2=bottom,3=top

    for(int i = 0; i < 4; ++i)
    {
        i2d::ntype pi = p[i];
        i2d::ntype qi = q[i];

        if(pi == 0)
        {
            // Line is parallel to this boundary.
            // If outside the half-space, no intersection.
            if(qi <= 0)
                return std::nullopt;
            // Inside or on boundary: no clipping from this edge.
            continue;
        }

        Rational r{qi, pi};

        if(pi < 0)
        {
            // Potential entering boundary
            if(r > t_exit)
                return std::nullopt;
            if(r >= t_enter)
            {
                t_enter = r;
                hit_edge = i;
            }
        }
        else
        {
            // Potential exiting boundary
            if(r < t_enter)
                return std::nullopt;
            if(r < t_exit)
                t_exit = r;
        }
    }

    // No intersection along segment AB
    if(!(t_enter <= t_exit && t_enter < Rational{1} && t_exit > Rational{0}))
        return std::nullopt;

    // Compute intersection point
    Rational2D P = Rational2D{A} + t_enter * Rational2D{d};

    // Build the edge Shape_Line
    Shape_Line edge;
    edge.velocity = rect.velocity;

    switch(hit_edge)
    {
        case 0: // left: x = min.x
            edge.position = i2d{min.x, max.y};;
            edge.node = i2d{0, - rect.dimensions.y};
            break;

        case 1: // right: x = max.x
            edge.position = i2d{max.x, min.y};
            edge.node = i2d{0, rect.dimensions.y};
            break;

        case 2: // bottom: y = min.y
            edge.position = min;
            edge.node = i2d{rect.dimensions.x, 0};
            break;

        case 3: // top: y = max.y
            edge.position = max;
            edge.node = i2d{- rect.dimensions.x, 0};
            break;

        default:
            return std::nullopt; // should not happen if there was a real hit
    }

    return Impact{P, t_enter, edge};
}

std::optional<Impact> raycast_unmoving(i2d A, i2d B, const Shape_Polygon & poly)
{
    assert(Shape_Polygon_area(poly) >= 0);

    i2d d = B - A;   // ray direction

    Rational t_enter{0};
    Rational t_exit{1};

    size_t n = poly.node_count();
    std::optional<size_t> hit_edge = std::nullopt;

    for(size_t i = 0; i < n; i++)
    {
        i2d p0 = poly.get_absolute(i);
        i2d p1 = poly.get_absolute((i + 1) % n);

        i2d e = p1 - p0;          // edge direction
        i2d nrm{-e.y, e.x};       // outward normal for CW polygon

        i2d::ntype p = -dot(nrm, d);
        i2d::ntype q =  dot(nrm, A - p0);

        if(p == 0)
        {
            // Ray is parallel to this edge
            if(q <= 0)
                return std::nullopt; // outside the half-space; no hit
            continue; // inside or on boundary; no clipping
        }

        Rational r = Rational(q, p).reduced();

        if(p < 0)
        {
            // entering
            if(r > t_exit)
                return std::nullopt;
            if(r >= t_enter)
            {
                t_enter = r;
                hit_edge = i;
            }
        }
        else
        {
            // leaving
            if(r < t_enter)
                return std::nullopt;
            if(r < t_exit)
                t_exit = r;
        }
    }
    if(!hit_edge)
        return std::nullopt;

    // No intersection along segment AB
    if(!(t_enter <= t_exit && t_enter < Rational{1} && t_exit > Rational{0}))
        return std::nullopt;

    // Compute intersection point
    Rational2D P = Rational2D{A} + t_enter * Rational2D{d};

    // Build the Shape_Line for the edge that was hit
    size_t i = *hit_edge;
    i2d p0 = poly.get_absolute(i);
    i2d p1 = poly.get_absolute((i + 1) % n);

    Shape_Line edge;
    edge.position = p0;
    edge.node = p1 - p0;
    edge.velocity = poly.velocity;

    if(poly.one_way && gcf::dot(edge.node, *poly.one_way) >= 0)
        return std::nullopt;

    return Impact{P, t_enter, edge};
}
bool impl::collides_unmoving_impl(Shape_Line l, Shape_Point shape)
{
    return bool(raycast_unmoving(l.position, l.node_absolute(), shape));
}
bool impl::collides_unmoving_impl(Shape_Line l, Shape_Line shape)
{
    shape.one_way = false;
    return bool(raycast_unmoving(l.position, l.node_absolute(), shape));
}
bool impl::collides_unmoving_impl(Shape_Line l, Shape_Rectangle shape)
{
    return bool(raycast_unmoving(l.position, l.node_absolute(), shape));
}
bool impl::collides_unmoving_impl(Shape_Line l, const Shape_Polygon & shape)
{
    return bool(raycast_unmoving(l.position, l.node_absolute(), shape));
}

std::vector<Impact_ID> raycast_Minkowski_Set(i2d A, i2d B, const Minkowski_Set & mset)
{
    std::vector<Impact_ID> filtered;

    const auto find_best_impact = [&](const auto & shape, size_t shape_id)
    {
        if(auto ni = raycast_unmoving(A, B, shape))
        {
            if(!filtered.empty() && ni->t < filtered[0].t)
            {
                filtered.clear();
            }

            if(filtered.empty() || ni->t == filtered[0].t)
            {
                filtered.emplace_back(*ni, shape_id);
            }
        }
    };

    for (size_t i = 0; i < mset.rects.size(); i++)
        find_best_impact(mset.rects[i], mset.rect_id[i]);
    for (size_t i = 0; i < mset.polys.size(); i++)
        find_best_impact(mset.polys[i], mset.poly_id[i]);

    return filtered;
}

} // namespace kint
