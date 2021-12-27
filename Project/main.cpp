#include <SFML/Graphics.hpp>
#include <iostream>
#include <thread>
#include <mutex>
#include "physics.h"

using namespace std;

class Object : public sf::Sprite
{
private: sf::Texture texture;

private: sf::Vector2f velocity;
private: sf::Vector2f acceleration;
private: sf::Vector2f exAccel; //extraAcceleration

private: mutex mtx;

public: Object()
{
    texture.loadFromFile(R"(F:\Project\Project\x64\Debug\Assets\Sprites\test.png)");
    setTexture(texture, true);
}

public: void UpdatePhysics()
{
    mtx.lock();
    acceleration = sf::Vector2f(0, 9.81f); //9.81 is gravitational acceleration
    acceleration += exAccel;
    exAccel = sf::Vector2f(0.f, 0.f);
    velocity = physics::Acceleration(velocity, 0.01f, acceleration);
    move(physics::Displacement(velocity, 0.01f, acceleration)); //0.01 is length of physics loop
    mtx.unlock();
}

public: void AddAcceleration(sf::Vector2f accel)
{
    exAccel += accel;
}

public: void setVelocity(sf::Vector2f vel, sf::Vector2f accel)
{
    velocity = vel;
}

public: sf::Vector2f getVelocity()
{
    return velocity;
}
};

bool jumpFlop = true;
int frameTime = 0;
int startTime = 0;

bool threadValid = true;

sf::CircleShape shape2(50.f);
sf::Vector2f shape2Vel = sf::Vector2f(0.f, 0.f);
sf::Vector2f shape2Accel = sf::Vector2f(0.f,0.f);

Object testObj;

void physicsLoop()
{
    using namespace chrono_literals;
    sf::Clock physicsClock;
    int targetTime = 10;
    while (threadValid)
    {
        if (physicsClock.getElapsedTime().asMilliseconds() >= targetTime)
        {
            shape2Accel = sf::Vector2f(0, 9.81f); //9.81 is gravitational acceleration
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space) && jumpFlop)
            {
                shape2Accel += physics::Impulse(sf::Vector2f(2.f, -2000.f), 1.5); // (force in N, time in s, mass in kg)
                jumpFlop = false;
            }
            if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Space) && !jumpFlop)
            {
                jumpFlop = true;
            }
            shape2Vel = physics::Acceleration(shape2Vel, 0.01f, shape2Accel);
            shape2.move(physics::Displacement(shape2Vel, 0.01f, shape2Accel)); //frameTime is /1000 to get time in seconds
            testObj.UpdatePhysics();
            targetTime += 10;
        }

        else
        {
            this_thread::sleep_for(500us);
        }
    }
}

int main()
{
    vector<sf::Drawable*> drawables;
    sf::RenderWindow window(sf::VideoMode(1920, 1080), "Game Window", sf::Style::Fullscreen);
    window.setVerticalSyncEnabled(true); //enables vSync if possible
    sf::Clock gameClock;
    sf::CircleShape shape(100.f);
    shape.setFillColor(sf::Color::Green);

    shape2.setFillColor(sf::Color::Blue);
    drawables.push_back(&shape);
    drawables.push_back(&shape2);
    std::cout << "Added Shape" << endl; //simple debug to make sure there isn't dome memory leak occuring

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

        window.clear(); //clear back buffer
        window.draw(testObj);
        for (int i = 0; i < drawables.size(); i++)
        {
            window.draw(*drawables[i]); //draw to back buffer
        }
        window.display(); //swap forward and back buffers
        frameTime = gameClock.getElapsedTime().asMilliseconds() - startTime;
        cout << frameTime << endl << endl;
    }

    threadValid = false; //tell physics thread to stop looping
    physicsThread.join(); //wait until physicsThread finishes
    return 0;
}