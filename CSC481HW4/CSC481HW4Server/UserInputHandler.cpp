#include "UserInputHandler.h"

UserInputHandler::UserInputHandler(std::map<int, Property*>* propertyMap, bool notifyWhileReplaying,
	int playerDirectedMovementId, int locationInSpaceId) 
	: EventHandler(propertyMap, notifyWhileReplaying)
{
	this->playerDirectedMovementId = playerDirectedMovementId;
	this->locationInSpaceId = locationInSpaceId;
}

void UserInputHandler::onEvent(Event* e)
{
	//if there is user input for movement or character is continuing to jump, process the movement
	if (e->getType() == ClientServerConsts::USER_INPUT_EVENT || e->getType() == ClientServerConsts::CHARACTER_STILL_JUMPING_EVENT)
	{
		//get id of character corresponding to client
		int characterId = e->getArgument(0).argValue.argAsInt;

		//get PlayerDirectedMovement and LocationInSpace
		PlayerDirectedMovement* playerDirectedMovement = (PlayerDirectedMovement*) propertyMap->at(playerDirectedMovementId);
		LocationInSpace* locationInSpace = (LocationInSpace*)propertyMap->at(locationInSpaceId);

		//make sure character still exists (could have disconnected)
		if (playerDirectedMovement->hasObject(characterId) && locationInSpace->hasObject(characterId))
		{
			//get whether character is currently jumping
			bool wasJumping = playerDirectedMovement->isJumpingUp(characterId);

			//get initial positions of character
			sf::Shape* charShape = locationInSpace->getObjectShape(characterId);
			float initialX = charShape->getPosition().x;
			float initialY = charShape->getPosition().y;

			//if there is movement from user input, get the key and process the movement
			//or if key says to start or stop recording a replay, send appropriate event
			if (e->getType() == ClientServerConsts::USER_INPUT_EVENT)
			{
				int keyType = e->getArgument(1).argValue.argAsInt;

				if (keyType == ClientServerConsts::LEFT_ARROW_KEY)
				{
					playerDirectedMovement->processMovement(characterId, sf::Keyboard::Left);
				}
				else if (keyType == ClientServerConsts::RIGHT_ARROW_KEY)
				{
					playerDirectedMovement->processMovement(characterId, sf::Keyboard::Right);
				}
				else if (keyType == ClientServerConsts::UP_ARROW_KEY)
				{
					playerDirectedMovement->processMovement(characterId, sf::Keyboard::Up);
				}
				else if (keyType == ClientServerConsts::R_KEY)
				{
					Event* replayRecordingStartEvent = new Event(EventManager::getManager()->getCurrentTime(),
						ClientServerConsts::REPLAY_RECORDING_START_EVENT, {});
					EventManager::getManager()->raise(replayRecordingStartEvent);
					return;
				}
				else if (keyType == ClientServerConsts::ONE_KEY)
				{
					struct Event::ArgumentVariant replaySpeedArg;
					replaySpeedArg.argType = Event::ArgumentType::TYPE_FLOAT;
					replaySpeedArg.argValue.argAsFloat = 2.f;
					Event* replayRecordingStopEvent = new Event(EventManager::getManager()->getCurrentTime(),
						ClientServerConsts::REPLAY_RECORDING_STOP_EVENT, { replaySpeedArg });
					EventManager::getManager()->raise(replayRecordingStopEvent);
					return;
				}
				else if (keyType == ClientServerConsts::TWO_KEY)
				{
					struct Event::ArgumentVariant replaySpeedArg;
					replaySpeedArg.argType = Event::ArgumentType::TYPE_FLOAT;
					replaySpeedArg.argValue.argAsFloat = 1.f;
					Event* replayRecordingStopEvent = new Event(EventManager::getManager()->getCurrentTime(),
						ClientServerConsts::REPLAY_RECORDING_STOP_EVENT, { replaySpeedArg });
					EventManager::getManager()->raise(replayRecordingStopEvent);
					return;
				}
				else if (keyType == ClientServerConsts::THREE_KEY)
				{
					struct Event::ArgumentVariant replaySpeedArg;
					replaySpeedArg.argType = Event::ArgumentType::TYPE_FLOAT;
					replaySpeedArg.argValue.argAsFloat = 0.5f;
					Event* replayRecordingStopEvent = new Event(EventManager::getManager()->getCurrentTime(),
						ClientServerConsts::REPLAY_RECORDING_STOP_EVENT, { replaySpeedArg });
					EventManager::getManager()->raise(replayRecordingStopEvent);
					return;
				}
			}
			//if movement is character still jumping, process movement without a specific key
			else if (e->getType() == ClientServerConsts::CHARACTER_STILL_JUMPING_EVENT)
			{
				playerDirectedMovement->processMovement(characterId);
			}

			//record new positions
			float newX = charShape->getPosition().x;
			float newY = charShape->getPosition().y;

			//raise movement event
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
			Event* characterMovementEvent = new Event(EventManager::getManager()->getCurrentTime(), ClientServerConsts::CHARACTER_MOVED_EVENT,
				{ objectIdArg, locationInSpaceIdArg, xArg, yArg, absoluteXArg, absoluteYArg });
			EventManager::getManager()->raise(characterMovementEvent);

			//see whether character is jumping after processing movement
			bool isJumping = playerDirectedMovement->isJumpingUp(characterId);

			//if character is jumping, check to see whether we just started jumping and raise appropriate events
			if (isJumping)
			{
				if (!wasJumping)
				{
					struct Event::ArgumentVariant characterIdArg = { Event::ArgumentType::TYPE_INTEGER, characterId };
					Event* startedJumpingEvent = new Event(EventManager::getManager()->getCurrentTime(),
						ClientServerConsts::CHARACTER_JUMP_START_EVENT, { characterIdArg });
					EventManager::getManager()->raise(startedJumpingEvent);
				}

				struct Event::ArgumentVariant characterIdArg = { Event::ArgumentType::TYPE_INTEGER, characterId };
				Event* stillJumpingEvent = new Event(EventManager::getManager()->getCurrentTime() + 100.f,
					ClientServerConsts::CHARACTER_STILL_JUMPING_EVENT, { characterIdArg });
				EventManager::getManager()->raise(stillJumpingEvent);
			}
			//if character has stopped jumping, raise appropriate event
			else if (wasJumping && !isJumping)
			{
				struct Event::ArgumentVariant characterIdArg = { Event::ArgumentType::TYPE_INTEGER, characterId };
				Event* stoppedJumpingEvent = new Event(EventManager::getManager()->getCurrentTime(),
					ClientServerConsts::CHARACTER_JUMP_END_EVENT, { characterIdArg });
				EventManager::getManager()->raise(stoppedJumpingEvent);
			}
		}
	}
	//if character has started falling, set that value
	else if (e->getType() == ClientServerConsts::CHARACTER_FALL_START_EVENT)
	{
		int characterId = e->getArgument(0).argValue.argAsInt;
		PlayerDirectedMovement* playerDirectedMovement = (PlayerDirectedMovement*)propertyMap->at(playerDirectedMovementId);

		//make sure character still exists (could have disconnected)
		if (playerDirectedMovement->hasObject(characterId))
		{
			playerDirectedMovement->setFallingDown(characterId, true);
		}
	}
	//if character has stopped falling, set that value
	else if (e->getType() == ClientServerConsts::CHARACTER_FALL_END_EVENT)
	{
		int characterId = e->getArgument(0).argValue.argAsInt;
		PlayerDirectedMovement* playerDirectedMovement = (PlayerDirectedMovement*)propertyMap->at(playerDirectedMovementId);

		//make sure character still exists (could have disconnected)
		if (playerDirectedMovement->hasObject(characterId))
		{
			playerDirectedMovement->setFallingDown(characterId, false);
		}
	}
}
