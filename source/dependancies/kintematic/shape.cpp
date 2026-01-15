#include "shape.hpp"

namespace kint
{
Shape_Polygon Shape_Point::as_polygon() const
{
    return Shape_Polygon
    {
        .position = this->position,
        .velocity = this->velocity,
        .nodes = {}
    };
}
Shape_Polygon Shape_Line::as_polygon() const
{
    return Shape_Polygon
    {
        .position = this->position,
        .velocity = this->velocity,
        .nodes = {this->node}
    };
}
Shape_Polygon Shape_Rectangle::as_polygon() const
{
    return Shape_Polygon
    {
        .position = this->position,
        .velocity = this->velocity,
        .nodes = {{this->dimensions.x, 0}, this->dimensions, {0, this->dimensions.y}}
    };
}
Shape_Point Shape_Point_from_i2d(i2d p)
{
    return Shape_Point
    {
        .position = p,
        .velocity = i2d{0, 0}
    };
}
Shape_Polygon Shape_Polygon_right_triangle(Shape_Rectangle rectangle, Right_Triangle_Corner rtc)
{
    const i2d tl = rectangle.position;
    const i2d tr = rectangle.position + i2d{rectangle.dimensions.x, 0};
    const i2d br = rectangle.position + rectangle.dimensions;
    const i2d bl = rectangle.position + i2d{0, rectangle.dimensions.y};

    switch (rtc)
    {
        case Right_Triangle_Corner::tl:
            // Keep TL, TR, BL
            return Shape_Polygon{
                .position = tl,
                .velocity = rectangle.velocity,
                .nodes = { tr - tl, bl - tl }
            };

        case Right_Triangle_Corner::tr:
            // Keep TR, BR, TL
            return Shape_Polygon{
                .position = tr,
                .velocity = rectangle.velocity,
                .nodes = { br - tr, tl - tr }
            };

        case Right_Triangle_Corner::br:
            // Keep BR, BL, TR
            return Shape_Polygon{
                .position = br,
                .velocity = rectangle.velocity,
                .nodes = { bl - br, tr - br }
            };

        case Right_Triangle_Corner::bl:
            // Keep BL, TL, BR
            return Shape_Polygon{
                .position = bl,
                .velocity = rectangle.velocity,
                .nodes = { tl - bl, br - bl }
            };
    }
    return {};
}
Shape_Polygon Shape_Polygon_from_points(std::span<const i2d> pts)
{
    Shape_Polygon poly;
    if(pts.empty())
        return poly;

    poly.position = pts[0];
    for (size_t i = 1; i < pts.size(); ++i)
        poly.nodes.push_back(pts[i] - pts[0]);

    return poly;
}
i2d::ntype Shape_Polygon_area(const Shape_Polygon & poly)
{
    const size_t n = poly.node_count();
    if (n < 3)
        return 0; // degenerate polygon

    i2d::ntype area2 = 0; // twice the signed area

    for (size_t i = 0; i < n; i++)
    {
        i2d a = poly.get_absolute(i);
        i2d b = poly.get_absolute((i + 1) % n);

        area2 += gcf::cross(a, b);
    }

    return area2 / 2;
}
}
