#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
//#include "Objects.h"
class Collider
{
	sf::Vector2f pos;
	sf::Vector2f size;

public:
	void move(sf::Vector2f amount)
	{
		setPosition(getPosition() + amount);
	}

public:
	sf::Vector2f getPosition()
	{
		return pos;
	}

public:
	sf::Vector2f getSize()
	{
		return size;
	}

public:
	void setPosition(sf::Vector2f Position)
	{
		pos = Position;
	}

public: 
	void setSize(sf::Vector2f Size)
	{
		size = Size;
	}
};
class BoxCollider : public Collider
{
public:
	BoxCollider()
	{
	}

public:
	BoxCollider(sf::Vector2f Position, sf::Vector2f Size)
	{
		setPosition(Position);
		setSize(Size);
	}

public:
	bool containsBox(BoxCollider* b)
	{
		sf::Vector2f aPos = getPosition(); //add offset later
		sf::Vector2f aSize = getSize();
		sf::Vector2f bPos = b->getPosition(); //add offset later
		sf::Vector2f bSize = b->getSize();
		if (aPos.x < bPos.x + bSize.x && aPos.x + aSize.x > bPos.x &&
			aPos.y > bPos.y - bSize.y && aPos.y - aSize.y < bPos.y)
		{
			return true;
		}
		return false;
	}

public:
	bool contains(sf::Vector2f point)
	{
		return (point.x > getPosition().x && point.y > getPosition().y && point.x < getPosition().x + getSize().x && point.y < getPosition().y + getSize().y);
	}
};
