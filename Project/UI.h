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
    CentredText() = default;

public:
    CentredText(int size, string path)
    {
        font.loadFromFile(path + R"(\Assets\ariblk.ttf)");
        text.setFont(font);
        text.setCharacterSize(size);
        text.setFillColor(sf::Color::Black);
        text.setOutlineColor(sf::Color::White);
    }

public:
    void setText(string newText)
    {
        text.setString(newText);
        setPosition(sf::Vector2f(text.getGlobalBounds().width / 2.f, text.getPosition().y));
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
        sf::FloatRect globalBounds = text.getGlobalBounds();
        setPosition(sf::Vector2f(pos.x - (globalBounds.width)/2.f, pos.y - (globalBounds.height)/2.f));
    }

public:
    float getWidth()
    {
        return text.getGlobalBounds().width;
    }

public:
    float getHeight()
    {
        return text.getLocalBounds().height;
    }

public:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        target.draw(text);
    }
};

class Stars : public Element
{
private:
    unsigned int count;
    sf::Texture starTex;
    sf::Sprite star1;
    sf::Sprite star2;
    sf::Sprite star3;

public:
    Stars(unsigned int number, string path)
    {
        count = number;
        starTex.loadFromFile(path + R"(\Assets\Sprites\Star.png)");
        star1.setTexture(starTex, true);
        star2.setTexture(starTex, true);
        star3.setTexture(starTex, true);
    }

public:
    void move(sf::Vector2f amount)
    {
        star1.move(amount);
        star2.move(amount + sf::Vector2f(20, 0));
        star3.move(amount + sf::Vector2f(40, 0));
    }

public:
    void setPosition(sf::Vector2f pos)
    {
        star1.setPosition(pos);
        star2.setPosition(pos + sf::Vector2f(20, 0));
        star3.setPosition(pos + sf::Vector2f(40, 0));
    }

public:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        if (count >= 1)
            target.draw(star1);

        if (count >= 2)
            target.draw(star2);

        if (count >= 3)
            target.draw(star3);
    }
};

class Button : public Element
{
    sf::RectangleShape bgRect;
    CentredText* text;

public:
    Button(string str, string path)
    {
        text = new CentredText(60, path);
        text->setText(str);
        bgRect.setSize(sf::Vector2f(80 + text->getWidth(), 40 + text->getHeight()));
        bgRect.setFillColor(sf::Color::White);
        text->setMiddlePos(sf::Vector2f(bgRect.getPosition().x + (bgRect.getSize().x * 0.5f), bgRect.getPosition().y - 100 + (bgRect.getSize().y * 0.3f))); //y is * by 0.3f to kep play and quit buttons centred-ish
    }

public:
    bool contains(sf::Vector2i position)
    {
        sf::Vector2f rectPos = bgRect.getPosition();
        sf::Vector2f size = bgRect.getSize();
        if (position.x >= rectPos.x && position.x <= rectPos.x + size.x && position.y >= rectPos.y && position.y <= rectPos.y + size.y)
        {
            return true;
        }
        return false;
    }

public:
    void setColour(sf::Color col)
    {
        bgRect.setFillColor(col);
    }

public:
    void setPosition(sf::Vector2f position)
    {
        bgRect.setPosition(position);
        //text.setMiddlePos((bgRect.getSize() * 0.5f) + bgRect.getPosition());
        text->setMiddlePos(sf::Vector2f(bgRect.getPosition().x + (bgRect.getSize().x * 0.5f), bgRect.getPosition().y + (bgRect.getSize().y * 0.3f))); //y size is * 0.3f to keep play and quit buttons centred

    }

public:
    float getWidth()
    {
        return bgRect.getLocalBounds().width;
    }

public:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        target.draw(bgRect);
        target.draw(*text);
    }

public:
    ~Button()
    {
        delete text;
    }
};

class TextBox : public Element
{
    sf::Text uiText;
    sf::Font font;
    sf::Text label;
    string text;
    sf::RectangleShape bgRect;

public:
    TextBox(string path)
    {
        font.loadFromFile(path + R"(\Assets\ariblk.ttf)");
        uiText.setFont(font);
        uiText.setCharacterSize(25);
        label.setFont(font);
        label.setCharacterSize(25);
        label.setString("Username:");
        label.setFillColor(sf::Color::Black);
        bgRect.setFillColor(sf::Color::Red);
        bgRect.setSize(sf::Vector2f(200, 50));
    }

public:
    bool valid(char c)
    {
        bool isValid = true;
        string allowedChars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 ";

        if (text.length() + 1 > 6)
        {
            isValid = false;
        }
        else
        {
            if (allowedChars.find(c) == string::npos)
            {
                isValid = false;
            }

            else
            {
                appendString(c);
            }
        }

        if (text.length() < 3)
            isValid = false;

        return isValid;
    }

public:
    void updateColour(bool valid)
    {
        if (valid)
            bgRect.setFillColor(sf::Color::Green);
        else
            bgRect.setFillColor(sf::Color::Red);
    }

public:
    bool isValid()
    {
        if (bgRect.getFillColor() == sf::Color::Green)
            return true;
        else
            return false;
    }

public:
    bool popBack()
    {
        if (text.length() <= 0)
            return false;
        text.pop_back();
        uiText.setString(text);
        if (text.length() < 3)
            return false;
        return true;
    }

public:
    void setPosition(sf::Vector2f position)
    {
        bgRect.setPosition(position);
        uiText.setPosition(position + sf::Vector2f(10, 10));
        label.setPosition(position - sf::Vector2f(160, -10));
    }

public:
    void appendString(string str)
    {
        text += str;
        uiText.setString(str);
    }

public:
    void appendString(char c)
    {
        text += c;
        uiText.setString(text);
    }

public:
    string getText()
    {
        return text;
    }

public:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        target.draw(bgRect);
        target.draw(label);
        target.draw(uiText);
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