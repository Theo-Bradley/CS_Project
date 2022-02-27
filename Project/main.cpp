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
vector<Truck*> trucks;
vector<sf::Drawable*> drawables;
vector<Object*> colliders; //for box on box collision
vector<Projectile*> projectiles;
float groundArray[1922];
sf::VertexArray groundObject;

sf::CircleShape shape2(50.f);

void generateGround()
{
    float v;
    float w = 80; //flat ground height

    for (int x = 0; x < 1922; x++)
    {
        v = (sin(((float)x - 615.f) * (1.f / 220.f)) * 216) + 90; //magic numbers: x offset, x scale, y scale, y offset
        groundArray[x] = max(round(v), w);
    }

    groundObject.clear();

    for (int i = 0; i < 1922; i++)
    {
        groundObject.append(sf::Vertex(sf::Vector2f((float)i, 1080 - groundArray[i]), sf::Vector2f((float)(i % 960), 0)));
        groundObject.append(sf::Vertex(sf::Vector2f((float)i, 1080), sf::Vector2f((float)(i % 960), groundArray[i]))); //256 + 50 is the max height of the ground
    }
}

float calcY(Object* sender, float xPos)
{
    //cout << "calcingY\n";
    int pos = (int)round(xPos);
    sf::Vector2f size = sender->box.getSize();
    if (sender->box.getPosition().y + size.y >= 1080 - groundArray[pos] || sender->box.getPosition().y + size.y >= 1080 - groundArray[(pos + (int)round(size.x))])
    {
        float left =  1080 - groundArray[pos] - size.y;
        float right = 1080 - groundArray[(pos + (int)round(size.x))] - size.y;
        sender->setVelocity(sf::Vector2f(sender->getVelocity().x, 0.f));
        return (left <= right) ? left : right;
    }

    else 
    {
        return sender->getPosition().y;
    }
}

sf::Vector2f resolveX(Object* a, Object* sender)
{
    float newPos = sender->getPosition().x;

    //obj = moving obj = sender
    //collider = stationary object = a

    if (sender->getVelocity().x == 0) //no horizontal movement
    {
        float leftDistance = a->box.getPosition().x + a->box.getSize().x + FLT_EPSILON;
        float rightDistance = a->box.getPosition().x - sender->box.getSize().x - FLT_EPSILON;
        newPos = (abs(sender->box.getPosition().x - leftDistance) < abs(sender->box.getPosition().x - rightDistance)) ? leftDistance : rightDistance;
        cout << "Stationary object overlapping, position resolved.\n" << endl;
    }

    if (sender->getVelocity().x < 0) //left movement
    {
        newPos = a->box.getPosition().x + a->box.getSize().x + FLT_EPSILON;
        sender->setVelocity(sf::Vector2f(0.f, sender->getVelocity().y));
    }

    if (sender->getVelocity().x > 0) //right movement
    {
        newPos = a->box.getPosition().x - sender->box.getSize().x - FLT_EPSILON;
        sender->setVelocity(sf::Vector2f(0.f, sender->getVelocity().y));
    }

    return sf::Vector2f(newPos, calcY(sender, newPos)); //use when calcY has been implemented
    //return sf::Vector2f(newPos, sender->getPosition().y);
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
            for (Truck* truck : trucks) //iterate through every object
            {
                truck->updatePhysics(); //call the update physics on the current object
                truck->setPosition(truck->getPosition().x, calcY(truck, truck->getPosition().x));
            }
            if (colliders[0]->box.containsBox(&colliders[1]->box))
            {
                colliders[0]->setPosition(resolveX(colliders[1], colliders[0]));
            }
            for (Projectile* projectile : projectiles)
            {
                projectile->updatePhysics();
            }

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
    sf::CircleShape shape(6);
    shape.setFillColor(sf::Color::Red); //set fill colour of the shapes
    shape.setOrigin(6, 6);
    shape2.setFillColor(sf::Color::Blue);
    drawables.push_back(&shape2);
    Object testObj(10.f, sf::Vector2f(660, 600), sf::Vector2f(256, 256)); //create an object with mass of 10kg
    testObj.setColor(sf::Color::Green);
    Projectile* currentProjectile = nullptr;
    //newObj.setPosition(sf::Vector2f(25.f, 25.f));
    sf::Texture groundTex;
    groundTex.loadFromFile(R"(C:\Users\Blade\Project\CS_Project\x64\BladeDebug\Assets\Sprites\groundTex.png)");
    sf::RenderStates groundState;
    groundState.texture = &groundTex;
    sf::Texture truckTex;
    truckTex.loadFromFile(R"(C:\Users\Blade\Project\CS_Project\Project\Assets\Sprites\truck.png)");
    Truck truckTest(20.f, sf::Vector2f(120, 0), sf::Vector2f(75, 35));
    truckTest.setTexture(truckTex, true);
    sf::Texture projectileTex;
    projectileTex.loadFromFile(R"(C:\Users\Blade\Project\CS_Project\x64\BladeDebug\Assets\Sprites\projectile.png)");
    drawables.push_back(&truckTest);
    drawables.push_back(&truckTest.arm);
    drawables.push_back(&shape); //test circle should draw ontop of the arm
    trucks.push_back(&truckTest);
    colliders.push_back(&truckTest);
    //objects.push_back(&testObj);
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

            groundObject.resize(3842);
            groundObject.setPrimitiveType(sf::PrimitiveType::Lines);
            generateGround();

            state = State::PlayerTurn; //switch to play state
            paused = false;//let execution of physics loop continue
            break;
        }

        case (State::PlayerTurn):
        {
            if (paused == false && escapeFlop == false && sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) //if escape is pressed and allowed
            {
                pausedTime = gameClock.getElapsedTime().asMilliseconds(); //start recording the paused time
                paused = true; //pause physics loop
                state = State::Paused; //change state
                escapeFlop = true;
            }

            if (escapeFlop == true && !sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
                escapeFlop = false;

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space) && jumpFlop == false) //space key pressed
            {
                //truckTest.addAcceleration(physics::impulse(sf::Vector2f(+5000.f, 0.f), truckTest.getMass()));
                sf::Vector2f endPoint = truckTest.spawnPoint();
                sf::Vector2f startPoint = truckTest.arm.getTransform().transformPoint(0, 6.5);
                currentProjectile = new Projectile(endPoint - startPoint, &projectileTex);
                currentProjectile->setPosition(endPoint);
                projectiles.push_back(currentProjectile);
                drawables.insert(drawables.begin(), currentProjectile);

                jumpFlop = true;
            }

            if (jumpFlop && !sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
                jumpFlop = false;

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
            {
                truckTest.rotateAngle(-0.1f * frameTime);
            }

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
            {
                truckTest.rotateAngle(+0.1f * frameTime);
            }

            //cout << "Playing" << endl;
            break;
        }

        case (State::Paused):
        {
            //to do: set drawables to paused drawables
            if (paused == true && escapeFlop == false && sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) //if escape is pressed and allowed
            {
                //to do: set drawables back to gameplay state
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
        window.draw(groundObject, groundState);
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