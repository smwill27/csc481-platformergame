#include "Respawning.h"
#include <iostream>
#include "LocationInSpace.h"

Respawning::SpawnPoint::SpawnPoint()
{
	this->objectId = -1;
	this->locationInSpaceId = -1;
}

Respawning::SpawnPoint::SpawnPoint(int objectId, int locationInSpaceId)
{
	this->objectId = objectId;
	this->locationInSpaceId = locationInSpaceId;
}

int Respawning::SpawnPoint::getObjectId()
{
	return objectId;
}

int Respawning::SpawnPoint::getLocationInSpaceId()
{
	return locationInSpaceId;
}

Respawning::Respawning(int id, std::map<int, Property*>* propertyMap) : Property(id, propertyMap)
{
	//construction taken care of in Property
}

void Respawning::addObject(int objectId, int locationInSpaceId, SpawnPoint spawnPoint)
{
	if (hasObject(objectId))
	{
		std::cerr << "Cannot add duplicate object to Respawning" << std::endl;
		return;
	}

	objects.push_back(objectId);
	locationsInSpace.insert(std::pair<int, int>(objectId, locationInSpaceId));
	spawnPoints.insert(std::pair<int, SpawnPoint>(objectId, spawnPoint));
}

void Respawning::removeObject(int objectId)
{
	if (!hasObject(objectId))
	{
		std::cerr << "No such object defined in Respawning" << std::endl;
		return;
	}

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

	//remove spawn point
	spawnPoints.erase(objectId);
}

void Respawning::setSpawnPoint(int objectId, SpawnPoint spawnPoint)
{
	if (!hasObject(objectId))
	{
		std::cerr << "No such object defined in Respawning" << std::endl;
		return;
	}

	spawnPoints[objectId] = spawnPoint;
}

void Respawning::respawn(int objectId)
{
	if (!hasObject(objectId))
	{
		std::cerr << "No such object defined in Respawning" << std::endl;
		return;
	}

	//set object's position to that of spawn point
	SpawnPoint currentSpawnPoint = spawnPoints.at(objectId);
	LocationInSpace* spawnLocationInSpace = (LocationInSpace*) propertyMap->at(currentSpawnPoint.getLocationInSpaceId());
	LocationInSpace* objectLocationInSpace = (LocationInSpace*)propertyMap->at(locationsInSpace.at(objectId));
	sf::Shape* spawnShape = spawnLocationInSpace->getObjectShape(currentSpawnPoint.getObjectId());
	sf::Shape* objectShape = objectLocationInSpace->getObjectShape(objectId);
	objectShape->setPosition(spawnShape->getPosition());
}

void Respawning::respawnAll()
{
	//respawn each object
	int objectsNum = objects.size();
	for (int i = 0; i < objectsNum; i++)
	{
		respawn(objects[i]);
	}
}
