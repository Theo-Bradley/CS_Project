class Object : public sf::Sprite
{
private:
    sf::Texture texture;

    sf::Vector2f velocity;
    sf::Vector2f acceleration;
    sf::Vector2f exAccel; //extraAcceleration
    float mass;

    mutex mtx;
    

public: 
    Object(float Mass)
    {
        texture.loadFromFile(R"(C:\Users\Blade\Project\CS_Project\x64\BladeDebug\Assets\Sprites\test.png)");
        setTexture(texture, true);
        mass = Mass;
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
    void setVelocity(sf::Vector2f vel, sf::Vector2f accel)
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
};