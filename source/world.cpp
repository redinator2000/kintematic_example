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

namespace kint
{
    i2d slide_trunc(i2d pos, Rational2D ideal_vel, const Impact & impact);
}
World smallworld()
{
    World world;
    world.player.shape = kint::Shape_Rectangle({16, -8}, {0, 0}, {4, 4});

    //world.shapes.emplace_back(kint::Shape_Rectangle({-2, -6}, {0, 0}, {4, 2}));
    //world.shape_colors.emplace_back(sf::Color(50, 50, 50));
    world.shapes.emplace_back(kint::Shape_Rectangle({8, -3}, {0, 1}, {4, 2}));
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
World make_a_level()
{
    World world;
    world.player.shape = kint::Shape_Rectangle({-256, -64 - 64 - 1}, {0, 0}, {64, 64});
    world.player.pre_clip_velocity = world.player.shape.velocity;

    world.shapes.emplace_back(kint::Shape_Rectangle({128, 0}, {0, 0}, {64, 64}));
    world.shape_colors.emplace_back(sf::Color(50, 150, 50));

    world.shapes.emplace_back(kint::Shape_Rectangle({-1000, 0}, {0, 0}, {2000, 16}));
    world.shape_colors.emplace_back(sf::Color(150, 50, 50));
    world.shapes.emplace_back(kint::Shape_Polygon_from_points(std::vector{kint::i2d{0, 0}, kint::i2d{-384, -16}, kint::i2d{-256, -64}}));
    world.shape_colors.emplace_back(sf::Color(50, 50, 150));
    world.shapes.emplace_back(kint::Shape_Rectangle({256, 0}, {0, 0}, {32, 32}));
    world.shape_colors.emplace_back(sf::Color(50, 50, 150));

    world.shapes.emplace_back(kint::Shape_Line({0, -128}, {0, 0}, {-64, 0}, true));
    world.shape_colors.emplace_back(sf::Color(200, 50, 200));

    world.shapes.emplace_back(kint::Shape_Line({612, 0}, {0, 0}, {0, -64}, true));
    world.shape_colors.emplace_back(sf::Color(200, 50, 200));
    world.shapes.emplace_back(kint::Shape_Line({622, -64}, {0, 0}, {0, 64}, true));
    world.shape_colors.emplace_back(sf::Color(200, 50, 200));

    world.shapes.emplace_back(kint::Shape_Polygon{{256 + 128, 0}, {0, 0}, std::vector{kint::i2d{128, -128}, kint::i2d{128, 0}}});
    world.shape_colors.emplace_back(sf::Color(50, 50, 150));

    assert(world.shapes.size() == world.shape_colors.size());
    return world;
}
void bouncer_think(kint::Shape_Variant & shapev, int ticks_total)
{
    std::visit([&](auto & shape)
    {
        if(ticks_total % 100 < 50)
            shape.velocity.y = -4;
        else
            shape.velocity.y = +4;
    }, shapev);
}
void World_update(World & world)
{
    player_think(world.player);
    world.player.pre_clip_velocity = world.player.shape.velocity;

    if(world.shapes.size())
        bouncer_think(world.shapes[0], world.ticks_total);

    kint::Platformer_Properties platformer_properties = kint::Platformer_Properties{.down_direction = kint::i2d{0, 1}};
    if(!world.player.recent_impacts.empty())
        platformer_properties.step_height = 17;
    if(world.player.was_walking == 0)
        platformer_properties.sticky_slope = kint::i2d{1, 1};

    kint::Minkowski_Set mset = minkowski_set_create(world.player.shape.dimensions, std::span<const kint::Shape_Variant>(world.shapes), std::views::iota(0));
    world.player.recent_impacts =  kint::move_and_slide(world.player.shape, mset, 32, platformer_properties);

    for(auto & shapev : world.shapes)
        std::visit([&](auto & shape)
        {
            shape.position += shape.velocity;
        }, shapev);

    if(world.player.stop_after_advance)
        world.player.shape.velocity = kint::i2d{0, 0};

    world.ticks_total++;

    if(world.shapes.size())
        bouncer_think(world.shapes[0], world.ticks_total);
}
void World_draw(const World & world, sf::RenderTarget & window, float interp_fraction)
{
    constexpr std::array<kint::i2d, 8> hairs = {kint::i2d{0, 1}, kint::i2d{1, 1}, kint::i2d{1, 0}, kint::i2d{1, -1}, kint::i2d{0, -1}, kint::i2d{-1, -1}, kint::i2d{-1, 0}, kint::i2d{-1, 1}};
    //constexpr std::array<kint::i2d, 1> hairs = {kint::i2d{-1, 1}};

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
    auto draw_impacts = [&](const kint::Shape_Line hat, const auto /*std::vector<Impact>*/ & impacts)
    {
        if(!impacts.size())
            return;
        for(const auto & impact : impacts)
        {
            shape_draw(window, interp_fraction, sf::Color::Green, impact.edge);
            //rational2d_draw(window, sf::Color::White, impact.position);

            kint::Rational2D vel = hat.node;

            kint::Rational2D fh = vel * impact.t;
            kint::Rational2D sh = (vel * (1 - impact.t)).reduced();
            kint::Rational2D edge = kint::Rational2D(impact.edge.node);
            kint::Rational2D sh_rotated = dot(sh, edge) * edge;
            kint::i2d::ntype edge_ls = dot(impact.edge.node, impact.edge.node);
            kint::Rational2D nv = (sh_rotated / edge_ls).reduced();
            kint::Rational2D cv = (fh + nv).reduced();

            assert(kint::Rational2D(hat.position) + fh == impact.position);
            rational2d_draw(window, sf::Color::White, kint::Rational2D(hat.position) + fh);
            rational2d_draw(window, sf::Color::Yellow, kint::Rational2D(hat.position) + cv);
            rational2d_draw(window, sf::Color::Blue, hat.position + slide_trunc(hat.position, cv, impact));
        }
    };
    const auto find_impacts = [](const auto & shapes, const kint::Shape_Line & hat, bool & hat_colliding, std::vector<kint::Impact> & hat_impacts)
    {
        for(const auto & s : shapes)
        {
            if(std::optional<kint::Impact> hat_impact = kint::raycast_unmoving(hat.position, hat.node_absolute(), s))
                hat_impacts.push_back(*hat_impact);
            if(kint::collides_unmoving(hat, s))
                hat_colliding = true;
        }
    };
    if(!show_minkowski)
    {
        for(const auto & h : hairs)
        {
            kint::Shape_Line hat = kint::Shape_Line(world.player.shape.position + h * 32 + kint::i2d{32, 32}, world.player.shape.velocity, h * 16, false);
            bool hat_colliding = false;
            std::vector<kint::Impact> hat_impacts = {};
            find_impacts(world.shapes, hat, hat_colliding, hat_impacts);
            shape_draw(window, interp_fraction, hat_colliding ? sf::Color::Red : sf::Color::Green,
                    hat);
            draw_impacts(hat, hat_impacts);
        }
    }
    else
    {
        const auto h = world.player.pre_clip_velocity;
        {
            kint::Shape_Line hat = kint::Shape_Line(world.player.shape.position, world.player.shape.velocity, h, false);
            std::vector<kint::Impact_ID> hat_impacts = raycast_Minkowski_Set(hat.position, hat.node_absolute(), mset);
            shape_draw(window, interp_fraction, sf::Color::White,
                       hat);
            draw_impacts(hat, hat_impacts);
        }
    }
}
