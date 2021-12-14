#include "RepeatedMovement.h"
#include <iostream>
#include "LocationInSpace.h"

RepeatedMovement::RepeatedMovementPosition::RepeatedMovementPosition(float x, float y, bool pauseHere) : sf::Vector2f(x, y)
{
	this->pauseHere = pauseHere;
}

void RepeatedMovement::RepeatedMovementPosition::setPauseHere(bool pauseHere)
{
	this->pauseHere = pauseHere;
}

bool RepeatedMovement::RepeatedMovementPosition::getPauseHere()
{
	return pauseHere;
}

RepeatedMovement::RepeatedMovement(int id, std::map<int, Property*>* propertyMap, Timeline* timeline) : Property(id, propertyMap)
{
	this->timeline = timeline;
}

void RepeatedMovement::addObject(int objectId, int locationInSpaceId, std::vector<RepeatedMovement::RepeatedMovementPosition> movementPattern, float pauseDuration, float velocity)
{
	if (hasObject(objectId))
	{
		std::cerr << "Cannot add duplicate object to RepeatedMovement" << std::endl;
		return;
	}

	//add object and given values to appropriate lists
	objects.push_back(objectId);
	locationsInSpace.insert(std::pair<int, int>(objectId, locationInSpaceId));
	objectMovementPatterns.insert(std::pair<int, std::vector<RepeatedMovement::RepeatedMovementPosition>>(objectId, movementPattern));
	pauseDurations.insert(std::pair<int, float>(objectId, pauseDuration));
	velocities.insert(std::pair<int, float>(objectId, velocity));

	//set starting values for rest
	timesAtLastMovement.insert(std::pair<int, float>(objectId, 0.f));
	paused.insert(std::pair<int, bool>(objectId, false));
	timesSincePause.insert(std::pair<int, float>(objectId, 0.f));
	targetIndices.insert(std::pair<int, int>(objectId, 1));
	lastXChanges.insert(std::pair<int, float>(objectId, 0.f));
	lastYChanges.insert(std::pair<int, float>(objectId, 0.f));
}

void RepeatedMovement::removeObject(int id)
{
	if (!hasObject(id))
	{
		std::cerr << "No such object defined in RepeatedMovement";
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

	//remove from maps
	locationsInSpace.erase(id);
	objectMovementPatterns.erase(id);
	pauseDurations.erase(id);
	velocities.erase(id);
	timesAtLastMovement.erase(id);
	paused.erase(id);
	timesSincePause.erase(id);
	targetIndices.erase(id);
	lastXChanges.erase(id);
	lastYChanges.erase(id);
}

float RepeatedMovement::getLastXChange(int id)
{
	if (!hasObject(id))
	{
		std::cerr << "No such object defined in RepeatedMovement";
		return 0.f;
	}

	return lastXChanges.at(id);
}

float RepeatedMovement::getLastYChange(int id)
{
	if (!hasObject(id))
	{
		std::cerr << "No such object defined in RepeatedMovement";
		return 0.f;
	}

	return lastYChanges.at(id);
}

void RepeatedMovement::updatePosition(int id)
{
	if (!hasObject(id))
	{
		std::cerr << "No such object defined in RepeatedMovement";
		return;
	}

	//determine time between last movement and next
	float currentTime = timeline->getTime();
	float timeForMovement = currentTime - (timesAtLastMovement.at(id) / timeline->getTic());
	timesAtLastMovement[id] = currentTime * timeline->getTic();

	//if paused, check to see if enough time has passed to unpause
	if (paused.at(id))
	{
		timesSincePause[id] += timeForMovement;

		if (fabs(timesSincePause.at(id) - pauseDurations.at(id)) < FLT_EPSILON || timesSincePause.at(id) > pauseDurations.at(id))
		{
			timesSincePause[id] = 0.f;
			paused[id] = false;
		}

		return;
	}

	float unitsToMove = velocities.at(id) * timeForMovement;
	LocationInSpace* locationInSpace = (LocationInSpace*) propertyMap->at(locationsInSpace.at(id));
	sf::Shape* objectShape = locationInSpace->getObjectShape(id);
	std::vector<RepeatedMovement::RepeatedMovementPosition> positionList = objectMovementPatterns.at(id);
	int targetIndex = targetIndices.at(id);

	//move toward current target position horizontally
	if (!(fabs(objectShape->getPosition().x - positionList[targetIndex].x) < FLT_EPSILON))
	{
		float xUnitsToMove = unitsToMove;

		//if we are moving to left, change should be negative value
		if (objectShape->getPosition().x > positionList[targetIndex].x)
		{
			//if amount to move would take us past target position, make adjustment
			if (objectShape->getPosition().x - unitsToMove < positionList[targetIndex].x)
			{
				xUnitsToMove = positionList[targetIndex].x - objectShape->getPosition().x;
			}
			//otherwise, just make value negative
			else
			{
				xUnitsToMove = 0.f - unitsToMove;
			}
		}
		//if we are moving to right and amount to move would take us past target position, make adjustment
		else if (objectShape->getPosition().x + unitsToMove > positionList[targetIndex].x)
		{
			xUnitsToMove = positionList[targetIndex].x - objectShape->getPosition().x;
		}

		lastXChanges[id] = xUnitsToMove;
		objectShape->setPosition(objectShape->getPosition().x + getLastXChange(id), objectShape->getPosition().y);
	}
	else
	{
		lastXChanges[id] = 0.f;
	}

	//move toward current target position vertically
	if (!(fabs(objectShape->getPosition().y - positionList[targetIndex].y) < FLT_EPSILON))
	{
		float yUnitsToMove = unitsToMove;

		//if we are moving up, change should be negative value
		if (objectShape->getPosition().y > positionList[targetIndex].y)
		{
			//if amount to move would take us past target position, make adjustment
			if (objectShape->getPosition().y - unitsToMove < positionList[targetIndex].y)
			{
				yUnitsToMove = positionList[targetIndex].y - objectShape->getPosition().y;
			}
			//otherwise, just make value negative
			else
			{
				yUnitsToMove = 0.f - unitsToMove;
			}
		}
		//if we are moving down and amount to move would take us past target position, make adjustment
		else if (objectShape->getPosition().y + unitsToMove > positionList[targetIndex].y)
		{
			yUnitsToMove = positionList[targetIndex].y - objectShape->getPosition().y;
		}

		lastYChanges[id] = yUnitsToMove;
		objectShape->setPosition(objectShape->getPosition().x, objectShape->getPosition().y + getLastYChange(id));
	}
	else
	{
		lastYChanges[id] = 0.f;
	}

	//if we are at target position, update target to next position and potentially initiate pause
	if ((fabs(objectShape->getPosition().x - positionList[targetIndex].x) < FLT_EPSILON) &&
		(fabs(objectShape->getPosition().y - positionList[targetIndex].y) < FLT_EPSILON))
	{
		paused[id] = positionList[targetIndex].getPauseHere();

		if (targetIndex == positionList.size() - 1)
		{
			targetIndices[id] = 0;
		}
		else
		{
			targetIndices[id]++;
		}
	}
}

void RepeatedMovement::updatePositionOfAll()
{
	//update position of each object
	int objectsNum = objects.size();
	for (int i = 0; i < objectsNum; i++)
	{
		updatePosition(objects[i]);
	}
}