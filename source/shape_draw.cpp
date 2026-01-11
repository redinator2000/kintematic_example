#include "shape_draw.hpp"
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include "dependancies/kintematic/minkowski.hpp"

constexpr sf::Vector2i to_sfV2i(const kint::i2d & v)
{
    return sf::Vector2i(v.x, v.y);
}
constexpr sf::Vector2f to_sfV2f(const kint::i2d & v)
{
    return sf::Vector2f(v.x, v.y);
}
constexpr sf::Vector2f to_sfV2f(const kint::Rational2D & v)
{
    return sf::Vector2f(v.x.to_float(), v.y.to_float());
}
sf::Color fill_color(sf::Color color)
{
    color.r = (int(color.r) * 2) / 3;
    color.g = (int(color.g) * 2) / 3;
    color.b = (int(color.b) * 2) / 3;
    color.a = 100;
    return color;
}
void sfV2f_draw(sf::RenderTarget & window, sf::Color color, sf::Vector2f p, float r)
{
    // 4 cross lines + 4 diamond lines = 8 vertices (each pair is a line)
    sf::VertexArray va(sf::PrimitiveType::Lines, 8);

    // Cross (horizontal + vertical)
    va[0].position = {p.x - r, p.y};
    va[1].position = {p.x + r, p.y};

    va[2].position = {p.x, p.y - r};
    va[3].position = {p.x, p.y + r};

    // Diamond (diagonals)
    va[4].position = {p.x - r, p.y};
    va[5].position = {p.x,     p.y - r};

    va[6].position = {p.x + r, p.y};
    va[7].position = {p.x,     p.y + r};

    for(auto & v : va)
        v.color = color;

    window.draw(va);
}
void rational2d_draw(sf::RenderTarget & window, sf::Color color, kint::Rational2D point)
{
    return sfV2f_draw(window, color, to_sfV2f(point), 0.5f);
}
void shape_draw(sf::RenderTarget & window, float interp_fraction, sf::Color color, kint::Shape_Point shape)
{
    return sfV2f_draw(window, color, to_sfV2f(shape.position) + to_sfV2f(shape.velocity) * interp_fraction, 2.5f);
}
void shape_draw(sf::RenderTarget & window, float interp_fraction, sf::Color color, kint::Shape_Line shape)
{
    sf::Vector2f p0 = to_sfV2f(shape.position)
        + to_sfV2f(shape.velocity) * interp_fraction;
    sf::Vector2f p1 = p0 + to_sfV2f(shape.node);

    sf::VertexArray va(sf::PrimitiveType::Lines, shape.one_way ? 4 : 2);

    va[0].position = p0;
    va[1].position = p1;

    if(shape.one_way)
    {
        sf::Vector2f mid = (p0 + p1) * 0.5f;

        sf::Vector2f dir = p1 - p0;
        sf::Vector2f stem = {dir.y, -dir.x};
        float len = stem.length();

        if(len > 0.0001f)
        {
            float arrow_len = 0.5f;

            sf::Vector2f stem_start = mid;
            sf::Vector2f stem_end   = mid + stem * (arrow_len / len);

            va[2].position = stem_start;
            va[3].position = stem_end;
        }
        else
        {
            va[2].position = p0;
            va[3].position = p0;
        }
    }
    for(auto & v : va)
        v.color = color;

    window.draw(va);
}

void shape_draw(sf::RenderTarget & window, float interp_fraction, sf::Color color, kint::Shape_Rectangle shape)
{
    shape_draw(window, interp_fraction, color, shape.as_polygon());
}
void shape_draw(sf::RenderTarget & window, float interp_fraction, sf::Color color, const kint::Shape_Polygon & shape)
{
    sf::Color fc = fill_color(color);

    sf::VertexArray fil = sf::VertexArray(sf::PrimitiveType::TriangleFan, shape.node_count());
    sf::VertexArray out = sf::VertexArray(sf::PrimitiveType::LineStrip, shape.node_count() + 1);
    sf::Vector2f movement = to_sfV2f(shape.velocity) * interp_fraction;

    for(size_t i = 0; i < shape.node_count(); i++)
    {
        sf::Vector2f p = to_sfV2f(shape.get_absolute(i)) + movement;
        out[i].position = p;
        fil[i].position = p;
    }
    out[shape.node_count()].position = out[0].position;

    for(auto & v : fil)
        v.color = fc;
    for(auto & v : out)
        v.color = color;
    window.draw(fil);
    window.draw(out);
}
sf::Color grid_color(int n)
{
    if(n == 0)
        return sf::Color(255, 165, 0);
    if(n % 16 == 0)
        return sf::Color::Blue;
    return (n % 2 == 0)
        ? sf::Color(100, 100, 100)
        : sf::Color(50, 50, 50);
}

void grid_draw(sf::RenderTarget& window, const sf::View& view)
{
    constexpr float spacing = 1.f;

    // Get view bounds in world coordinates
    sf::Vector2f center = view.getCenter();
    sf::Vector2f size   = view.getSize();

    float left   = center.x - size.x * 0.5f;
    float right  = center.x + size.x * 0.5f;
    float top    = center.y - size.y * 0.5f;
    float bottom = center.y + size.y * 0.5f;

    // Compute how many vertical and horizontal lines we need
    int first_vert  = static_cast<int>(std::floor(left   / spacing));
    int last_vert   = static_cast<int>(std::ceil (right  / spacing));

    int first_horiz = static_cast<int>(std::floor(top    / spacing));
    int last_horiz  = static_cast<int>(std::ceil (bottom / spacing));

    int lines_vert  = last_vert  - first_vert  + 1;
    int lines_horiz = last_horiz - first_horiz + 1;

    sf::VertexArray va(sf::PrimitiveType::Lines, 2 * (lines_vert + lines_horiz));

    std::size_t idx = 0;

    // Vertical lines
    for(int i = first_vert; i <= last_vert; ++i)
    {
        float x = i * spacing;
        sf::Color color = grid_color(i);

        va[idx].position     = sf::Vector2f(x, top);
        va[idx].color        = color;
        va[idx + 1].position = sf::Vector2f(x, bottom);
        va[idx + 1].color    = color;

        idx += 2;
    }

    // Horizontal lines
    for(int i = first_horiz; i <= last_horiz; ++i)
    {
        float y = i * spacing;
        sf::Color color = grid_color(i);

        va[idx].position     = sf::Vector2f(left, y);
        va[idx].color        = color;
        va[idx + 1].position = sf::Vector2f(right, y);
        va[idx + 1].color    = color;

        idx += 2;
    }

    window.draw(va);
}

