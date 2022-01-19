#include <SFML/Graphics.hpp>
#include <iostream>
#include <thread>
#include <mutex>
#include "physics.h"
#include "Objects.h"

using namespace std;

int frameTime = 0;
int startTime = 0;

int offset = 0;
bool paused = false;
int pausedTime = 0;
bool escapeFlop = false;

bool threadValid = true;

enum class State {MainMenu, Setup, PlayerTurn, Paused, WinLoss};
vector<Object*> objects;
vector<sf::Drawable*> drawables;


sf::CircleShape shape2(50.f);
void physicsLoop()
{
    using namespace chrono_literals;
    sf::Clock physicsClock;
    int targetTime = 10;
    int startTime;
    while (threadValid)
    {
        if (physicsClock.getElapsedTime().asMilliseconds() - offset >= targetTime && paused == false)
        {
            startTime = physicsClock.getElapsedTime().asMilliseconds() - offset;
            for (Object* obj : objects)
            {
                obj->updatePhysics();
            }
            targetTime += 10 - ((physicsClock.getElapsedTime().asMilliseconds() - offset) - startTime);
        }

        else
        {
            this_thread::sleep_for(500us);
        }
    }
}

int main()
{
    sf::RenderWindow window(sf::VideoMode(1920, 1080), "Game Window", sf::Style::Default);
    window.setVerticalSyncEnabled(true); //enables vSync if possible
    sf::Clock gameClock;
    State state = State::MainMenu;

    sf::CircleShape shape(100.f);
    shape.setFillColor(sf::Color::Green);

    shape2.setFillColor(sf::Color::Blue);
    drawables.push_back(&shape);
    drawables.push_back(&shape2);
    Object newObj(20.f);
    Object testObj(10.f);
    newObj.setPosition(25.f, 25.f);
    objects.push_back(&newObj);
    drawables.push_back(&newObj);
    objects.push_back(&testObj);
    drawables.push_back(&testObj);
    //newObj.addAcceleration(physics::impulse(sf::Vector2f(50.f, -1333.f), 2.f));

    thread physicsThread(physicsLoop);

    while (window.isOpen())
    {
        startTime = gameClock.getElapsedTime().asMilliseconds();
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        switch (state)
        {
        case (State::MainMenu):
        {
            cout << "Main Menu" << endl;
            state = State::Setup;
            break;
        }
        case (State::Setup):
        {
            cout << "Setup" << endl;
            state = State::PlayerTurn;
            break;
        }

        case (State::PlayerTurn):
        {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
            {
                newObj.addAcceleration(physics::impulse(sf::Vector2f(50.f, -2000.f), newObj.getMass()));
            }

            if (paused == false && escapeFlop == false && sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
            {
                pausedTime = gameClock.getElapsedTime().asMilliseconds();
                paused = true;
                state = State::Paused;
                escapeFlop = true;
            }

            if (escapeFlop == true && !sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
                escapeFlop = false;

            cout << "Playing" << endl;
            break;
        }

        case (State::Paused):
        {
            
            if (paused == true && escapeFlop == false && sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
            {
                state = State::PlayerTurn;
                paused = false;
                escapeFlop = true;
                offset += gameClock.getElapsedTime().asMilliseconds() - pausedTime;
            }

            if (escapeFlop == true && !sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
                escapeFlop = false;
            
            cout << "Paused" << endl;
            break;
        }

        case (State::WinLoss):
        {
            cout << "winloss" << endl;
            break;
        }
        }

        window.clear(); //clear back buffer
        for (int i = 0; i < drawables.size(); i++)
        {
            window.draw(*drawables[i]); //draw to back buffer
        }
        window.display(); //swap forward and back buffers
        frameTime = gameClock.getElapsedTime().asMilliseconds() - startTime;
        cout << frameTime << endl;
    }

    threadValid = false; //tell physics thread to stop looping
    physicsThread.join(); //wait until physicsThread finishes
    return 0;
}