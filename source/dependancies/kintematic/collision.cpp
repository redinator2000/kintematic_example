#include "collision.hpp"
#include "minkowski.hpp"
#include "rational.hpp"
#include <algorithm>

namespace kint
{
bool impl::collides_unmoving_impl(Shape_Point a, Shape_Point b)
{
    return a.position == b.position;
}
bool impl::collides_unmoving_impl(Shape_Point a, Shape_Rectangle b)
{
    return a.position.x > b.position.x &&
           a.position.y > b.position.y &&
           a.position.x < b.position.x + b.dimensions.x &&
           a.position.y < b.position.y + b.dimensions.y;
}
bool impl::collides_unmoving_impl(Shape_Point p, const Shape_Polygon & poly)
{
    size_t n = poly.node_count();
    for(size_t i = 0; i < n; i++)
    {
        i2d a = poly.get_absolute(i);
        i2d b = poly.get_absolute((i + 1) % n);
        if(gcf::cross(b - a, p.position - a) <= 0)
            return false;
    }
    return true;
}
bool impl::collides_unmoving_impl(Shape_Rectangle a, Shape_Rectangle b)
{
    // return collides_unmoving_impl(shape_position_point(a), minkowski(a.dimensions, b));
    Rational a_left   = a.position.x;
    Rational a_top    = a.position.y;
    Rational a_right  = a.position.x + a.dimensions.x;
    Rational a_bottom = a.position.y + a.dimensions.y;

    Rational b_left   = b.position.x;
    Rational b_top    = b.position.y;
    Rational b_right  = b.position.x + b.dimensions.x;
    Rational b_bottom = b.position.y + b.dimensions.y;

    if(a_right  <= b_left)   return false;
    if(a_left   >= b_right)  return false;
    if(a_bottom <= b_top)    return false;
    if(a_top    >= b_bottom) return false;

    return true;
}

bool impl::collides_unmoving_impl(Shape_Rectangle a, const Shape_Polygon & b)
{
    return collides_unmoving_impl(shape_position_point(a), minkowski_rectangle(a.dimensions, b));
}
bool impl::collides_unmoving_impl(const Shape_Polygon & a, const Shape_Polygon & b)
{
    return collides_unmoving_impl(shape_position_point(a), minkowski_polygon(a.nodes, b));
}
collides_Minkowski_Set_return collides_Minkowski_Set(i2d position, const Minkowski_Set & mset) // on-way polygons are ignored, unless they have velocity pushing on edge of the point
{
    Shape_Point point{.position = position, .velocity = i2d{0, 0}};
    collides_Minkowski_Set_return out;
    for(size_t s = 0; s < mset.rects.size(); s++)
        if(collides_unmoving(point, mset.rects[s]))
            out.rect_collisions.push_back(s);
    for(size_t s = 0; s < mset.polys.size(); s++)
    {
        if(!collides_unmoving(point, mset.polys[s]))
            continue;
        if(mset.polys[s].one_way)
        {
            if(gcf::dot(i2d{-mset.polys[s].one_way->y, mset.polys[s].one_way->x}, mset.polys[s].velocity) > 0) // one-way is moving in direction
            {
                Shape_Polygon prev_assume = mset.polys[s];
                prev_assume.position -= prev_assume.velocity;
                if(!collides_unmoving(point, prev_assume)) // on the edge of the one-way
                    out.poly_collisions.push_back(s);
            }
        }
        else
            out.poly_collisions.push_back(s);
    }
    return out;
}
}
