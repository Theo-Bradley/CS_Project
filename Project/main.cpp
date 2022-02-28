#include <SFML/Graphics.hpp>
#include <iostream>
#include <thread>
#include <mutex>
#include "physics.h"
#include "Objects.h"
#include "Collider.h"
#include "UI.h"

# define PI           3.14159265358979323846f  /* pi */

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
vector<sf::Drawable*> drawables;
vector<Element*> uiables;
vector<Object*> colliders; //for box on box collision
vector<Projectile*> projectiles;
vector<Element*> playUIables;
vector<sf::Drawable*> playDrawables;
float groundArray[1922];
sf::VertexArray groundObject;
bool playerTurn = true;

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


void physicsLoop(Truck* playerTruck, AITruck* aiTruck)
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
            playerTruck->updatePhysics(); //call the update physics on the player Truck
            playerTruck->setPosition(playerTruck->getPosition().x, calcY(playerTruck, playerTruck->getPosition().x));
            aiTruck->updatePhysics(); //call the update physics on the AI truck
            aiTruck->setPosition(aiTruck->getPosition().x, calcY(aiTruck, aiTruck->getPosition().x));

            if (playerTurn)
            {
                if (colliders[0]->box.containsBox(&colliders[1]->box))
                {
                    colliders[0]->setPosition(resolveX(colliders[1], colliders[0]));
                }
            }
            if (!playerTurn)
            {
                if (colliders[1]->box.containsBox(&colliders[0]->box))
                {
                    colliders[1]->setPosition(resolveX(colliders[0], colliders[1]));
                }
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
    int winner = -1;
    int difficulty = 1;
    bool shootFlop = true;
    Projectile* currentProjectile = nullptr;
    sf::Texture backgroundTex;
    backgroundTex.loadFromFile(R"(C:\Users\Blade\Project\CS_Project\x64\BladeDebug\Assets\Sprites\background.png)");
    sf::Sprite backgroundSprite;
    backgroundSprite.setTexture(backgroundTex, true);
    sf::Texture groundTex;
    groundTex.loadFromFile(R"(C:\Users\Blade\Project\CS_Project\x64\BladeDebug\Assets\Sprites\groundTex.png)");
    sf::RenderStates groundState;
    groundState.texture = &groundTex;
    sf::Texture truckTex;
    truckTex.loadFromFile(R"(C:\Users\Blade\Project\CS_Project\Project\Assets\Sprites\truck.png)");
    Truck playerTruck(20.f, sf::Vector2f(120, 700), sf::Vector2f(75, 35));
    playerTruck.setTexture(truckTex, true);
    AITruck aiTruck(20.f, sf::Vector2f(1800, 700), sf::Vector2f(75, 35));
    aiTruck.setTexture(truckTex, false);
    aiTruck.setTextureRect(sf::IntRect(sf::Vector2i(80, 0), sf::Vector2i(-80, 43)));
    sf::Texture projectileTex;
    projectileTex.loadFromFile(R"(C:\Users\Blade\Project\CS_Project\x64\BladeDebug\Assets\Sprites\projectile.png)");
    drawables.push_back(&playerTruck);
    drawables.push_back(&playerTruck.arm);
    colliders.push_back(&playerTruck);
    drawables.push_back(&aiTruck);
    drawables.push_back(&aiTruck.arm);
    colliders.push_back(&aiTruck);

    CentredText playerNameText;
    playerNameText.setText("Foo");
    uiables.push_back(&playerNameText);
    Healthbar playerHealthbar;
    uiables.push_back(&playerHealthbar);
    playerHealthbar.setPercent(100.f);

    CentredText aiNameText;
    aiNameText.setText("AI");
    uiables.push_back(&aiNameText);
    Healthbar aiHealthbar;
    uiables.push_back(&aiHealthbar);
    aiHealthbar.setPercent(100.f);
    PowerBar powerBar;
    powerBar.setPosition(sf::Vector2f(1880, 240));
    uiables.push_back(&powerBar);
    powerBar.setPercent(50.f);

    thread physicsThread(physicsLoop,&playerTruck, &aiTruck);

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
            std::cout << "Main Menu" << endl;
            state = State::Setup; //switch to set state
            break;
        }
        case (State::Setup):
        {
            std::cout << "Setup" << endl;

            groundObject.resize(3842);
            groundObject.setPrimitiveType(sf::PrimitiveType::Lines);
            generateGround();

            playerTruck.health = 100.f;
            aiTruck.health = 100.f;
            powerBar.setPercent(50.f);
            playerHealthbar.setPercent(100.f);
            aiHealthbar.setPercent(100.f);

            playerTruck.setPosition(sf::Vector2f(120, 700));
            aiTruck.setPosition(sf::Vector2f(1800, 700));

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

            if (playerTurn)
            {
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space) && jumpFlop == false && currentProjectile == nullptr) //space key pressed
                {
                    sf::Vector2f endPoint = playerTruck.spawnPoint();
                    sf::Vector2f startPoint = playerTruck.arm.getTransform().transformPoint(0, 6.5f); //transform point 35,6.5 for ai
                    currentProjectile = new Projectile(endPoint - startPoint, playerTruck.shotPower, &projectileTex);
                    currentProjectile->setPosition(endPoint);
                    projectiles.push_back(currentProjectile);
                    drawables.insert(drawables.begin(), currentProjectile);

                    jumpFlop = true;
                }

                if (jumpFlop && !sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
                    jumpFlop = false;

                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
                {
                    playerTruck.move(sf::Vector2f(frameTime * -0.2f, 0.f));
                    playerTruck.setPosition(playerTruck.getPosition().x, calcY(&playerTruck, playerTruck.getPosition().x));
                }

                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
                {
                    playerTruck.move(sf::Vector2f(frameTime * 0.2f, 0.f));
                    playerTruck.setPosition(playerTruck.getPosition().x, calcY(&playerTruck, playerTruck.getPosition().x));
                }

                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
                {
                    playerTruck.rotateAngle(-0.1f * frameTime);
                }

                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
                {
                    playerTruck.rotateAngle(+0.1f * frameTime);
                }

                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
                {
                    playerTruck.changePower(+0.1f * frameTime);
                    powerBar.setPercent(((playerTruck.shotPower + 500.f) / 1000.f) * 100.f);
                }

                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
                {
                    playerTruck.changePower(-0.1f * frameTime);
                    powerBar.setPercent(((playerTruck.shotPower + 500.f) / 1000.f) * 100.f);
                }

                if (currentProjectile != nullptr)
                {                    
                    switch (currentProjectile->coltest(&aiTruck.box, groundArray)) //make a new one for ai truck's turn
                    {
                    case 0:
                        drawables.erase(drawables.begin());
                        projectiles.pop_back();
                        playerTurn = false;
                        currentProjectile = nullptr;
                        delete currentProjectile;
                        break;

                    case 1:
                        aiTruck.changeHealth(30.f);
                        aiHealthbar.setPercent(aiTruck.health);
                        drawables.erase(drawables.begin());
                        projectiles.pop_back();
                        playerTurn = false;
                        currentProjectile = nullptr;
                        delete currentProjectile;
                        break;

                    case 2:
                        float distance = abs(aiTruck.box.getPosition().x - (aiTruck.box.getSize().x / 0.5f)) - (playerTruck.box.getPosition().x - (playerTruck.box.getSize().x * 0.5f));
                        aiTruck.changeHealth((1 / distance) * 20000.f);
                        aiHealthbar.setPercent(aiTruck.health);
                        drawables.erase(drawables.begin());
                        projectiles.pop_back();
                        playerTurn = false;
                        currentProjectile = nullptr;
                        delete currentProjectile;
                        break;
                    }

                    jumpFlop = true;
                }
            }

            else
            {
                /*
                * AI Code
                */
                if (jumpFlop)
                {
                    float distance = abs(playerTruck.getPosition().x - aiTruck.getPosition().x);
                    srand(gameClock.getElapsedTime().asMilliseconds());

                    switch (difficulty)
                    {
                    case 1:
                        distance += (rand() % 400) - 200;
                        break;

                    case 2:
                        distance += (rand() % 200) - 100;
                        break;

                    case 3:
                        distance += (rand() % 100) - 50;
                        break;
                    }

                    float top = distance * 9.81f;
                    float bottom = pow(physics::acceleration(sf::Vector2f(0, 0), 0.1, sf::Vector2f(1400, 1400)).x, 2);

                    float theta = asin(top / bottom);
                    theta *= 0.5;

                    aiTruck.setAngle((theta / PI) * 180.f);

                    sf::Vector2f endPoint = aiTruck.spawnPoint();
                    sf::Vector2f startPoint = aiTruck.arm.getTransform().transformPoint(35, 6.5f); //transform point 35,6.5 for ai
                    currentProjectile = new Projectile(endPoint - startPoint, 500.f, &projectileTex);
                    currentProjectile->setPosition(endPoint);
                    projectiles.push_back(currentProjectile);
                    drawables.insert(drawables.begin(), currentProjectile);
                    jumpFlop = false;
                }

                if (currentProjectile != nullptr)
                {
                    switch (currentProjectile->coltest(&playerTruck.box, groundArray)) //make a new one for ai truck's turn
                    {
                    case 0:
                        drawables.erase(drawables.begin());
                        projectiles.pop_back();
                        playerTurn = true;
                        currentProjectile = nullptr;
                        delete currentProjectile;
                        break;

                    case 1:
                        playerTruck.changeHealth(30.f);
                        aiHealthbar.setPercent(playerTruck.health);
                        drawables.erase(drawables.begin());
                        projectiles.pop_back();
                        playerTurn = true;
                        currentProjectile = nullptr;
                        delete currentProjectile;
                        break;

                    case 2:
                        float distance = abs(aiTruck.box.getPosition().x - (aiTruck.box.getSize().x / 0.5f)) - (playerTruck.box.getPosition().x - (playerTruck.box.getSize().x * 0.5f));
                        playerTruck.changeHealth((1 / distance) * 5000.f * difficulty);
                        aiHealthbar.setPercent(playerTruck.health);
                        drawables.erase(drawables.begin());
                        projectiles.pop_back();
                        playerTurn = true;
                        currentProjectile = nullptr;
                        delete currentProjectile;
                        break;
                    }
                }
            }
            playerHealthbar.setPercent(playerTruck.health);

            playerNameText.setMiddlePos(playerTruck.getPosition() + sf::Vector2f(40, -70));
            playerHealthbar.setPosition(sf::Vector2f(playerTruck.getPosition() + sf::Vector2f(-10, -50)));

            aiNameText.setMiddlePos(aiTruck.getPosition() + sf::Vector2f(40, -70));
            aiHealthbar.setPosition(sf::Vector2f(aiTruck.getPosition() + sf::Vector2f(-10, -50)));

            if (playerTruck.health <= 0)
            {
                winner = 1; //ai won
                state = State::WinLoss;
            }

            if (aiTruck.health <= 0)
            {
                winner = 0; //player won
                state = State::WinLoss;
            }

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
            
            std::cout << "Paused" << endl;
            break;
        }

        case (State::WinLoss):
        {
            std::cout << "winloss" << endl;
            playDrawables = drawables;
            playUIables = uiables;
            drawables.empty();
            uiables.empty();
            break;
        }
        }

        window.clear(); //clear back buffer
        window.draw(backgroundSprite);
        window.draw(groundObject, groundState);
        for (int i = 0; i < drawables.size(); i++)
        {
            window.draw(*drawables[i]); //draw to back buffer
        }

        for (Element* element : uiables)
        {
            window.draw(*element);
        }
        window.display(); //swap forward and back buffers
        frameTime = gameClock.getElapsedTime().asMilliseconds() - startTime; //calculate frametime
        //cout << frameTime << "ms" << endl; //print the frame 
    }

    std::cout << "quitting" << endl;
    threadValid = false; //tell physics thread to stop looping
    physicsThread.join(); //wait until physicsThread finishes
    return 0;
}