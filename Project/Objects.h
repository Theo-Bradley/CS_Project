#include "Collider.h"

class Object : public sf::Sprite
{
public:
    BoxCollider box;

private:
    sf::Texture texture;

    sf::Vector2f velocity;
    sf::Vector2f acceleration;
    sf::Vector2f exAccel; //extraAcceleration
    float mass;
    sf::Vector2f offset;

    mutex mtx;
    

public: 
    Object(float Mass, sf::Vector2f Position, sf::Vector2f Size)
    {
        texture.loadFromFile(R"(C:\Users\Blade\Project\CS_Project\x64\BladeDebug\Assets\Sprites\test.png)");
        setTexture(texture, true);
        mass = Mass;
        //box.setPosition(Position + offset);
        box.setSize(Size);
        setPosition(Position);
    }

public:
    void updatePhysics()
    {
        mtx.lock();
        acceleration = sf::Vector2f(0, 9.81f); //9.81 is gravitational acceleration
        acceleration += exAccel;
        exAccel = sf::Vector2f(0.f, 0.f);
        velocity = physics::acceleration(velocity, 0.01f, acceleration);
        move(physics::displacement(velocity, 0.01f, acceleration)); //0.01 is length of physics loop
        mtx.unlock();
    }

public:
    void addAcceleration(sf::Vector2f accel)
    {
        exAccel += accel;
    }

public:
    void move(sf::Vector2f amount)
    {
        sf::Sprite::move(amount);
        box.move(amount);
    }

public:
    void setVelocity(sf::Vector2f vel)
    {
        velocity = vel;
    }

public:
    sf::Vector2f getVelocity()
    {
        return velocity;
    }

public:
    float getMass()
    {
        return mass;
    }

public:
    int getWidth()
    {
        return sf::Sprite::getTextureRect().width;
    }

public:
    int getHeight()
    {
        return sf::Sprite::getTextureRect().height;
    }

public:
    void setPosition(sf::Vector2f Position)
    {
        sf::Sprite::setPosition(Position);
        box.setPosition(Position);
    }

public:
    void setPosition(float x, float y)
    {
        sf::Sprite::setPosition(x, y);
        box.setPosition(x, y);
    }
};