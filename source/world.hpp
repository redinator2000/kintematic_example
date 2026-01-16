#ifndef WORLD_HPP
#define WORLD_HPP

#include "dependancies/kintematic/shape.hpp"
#include <SFML/Graphics/Color.hpp>
#include "dependancies/kintematic/raycast.hpp"

struct Player
{
    kint::i2d pre_clip_velocity = kint::i2d{0, 0};
    kint::Shape_Rectangle shape;
    int moving_time = 0;
    bool stop_after_advance = false;
    bool flying = false;
    std::vector<kint::Impact_ID> recent_impacts = {};
};
struct World
{
    int ticks_total = 0;

    std::vector<kint::Shape_Variant> shapes = {};
    std::vector<sf::Color> shape_colors = {};

    Player player;
};
World make_a_level();

namespace sf { class RenderTarget; }
void World_draw(const World &, sf::RenderTarget &, float interp_fraction);
void World_update(World &);

#endif // WORLD_HPP
