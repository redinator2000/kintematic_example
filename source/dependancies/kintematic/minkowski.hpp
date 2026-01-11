#ifndef KINT_MINKOWSKI_HPP
#define KINT_MINKOWSKI_HPP

#include "shape.hpp"
#include <span>

namespace kint
{
Shape_Rectangle minkowski_rectangle(i2d rectangle_dimensions, Shape_Point);
Shape_Polygon minkowski_rectangle(i2d rectangle_dimensions, Shape_Line);
Shape_Rectangle minkowski_rectangle(i2d rectangle_dimensions, Shape_Rectangle);
Shape_Polygon minkowski_rectangle(i2d rectangle_dimensions, const Shape_Polygon &);

Shape_Polygon minkowski_polygon(std::span<const i2d>, const Shape_Polygon &); //nodes have another implicit node at 0, 0

struct Minkowski_Set
{
    std::vector<Shape_Rectangle> rects;
    std::vector<size_t> rect_id;
    std::vector<Shape_Polygon> polys;
    std::vector<size_t> poly_id;
};

template<typename Shape>
void minkowski_set_append(Minkowski_Set & out,
                          i2d rectangle_dimensions,
                          std::span<const Shape> shapes,
                          std::span<const size_t> shape_id)
{
    for(size_t i = 0; i < shapes.size(); i++)
    {
        const Shape& sv = shapes[i];
        size_t id = shape_id[i];

        std::visit([&](const auto& shape)
        {
            auto result = minkowski_rectangle(rectangle_dimensions, shape);

            using R = std::decay_t<decltype(result)>;

            if constexpr (std::is_same_v<R, Shape_Rectangle>)
            {
                out.rects.push_back(std::move(result));
                out.rect_id.push_back(id);
            }
            else
            {
                out.polys.push_back(std::move(result));
                out.poly_id.push_back(id);
            }

        }, sv);
    }
}
template<typename Shape>
Minkowski_Set minkowski_set_create(i2d rectangle_dimensions,
                                   std::span<const Shape> shapes,
                                   std::span<const size_t> shape_id)
{
    Minkowski_Set out;
    minkowski_set_append(out, rectangle_dimensions, shapes, shape_id);
    return out;
}


} // namespace kint

#endif // KINT_MINKOWSKI_HPP
