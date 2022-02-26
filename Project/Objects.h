#include "Collider.h"
#include <algorithm>
# define PI           3.14159265358979323846  /* pi */

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
        if (box.getPosition().x + amount.x > 1920 - box.getSize().x)
        {
            Object::setPosition(1920 - box.getSize().x, box.getPosition().y);
            return;
        }

        if (box.getPosition().x + amount.x < 0)
        {
            Object::setPosition(0, box.getPosition().y);
            return;
        }

        else
        {
            sf::Sprite::move(amount);
            box.move(amount);
        }
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
        Position.x = clamp(Position.x, 0.f, 1920.f);
        sf::Sprite::setPosition(Position);
        box.setPosition(Position);
    }

public:
    void setPosition(float x, float y)
    {
        x = clamp(x, 0.f, 1920.f);
        sf::Sprite::setPosition(x, y);
        box.setPosition(x, y);
    }
};

class Truck : public Object
{
public:
    sf::RectangleShape arm;

private:
    float aimAngle = 0.f; //degrees

public:
    Truck(float Mass, sf::Vector2f Position, sf::Vector2f Size) : Object(Mass, Position, Size)
    {
        arm.setSize(sf::Vector2f(35, 13));
        arm.setPosition(Position + sf::Vector2f(3, 15));
        arm.setOrigin(0, 13);
        arm.setFillColor(sf::Color::Green);
    }

public:
    void setPosition(sf::Vector2f position)
    {
        Object::setPosition(position);
        arm.setPosition(position + sf::Vector2f(3, 15));
    }

public:
    void setPosition(float x, float y)
    {
        Object::setPosition(x, y);
        arm.setPosition(x + 3, y + 15);
    }

public:
    void move(sf::Vector2f amount)
    {
        Object::move(amount);
        arm.move(amount);
    }

public:
    void setAngle(float angle)
    {
        aimAngle = angle;
        aimAngle = remainder(aimAngle, 360.f);
        aimAngle = std::clamp(aimAngle, -90.f, 0.f);
        arm.setRotation(angle);
    }

public: 
    void rotateAngle(float amount)
    {
        aimAngle += amount;
        aimAngle = remainder(aimAngle, 360.f);
        aimAngle = clamp(aimAngle, -90.f, 0.f);
        arm.setRotation(aimAngle);
    }

public:
    sf::Vector2f spawnPoint()
    {
        sf::Vector2f armPos = arm.getTransform().transformPoint(sf::Vector2f(0.f, 6.5f));
        float radAngle = (aimAngle / 180) * PI;
        sf::Vector2f spawnPoint = sf::Vector2f(armPos.x + cos(radAngle) * 28.5f, armPos.y + sin(radAngle) * 28.5f); //35 is the arm length
        return spawnPoint;
    }
};