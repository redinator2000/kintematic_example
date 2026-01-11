#ifndef PLAYER_CONTROLS_HPP
#define PLAYER_CONTROLS_HPP

#include <SFML/Window/Event.hpp>
#include "world.hpp"

void handle_event(const sf::Event::KeyPressed &);
void player_think(Player &);

#endif // PLAYER_CONTROLS_HPP
