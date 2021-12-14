#include "CharacterDeathHandler.h"

CharacterDeathHandler::CharacterDeathHandler(std::map<int, Property*>* propertyMap, bool notifyWhileReplaying, int deathZoneCollisionId)
	: EventHandler(propertyMap, notifyWhileReplaying)
{
	this->deathZoneCollisionId = deathZoneCollisionId;
}

void CharacterDeathHandler::onEvent(Event* e)
{
	//if a collision has occurred, check if it is with death zone
	if (e->getType() == ClientServerConsts::CHARACTER_COLLISION_EVENT)
	{
		//get death zone collision
		Collision* deathZoneCollision = (Collision*)propertyMap->at(deathZoneCollisionId);

		//get character id and id of colliding object
		int characterId = e->getArgument(0).argValue.argAsInt;
		int collidingObjectId = e->getArgument(1).argValue.argAsInt;

		//if colliding object is death zone, raise character death event
		if (deathZoneCollision->hasObject(collidingObjectId))
		{
			struct Event::ArgumentVariant charIdArg = { Event::ArgumentType::TYPE_INTEGER, characterId };
			Event* characterDeathEvent = new Event(EventManager::getManager()->getCurrentTime(),
				ClientServerConsts::CHARACTER_DEATH_EVENT,
				{ charIdArg });
			EventManager::getManager()->raise(characterDeathEvent);
		}
	}
}
