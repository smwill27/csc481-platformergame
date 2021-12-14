#include "PlayerDirectedMovement.h"
#include <iostream>
#include "LocationInSpace.h"
#include "Collision.h"
//#pragma warning disable 26812

PlayerDirectedMovement::MovementOnInput::MovementOnInput() : sf::Vector2f(0.f, 0.f)
{
	this->key = sf::Keyboard::Space;
	this->isJump = true;
}

PlayerDirectedMovement::MovementOnInput::MovementOnInput(float x, float y, sf::Keyboard::Key key, bool isJump) : sf::Vector2f(x, y)
{
	this->key = key;
	this->isJump = isJump;
}

void PlayerDirectedMovement::MovementOnInput::setKey(sf::Keyboard::Key key)
{
	this->key = key;
}

sf::Keyboard::Key PlayerDirectedMovement::MovementOnInput::getKey()
{
	return key;
}

void PlayerDirectedMovement::MovementOnInput::setIsJump(bool isJump)
{
	this->isJump = isJump;
}

bool PlayerDirectedMovement::MovementOnInput::getIsJump()
{
	return isJump;
}

PlayerDirectedMovement::PlayerDirectedMovement(int id, std::map<int, Property*>* propertyMap, Timeline* timeline)
	: Property(id, propertyMap)
{
	this->timeline = timeline;
}

void PlayerDirectedMovement::addObject(int objectId, int locationInSpaceId, int collisionId, std::vector <PlayerDirectedMovement::MovementOnInput > movements, float velocity, bool collisionStatus, std::vector<int> checkedCollisions)
{
	if (hasObject(objectId))
	{
		std::cerr << "Cannot add duplicate in PlayerDirectedMovement" << std::endl;
		return;
	}

	//insert given values
	objects.push_back(objectId);
	locationsInSpace.insert(std::pair<int, int>(objectId, locationInSpaceId));
	collisions.insert(std::pair<int, int>(objectId, collisionId));
	movementsOnInput.insert(std::pair<int, std::vector<PlayerDirectedMovement::MovementOnInput>>(objectId, movements));
	velocities.insert(std::pair<int, float>(objectId, velocity));
	collisionStatuses.insert(std::pair<int, bool>(objectId, collisionStatus));
	collisionsToCheck.insert(std::pair<int, std::vector<int>>(objectId, checkedCollisions));

	//set initial values for rest
	jumpingUpValues.insert(std::pair<int, bool>(objectId, false));
	fallingDownValues.insert(std::pair<int, bool>(objectId, false));
	jumpsBeingPerformed.insert(std::pair<int, MovementOnInput>(objectId, MovementOnInput(0.f, 0.f, sf::Keyboard::Space, true)));
	amountsYOfJumpPerformed.insert(std::pair<int, float>(objectId, 0.f));
	amountsXOfJumpPerformed.insert(std::pair<int, float>(objectId, 0.f));
}

