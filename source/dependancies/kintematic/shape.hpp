#ifndef KINT_SHAPE_HPP
#define KINT_SHAPE_HPP

#include <variant>
#include <vector>
#include <span>

#include "i2d.hpp"

namespace kint
{

struct Shape_Polygon;

struct Shape_Point
{
    i2d position;
    i2d velocity;
    Shape_Polygon as_polygon() const;
};
struct Shape_Line
{
    i2d position;
    i2d velocity;
    i2d node; // relative to position
    bool one_way = false;

    i2d node_absolute() const
    {
        return position + node;
    }
    Shape_Polygon as_polygon() const;
};
struct Shape_Rectangle
{
    i2d position;
    i2d velocity;
    i2d dimensions;

    Shape_Polygon as_polygon() const;
};
struct Shape_Polygon //must be convex
{
    i2d position;
    i2d velocity;
    std::vector<i2d> nodes; // clockwise (with +x right +y down). position is also a node, the first/last one

    size_t node_count() const
    {
        return nodes.size() + 1;
    }

    auto get_absolute(this auto& self, size_t index)
        -> decltype(auto)
    {
        if(index == self.nodes.size())
            return self.position;
        return self.nodes[index] + self.position;
    }
    Shape_Point position_point() const;
};

using Shape_Variant = std::variant<Shape_Point,
                                   Shape_Line,
                                   Shape_Rectangle,
                                   Shape_Polygon>;

Shape_Point Shape_Point_from_i2d(i2d);
enum class Right_Triangle_Corner { tl, tr, br, bl }; // corner with the right angle
Shape_Polygon Shape_Polygon_right_triangle(Shape_Rectangle, Right_Triangle_Corner);
Shape_Polygon Shape_Polygon_from_points(std::span<const i2d>); //must be convex

std::vector<i2d> convex_hull(std::vector<i2d> && points);

i2d::ntype Shape_Polygon_area(const Shape_Polygon & poly);

template<typename S>
Shape_Point shape_position_point(const S & s)
{
    return Shape_Point
    {
        .position = s.position,
        .velocity = s.velocity,
    };
}
}

#endif // KINT_SHAPE_HPP
