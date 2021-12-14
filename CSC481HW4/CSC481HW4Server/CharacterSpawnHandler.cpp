#include "CharacterSpawnHandler.h"

CharacterSpawnHandler::CharacterSpawnHandler(std::map<int, Property*>* propertyMap, bool notifyWhileReplaying,
	int locationInSpaceId, int respawningId)
	: EventHandler(propertyMap, notifyWhileReplaying)
{
	this->locationInSpaceId = locationInSpaceId;
	this->respawningId = respawningId;
}

void CharacterSpawnHandler::onEvent(Event* e)
{
	//if character has died, respawn them
	if (e->getType() == ClientServerConsts::CHARACTER_DEATH_EVENT)
	{
		//get character id
		int characterId = e->getArgument(0).argValue.argAsInt;

		//get respawning and location in space
		Respawning* respawning = (Respawning*)propertyMap->at(respawningId);
		LocationInSpace* locationInSpace = (LocationInSpace*)propertyMap->at(locationInSpaceId);

		//make sure character still exists (client could have disconnected)
		if (respawning->hasObject(characterId) && locationInSpace->hasObject(characterId))
		{
			//respawn character
			respawning->respawn(characterId);

			//get character's position
			sf::Shape* charShape = locationInSpace->getObjectShape(characterId);
			float newX = charShape->getPosition().x;
			float newY = charShape->getPosition().y;

			//raise spawn event
			struct Event::ArgumentVariant charIdArg = { Event::ArgumentType::TYPE_INTEGER, characterId };
			struct Event::ArgumentVariant locationInSpaceIdArg = { Event::ArgumentType::TYPE_INTEGER, locationInSpaceId };
			struct Event::ArgumentVariant xArg;
			xArg.argType = Event::ArgumentType::TYPE_FLOAT;
			xArg.argValue.argAsFloat = newX;
			struct Event::ArgumentVariant yArg;
			yArg.argType = Event::ArgumentType::TYPE_FLOAT;
			yArg.argValue.argAsFloat = newY;
			Event* characterSpawnEvent = new Event(EventManager::getManager()->getCurrentTime(),
				ClientServerConsts::CHARACTER_SPAWN_EVENT,
				{ charIdArg, locationInSpaceIdArg, xArg, yArg });
			EventManager::getManager()->raise(characterSpawnEvent);
		}
	}
}
