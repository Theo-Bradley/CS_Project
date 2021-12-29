#pragma once
#include <SFML/Graphics.hpp>
#include <math.h>
#include <iostream>

using namespace std;

namespace physics
{
	const float g = 9.81f; //gravitaional field strength
	const float sF = 50.f; //scale factor of 50 Pixels per Metre

	sf::Vector2f displacement(sf::Vector2f u, float t, sf::Vector2f a)
	{
		//s = ut + 1/2at^2
		float sX = (u.x * t) + (0.5f * a.x * pow(t, 2.f));
		float sY = (u.y * t) + (0.5f * a.y * pow(t, 2.f));
		return sf::Vector2f(sX * sF, sY * sF);
	}

	sf::Vector2f acceleration(sf::Vector2f u, float t, sf::Vector2f a)
	{
		//v = u + at
		float vX = u.x + (a.x * t);
		float vY = u.y + (a.y * t);
		return sf::Vector2f(vX, vY);
	}

	sf::Vector2f impulse(sf::Vector2f force, float mass)
	{
		//f = ma so a = f/m
		sf::Vector2f a = force / mass;
		return a;
	}
}