void PlayerDirectedMovement::removeObject(int objectId)
{
	if (!hasObject(objectId))
	{
		std::cerr << "No such object defined in PlayerDirectedMovement";
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
	movementsOnInput.erase(objectId);
	jumpingUpValues.erase(objectId);
	fallingDownValues.erase(objectId);
	velocities.erase(objectId);
	locationsInSpace.erase(objectId);
	collisions.erase(objectId);
	collisionStatuses.erase(objectId);
	collisionsToCheck.erase(objectId);
	jumpsBeingPerformed.erase(objectId);
	amountsYOfJumpPerformed.erase(objectId);
	amountsXOfJumpPerformed.erase(objectId);
}

bool PlayerDirectedMovement::isJumpingUp(int objectId)
{
	if (!hasObject(objectId))
	{
		std::cerr << "No such object defined in PlayerDirectedMovement";
		return false;
	}

	return jumpingUpValues.at(objectId);
}

bool PlayerDirectedMovement::isFallingDown(int objectId)
{
	if (!hasObject(objectId))
	{
		std::cerr << "No such object defined in PlayerDirectedMovement";
		return false;
	}

	return fallingDownValues.at(objectId);
}

void PlayerDirectedMovement::setFallingDown(int objectId, bool fallingDown)
{
	if (!hasObject(objectId))
	{
		std::cerr << "No such object defined in PlayerDirectedMovement";
		return;
	}

	fallingDownValues[objectId] = fallingDown;
}

void PlayerDirectedMovement::processMovement(int objectId, sf::Keyboard::Key input)
{
	if (!hasObject(objectId))
	{
		std::cerr << "No such object defined in PlayerDirectedMovement";
		return;
	}

	//don't try to make any movements if timeline is paused
	if (timeline->isPaused())
	{
		return;
	}

	//get object's shape
	LocationInSpace* locationInSpace = (LocationInSpace*)propertyMap->at(locationsInSpace.at(objectId));
	sf::Shape* objectShape = locationInSpace->getObjectShape(objectId);
	Collision* objectCollision = (Collision*)propertyMap->at(collisions.at(objectId));

	bool movementFound = false;
	MovementOnInput currentMovement(0.f, 0.f, sf::Keyboard::Space, true);
	std::vector<MovementOnInput> objectMovements = movementsOnInput.at(objectId);
	int objectMovementsNum = objectMovements.size();
	for (int i = 0; i < objectMovementsNum; i++)
	{
		if (objectMovements[i].getKey() == input)
		{
			movementFound = true;
			currentMovement = objectMovements[i];
			break;
		}
	}

	if (!movementFound)
	{
		std::cerr << "No such movement defined for object in PlayerDirectedMovement" << std::endl;
		return;
	}

	bool moved = false;

	//if movement is jump, try to initiate jump
	if (currentMovement.getIsJump())
	{
		//only initiate jump if object is not currently jumping up or falling down
		if (!jumpingUpValues.at(objectId) && !fallingDownValues.at(objectId))
		{
			jumpingUpValues[objectId] = true;
			jumpsBeingPerformed[objectId] = currentMovement;
			amountsXOfJumpPerformed[objectId] = 0.f;
			amountsYOfJumpPerformed[objectId] = 0.f;
		}
	}
	//otherwise, try to make ordinary movement
	else
	{
		//adjust object's position to make movement
		objectShape->setPosition(objectShape->getPosition().x + currentMovement.x, objectShape->getPosition().y + currentMovement.y);

		bool canMove = true;

		//if we need to check for collision before finalizing movement, do so
		if (collisionStatuses.at(objectId))
		{
			std::vector<int> collisionsBeingChecked = collisionsToCheck.at(objectId);
			int collisionsNum = collisionsBeingChecked.size();
			for (int i = 0; i < collisionsNum; i++)
			{
				if (objectCollision->isCollidingWithAny(objectId, collisionsBeingChecked[i]))
				{
					canMove = false;
					break;
				}
			}
		}

		//if movement led to collision, reverse it
		if (!canMove)
		{
			objectShape->setPosition(objectShape->getPosition().x - currentMovement.x, objectShape->getPosition().y - currentMovement.y);
		}
		else
		{
			moved = true;
		}
	}

	//if object is jumping up, progress jump if possible and check to see if jump is completed
	if (jumpingUpValues.at(objectId))
	{
		MovementOnInput jump = jumpsBeingPerformed.at(objectId);
		float jumpAmountY = 0.f;
		float jumpAmountX = 0.f;
		bool yToMoveLeft = true;
		bool xToMoveLeft = true;
		bool jumpPerformed = true;

		//check to see if we still need to move more in x direction
		if (fabs(jump.x - amountsXOfJumpPerformed.at(objectId)) < FLT_EPSILON || fabs(jump.x) < fabs(amountsXOfJumpPerformed.at(objectId)))
		{
			xToMoveLeft = false;
		}

		//check to see if we still need to move more in y direction
		if (fabs(jump.y - amountsYOfJumpPerformed.at(objectId)) < FLT_EPSILON || fabs(jump.y) < fabs(amountsYOfJumpPerformed.at(objectId)))
		{
			yToMoveLeft = false;
		}

		//determine amount to move in x direction
		if (xToMoveLeft)
		{
			if (jump.x > 0.f)
			{
				jumpAmountX = 1.f;
			}
			else
			{
				jumpAmountX = -1.f;
			}
		}

		//determing amount to move in y direction
		if (yToMoveLeft)
		{
			if (jump.y > 0.f)
			{
				jumpAmountY = 1.f;
			}
			else
			{
				jumpAmountY = -1.f;
			}
		}

		if (xToMoveLeft || yToMoveLeft)
		{
			//adjust object's position to make movement
			objectShape->setPosition(objectShape->getPosition().x + jumpAmountX, objectShape->getPosition().y + jumpAmountY);

			bool canMove = true;

			//if we need to check for collision before finalizing movement, do so
			if (collisionStatuses.at(objectId))
			{
				std::vector<int> collisionsBeingChecked = collisionsToCheck.at(objectId);
				int collisionsNum = collisionsBeingChecked.size();
				for (int i = 0; i < collisionsNum; i++)
				{
					if (objectCollision->isCollidingWithAny(objectId, collisionsBeingChecked[i]))
					{
						canMove = false;
						break;
					}
				}
			}

			//if movement led to collision, reverse it
			if (!canMove)
			{
				objectShape->setPosition(objectShape->getPosition().x - jumpAmountX, objectShape->getPosition().y - jumpAmountY);
				jumpPerformed = false;
			}
			else
			{
				moved = true;
				amountsXOfJumpPerformed[objectId] += jumpAmountX;
				amountsYOfJumpPerformed[objectId] += jumpAmountY;
			}
		}
		else
		{
			jumpPerformed = false;
		}

		//if we didn't progress jump, we must be done jumping (collided or finished jump)
		if (!jumpPerformed)
		{
			jumpingUpValues[objectId] = false;
			amountsXOfJumpPerformed[objectId] = 0.f;
			amountsYOfJumpPerformed[objectId] = 0.f;
		}
	}

	//if object was moved, loop for designated time to simulate spending designated time on movement
	if (moved)
	{
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
}

void PlayerDirectedMovement::processMovementForAll(sf::Keyboard::Key input)
{
	//process movement for each object
	int objectsNum = objects.size();
	for (int i = 0; i < objectsNum; i++)
	{
		processMovement(objects[i], input);
	}
}

void PlayerDirectedMovement::processMovement(int objectId)
{
	if (!hasObject(objectId))
	{
		std::cerr << "No such object defined in PlayerDirectedMovement";
		return;
	}

	//don't try to make any movements if timeline is paused
	if (timeline->isPaused())
	{
		return;
	}

	//get object's shape
	LocationInSpace* locationInSpace = (LocationInSpace*)propertyMap->at(locationsInSpace.at(objectId));
	sf::Shape* objectShape = locationInSpace->getObjectShape(objectId);
	Collision* objectCollision = (Collision*)propertyMap->at(collisions.at(objectId));

	//if object is jumping up, progress jump if possible and check to see if jump is completed
	if (jumpingUpValues.at(objectId))
	{
		MovementOnInput jump = jumpsBeingPerformed.at(objectId);
		float jumpAmountY = 0.f;
		float jumpAmountX = 0.f;
		bool yToMoveLeft = true;
		bool xToMoveLeft = true;
		bool jumpPerformed = true;

		//check to see if we still need to move more in x direction
		if (fabs(jump.x - amountsXOfJumpPerformed.at(objectId)) < FLT_EPSILON || fabs(jump.x) < fabs(amountsXOfJumpPerformed.at(objectId)))
		{
			xToMoveLeft = false;
		}

		//check to see if we still need to move more in y direction
		if (fabs(jump.y - amountsYOfJumpPerformed.at(objectId)) < FLT_EPSILON || fabs(jump.y) < fabs(amountsYOfJumpPerformed.at(objectId)))
		{
			yToMoveLeft = false;
		}

		//determine amount to move in x direction
		if (xToMoveLeft)
		{
			if (jump.x > 0.f)
			{
				jumpAmountX = 1.f;
			}
			else
			{
				jumpAmountX = -1.f;
			}
		}

		//determing amount to move in y direction
		if (yToMoveLeft)
		{
			if (jump.y > 0.f)
			{
				jumpAmountY = 1.f;
			}
			else
			{
				jumpAmountY = -1.f;
			}
		}

		if (xToMoveLeft || yToMoveLeft)
		{
			//adjust object's position to make movement
			objectShape->setPosition(objectShape->getPosition().x + jumpAmountX, objectShape->getPosition().y + jumpAmountY);

			bool canMove = true;

			//if we need to check for collision before finalizing movement, do so
			if (collisionStatuses.at(objectId))
			{
				std::vector<int> collisionsBeingChecked = collisionsToCheck.at(objectId);
				int collisionsNum = collisionsBeingChecked.size();
				for (int i = 0; i < collisionsNum; i++)
				{
					if (objectCollision->isCollidingWithAny(objectId, collisionsBeingChecked[i]))
					{
						canMove = false;
						break;
					}
				}
			}

			//if movement led to collision, reverse it
			if (!canMove)
			{
				objectShape->setPosition(objectShape->getPosition().x - jumpAmountX, objectShape->getPosition().y - jumpAmountY);
				jumpPerformed = false;
			}
			else
			{
				amountsXOfJumpPerformed[objectId] += jumpAmountX;
				amountsYOfJumpPerformed[objectId] += jumpAmountY;
			}
		}
		else
		{
			jumpPerformed = false;
		}

		//if we didn't progress jump, we must be done jumping (collided or finished jump)
		if (!jumpPerformed)
		{
			jumpingUpValues[objectId] = false;
			amountsXOfJumpPerformed[objectId] = 0.f;
			amountsYOfJumpPerformed[objectId] = 0.f;
		}
	}
}

void PlayerDirectedMovement::processMovementForAll()
{
	int objectsNum = objects.size();
	for (int i = 0; i < objectsNum; i++)
	{
		processMovement(objects[i]);
	}
}