#ifndef KINT_RAYCAST_HPP
#define KINT_RAYCAST_HPP

#include "shape.hpp"
#include "rational.hpp"
#include <optional>

namespace kint
{
struct Impact
{
    Rational2D position;
    Rational t;
    Shape_Line edge;
    constexpr i2d edge_normal() const // not normalized
    {
        return i2d{edge.node.y, -edge.node.x};
    }
};
std::optional<Impact> raycast_unmoving(i2d A, i2d B, Shape_Point);
std::optional<Impact> raycast_unmoving(i2d A, i2d B, Shape_Line);
std::optional<Impact> raycast_unmoving(i2d A, i2d B, Shape_Rectangle);
std::optional<Impact> raycast_unmoving(i2d A, i2d B, const Shape_Polygon &);
std::optional<Impact> raycast_unmoving(i2d A, i2d B, const Shape_Variant &);
struct Minkowski_Set;
std::vector<Impact> raycast_Minkowski_Set(i2d A, i2d B, const Minkowski_Set &);
}

#endif // KINT_RAYCAST_HPP
