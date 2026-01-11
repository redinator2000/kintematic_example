#include "player_controls.hpp"
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
void player_think(Player & player)
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
        player.want_velocity = new_vel;
        player.stop_after_advance = false;
    }

    if(new_vel == kint::i2d{0, 0})
        player.moving_time = 0;
    else
        player.moving_time++;

    for(auto & c : keyboardCatches)
        c = false;
}
