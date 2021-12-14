#include "Collision.h"
#include <iostream>
#include "LocationInSpace.h"

Collision::Collision(int id, std::map<int, Property*>* propertyMap) : Property(id, propertyMap)
{
	//construction taken care of in Property
}

void Collision::addObject(int objectId, int locationInSpaceId)
{
	if (hasObject(objectId))
	{
		std::cerr << "Cannot add duplicate object to Collision" << std::endl;
		return;
	}

	objects.push_back(objectId);
	locationsInSpace.insert(std::pair<int, int>(objectId, locationInSpaceId));
}

void Collision::removeObject(int objectId)
{
	//remove from objects list
	int objectsNum = objects.size();
	for (int i = 0; i < objectsNum; i++)
	{
		if (objects[i] == objectId)
		{
			objects.erase(objects.begin() + i);
		}
	}

	//remove LocationInSpace
	locationsInSpace.erase(objectId);
}

std::map<int, int>* Collision::getLocationsInSpace()
{
	return &locationsInSpace;
}

std::vector<int> Collision::getObjects()
{
	return objects;
}

bool Collision::isCollidingWithAny(int objectId, int collision)
{
	if (!hasObject(objectId))
	{
		std::cerr << "No such object defined in Collision" << std::endl;
		return false;
	}

	LocationInSpace* objectLocationInSpace = (LocationInSpace*)propertyMap->at(locationsInSpace.at(objectId));
	sf::Shape* objectShape = objectLocationInSpace->getObjectShape(objectId);
	Collision* collisionProperty = (Collision*)propertyMap->at(collision);
	std::map<int, int>* otherLocationsInSpace = collisionProperty->getLocationsInSpace();
	std::vector<int> otherObjects = collisionProperty->getObjects();

	//go through each object in other collision property, returning true if a collision is detected
	int otherObjectsNum = otherObjects.size();
	for (int i = 0; i < otherObjectsNum; i++)
	{
		int otherLocationInSpaceId = otherLocationsInSpace->at(otherObjects[i]);
		LocationInSpace* otherLocationInSpace = (LocationInSpace*)propertyMap->at(otherLocationInSpaceId);
		sf::Shape* otherObjectShape = otherLocationInSpace->getObjectShape(otherObjects[i]);

		if (objectShape->getGlobalBounds().intersects(otherObjectShape->getGlobalBounds()))
		{
			return true;
		}
	}

	//return false if no collisions detected
	return false;
}

bool Collision::isCollidingWith(int objectId, int otherObjectId, int collision)
{
	if (!hasObject(objectId))
	{
		std::cerr << "No such object defined in Collision" << std::endl;
		return false;
	}

	LocationInSpace* objectLocationInSpace = (LocationInSpace*)propertyMap->at(locationsInSpace.at(objectId));
	sf::Shape* objectShape = objectLocationInSpace->getObjectShape(objectId);
	Collision* collisionProperty = (Collision*)propertyMap->at(collision);
	LocationInSpace* otherLocationInSpace = (LocationInSpace*)propertyMap->at(collisionProperty->getLocationsInSpace()->at(otherObjectId));
	sf::Shape* otherObjectShape = otherLocationInSpace->getObjectShape(otherObjectId);

	if (objectShape->getGlobalBounds().intersects(otherObjectShape->getGlobalBounds()))
	{
		return true;
	}

	//return false if no collision detected
	return false;
}


