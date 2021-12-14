#include "LocationInSpace.h"
#include <iostream>

LocationInSpace::LocationInSpace(int id, std::map<int, Property*>* propertyMap) : Property(id, propertyMap)
{
	//construction taken care of in Property
}

void LocationInSpace::addObject(int id, sf::Shape* shape)
{
	if (hasObject(id))
	{
		std::cerr << "Cannot add duplicate object to LocationInSpace" << std::endl;
		return;
	}

	objects.push_back(id);
	objectShapes.insert(std::pair<int, sf::Shape*>(id, shape));
}

void LocationInSpace::removeObject(int id)
{
	if (!hasObject(id))
	{
		std::cerr << "No such object defined in LocationInSpace" << std::endl;
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

	//remove from shapes map
	objectShapes.erase(id);
}

sf::Shape* LocationInSpace::getObjectShape(int id)
{
	sf::Shape* shapeToReturn;

	try
	{
		shapeToReturn = objectShapes.at(id);
	}
	catch (std::out_of_range)
	{
		return nullptr;
	}

	return shapeToReturn;
}

std::vector<int> LocationInSpace::getObjects()
{
	return objects;
}
