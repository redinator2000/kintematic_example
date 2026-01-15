#ifndef SHAPE_DRAW_HPP
#define SHAPE_DRAW_HPP

#include "dependancies/kintematic/shape.hpp"
#include "dependancies/kintematic/rational.hpp"
#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector2.hpp>
namespace sf { class RenderTarget; class View; }

void rational2d_draw(sf::RenderTarget &, sf::Color, kint::Rational2D);
void shape_draw(sf::RenderTarget &, float interp_fraction, sf::Color, kint::Shape_Point);
void shape_draw(sf::RenderTarget &, float interp_fraction, sf::Color, kint::Shape_Line);
void shape_draw(sf::RenderTarget &, float interp_fraction, sf::Color, kint::Shape_Rectangle);
void shape_draw(sf::RenderTarget &, float interp_fraction, sf::Color, const kint::Shape_Polygon &);

void grid_draw(sf::RenderTarget &, const sf::View &);

constexpr sf::Vector2i to_sfV2i(const kint::i2d & v)
{
    return sf::Vector2i(v.x, v.y);
}
constexpr sf::Vector2f to_sfV2f(const kint::i2d & v)
{
    return sf::Vector2f(v.x, v.y);
}

#endif // SHAPE_DRAW_HPP
