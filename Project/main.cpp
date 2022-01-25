#include <SFML/Graphics.hpp>
#include <iostream>
#include <thread>
#include <mutex>
#include "physics.h"
#include "Objects.h"
#include "Collider.h"

using namespace std;

int frameTime = 0;
int startTime = 0;

int offset = 0;
bool paused = false;
int pausedTime = 0;

bool escapeFlop = false;
bool jumpFlop = false;

bool threadValid = true;

enum class State {MainMenu, Setup, PlayerTurn, Paused, WinLoss};
vector<Object*> objects;
vector<sf::Drawable*> drawables;
vector<Object*> colliders; //for box on box collision
//vector<> for bullets

sf::CircleShape shape2(50.f);

int calcY(float xPos)
{
    return 0.f;
}

    sf::Vector2f resolve(Object* a, Object* sender)
    {
        int sample = 0;
        int samples = 8;
        int n = 1; //number or pixels to jump each time
        bool empty = false;
        float newPos = sender->getPosition().x;

        sf::Vector2f hitPos;

        //obj = moving obj = sender
        //collider = stationary object = a
        if (sender->getVelocity().x < 0) //left
        {
            hitPos = sf::Vector2f(sender->getPosition().x - (sample * n), sender->getPosition().y + sender->getHeight());
            for (int i = sender->getPosition().x - hitPos.x; i < sender->getPosition().x; i++)
            {
                if (!empty)
                    empty = !a->box.contains(hitPos);
                else
                {
                    newPos = i;
                    break;
                }
            }
        }

        if (sender->getVelocity().x > 0) //right
            hitPos = sf::Vector2f(sender->getVelocity().x + sender->box.getSize().x - (sample * n), sender->getPosition().y + sender->box.getSize().y);
            for (int i = hitPos.x - (sender->getPosition().x + sender->box.getSize().x); i < sender->getPosition().x + sender->box.getSize().x; i--)
            {
                if (!empty)
                    empty = !a->box.contains(hitPos);
                else
                {
                    newPos = i;
                    break;
                }
            }
            return sf::Vector2f(newPos, calcY(newPos));
    }


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
            for (Object* obj : objects) //iterate through every object
            {
                obj->updatePhysics(); //call the update physics on the current object
            }
            /*Simple collision resolution
            if (colliders[0]->box.containsBox(&colliders[1]->box))
            {
                sf::Vector2f dir;
                dir.x = (colliders[0]->box.getPosition().x < colliders[1]->box.getPosition().x) ? colliders[0]->box.getPosition().x - colliders[1]->box.getPosition().x : colliders[1]->box.getPosition().x - colliders[0]->box.getPosition().x;
                //cout << dir.x << endl;
                colliders[1]->move(physics::displacement(sf::Vector2f(0.f, 0.f), 0.1, physics::acceleration(sf::Vector2f(0.f, 0.f), 0.1f, physics::impulse(dir, 2))));
            }
            */
            targetTime += 10 - ((physicsClock.getElapsedTime().asMilliseconds() - offset) - startTime); // increment the target time by 10ms minus the time taken to run the loop
        }

        else
        {
            this_thread::sleep_for(500us); //wait for 0.5ms
        }
    }
}

int main()
{
    sf::RenderWindow window(sf::VideoMode(1920, 1080), "Game Window", sf::Style::Default); //create a new 1080p window with title game window and not a fullscreen style
    window.setVerticalSyncEnabled(true); //enables vSync if possible
    sf::Clock gameClock; //create a new clock
    State state = State::MainMenu; //set the initial state to the main menu state
    paused = true; //pause physics loop while everything is set up

    sf::CircleShape shape(100.f);
    shape.setFillColor(sf::Color::Green); //set fill colour of the shapes
    shape2.setFillColor(sf::Color::Blue);
    drawables.push_back(&shape); //add to render objects vector
    drawables.push_back(&shape2);
    Object newObj(20.f, sf::Vector2f(0, 0), sf::Vector2f(256, 256)); //create an object with mass of 20kg
    Object testObj(10.f, sf::Vector2f(0, 0), sf::Vector2f(256, 256)); //create an object with mass of 10kg
    newObj.setPosition(sf::Vector2f(25.f, 25.f));
    objects.push_back(&newObj); //add to physics objects vector
    drawables.push_back(&newObj); //add to render objects vector
    colliders.push_back(&newObj);
    objects.push_back(&testObj);
    drawables.push_back(&testObj);
    colliders.push_back(&testObj);

    thread physicsThread(physicsLoop);

    while (window.isOpen())
    {
        startTime = gameClock.getElapsedTime().asMilliseconds(); //initial time on each frame
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
            state = State::Setup; //switch to set state
            break;
        }
        case (State::Setup):
        {
            cout << "Setup" << endl;
            state = State::PlayerTurn; //switch to play state
            paused = false;//let execution of physics loop continue
            break;
        }

        case (State::PlayerTurn):
        {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space) && jumpFlop == false) //space key pressed
            {
                newObj.addAcceleration(physics::impulse(sf::Vector2f(50.f, -20000.f), newObj.getMass()));
                jumpFlop = true;
            }
            if (jumpFlop && !sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
                jumpFlop = false;

            if (paused == false && escapeFlop == false && sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) //if escape is pressed and allowed
            {
                pausedTime = gameClock.getElapsedTime().asMilliseconds(); //start recording the paused time
                paused = true; //pause physics loop
                state = State::Paused; //change state
                escapeFlop = true;
            }

            if (escapeFlop == true && !sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
                escapeFlop = false;

            //cout << "Playing" << endl;
            break;
        }

        case (State::Paused):
        {
            
            if (paused == true && escapeFlop == false && sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) //if escape is pressed and allowed
            {
                state = State::PlayerTurn; //change the state
                paused = false; //tell physics loop to continue
                escapeFlop = true;
                offset += gameClock.getElapsedTime().asMilliseconds() - pausedTime; //apply offset to physics clock
            }

            if (escapeFlop == true && !sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) //if the escape key is not pressed, reset the flop
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
        frameTime = gameClock.getElapsedTime().asMilliseconds() - startTime; //calculate frametime
        //cout << frameTime << "ms" << endl; //print the frame 
    }

    cout << "quitting" << endl;
    threadValid = false; //tell physics thread to stop looping
    physicsThread.join(); //wait until physicsThread finishes
    return 0;
}