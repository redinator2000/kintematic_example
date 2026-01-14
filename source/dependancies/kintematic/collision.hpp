#ifndef KINT_COLLISION_HPP
#define KINT_COLLISION_HPP

#include "shape.hpp"

namespace kint
{
namespace impl
{
// normal collision algorithms
bool collides_unmoving_impl(Shape_Point, Shape_Point);
bool collides_unmoving_impl(Shape_Point, Shape_Rectangle);
bool collides_unmoving_impl(Shape_Point, const Shape_Polygon &);
bool collides_unmoving_impl(Shape_Rectangle, Shape_Rectangle);
// implemented in raycast.cpp
bool collides_unmoving_impl(Shape_Line, Shape_Point);
bool collides_unmoving_impl(Shape_Line, Shape_Line);
bool collides_unmoving_impl(Shape_Line, Shape_Rectangle);
bool collides_unmoving_impl(Shape_Line, const Shape_Polygon &);

// minkowski to become a point collision
bool collides_unmoving_impl(Shape_Rectangle, const Shape_Polygon &);
bool collides_unmoving_impl(const Shape_Polygon &, const Shape_Polygon &);



template<int N> struct priority_tag : priority_tag<N-1> {};
template<> struct priority_tag<0> {};

// 1) direct impl(A,B)
template<typename A, typename B>
requires requires (A a, B b) {
    collides_unmoving_impl(a, b);
}
bool collides_unmoving_dispatch(A a, B b, priority_tag<3>)
{
    return collides_unmoving_impl(a, b);
}

// 2) swapped impl(B,A)
template<typename A, typename B>
requires requires (A a, B b) {
    collides_unmoving_impl(b, a);
}
bool collides_unmoving_dispatch(A a, B b, priority_tag<2>)
{
    return collides_unmoving_impl(b, a);
}

// 3) A is variant, B is not
template<typename A, typename B>
bool collides_unmoving_dispatch(A a, B b, priority_tag<1>)
{
    if constexpr (std::is_same_v<A, Shape_Variant> && !std::is_same_v<B, Shape_Variant>)
    {
        return std::visit(
            [&](auto&& aa)
            {
                return collides_unmoving(aa, b); // call public dispatcher
            },
            a
        );
    }
    else if constexpr (!std::is_same_v<A, Shape_Variant> && std::is_same_v<B, Shape_Variant>)
    {
        return std::visit(
            [&](auto&& bb)
            {
                return collides_unmoving(a, bb); // call public dispatcher
            },
            b
        );
    }
    else
    {
        return collides_unmoving_dispatch(a, b, priority_tag<0>{});
    }
}

// 4) both are variants
template<typename A, typename B>
bool collides_unmoving_dispatch(A a, B b, priority_tag<0>)
{
    if constexpr (std::is_same_v<A, Shape_Variant> && std::is_same_v<B, Shape_Variant>)
    {
        return std::visit(
            [&](auto&& aa, auto&& bb)
            {
                return collides_unmoving(aa, bb); // public dispatcher again
            },
            a, b
        );
    }

    static_assert(!sizeof(A), "No matching collides_unmoving overload found.");
}

} // namespace impl

template<typename A, typename B>
bool collides_unmoving(A a, B b)
{
    return impl::collides_unmoving_dispatch(a, b, impl::priority_tag<3>{});
}
struct Minkowski_Set;
struct collides_Minkowski_Set_return
{
    std::vector<size_t> rect_collisions;
    std::vector<size_t> poly_collisions;
    constexpr bool any_collision() const
    {
        return rect_collisions.size() || poly_collisions.size();
    }
};
collides_Minkowski_Set_return collides_Minkowski_Set(i2d, const Minkowski_Set &); //returns max movement
} // namespace kint

#endif // KINT_COLLISION_HPP
