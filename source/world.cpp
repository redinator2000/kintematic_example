#include "world.hpp"
#include "shape_draw.hpp"
#include <optional>
#include "dependancies/kintematic/minkowski.hpp"
#include "dependancies/kintematic/collision.hpp"
#include "dependancies/kintematic/advance.hpp"
#include "dependancies/kintematic/raycast.hpp"
#include <cassert>
#include <SFML/Window/Keyboard.hpp>
#include "player_controls.hpp"

World make_a_level()
{
    World world;
    world.player.shape = kint::Shape_Rectangle({2, -6}, {0, 0}, {4, 4});

    //world.shapes.emplace_back(kint::Shape_Rectangle({-2, -6}, {0, 0}, {4, 2}));
    //world.shape_colors.emplace_back(sf::Color(50, 50, 50));
    world.shapes.emplace_back(kint::Shape_Rectangle({8, -3}, {0, 0}, {4, 2}));
    world.shape_colors.emplace_back(sf::Color(50, 150, 50));

    world.shapes.emplace_back(kint::Shape_Rectangle({-24, 8}, {0, 0}, {8, 8}));
    world.shape_colors.emplace_back(sf::Color(200, 100, 50));
    world.shapes.emplace_back(kint::Shape_Polygon_right_triangle(kint::Shape_Rectangle({-16, 0}, {0, 0}, {16, 8}), kint::Right_Triangle_Corner::br));
    world.shape_colors.emplace_back(sf::Color(100, 50, 200));
    world.shapes.emplace_back(kint::Shape_Rectangle({0, 0}, {0, 0}, {8, 8}));
    world.shape_colors.emplace_back(sf::Color(200, 100, 50));
    world.shapes.emplace_back(kint::Shape_Rectangle({8, 0}, {0, 0}, {8, 8}));
    world.shape_colors.emplace_back(sf::Color(100, 100, 200));
    world.shapes.emplace_back(kint::Shape_Rectangle({20, 0}, {0, 0}, {4, 8}));
    world.shape_colors.emplace_back(sf::Color(200, 100, 50));
    world.shapes.emplace_back(kint::Shape_Polygon_right_triangle(kint::Shape_Rectangle({24, -8}, {0, 0}, {16, 8}), kint::Right_Triangle_Corner::br));
    world.shape_colors.emplace_back(sf::Color(100, 50, 200));
    world.shapes.emplace_back(kint::Shape_Rectangle({36, -8}, {0, 0}, {8, 8}));
    world.shape_colors.emplace_back(sf::Color(100, 100, 200));

    world.shapes.emplace_back(kint::Shape_Line({-16, -4}, {0, 0}, {8, -4}, false));
    world.shape_colors.emplace_back(sf::Color(200, 50, 200));
    world.shapes.emplace_back(kint::Shape_Line({-8, -4}, {0, 0}, {8, -4}, true));
    world.shape_colors.emplace_back(sf::Color(200, 50, 200));
    world.shapes.emplace_back(kint::Shape_Line({-16, -8}, {0, 0}, {-8, 4}, true));
    world.shape_colors.emplace_back(sf::Color(200, 50, 200));

    assert(world.shapes.size() == world.shape_colors.size());
    return world;
}

