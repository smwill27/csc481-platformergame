#include "CharacterCollisionHandler.h"

CharacterCollisionHandler::CharacterCollisionHandler(std::map<int, Property*>* propertyMap, bool notifyWhileReplaying,
	int collisionId, std::vector<int> collisionsToCheck)
	: EventHandler(propertyMap, notifyWhileReplaying)
{
	this->collisionId = collisionId;
	this->collisionsToCheck = collisionsToCheck;
}

void CharacterCollisionHandler::onEvent(Event* e)
{
	//if character moved, check to see if there is a collision to report
	if (e->getType() == ClientServerConsts::CHARACTER_MOVED_EVENT || e->getType() == ClientServerConsts::CHARACTER_MOVED_BY_GRAVITY_EVENT
		|| e->getType() == ClientServerConsts::CHARACTER_MOVED_BY_PLATFORM_EVENT)
	{
		//get id of character that moved
		int characterId = e->getArgument(0).argValue.argAsInt;

		//get character collision
		Collision* charCollision = (Collision*)propertyMap->at(collisionId);

		//make sure character still exists (client could have disconnected)
		if (charCollision->hasObject(characterId))
		{
			int collisionsToCheckNum = collisionsToCheck.size();

			//check for collisions with each object in each collision to be checked
			for (int i = 0; i < collisionsToCheckNum; i++)
			{
				Collision* collisionBeingChecked = (Collision*)propertyMap->at(collisionsToCheck[i]);
				std::vector<int> collisionObjects = collisionBeingChecked->getObjects();
				int collisionObjectsNum = collisionObjects.size();

				for (int j = 0; j < collisionObjectsNum; j++)
				{
					//if a collision is found, raise an appropriate event
					if (charCollision->isCollidingWith(characterId, collisionObjects[j], collisionsToCheck[i]))
					{
						struct Event::ArgumentVariant charIdArg = { Event::ArgumentType::TYPE_INTEGER, characterId };
						struct Event::ArgumentVariant objectIdArg = { Event::ArgumentType::TYPE_INTEGER, collisionObjects[j] };
						Event* characterCollisionEvent = new Event(EventManager::getManager()->getCurrentTime(),
							ClientServerConsts::CHARACTER_COLLISION_EVENT,
							{ charIdArg, objectIdArg });
						EventManager::getManager()->raise(characterCollisionEvent);
					}
				}
			}
		}
	}
}
