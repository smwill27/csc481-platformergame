#include "PlatformMovingCharacterHandler.h"

PlatformMovingCharacterHandler::PlatformMovingCharacterHandler(std::map<int, Property*>* propertyMap, 
	bool notifyWhileReplaying,
	std::vector<int> platformCollisionIds, int characterCollisionId, int characterGravityId, int characterLocationInSpaceId,
	std::vector<int> possibleObjectCollisionIds)
	: EventHandler(propertyMap, notifyWhileReplaying)
{
	this->platformCollisionIds = platformCollisionIds;
	this->characterCollisionId = characterCollisionId;
	this->characterGravityId = characterGravityId;
	this->characterLocationInSpaceId = characterLocationInSpaceId;
	this->possibleObjectCollisionIds = possibleObjectCollisionIds;
}

void PlatformMovingCharacterHandler::onEvent(Event* e)
{
	//if platform has moved, check whether we need to move any characters
	if (e->getType() == ClientServerConsts::PLATFORM_MOVED_EVENT)
	{
		//get platform id and the amounts moved in x and y directions
		int platformId = e->getArgument(0).argValue.argAsInt;
		float xMoved = e->getArgument(2).argValue.argAsFloat;
		float yMoved = e->getArgument(3).argValue.argAsFloat;

		//get character collision, gravity, and location in space
		Collision* characterCollision = (Collision*)propertyMap->at(characterCollisionId);
		Gravity* characterGravity = (Gravity*)propertyMap->at(characterGravityId);
		LocationInSpace* characterLocationInSpace = (LocationInSpace*)propertyMap->at(characterLocationInSpaceId);

		//find Collision property pertaining to platform that moved
		Collision* platformCollision = nullptr;
		int platformCollisionsNum = platformCollisionIds.size();
		for (int i = 0; i < platformCollisionsNum; i++)
		{
			platformCollision = (Collision*)propertyMap->at(platformCollisionIds[i]);
			if (platformCollision->hasObject(platformId))
			{
				break;
			}
		}

		//check through each character to see if we need to move them
		std::vector<int> charactersToCheck = characterCollision->getObjects();
		int charactersToCheckNum = charactersToCheck.size();
		for (int i = 0; i < charactersToCheckNum; i++)
		{
			//check if character was standing on platform or is now colliding with it
			if ((characterGravity->isStanding(charactersToCheck[i])
				&& characterGravity->objectStandingOn(charactersToCheck[i]) == platformId)
				|| platformCollision->isCollidingWith(platformId, charactersToCheck[i], characterCollisionId))
			{
				//move character
				sf::Shape* charShape = characterLocationInSpace->getObjectShape(charactersToCheck[i]);
				charShape->move(xMoved, yMoved);

				//if collision occurs, retract movement
				bool characterMoved = true;
				int possibleObjectCollisionIdsNum = possibleObjectCollisionIds.size();
				for (int j = 0; j < possibleObjectCollisionIdsNum; j++)
				{
					if (characterCollision->isCollidingWithAny(charactersToCheck[i], possibleObjectCollisionIds[j]))
					{
						charShape->setPosition(charShape->getPosition().x - xMoved, charShape->getPosition().y - yMoved);
						characterMoved = false;
						break;
					}
				}

				//if no collision, raise event indicating character movement
				if (characterMoved)
				{
					struct Event::ArgumentVariant charIdArg = { Event::ArgumentType::TYPE_INTEGER, charactersToCheck[i] };
					struct Event::ArgumentVariant locationInSpaceIdArg = { Event::ArgumentType::TYPE_INTEGER, characterLocationInSpaceId };
					struct Event::ArgumentVariant xMovedArg;
					xMovedArg.argType = Event::ArgumentType::TYPE_FLOAT;
					xMovedArg.argValue.argAsFloat = xMoved;
					struct Event::ArgumentVariant yMovedArg;
					yMovedArg.argType = Event::ArgumentType::TYPE_FLOAT;
					yMovedArg.argValue.argAsFloat = yMoved;
					struct Event::ArgumentVariant absoluteXArg;
					absoluteXArg.argType = Event::ArgumentType::TYPE_FLOAT;
					absoluteXArg.argValue.argAsFloat = charShape->getPosition().x;
					struct Event::ArgumentVariant absoluteYArg;
					absoluteYArg.argType = Event::ArgumentType::TYPE_FLOAT;
					absoluteYArg.argValue.argAsFloat = charShape->getPosition().y;
					Event* characterMovedByPlatformEvent = new Event(EventManager::getManager()->getCurrentTime(),
						ClientServerConsts::CHARACTER_MOVED_BY_PLATFORM_EVENT,
						{ charIdArg, locationInSpaceIdArg, xMovedArg, yMovedArg, absoluteXArg, absoluteYArg });
					EventManager::getManager()->raise(characterMovedByPlatformEvent);
				}
			}
		}
	}
}
