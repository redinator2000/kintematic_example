#include "minkowski.hpp"
#include <algorithm>
#include <cassert>

namespace kint
{
Shape_Rectangle minkowski_rectangle(i2d rectangle_dimensions, Shape_Point shape)
{
    return Shape_Rectangle{
            .position = shape.position - rectangle_dimensions,
            .velocity = shape.velocity,
            .dimensions = rectangle_dimensions
           };
}
Shape_Rectangle minkowski_rectangle(i2d rectangle_dimensions, Shape_Rectangle shape)
{
    return Shape_Rectangle{
            .position = shape.position - rectangle_dimensions,
            .velocity = shape.velocity,
            .dimensions = shape.dimensions + rectangle_dimensions
           };
}

std::vector<i2d> convex_hull(std::vector<i2d> && pts)
{
    const size_t n = pts.size();
    if (n == 0)
        return {};

    std::sort(pts.begin(), pts.end(), [](const i2d& a, const i2d& b) {
        return (a.x < b.x) || (a.x == b.x && a.y < b.y);
    });

    std::vector<i2d> hull;
    hull.reserve(n * 2);

    const auto cross_origin = [](const i2d & o, const i2d & a, const i2d & b)
    {
        return cross(a - o, b - o);
    };

    // Build lower hull
    for (const i2d& p : pts)
    {
        while (hull.size() >= 2 &&
               cross_origin(hull[hull.size() - 2], hull[hull.size() - 1], p) <= 0)
        {
            hull.pop_back();
        }
        hull.push_back(p);
    }

    // Build upper hull
    size_t lower_size = hull.size();
    for (size_t i = pts.size(); i-- > 0; )
    {
        const i2d& p = pts[i];
        while (hull.size() > lower_size &&
               cross_origin(hull[hull.size() - 2], hull[hull.size() - 1], p) <= 0)
        {
            hull.pop_back();
        }
        hull.push_back(p);
    }

    // Remove duplicate last point
    hull.pop_back();

    return hull;
}
Shape_Polygon Shape_Polygon_from_scatter(std::vector<i2d> && scattered_points, i2d vel)
{
    std::vector<i2d> convex = convex_hull(std::move(scattered_points));
    Shape_Polygon sp = Shape_Polygon_from_points(convex);
    sp.velocity = vel;
    return sp;
}
Shape_Polygon minkowski_rectangle(i2d rectangle_dimensions, Shape_Line shape)
{
    std::array<i2d, 4> rect_points = {
        -rectangle_dimensions,
        i2d{0, -rectangle_dimensions.y},
        i2d{0, 0},
        i2d{-rectangle_dimensions.x, 0},
    };
    std::array<i2d, 2> line_points = {
        shape.position,
        shape.node_absolute()
    };

    std::vector<i2d> pts;
    pts.reserve(8);
    for(size_t r = 0; r < 4; r++)
        for(size_t l = 0; l < 2; l++)
            pts.push_back(rect_points[r] + line_points[l]);

    return Shape_Polygon_from_scatter(std::move(pts), shape.velocity);
}
Shape_Polygon minkowski_rectangle(i2d rectangle_dimensions, const Shape_Polygon & shape)
{
    std::array<i2d, 4> rect_points = {
        -rectangle_dimensions,
        i2d{0, -rectangle_dimensions.y},
        i2d{0, 0},
        i2d{-rectangle_dimensions.x, 0},
    };

    std::vector<i2d> pts;
    pts.reserve(shape.node_count() * 4);

    for(const i2d & r : rect_points)
        for(size_t i = 0; i < shape.node_count(); ++i)
            pts.push_back(r + shape.get_absolute(i));

    return Shape_Polygon_from_scatter(std::move(pts), shape.velocity);
}
Shape_Polygon minkowski_polygon(std::span<const i2d> nodes, const Shape_Polygon & shape) //nodes have another implicit node at 0, 0
{
    std::vector<i2d> pts;
    pts.reserve(shape.node_count() * (nodes.size() + 1));
    for(size_t i = 0; i < shape.node_count(); ++i)
        pts.push_back(shape.get_absolute(i)); // 0, 0
    for(const i2d & n : nodes)
        for(size_t i = 0; i < shape.node_count(); ++i)
            pts.push_back(shape.get_absolute(i) - n);

    return Shape_Polygon_from_scatter(std::move(pts), shape.velocity);
}
Shape_Polygon minkowski_motion(const Shape_Polygon & shape)
{
    std::vector<i2d> pts;
    pts.reserve(shape.node_count() * 2);
    for(size_t i = 0; i < shape.node_count(); ++i)
    {
        pts.push_back(shape.get_absolute(i));
        pts.push_back(shape.get_absolute(i) + shape.velocity);
    }
    return Shape_Polygon_from_scatter(std::move(pts), shape.velocity);
}
Shape_Polygon minkowski_motion(Shape_Rectangle shape)
{
    return minkowski_motion(shape.as_polygon());
}
Shape_Rectangle minkowski_motion_axis_aligned(Shape_Rectangle shape)
{
    assert(axis_aligned(shape.velocity));
    if(shape.velocity.x < 0 || shape.velocity.y < 0)
    {
        shape.position += shape.velocity;
        shape.dimensions -= shape.velocity;
    }
    else
        shape.dimensions += shape.velocity;
    return shape;
}

} // namespace kint
