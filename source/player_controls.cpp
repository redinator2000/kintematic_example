#include "player_controls.hpp"
#include <algorithm>

std::array<bool, sf::Keyboard::KeyCount> keyboardCatches = {0};

void handle_event(const sf::Event::KeyPressed & e)
{
    keyboardCatches[(size_t)e.code] = true;
}
bool key_held(sf::Keyboard::Key key)
{
    if(sf::Keyboard::isKeyPressed(key))
        return true;
    return keyboardCatches[(size_t)key];
}
void player_fly(Player & player)
{
    kint::i2d new_vel = {0, 0};
    if(key_held(sf::Keyboard::Key::W))    new_vel.y -= 1;
    if(key_held(sf::Keyboard::Key::S))    new_vel.y += 1;
    if(key_held(sf::Keyboard::Key::A))    new_vel.x -= 1;
    if(key_held(sf::Keyboard::Key::D))    new_vel.x += 1;

    bool finish_moving = (new_vel == kint::i2d{0, 0} && player.moving_time > 1);
    if(finish_moving)
        player.stop_after_advance = true;
    else
    {
        player.shape.velocity = new_vel * 8;
        player.stop_after_advance = false;
    }

    if(new_vel == kint::i2d{0, 0})
        player.moving_time = 0;
    else
        player.moving_time++;
}
void player_walk(Player & player)
{
    player.shape.velocity.y += 1;
    bool left = key_held(sf::Keyboard::Key::A);
    bool right = key_held(sf::Keyboard::Key::D);
    if(left && right)
    {
        left = false;
        right = false;
    }
    if(left || right)
        player.was_walking = 10;
    if(right && player.shape.velocity.x < 10)
        player.shape.velocity.x += 1;
    if(left  && player.shape.velocity.x > -10)
        player.shape.velocity.x -= 1;
    if(!right && player.shape.velocity.x > 0)
        player.shape.velocity.x -= 1;
    if(!left && player.shape.velocity.x < 0)
        player.shape.velocity.x += 1;

    std::optional<kint::Impact> ground = [&]() -> std::optional<kint::Impact>
    {
        for(const auto & i : player.recent_impacts)
            if(gcf::dot(i.edge_normal(), kint::i2d{0, 1}) < 0)
                return i;
        return std::nullopt;
    }();
    if(ground && key_held(sf::Keyboard::Key::W))
        player.shape.velocity.y = ground->edge.velocity.y - 12;
}
void player_think(Player & player)
{
    if(player.flying)
        player_fly(player);
    else
        player_walk(player);

    for(auto & c : keyboardCatches)
        c = false;

    if(player.was_walking)
        player.was_walking--;

    // player.want_velocity = kint::i2d{-3, 3};
}
