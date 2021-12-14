#include "Gravity.h"
#include <iostream>
#include "LocationInSpace.h"
#include "Collision.h"
Gravity::Gravity(int id, std::map<int, Property*>* propertyMap, Timeline* timeline) : Property(id, propertyMap)
{
	this->timeline = timeline;
}

void Gravity::addObject(int objectId, int locationInSpaceId, int collisionId, 
	std::vector<int> collisionsToCheck, int objectStandingOn, float velocity)
{
	if (hasObject(objectId))
	{
		std::cerr << "Cannot add duplicate to Gravity" << std::endl;
		return;
	}

	//insert given values
	objects.push_back(objectId);
	locationsInSpace.insert(std::pair<int, int>(objectId, locationInSpaceId));
	collisions.insert(std::pair<int, int>(objectId, collisionId));
	this->collisionsToCheck.insert(std::pair<int, std::vector<int>>(objectId, collisionsToCheck));
	objectsStoodOn.insert(std::pair<int, int>(objectId, objectStandingOn));
	velocities.insert(std::pair<int, float>(objectId, velocity));

	//insert default values for rest
	jumpingUpValues.insert(std::pair<int, bool>(objectId, false));
	fallingDownValues.insert(std::pair<int, bool>(objectId, false));
	standingOnObjectsValues.insert(std::pair<int, bool>(objectId, true));
}

void Gravity::removeObject(int objectId)
{
	if (!hasObject(objectId))
	{
		std::cerr << "No such object defined in Gravity";
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

	//remove from maps
	locationsInSpace.erase(objectId);
	collisions.erase(objectId);
	jumpingUpValues.erase(objectId);
	fallingDownValues.erase(objectId);
	standingOnObjectsValues.erase(objectId);
	collisionsToCheck.erase(objectId);
	objectsStoodOn.erase(objectId);
	velocities.erase(objectId);
}

bool Gravity::isJumpingUp(int objectId)
{
	if (!hasObject(objectId))
	{
		std::cerr << "No such object defined in Gravity";
		return false;
	}

	return jumpingUpValues.at(objectId);
}

void Gravity::setJumpingUp(int objectId, bool jumpingUp)
{
	if (!hasObject(objectId))
	{
		std::cerr << "No such object defined in Gravity";
		return;
	}

	jumpingUpValues[objectId] = jumpingUp;
}

bool Gravity::getFallingDown(int objectId)
{
	if (!hasObject(objectId))
	{
		std::cerr << "No such object defined in Gravity";
		return false;
	}

	return fallingDownValues.at(objectId);
}

bool Gravity::isStanding(int objectId)
{
	if (!hasObject(objectId))
	{
		std::cerr << "No such object defined in Gravity";
		return false;
	}

	return standingOnObjectsValues.at(objectId);
}

int Gravity::objectStandingOn(int objectId)
{
	if (!hasObject(objectId))
	{
		std::cerr << "No such object defined in Gravity";
		return -1;
	}

	return objectsStoodOn[objectId];
}

void Gravity::processGravity(int objectId)
{
	if (!hasObject(objectId))
	{
		std::cerr << "No such object defined in Gravity";
		return;
	}

	//don't try to make any movements if timeline is paused
	if (timeline->isPaused())
	{
		return;
	}

	//if object isn't jumping, we need to check if it is falling
	if (!jumpingUpValues.at(objectId))
	{
		//get object's shape
		LocationInSpace* locationInSpace = (LocationInSpace*)propertyMap->at(locationsInSpace.at(objectId));
		sf::Shape* objectShape = locationInSpace->getObjectShape(objectId);
		Collision* objectCollision = (Collision*)propertyMap->at(collisions.at(objectId));

		//move object downward
		objectShape->setPosition(objectShape->getPosition().x, objectShape->getPosition().y + 1.f);

		bool falling = true;

		//check to see if object is colliding, and if so if that collision is downward
		std::vector<int> collisionsBeingChecked = collisionsToCheck.at(objectId);
		int collisionsNum = collisionsBeingChecked.size();
		for (int i = 0; i < collisionsNum; i++)
		{
			Collision* collisionChecking = (Collision*)propertyMap->at(collisionsBeingChecked[i]);
			std::vector<int> currentObjects = collisionChecking->getObjects();
			int objectsNum = currentObjects.size();

			//for each object in each collision property, check to see if we are colliding and if so if our heights align
			for (int j = 0; j < objectsNum; j++)
			{
				if (objectCollision->isCollidingWith(objectId, currentObjects[j], collisionsBeingChecked[i]))
				{
					falling = false;
					standingOnObjectsValues[objectId] = true;
					objectsStoodOn[objectId] = currentObjects[j];
					fallingDownValues[objectId] = false;
					break;
				}
			}
		}

		//if we're falling, make sure values are appropriate and process time spent on movement
		if (falling)
		{
			standingOnObjectsValues[objectId] = false;
			fallingDownValues[objectId] = true;

			bool enoughTimePassed = false;
			float initialTime = timeline->getTime();

			while (!enoughTimePassed)
			{
				float currentTime = timeline->getTime();

				if (fabs((currentTime - initialTime) - velocities.at(objectId)) < FLT_EPSILON
					|| (currentTime - initialTime) > velocities.at(objectId))
				{
					enoughTimePassed = true;
				}
			}
		}
		//if we're not falling, retract movement and set values
		else
		{
			objectShape->setPosition(objectShape->getPosition().x, objectShape->getPosition().y - 1.f);
		}
	}
}

void Gravity::processGravityForAll()
{
	//process gravity for each object
	int objectsNum = objects.size();
	for (int i = 0; i < objectsNum; i++)
	{
		processGravity(objects[i]);
	}
}