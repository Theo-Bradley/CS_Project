#pragma once
#include <SFML/Graphics.hpp>
#include <iostream> 
#include <algorithm>

using namespace std;

class Element : public sf::Transformable, public sf::Drawable
{
public:
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        cout << "not working\n";
    }
};

class Healthbar : public Element
{
private:
    float percent;

    sf::RectangleShape green;
    sf::RectangleShape red;

public:
    Healthbar()
    {
        green.setFillColor(sf::Color::Green);
        green.setSize(sf::Vector2f(100, 10));
        red.setFillColor(sf::Color::Red);
        red.setSize(sf::Vector2f(100, 10));
        percent = 100.f;
    }

public:
    void setPercent(float newPercent)
    {
        percent = clamp(newPercent, 0.f, 100.f);
        green.setSize(sf::Vector2f(percent, 10)); //percent just so happens to work for size
    }

public:
    void setPosition(sf::Vector2f position)
    {
        Transformable::setPosition(position);
        red.setPosition(position);
        green.setPosition(position);
    }

public:
    void move(sf::Vector2f amount)
    {
        Transformable::move(amount);
        red.move(amount);
        green.move(amount);
    }

public:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        target.draw(red);
        target.draw(green);
    }
};

class PowerBar : public Element
{
private:
    float percent;

    sf::RectangleShape white;
    sf::RectangleShape yellow;

public:
    PowerBar()
    {
        white.setFillColor(sf::Color(200, 200, 200));
        white.setSize(sf::Vector2f(20, 400));
        yellow.setFillColor(sf::Color::Yellow);
        yellow.setSize(sf::Vector2f(20, 400));
        percent = 100.f;
    }

public:
    void setPercent(float newPercent)
    {
        percent = clamp(newPercent, 0.f, 100.f);
        yellow.setSize(sf::Vector2f(20, (percent/100.f) * 400)); //percent just so happens to work for size
        yellow.setPosition(sf::Vector2f(yellow.getPosition().x, white.getPosition().y + 400 - yellow.getSize().y));
    }

public:
    void setPosition(sf::Vector2f position)
    {
        Transformable::setPosition(position);
        white.setPosition(position);
        yellow.setPosition(position);
    }

public:
    void move(sf::Vector2f amount)
    {
        Transformable::move(amount);
        white.move(amount);
        yellow.move(amount);
    }

public:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        target.draw(white);
        target.draw(yellow);
    }
};

class CentredText : public Element
{
private:
    sf::Text text;
    sf::Font font;

    sf::Vector2f middlePos;

public:
    CentredText()
    {
        font.loadFromFile(R"(C:\Users\Blade\Project\CS_Project\x64\BladeDebug\Assets\ariblk.ttf)");
        text.setFont(font);
        text.setCharacterSize(16);
        text.setFillColor(sf::Color::Black);
        text.setOutlineColor(sf::Color::White);
    }

public:
    void setText(string newText)
    {
        text.setString(newText);
        setPosition(sf::Vector2f(middlePos.x - newText.length() * (16/2), text.getPosition().y));
    }

public:
    void setPosition(sf::Vector2f position)
    {
        Transformable::setPosition(position);
        text.setPosition(position);
    }

public:
    void move(sf::Vector2f amount)
    {
        Transformable::move(amount);
        text.move(amount);
    }

public:
    void setMiddlePos(sf::Vector2f pos)
    {
        setPosition(sf::Vector2f(pos.x - (text.getString().getSize() * 16.f)/2.f, pos.y - 7.5f));
    }

public:
    sf::Vector2f getTextSize()
    {
        return sf::Vector2f(16.f * (int)text.getString().getSize(), 16.f);
    }

public:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        target.draw(text);
    }
};

class Button : public Element
{
    sf::RectangleShape bgRect;
    CentredText text;
    sf::Font font;

public:
    Button(string str)
    {
        font.loadFromFile(R"(C:\Users\Blade\Project\CS_Project\x64\BladeDebug\Assets\ariblk.ttf)");
        text.setText(str);
        bgRect.setSize(sf::Vector2f(40, 20) + text.getTextSize());
        bgRect.setFillColor(sf::Color::White);
        text.setMiddlePos((bgRect.getSize() * 0.5f) + bgRect.getPosition());
    }

public:
    void setPosition(sf::Vector2f position)
    {
        bgRect.setPosition(position);
        text.setMiddlePos((bgRect.getSize() * 0.5f) + bgRect.getPosition());
    }

public:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        target.draw(bgRect);
        target.draw(text);
    }
};

class foo : public Element //example class
{
public:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        cout << "working\n";
    }
};