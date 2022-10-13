#include "SFML/Graphics.hpp"
#include "SFML/Window.hpp"
#include <vector>
#include <iostream>
#include <algorithm>
#include <random>


const short unsigned int widthScreen = 800;
const short unsigned int heidthScreen = 600;
const short unsigned int fps = 120;
const short unsigned int radiusOfTriangle = 250;
const float thickness = 5.f;
const short unsigned int radiusOfPoint = 2;
const unsigned int numOfPoints = 250000;
const sf::Color screenColor = sf::Color::White;
const sf::Color triangleColor = sf::Color::Red;
const sf::Color pointColor = sf::Color::Black;

std::random_device rd;
std::mt19937 gen(rd());

void get_points(sf::CircleShape& figure, std::vector<sf::Vector2f>& points);
sf::Vector2f& gen_first_point_in_triangle(std::vector<sf::Vector2f>& pointsOfTriangle, std::vector<sf::CircleShape>& pointsInTriangle);
void draw_all_points(sf::RenderWindow& window, std::vector<sf::CircleShape>& points, std::vector<sf::Vector2f>& pointsOfTriangle, sf::Vector2f& currentPos)
{
	for (int i = 0; i < points.size();i++)
		window.draw(points[i]);
	if (points.size() < numOfPoints)
	{
		std::uniform_int_distribution<int> point(0, 2);
		sf::CircleShape newPoint(radiusOfPoint);
		newPoint.setFillColor(pointColor);
		size_t index = point(gen);
		currentPos.x = (pointsOfTriangle[index].x + currentPos.x) / 2;
		currentPos.y = (pointsOfTriangle[index].y + currentPos.y) / 2;
		newPoint.setPosition(currentPos);
		points.push_back(newPoint);
	}
}

int main()
{
	sf::RenderWindow window(sf::VideoMode(widthScreen, heidthScreen), "The Serpinsky triangle");
	window.setFramerateLimit(fps);
	window.setActive(false);
	std::vector<sf::Vector2f> pointsOfTriangle;
	pointsOfTriangle.push_back({ widthScreen / 2,heidthScreen * 0.1 });
	pointsOfTriangle.push_back({ widthScreen * 0.1, heidthScreen * 0.9 });
	pointsOfTriangle.push_back({ widthScreen * 0.9, heidthScreen * 0.9 });
	sf::ConvexShape triangle(3);
	triangle.setPoint(0, pointsOfTriangle[1]);
	triangle.setPoint(1, pointsOfTriangle[0]);
	triangle.setPoint(2, pointsOfTriangle[2]);
	triangle.setFillColor(triangleColor);
	triangle.setOutlineThickness(thickness);
	triangle.setOutlineColor(triangleColor);
	std::vector<sf::CircleShape> pointsInTriangle;
	sf::Vector2f currentPos = gen_first_point_in_triangle(pointsOfTriangle, pointsInTriangle);
	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
		}
		window.clear(screenColor);
		window.draw(triangle);
		window.draw(pointsInTriangle[0]);
		draw_all_points(window, pointsInTriangle, pointsOfTriangle, currentPos);
		window.display();
	}
}

void get_points(sf::CircleShape& figure, std::vector<sf::Vector2f>& points)
{
	for (size_t i = 0; i < figure.getPointCount();i++)
		points.push_back(figure.getPoint(i));
}

sf::Vector2f& gen_first_point_in_triangle(std::vector<sf::Vector2f>& pOt, std::vector<sf::CircleShape>& pointsInTriangle)
{

	float maxY = std::max_element(pOt.begin(), pOt.end(), [](sf::Vector2f& left, sf::Vector2f& right)->bool {return left.y < right.y;})->y;
	float minY = std::min_element(pOt.begin(), pOt.end(), [](sf::Vector2f& left, sf::Vector2f& right)->bool {return left.y < right.y;})->y;
	float maxX = std::max_element(pOt.begin(), pOt.end(), [](sf::Vector2f& left, sf::Vector2f& right)->bool {return left.x < right.x;})->x;
	float minX = std::min_element(pOt.begin(), pOt.end(), [](sf::Vector2f& left, sf::Vector2f& right)->bool {return left.x < right.x;})->x;
	float h1 = abs(maxY - minY);
	float l1 = abs(maxX - minX);
	std::uniform_int_distribution<int> disY(minY + thickness, maxY - thickness);
	int randY = disY(gen);
	float h2 = abs(minY - randY);
	float l2 = (h2 / h1) * l1;
	maxX = widthScreen / 2 + l2 / 2;
	minX = widthScreen / 2 - l2 / 2;
	std::uniform_int_distribution<int> disX(minX + 1, maxX - 1);
	while (1)
	{
		int randX = disX(gen);
		if (randX < maxX - thickness || randX > minX + thickness)
		{
			sf::CircleShape firstPoint(radiusOfPoint);
			firstPoint.setFillColor(pointColor);
			sf::Vector2f currentPos(randX, randY);
			firstPoint.setPosition(currentPos);
			pointsInTriangle.push_back(firstPoint);
			return currentPos;
		}
	}
}