void World_update(World & world)
{
    player_think(world.player);
    world.player.shape.velocity = world.player.want_velocity;

    std::vector<size_t> shape_ids(world.shapes.size());
    std::ranges::iota(shape_ids, 0);
    kint::Minkowski_Set mset = minkowski_set_create(world.player.shape.dimensions, std::span<const kint::Shape_Variant>(world.shapes), std::span<const size_t>(shape_ids));
    world.player.shape.velocity = kint::clip_and_slide(shape_position_point(world.player.shape), mset);
    world.player.shape.position += world.player.shape.velocity;
    if(world.player.stop_after_advance)
        world.player.shape.velocity = kint::i2d{0, 0};
    world.player.shape.velocity = kint::clip_and_slide(shape_position_point(world.player.shape), mset); //just for visual velocity interpolation
}
void World_draw(const World & world, sf::RenderTarget & window, float interp_fraction)
{
    constexpr std::array<kint::i2d, 8> hairs = {kint::i2d{0, 1}, kint::i2d{1, 1}, kint::i2d{1, 0}, kint::i2d{1, -1}, kint::i2d{0, -1}, kint::i2d{-1, -1}, kint::i2d{-1, 0}, kint::i2d{-1, 1}};
    //constexpr std::array<kint::i2d, 1> hairs = {kint::i2d{1, 1}};

    std::optional<kint::i2d> show_minkowski = std::nullopt;
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Tab))
        show_minkowski = world.player.shape.dimensions;
    kint::Minkowski_Set mset;
    if(show_minkowski)
    {
        std::vector<size_t> shape_ids(world.shapes.size());
        std::ranges::iota(shape_ids, 0);
        minkowski_set_append(mset, world.player.shape.dimensions, std::span<const kint::Shape_Variant>(world.shapes), std::span<const size_t>(shape_ids));
        for(size_t i = 0; i < mset.rects.size(); i++)
            shape_draw(window, interp_fraction, world.shape_colors[mset.rect_id[i]], mset.rects[i]);
        for(size_t i = 0; i < mset.polys.size(); i++)
            shape_draw(window, interp_fraction, world.shape_colors[mset.poly_id[i]], mset.polys[i]);
    }
    else
    {
        for(unsigned int s = 0; s < world.shapes.size(); s++)
        {
            sf::Color color = world.shape_colors[s];
            std::visit([&window, interp_fraction, color, show_minkowski](const auto & shape)
            {
                shape_draw(window, interp_fraction, color, shape);
            }, world.shapes[s]);
        }
    }
    bool player_colliding = false;
    for(const auto & s : world.shapes)
        if(kint::collides_unmoving(world.player.shape, s))
            player_colliding = true;
    if(show_minkowski)
        shape_draw(window, interp_fraction, player_colliding ? sf::Color::Red : sf::Color::Green,
                   kint::shape_position_point(world.player.shape));
    else
        shape_draw(window, interp_fraction, player_colliding ? sf::Color::Red : sf::Color::Green,
                   world.player.shape);

    auto draw_impacts = [&](const std::vector<kint::Impact> & unflitered)
    {
        std::vector<kint::Impact> impacts = kint::impact_occlusion_filter(unflitered);
        if(impacts.size())
        {
            kint::Rational closest_t = 1;
            for(const auto & impact : impacts)
                if(impact.t < closest_t)
                    closest_t = impact.t;
            for(const auto & impact : impacts)
            {
                rational2d_draw(window, impact.t == closest_t ? sf::Color::White : sf::Color(100, 100, 100, 155), impact.position);
                if(impact.t == closest_t)
                    shape_draw(window, interp_fraction, sf::Color::Yellow, impact.edge);
            }
        }
    };
    const auto find_impacts = [](const auto & shapes, const kint::Shape_Line & hat, bool & hat_colliding, std::vector<kint::Impact> & hat_impacts)
    {
        for(const auto & s : shapes)
        {
            if(std::optional<kint::Impact> hat_impact = raycast_unmoving(hat.position, hat.node_absolute(), s))
                hat_impacts.push_back(*hat_impact);
            if(kint::collides_unmoving(hat, s))
                hat_colliding = true;
        }
    };
    if(!show_minkowski)
    {
        for(const auto & h : hairs)
        {
            kint::Shape_Line hat = kint::Shape_Line(world.player.shape.position + h * 2 + kint::i2d{2, 2}, world.player.shape.velocity, h * 3, false);
            bool hat_colliding = false;
            std::vector<kint::Impact> hat_impacts = {};
            find_impacts(world.shapes, hat, hat_colliding, hat_impacts);
            shape_draw(window, interp_fraction, hat_colliding ? sf::Color::Red : sf::Color::Green,
                    hat);
            draw_impacts(hat_impacts);
        }
    }
    else
    {
        const auto h = world.player.want_velocity;
        {
            kint::Shape_Line hat = kint::Shape_Line(world.player.shape.position, world.player.shape.velocity, h, false);
            bool hat_colliding = false;
            std::vector<kint::Impact> hat_impacts = {};
            find_impacts(mset.polys, hat, hat_colliding, hat_impacts);
            find_impacts(mset.rects, hat, hat_colliding, hat_impacts);
            shape_draw(window, interp_fraction, hat_colliding ? sf::Color::Red : sf::Color::White,
                       hat);
            draw_impacts(hat_impacts);
        }
    }
}
