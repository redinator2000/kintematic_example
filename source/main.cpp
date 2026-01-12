#include <SFML/Graphics.hpp>
#include "shape_draw.hpp"
#include "world.hpp"
#include "player_controls.hpp"

sf::View window_default_view_fixed(sf::Vector2u window_size)
{
    sf::View view;
    view.setCenter(sf::Vector2f(window_size) / 2.0f);
    view.setSize(sf::Vector2f(window_size));
    return view;
}

float fixed_update(sf::Clock & tick_clock, auto callable) //returns interp_fraction
{
    constexpr sf::Time update_rate = sf::microseconds(16666 * 20);
    sf::Time delta = tick_clock.getElapsedTime();
    if(delta >= update_rate)
    {
        tick_clock.restart();
        for(int i = 0; i < 4; i++) //this would only happen for big frame hickups. If one callabe() took a long time, the next callable() might happen a bunch to catch up
        {
            callable();
            delta -= update_rate;
            if(delta < update_rate)
                return delta.asSeconds() / update_rate.asSeconds();
        }
    }
    return delta.asSeconds() / update_rate.asSeconds();
}

int main()
{
    auto window = sf::RenderWindow{ sf::VideoMode{ { 800u, 600u } }, "kintematic" };
    window.setFramerateLimit(144);

    World world = make_a_level();

    sf::Clock tick_clock;
    while(window.isOpen())
    {
        while(const std::optional event = window.pollEvent())
        {
            if(event->is<sf::Event::Closed>())
            {
                window.close();
            }
            else if(const auto * keyPressed = event->getIf<sf::Event::KeyPressed>())
            {
                if(keyPressed->code == sf::Keyboard::Key::R)
                    world = make_a_level();
                handle_event(*keyPressed);
            }
        }

        float interp_fraction = fixed_update(tick_clock, [&]
        {
            World_update(world);
        });

        window.clear();
        sf::View view = sf::View(sf::Vector2f(12.0f, 0.0f), sf::Vector2f(window.getSize()) / 16.0f);
        window.setView(view);
        grid_draw(window, view);
        World_draw(world, window, interp_fraction);
        window.display();
    }
}
