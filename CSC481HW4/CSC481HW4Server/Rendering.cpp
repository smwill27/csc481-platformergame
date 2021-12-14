#include "Rendering.h"
#include <iostream>
#include "LocationInSpace.h"

Rendering::Rendering(int id, std::map<int, Property*>* propertyMap, sf::RenderWindow* window) : Property(id, propertyMap)
{
	this->window = window;
}

void Rendering::addObject(int objectId, int locationInSpaceId)
{
	if (hasObject(objectId))
	{
		std::cerr << "Cannot add duplicate object to Rendering" << std::endl;
		return;
	}

	objects.push_back(objectId);
	locationsInSpace.insert(std::pair<int, int>(objectId, locationInSpaceId));
}

void Rendering::removeObject(int id)
{
	if (!hasObject(id))
	{
		std::cerr << "No such object defined in Rendering" << std::endl;
		return;
	}

	//remove from objects list
	int objectsNum = objects.size();
	for (int i = 0; i < objectsNum; i++)
	{
		if (objects[i] == id)
		{
			objects.erase(objects.begin() + i);
		}
	}

	//remove from map of locations in space
	locationsInSpace.erase(id);
}

void Rendering::drawObject(int objectId)
{
	//make sure object is defined in this property
	if (!hasObject(objectId))
	{
		std::cerr << "Unable to draw object: No object with this id defined in Rendering" << std::endl;
		return;
	}

	LocationInSpace* locationInSpace;

	//get location in space for object
	try
	{
		locationInSpace = (LocationInSpace*) propertyMap->at(locationsInSpace.at(objectId));
	}
	catch (std::out_of_range)
	{
		std::cerr << "Unable to draw object: invalid LocationInSpace" << std::endl;
		return;
	}

	//retrieve object's shape
	sf::Shape* objectShape = (sf::Shape*) locationInSpace->getObjectShape(objectId);

	//draw object to window
	window->draw(*objectShape);
}

void Rendering::drawAllObjects()
{
	//draw all objects
	int objectsNum = objects.size();
	for (int i = 0; i < objectsNum; i++)
	{
		drawObject(objects[i]);
	}
}


