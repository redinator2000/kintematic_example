#ifndef SHAPE_DRAW_HPP
#define SHAPE_DRAW_HPP

#include "dependancies/kintematic/shape.hpp"
#include "dependancies/kintematic/rational.hpp"
#include <SFML/Graphics/Color.hpp>
namespace sf { class RenderTarget; class View; }

void rational2d_draw(sf::RenderTarget &, sf::Color, kint::Rational2D);
void shape_draw(sf::RenderTarget &, float interp_fraction, sf::Color, kint::Shape_Point);
void shape_draw(sf::RenderTarget &, float interp_fraction, sf::Color, kint::Shape_Line);
void shape_draw(sf::RenderTarget &, float interp_fraction, sf::Color, kint::Shape_Rectangle);
void shape_draw(sf::RenderTarget &, float interp_fraction, sf::Color, const kint::Shape_Polygon &);

void grid_draw(sf::RenderTarget &, const sf::View &);

#endif // SHAPE_DRAW_HPP
