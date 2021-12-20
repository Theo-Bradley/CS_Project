#include <SFML/Graphics.hpp>
#include <iostream>

using namespace std;

int main()
{
    vector<sf::Drawable*> drawables;
    sf::RenderWindow window(sf::VideoMode(200, 200), "SFML works!");
    sf::CircleShape shape(100.f);
    shape.setFillColor(sf::Color::Green);
    sf::CircleShape shape2(50.f);
    shape2.setFillColor(sf::Color::Blue);
    drawables.push_back(&shape);
    drawables.push_back(&shape2);
    std::cout << "Added Shape" << endl; //simple debug to make sure there isn't dome memory leak occuring

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }
        window.clear(); //clear back buffer
        for (int i = 0; i < drawables.size(); i++)
        {
            window.draw(*drawables[i]); //draw to back buffer
        }
        window.display(); //swap forward and back buffers
    }

    return 0;
}