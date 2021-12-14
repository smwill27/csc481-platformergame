#include "GravityHandler.h"

GravityHandler::GravityHandler(std::map<int, Property*>* propertyMap, bool notifyWhileReplaying,
	int gravityId, int locationInSpaceId)
	: EventHandler(propertyMap, notifyWhileReplaying)
{
	this->gravityId = gravityId;
	this->locationInSpaceId = locationInSpaceId;
}

void GravityHandler::onEvent(Event* e)
{
	//if character has moved, is currently falling, or has respawned, process gravity for them
	if (e->getType() == ClientServerConsts::CHARACTER_MOVED_EVENT || e->getType() == ClientServerConsts::CHARACTER_STILL_FALLING_EVENT
		|| e->getType() == ClientServerConsts::CHARACTER_SPAWN_EVENT 
		|| e->getType() == ClientServerConsts::CHARACTER_MOVED_BY_PLATFORM_EVENT)
	{
		//get id of character that moved
		int characterId = e->getArgument(0).argValue.argAsInt;

		//get Gravity and LocationInSpace
		Gravity* gravity = (Gravity*)propertyMap->at(gravityId);
		LocationInSpace* locationInSpace = (LocationInSpace*)propertyMap->at(locationInSpaceId);

		//make sure character still exists (could have disconnected)
		if (gravity->hasObject(characterId) && locationInSpace->hasObject(characterId))
		{

			//get whether character is currently falling
			bool wasFalling = gravity->getFallingDown(characterId);

				//get initial positions of character
				sf::Shape* charShape = locationInSpace->getObjectShape(characterId);
				float initialX = charShape->getPosition().x;
				float initialY = charShape->getPosition().y;

				//process gravity for character
				gravity->processGravity(characterId);

			//record new positions
			float newX = charShape->getPosition().x;
			float newY = charShape->getPosition().y;

			//raise gravity-specific movement event so we don't catch our own event
			struct Event::ArgumentVariant objectIdArg = { Event::ArgumentType::TYPE_INTEGER, characterId };
			struct Event::ArgumentVariant locationInSpaceIdArg = { Event::ArgumentType::TYPE_INTEGER, locationInSpaceId };
			struct Event::ArgumentVariant xArg;
			xArg.argType = Event::ArgumentType::TYPE_FLOAT;
			xArg.argValue.argAsFloat = newX - initialX;
			struct Event::ArgumentVariant yArg;
			yArg.argType = Event::ArgumentType::TYPE_FLOAT;
			yArg.argValue.argAsFloat = newY - initialY;
			struct Event::ArgumentVariant absoluteXArg;
			absoluteXArg.argType = Event::ArgumentType::TYPE_FLOAT;
			absoluteXArg.argValue.argAsFloat = newX;
			struct Event::ArgumentVariant absoluteYArg;
			absoluteYArg.argType = Event::ArgumentType::TYPE_FLOAT;
			absoluteYArg.argValue.argAsFloat = newY;
			Event* characterMovementEvent = new Event(EventManager::getManager()->getCurrentTime(),
				ClientServerConsts::CHARACTER_MOVED_BY_GRAVITY_EVENT,
				{ objectIdArg, locationInSpaceIdArg, xArg, yArg, absoluteXArg, absoluteYArg });
			EventManager::getManager()->raise(characterMovementEvent);

			//see whether character is falling after processing gravity
			bool isFalling = gravity->getFallingDown(characterId);

			//if character is falling, check to see whether we just started falling and raise appropriate events
			if (isFalling)
			{
				if (!wasFalling)
				{
					struct Event::ArgumentVariant characterIdArg = { Event::ArgumentType::TYPE_INTEGER, characterId };
					Event* startedFallingEvent = new Event(EventManager::getManager()->getCurrentTime(),
						ClientServerConsts::CHARACTER_FALL_START_EVENT, { characterIdArg });
					EventManager::getManager()->raise(startedFallingEvent);
				}

				struct Event::ArgumentVariant characterIdArg = { Event::ArgumentType::TYPE_INTEGER, characterId };
				Event* stillFallingEvent = new Event(EventManager::getManager()->getCurrentTime() + 100.f,
					ClientServerConsts::CHARACTER_STILL_FALLING_EVENT, { characterIdArg });
				EventManager::getManager()->raise(stillFallingEvent);
			}
			//if character has stopped falling, raise appropriate event
			else if (wasFalling && !isFalling)
			{
				struct Event::ArgumentVariant characterIdArg = { Event::ArgumentType::TYPE_INTEGER, characterId };
				Event* stoppedFallingEvent = new Event(EventManager::getManager()->getCurrentTime(),
					ClientServerConsts::CHARACTER_FALL_END_EVENT, { characterIdArg });
				EventManager::getManager()->raise(stoppedFallingEvent);
			}
		}
	}
	//if character has started jumping, set that value in gravity
	else if (e->getType() == ClientServerConsts::CHARACTER_JUMP_START_EVENT)
	{
		int characterId = e->getArgument(0).argValue.argAsInt;
		Gravity* gravity = (Gravity*)propertyMap->at(gravityId);

		//make sure character still exists (could have disconnected)
		if (gravity->hasObject(characterId))
		{
			gravity->setJumpingUp(characterId, true);
		}
	}
	//if character has stopped jumping, set that value in gravity
	else if (e->getType() == ClientServerConsts::CHARACTER_JUMP_END_EVENT)
	{
		int characterId = e->getArgument(0).argValue.argAsInt;
		Gravity* gravity = (Gravity*)propertyMap->at(gravityId);

		//make sure character still exists (could have disconnected)
		if (gravity->hasObject(characterId))
		{
			gravity->setJumpingUp(characterId, false);
		}
	}
}
