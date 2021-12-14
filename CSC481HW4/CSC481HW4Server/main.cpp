#include <zmq.hpp>
#include <stdlib.h>
#include <string>
#include <stdio.h>
#include <iostream>
#include <SFML/Graphics.hpp>
#include <iostream>
#include "Timeline.h"
#include <thread>
#include <chrono>
#include "ClientServerConsts.h"
#include "Property.h"
#include "LocationInSpace.h"
#include "Collision.h"

#ifndef _WIN32
#include <unistd.h>
#else
#include <windows.h>

#define sleep(n)    Sleep(n)
#endif
#include "RepeatedMovement.h"
#include "RealTimeline.h"
#include "GameTimeline.h"
#include "ServerClientPositionCommunication.h"
#include <mutex>
#include "Event.h"
#include "EventHandler.h"
#include "EventManager.h"
#include "PositionalUpdateHandler.h"
#include "PlayerDirectedMovement.h"
#include "UserInputHandler.h"
#include "Gravity.h"
#include "GravityHandler.h"
#include "Respawning.h"
#include "CharacterCollisionHandler.h"
#include "CharacterDeathHandler.h"
#include "CharacterSpawnHandler.h"
#include "PlatformMovingCharacterHandler.h"
#include "ReplayHandler.h"

/* context for all sockets */
zmq::context_t* context;

/* socket used for receiving connection/disconnection requests from clients */
zmq::socket_t* connectDisconnectReqRepSocket;

/* socket used for sending connection/disconnection updates to all clients */
zmq::socket_t* connectDisconnectPubSubSocket;

/* socket used for sending movement updates to all clients */
zmq::socket_t* movementUpdatePubSubSocket;

/* socket used for sending object creation messages to connecting client */
zmq::socket_t* objectCreationReqRepSocket;

/* counter providing next GUID to be assigned */
int idCounter;

/* map with all properties */
std::map<int, Property*> propertyMap;

/*character properties*/
LocationInSpace* characterLocationsInSpace;
Collision* characterCollisions;
ServerClientPositionCommunication* characterServerClientPositionCommunications;
PlayerDirectedMovement* characterPlayerDirectedMovements;
Gravity* characterGravity;
Respawning* characterRespawning;

/*platform properties*/
LocationInSpace* firstScreenStaticPlatformLocationsInSpace;
LocationInSpace* firstScreenMovingPlatformLocationsInSpace;
Collision* firstScreenStaticPlatformCollisions;
Collision* firstScreenMovingPlatformCollisions;

/* spawn point properties */
LocationInSpace* spawnPointLocationsInSpace;

/* death zone properties */
LocationInSpace* deathZoneLocationsInSpace;
Collision* deathZoneCollisions;

/*ids of rendering properties for clients*/
int firstScreenStaticPlatformRendering;
int firstScreenMovingPlatformRendering;
int characterRendering;

/* manager for events */
EventManager* eventManager;

/* modifier for individual client event port numbers */
int eventPortModifier;

/* mutex controlling access to event queue */
std::mutex queueLock;

/* id of starting platform */
int platform1Id;

/* id of starting spawn point */
int spawnPoint1Id;

/*
* Returns the next id for assignment and then updates the counter.
* 
* returns: id to use for next assignment
*/
int getNextId()
{
	int nextId = idCounter;
	idCounter++;
	return nextId;
}

/*
* Preps a message to be sent to the client for creating a particular property or object.
* 
* objectOrPropertyType: type of property or object being created
* objectOrPropertyId: id of property or object being created
* locationInSpacePropertyId: if object, id of corresponding locationInSpace property (filler if property)
* renderingPropertyId: if object, id of corresponding rendering property (filler if property)
* color: if object, color to give it (filler if property)
* x: if object, x position (filler of property)
* y: if object, y position (filler if property)
* 
* returns: pointer to message to send to client
*/
zmq::message_t* prepCreationMessage(int objectOrPropertyType, int objectOrPropertyId, int locationInSpacePropertyId,
	int renderingPropertyId, int color, float x, float y)
{
	std::string replyString = std::to_string(objectOrPropertyType) + " " + std::to_string(objectOrPropertyId) + " "
		+ std::to_string(locationInSpacePropertyId) + " " + std::to_string(renderingPropertyId) + " " +
		std::to_string(color) + " " + std::to_string(x) + " " + std::to_string(y);
	zmq::message_t* msg = new zmq::message_t(replyString.length() + 1);
	const char* replyChars = replyString.c_str();
	memcpy(msg->data(), replyChars, replyString.length() + 1);

	return msg;
}

/*
* Should be run in a separate thread for each client to allow checking for client event requests in parallel.
*
* eventPortNum: port number to use in creating socket
*/
void handleClientEventRequests(int eventPortNum)
{
	//std::cout << "Client thread starting" << std::endl;

	//prepare and bind socket
	zmq::socket_t eventRaisingReqRepSocket(*context, ZMQ_REP);
	//std::cout << "Created thread socket" << std::endl;
	
	eventRaisingReqRepSocket.bind(ClientServerConsts::EVENT_RAISING_REQ_REP_SERVER_PORT_START + std::to_string(eventPortNum));
	//std::cout << "Bound thread socket" << std::endl;

	bool clientStillConnected = true;

	while (clientStillConnected)
	{
		//sleep for a bit
		sleep(5);

		//see if there's a request from client, and if so handle it
		try
		{
			std::lock_guard<std::mutex> lock(queueLock);
			zmq::message_t clientRequest;

			if (eventRaisingReqRepSocket.recv(clientRequest, zmq::recv_flags::dontwait))
			{
				//determine event type
				int eventType = -1;
				int scan = sscanf_s(clientRequest.to_string().c_str(), "%d", &eventType);

				if (eventType == ClientServerConsts::USER_INPUT_EVENT_CODE)
				{
					//retrieve arguments
					int characterId = -1;
					int keyType = -1;

					int nextScan = sscanf_s(clientRequest.to_string().c_str(), "%d %d %d", &eventType, &characterId, &keyType);

					//if we are playing replay, only raise user input event if it is one of the three speed change keys
					if (!EventManager::getManager()->isPlayingReplay()
						|| (EventManager::getManager()->isPlayingReplay() && (keyType == ClientServerConsts::ONE_KEY ||
							keyType == ClientServerConsts::TWO_KEY || keyType == ClientServerConsts::THREE_KEY)))
					{
						//create and raise event
						struct Event::ArgumentVariant characterIdArg = { Event::ArgumentType::TYPE_INTEGER, characterId };
						struct Event::ArgumentVariant keyTypeArg = { Event::ArgumentType::TYPE_INTEGER, keyType };
						float eventTime = 0.f;
						//if we are playing replay, make sure to set time so that event will be taken care of right away
						if (EventManager::getManager()->isPlayingReplay())
						{
							eventTime = 0.f;
						}
						else
						{
							eventTime = EventManager::getManager()->getCurrentTime();
						}

						Event* userInputEvent = new Event(eventTime,
							ClientServerConsts::USER_INPUT_EVENT,
							{ characterIdArg, keyTypeArg });
						EventManager::getManager()->raise(userInputEvent);
					}
				}
				else if (eventType == ClientServerConsts::CLIENT_DISCONNECT_EVENT_CODE)
				{
					clientStillConnected = false;
				}

				//send back response
				std::string replyString = "Event Received";
				zmq::message_t* msg = new zmq::message_t(replyString.length() + 1);
				const char* replyChars = replyString.c_str();
				memcpy(msg->data(), replyChars, replyString.length() + 1);
				eventRaisingReqRepSocket.send(*msg, zmq::send_flags::none);
			}
		}
		catch (...)
		{
			std::cerr << "Error while checking for/handling client event request" << std::endl;
		}
	}

	//std::cout << "Client thread ending" << std::endl;
}

/*
* Checks for any connection/disconnection requests and processes them. Returns true if a connection was established to allow
* adding a new thread to handle it.
*/
bool checkForConnectionUpdate()
{
	zmq::message_t connectDisconnectRequest;
	if (connectDisconnectReqRepSocket->recv(connectDisconnectRequest, zmq::recv_flags::dontwait))
	{
		//parse out request
		int connectionObjectId = 0;
		int scan = sscanf_s(connectDisconnectRequest.to_string().c_str(), "%d", &connectionObjectId);
		//std::cout << "Received from client: " + std::to_string(connectionObjectId) << std::endl;

		//if incorrect request, print error and send back error message
		if (scan != 1)
		{
			std::cerr << "Error in communication while processing connection request" << std::endl;
			std::string replyString = std::to_string(ClientServerConsts::CONNECT_ERROR_CODE) + " " + std::to_string(-1);
			zmq::message_t replyMsg(replyString.length() + 1);
			const char* replyChars = replyString.c_str();
			memcpy(replyMsg.data(), replyChars, replyString.length() + 1);
			connectDisconnectReqRepSocket->send(replyMsg, zmq::send_flags::none);
			return false;
		}
		//if client is connecting, add character for them, send back message with character id
		//and event port number, and publish connection update to all clients
		else if (connectionObjectId == ClientServerConsts::CONNECT_CODE)
		{
			//create character shape
			sf::CircleShape* characterShape = new sf::CircleShape(ClientServerConsts::CHARACTER_RADIUS, ClientServerConsts::CHARACTER_NUM_OF_POINTS);
			characterShape->setFillColor(sf::Color::Red);
			characterShape->setPosition(sf::Vector2f(50.f, 0.f));
			int charId = getNextId();

			//add character to properties
			characterLocationsInSpace->addObject(charId, characterShape);
			characterCollisions->addObject(charId, characterLocationsInSpace->getId());
			characterServerClientPositionCommunications->addObject(charId, characterLocationsInSpace->getId());
			characterPlayerDirectedMovements->addObject(charId, characterLocationsInSpace->getId(), characterCollisions->getId(),
				{ PlayerDirectedMovement::MovementOnInput(-1.f, 0.f, sf::Keyboard::Left, false),
				PlayerDirectedMovement::MovementOnInput(1.f, 0.f, sf::Keyboard::Right, false),
				PlayerDirectedMovement::MovementOnInput(0.f, -100.f, sf::Keyboard::Up, true) },
				ClientServerConsts::CHARACTER_VELOCITY, true, { firstScreenStaticPlatformCollisions->getId(),
				firstScreenMovingPlatformCollisions->getId() });
			characterGravity->addObject(charId, characterLocationsInSpace->getId(), characterCollisions->getId(),
				{ firstScreenStaticPlatformCollisions->getId(),
				firstScreenMovingPlatformCollisions->getId() },
				platform1Id, ClientServerConsts::CHARACTER_VELOCITY);
			characterRespawning->addObject(charId, characterLocationsInSpace->getId(),
				Respawning::SpawnPoint(spawnPoint1Id, spawnPointLocationsInSpace->getId()));

			//send message with character id and event port number
			int newEventPortNum = ClientServerConsts::EVENT_RAISING_REQ_REP_NUM_START + eventPortModifier;
			std::string replyString = std::to_string(charId) + " " + std::to_string(newEventPortNum);
			zmq::message_t replyMsg(replyString.length() + 1);
			const char* replyChars = replyString.c_str();
			memcpy(replyMsg.data(), replyChars, replyString.length() + 1);
			connectDisconnectReqRepSocket->send(replyMsg, zmq::send_flags::none);

			//send messages to client to create location in space properties
			objectCreationReqRepSocket->recv(connectDisconnectRequest, zmq::recv_flags::none);
			zmq::message_t* firstScreenStaticPlatformMsg = prepCreationMessage(ClientServerConsts::PROPERTY_LOCATION_IN_SPACE,
				firstScreenStaticPlatformLocationsInSpace->getId(), ClientServerConsts::CREATION_FILLER, 
				ClientServerConsts::CREATION_FILLER, ClientServerConsts::CREATION_FILLER,
				ClientServerConsts::CREATION_FILLER, ClientServerConsts::CREATION_FILLER);
			objectCreationReqRepSocket->send(*firstScreenStaticPlatformMsg, zmq::send_flags::none);
			objectCreationReqRepSocket->recv(connectDisconnectRequest, zmq::recv_flags::none);
			zmq::message_t* firstScreenMovingPlatformMsg = prepCreationMessage(ClientServerConsts::PROPERTY_LOCATION_IN_SPACE,
				firstScreenMovingPlatformLocationsInSpace->getId(), ClientServerConsts::CREATION_FILLER,
				ClientServerConsts::CREATION_FILLER, ClientServerConsts::CREATION_FILLER,
				ClientServerConsts::CREATION_FILLER, ClientServerConsts::CREATION_FILLER);
			objectCreationReqRepSocket->send(*firstScreenMovingPlatformMsg, zmq::send_flags::none);
			objectCreationReqRepSocket->recv(connectDisconnectRequest, zmq::recv_flags::none);
			zmq::message_t* existingCharsMsg = prepCreationMessage(ClientServerConsts::PROPERTY_LOCATION_IN_SPACE,
				characterLocationsInSpace->getId(), ClientServerConsts::CREATION_FILLER,
				ClientServerConsts::CREATION_FILLER, ClientServerConsts::CREATION_FILLER,
				ClientServerConsts::CREATION_FILLER, ClientServerConsts::CREATION_FILLER);
			objectCreationReqRepSocket->send(*existingCharsMsg, zmq::send_flags::none);

			//send messages to client to create rendering properties
			objectCreationReqRepSocket->recv(connectDisconnectRequest, zmq::recv_flags::none);
			zmq::message_t* firstScreenStaticPlatformRenderingMsg = prepCreationMessage(ClientServerConsts::PROPERTY_RENDERING,
				firstScreenStaticPlatformRendering, ClientServerConsts::CREATION_FILLER,
				ClientServerConsts::CREATION_FILLER, ClientServerConsts::CREATION_FILLER,
				ClientServerConsts::CREATION_FILLER, ClientServerConsts::CREATION_FILLER);
			objectCreationReqRepSocket->send(*firstScreenStaticPlatformRenderingMsg, zmq::send_flags::none);
			objectCreationReqRepSocket->recv(connectDisconnectRequest, zmq::recv_flags::none);
			zmq::message_t* firstScreenMovingPlatformRenderingMsg = prepCreationMessage(ClientServerConsts::PROPERTY_RENDERING,
				firstScreenMovingPlatformRendering, ClientServerConsts::CREATION_FILLER,
				ClientServerConsts::CREATION_FILLER, ClientServerConsts::CREATION_FILLER,
				ClientServerConsts::CREATION_FILLER, ClientServerConsts::CREATION_FILLER);
			objectCreationReqRepSocket->send(*firstScreenMovingPlatformRenderingMsg, zmq::send_flags::none);
			objectCreationReqRepSocket->recv(connectDisconnectRequest, zmq::recv_flags::none);
			zmq::message_t* existingCharsRenderingMsg = prepCreationMessage(ClientServerConsts::PROPERTY_RENDERING,
				characterRendering, ClientServerConsts::CREATION_FILLER,
				ClientServerConsts::CREATION_FILLER, ClientServerConsts::CREATION_FILLER,
				ClientServerConsts::CREATION_FILLER, ClientServerConsts::CREATION_FILLER);
			objectCreationReqRepSocket->send(*existingCharsRenderingMsg, zmq::send_flags::none);

			//send messages to client to create first screen static platforms
			std::vector<int> firstScreenStaticPlatforms = firstScreenStaticPlatformLocationsInSpace->getObjects();
			int firstScreenStaticPlatformsNum = firstScreenStaticPlatforms.size();
			for (int i = 0; i < firstScreenStaticPlatformsNum; i++)
			{
				objectCreationReqRepSocket->recv(connectDisconnectRequest, zmq::recv_flags::none);
				sf::Shape* staticPlatformShape = firstScreenStaticPlatformLocationsInSpace->getObjectShape(firstScreenStaticPlatforms[i]);
				zmq::message_t* staticPlatformCreationMsg = prepCreationMessage(ClientServerConsts::STATIC_PLATFORM,
					firstScreenStaticPlatforms[i], firstScreenStaticPlatformLocationsInSpace->getId(),
					firstScreenStaticPlatformRendering, staticPlatformShape->getFillColor().toInteger(),
					staticPlatformShape->getPosition().x, staticPlatformShape->getPosition().y);
				objectCreationReqRepSocket->send(*staticPlatformCreationMsg, zmq::send_flags::none);
			}

			//send messages to client to create first screen moving platforms
			std::vector<int> firstScreenMovingPlatforms = firstScreenMovingPlatformLocationsInSpace->getObjects();
			int firstScreenMovingPlatformsNum = firstScreenMovingPlatforms.size();
			for (int i = 0; i < firstScreenMovingPlatformsNum; i++)
			{
				objectCreationReqRepSocket->recv(connectDisconnectRequest, zmq::recv_flags::none);
				sf::Shape* movingPlatformShape = firstScreenMovingPlatformLocationsInSpace->getObjectShape(firstScreenMovingPlatforms[i]);
				zmq::message_t* movingPlatformCreationMsg = prepCreationMessage(ClientServerConsts::MOVING_PLATFORM,
					firstScreenMovingPlatforms[i], firstScreenMovingPlatformLocationsInSpace->getId(),
					firstScreenMovingPlatformRendering, movingPlatformShape->getFillColor().toInteger(),
					movingPlatformShape->getPosition().x, movingPlatformShape->getPosition().y);
				objectCreationReqRepSocket->send(*movingPlatformCreationMsg, zmq::send_flags::none);
			}

			//send messages to client to create existing characters
			std::vector<int> existingChars = characterLocationsInSpace->getObjects();
			int existingCharsNum = existingChars.size();
			for (int i = 0; i < existingCharsNum; i++)
			{
				objectCreationReqRepSocket->recv(connectDisconnectRequest, zmq::recv_flags::none);
				sf::Shape* charShape = characterLocationsInSpace->getObjectShape(existingChars[i]);
				zmq::message_t* charCreationMsg = prepCreationMessage(ClientServerConsts::CHARACTER,
					existingChars[i], characterLocationsInSpace->getId(),
					characterRendering, charShape->getFillColor().toInteger(),
					charShape->getPosition().x, charShape->getPosition().y);
				objectCreationReqRepSocket->send(*charCreationMsg, zmq::send_flags::none);
			}

			//send message to client indicating end of object/property creation
			objectCreationReqRepSocket->recv(connectDisconnectRequest, zmq::recv_flags::none);
			 zmq::message_t* endCreationMsg = prepCreationMessage(ClientServerConsts::CREATION_END, ClientServerConsts::CREATION_FILLER,
				ClientServerConsts::CREATION_FILLER, ClientServerConsts::CREATION_FILLER, ClientServerConsts::CREATION_FILLER,
				ClientServerConsts::CREATION_FILLER, ClientServerConsts::CREATION_FILLER);
			objectCreationReqRepSocket->send(*endCreationMsg, zmq::send_flags::none);

			//publish connection message to all clients so they can add new character
			std::string connectMsgString = std::to_string(ClientServerConsts::CONNECT_CODE) + " " + std::to_string(charId) + " " 
				+ std::to_string(characterLocationsInSpace->getId()) + " " + std::to_string(characterRendering)
				+ " " + std::to_string(characterShape->getPosition().x) + " " + std::to_string(characterShape->getPosition().y);
			zmq::message_t connectMsg(connectMsgString.length() + 1);
			const char* connectChars = connectMsgString.c_str();
			memcpy(connectMsg.data(), connectChars, connectMsgString.length() + 1);
			connectDisconnectPubSubSocket->send(connectMsg, zmq::send_flags::none);

			return true;
		}
		//if client is disconnecting, remove character from all properties in which it exists, inform thread to stop running,
		//send back message with success code, and publish disconnection update to all clients
		else
		{
			characterLocationsInSpace->removeObject(connectionObjectId);
			characterCollisions->removeObject(connectionObjectId);
			characterServerClientPositionCommunications->removeObject(connectionObjectId);
			characterPlayerDirectedMovements->removeObject(connectionObjectId);
			characterGravity->removeObject(connectionObjectId);
			characterRespawning->removeObject(connectionObjectId);

			//send message with disconnect success code
			std::string replyString = std::to_string(ClientServerConsts::DISCONNECT_SUCCESS_CODE);
			zmq::message_t replyMsg(replyString.length() + 1);
			const char* replyChars = replyString.c_str();
			memcpy(replyMsg.data(), replyChars, replyString.length() + 1);
			connectDisconnectReqRepSocket->send(replyMsg, zmq::send_flags::none);

			//publish disconnection update to all clients
			std::string updateString = std::to_string(ClientServerConsts::DISCONNECT_CODE) + " " + std::to_string(connectionObjectId)
				+ " " + std::to_string(characterLocationsInSpace->getId()) + " " + std::to_string(characterRendering)
				+ " " + std::to_string(ClientServerConsts::CONNECTION_FILLER) + " " 
				+ std::to_string(ClientServerConsts::CONNECTION_FILLER);
			zmq::message_t updateMsg(updateString.length() + 1);
			const char* updateChars = updateString.c_str();
			memcpy(updateMsg.data(), updateChars, updateString.length() + 1);
			connectDisconnectPubSubSocket->send(updateMsg, zmq::send_flags::none);

			return false;
		}
	}

	return false;
}

int main()
{

	/*TODO: PREPARE STATE*/
	//initialize GUID counter and port modifier
	idCounter = 0;
	eventPortModifier = 0;

	// Prepare our context and sockets
	context = new zmq::context_t(1);
	connectDisconnectReqRepSocket = new zmq::socket_t(*context, ZMQ_REP);
	connectDisconnectPubSubSocket = new zmq::socket_t(*context, ZMQ_PUB);
	movementUpdatePubSubSocket = new zmq::socket_t(*context, ZMQ_PUB);
	objectCreationReqRepSocket = new zmq::socket_t(*context, ZMQ_REP);

	//bind sockets
	connectDisconnectReqRepSocket->bind(ClientServerConsts::CONNECT_DISCONNECT_REQ_REP_SERVER_PORT);
	connectDisconnectPubSubSocket->bind(ClientServerConsts::CONNECT_DISCONNECT_PUB_SUB_SERVER_PORT);
	connectDisconnectPubSubSocket->set(zmq::sockopt::sndhwm, 10);
	movementUpdatePubSubSocket->bind(ClientServerConsts::MOVEMENT_UPDATE_PUB_SUB_SERVER_PORT);
	movementUpdatePubSubSocket->set(zmq::sockopt::sndhwm, 10);
	objectCreationReqRepSocket->bind(ClientServerConsts::OBJECT_CREATION_REQ_REP_SERVER_PORT);

	/* create static platform shapes for first "screen"*/
	//starting platform
	sf::RectangleShape* platform1Shape = new sf::RectangleShape(sf::Vector2f(ClientServerConsts::PLATFORM_WIDTH,
		ClientServerConsts::PLATFORM_HEIGHT));
	platform1Shape->setFillColor(sf::Color::Green);
	platform1Shape->setPosition(sf::Vector2f(50.f, 100.f));
	platform1Id = getNextId();
	//platform to right of starting one
	sf::RectangleShape* platform2Shape = new sf::RectangleShape(sf::Vector2f(ClientServerConsts::PLATFORM_WIDTH,
		ClientServerConsts::PLATFORM_HEIGHT));
	platform2Shape->setFillColor(sf::Color::Black);
	platform2Shape->setPosition(sf::Vector2f(platform1Shape->getPosition().x + ClientServerConsts::PLATFORM_WIDTH + 50.f,
		platform1Shape->getPosition().y));
	int platform2Id = getNextId();
	//platform left of first "elevator"
	sf::RectangleShape* platform3Shape = new sf::RectangleShape(sf::Vector2f(ClientServerConsts::PLATFORM_WIDTH,
		ClientServerConsts::PLATFORM_HEIGHT));
	platform3Shape->setFillColor(sf::Color::Yellow);
	platform3Shape->setPosition(sf::Vector2f(platform2Shape->getPosition().x + 50.f, platform2Shape->getPosition().y + 200.f));
	int platform3Id = getNextId();
	//platform at end of multidirectional platform route
	sf::RectangleShape* platform4Shape = new sf::RectangleShape(sf::Vector2f(ClientServerConsts::PLATFORM_WIDTH,
		ClientServerConsts::PLATFORM_HEIGHT));
	platform4Shape->setFillColor(sf::Color::Green);
	platform4Shape->setPosition(sf::Vector2f(platform1Shape->getPosition().x + 100.f, platform1Shape->getPosition().y + 400.f));
	int platform4Id = getNextId();
	//platform to left of transitional platform
	sf::RectangleShape* platform5Shape = new sf::RectangleShape(sf::Vector2f(ClientServerConsts::PLATFORM_WIDTH,
		ClientServerConsts::PLATFORM_HEIGHT));
	platform5Shape->setFillColor(sf::Color::Black);
	platform5Shape->setPosition(sf::Vector2f(platform4Shape->getPosition().x + ClientServerConsts::PLATFORM_WIDTH + 200.f,
		platform4Shape->getPosition().y));
	int platform5Id = getNextId();

	//create static platforms for first "screen" by defining their properties (LocationInSpace and Collision; no Rendering on server)
	firstScreenStaticPlatformLocationsInSpace = new LocationInSpace(getNextId(), &propertyMap);
	propertyMap.insert(std::pair<int, Property*>(firstScreenStaticPlatformLocationsInSpace->getId(), 
		firstScreenStaticPlatformLocationsInSpace));
	firstScreenStaticPlatformLocationsInSpace->addObject(platform1Id, platform1Shape);
	firstScreenStaticPlatformLocationsInSpace->addObject(platform2Id, platform2Shape);
	firstScreenStaticPlatformLocationsInSpace->addObject(platform3Id, platform3Shape);
	firstScreenStaticPlatformLocationsInSpace->addObject(platform4Id, platform4Shape);
	firstScreenStaticPlatformLocationsInSpace->addObject(platform5Id, platform5Shape);
	firstScreenStaticPlatformCollisions = new Collision(getNextId(), &propertyMap);
	propertyMap.insert(std::pair<int, Property*>(firstScreenStaticPlatformCollisions->getId(), firstScreenStaticPlatformCollisions));
	firstScreenStaticPlatformCollisions->addObject(platform1Id, firstScreenStaticPlatformLocationsInSpace->getId());
	firstScreenStaticPlatformCollisions->addObject(platform2Id, firstScreenStaticPlatformLocationsInSpace->getId());
	firstScreenStaticPlatformCollisions->addObject(platform3Id, firstScreenStaticPlatformLocationsInSpace->getId());
	firstScreenStaticPlatformCollisions->addObject(platform4Id, firstScreenStaticPlatformLocationsInSpace->getId());
	firstScreenStaticPlatformCollisions->addObject(platform5Id, firstScreenStaticPlatformLocationsInSpace->getId());
	//only need id of rendering to send to client
	firstScreenStaticPlatformRendering = getNextId();

	/*create moving platform shapes for first "screen"*/
	//first "elevator" platform
	sf::RectangleShape* movingPlatform1Shape = new sf::RectangleShape(sf::Vector2f(ClientServerConsts::PLATFORM_WIDTH,
		ClientServerConsts::PLATFORM_HEIGHT));
	movingPlatform1Shape->setFillColor(sf::Color::White);
	movingPlatform1Shape->setPosition(sf::Vector2f(platform2Shape->getPosition().x + ClientServerConsts::PLATFORM_WIDTH + 100.f,
		platform2Shape->getPosition().y));
	int movingPlatform1Id = getNextId();
	//multidirectional platform
	sf::RectangleShape* movingPlatform2Shape = new sf::RectangleShape(sf::Vector2f(ClientServerConsts::PLATFORM_WIDTH,
		ClientServerConsts::PLATFORM_HEIGHT));
	movingPlatform2Shape->setFillColor(sf::Color::White);
	movingPlatform2Shape->setPosition(sf::Vector2f(platform3Shape->getPosition().x - ClientServerConsts::PLATFORM_WIDTH - 50.f,
		platform3Shape->getPosition().y));
	int movingPlatform2Id = getNextId();
	//second "elevator" platform
	sf::RectangleShape* movingPlatform3Shape = new sf::RectangleShape(sf::Vector2f(ClientServerConsts::PLATFORM_WIDTH,
		ClientServerConsts::PLATFORM_HEIGHT));
	movingPlatform3Shape->setFillColor(sf::Color::White);
	movingPlatform3Shape->setPosition(sf::Vector2f(platform4Shape->getPosition().x + ClientServerConsts::PLATFORM_WIDTH + 50.f,
		platform4Shape->getPosition().y));
	int movingPlatform3Id = getNextId();
	//diagonally moving platform
	sf::RectangleShape* movingPlatform4Shape = new sf::RectangleShape(sf::Vector2f(ClientServerConsts::PLATFORM_WIDTH,
		ClientServerConsts::PLATFORM_HEIGHT));
	movingPlatform4Shape->setFillColor(sf::Color::White);
	movingPlatform4Shape->setPosition(sf::Vector2f(platform5Shape->getPosition().x + 100.f,
		platform5Shape->getPosition().y + -ClientServerConsts::PLATFORM_HEIGHT - 60.f));
	int movingPlatform4Id = getNextId();
	//transitional platform
	sf::RectangleShape* movingPlatform5Shape = new sf::RectangleShape(sf::Vector2f(ClientServerConsts::PLATFORM_WIDTH,
		ClientServerConsts::PLATFORM_HEIGHT));
	movingPlatform5Shape->setFillColor(sf::Color::White);
	movingPlatform5Shape->setPosition(sf::Vector2f(platform5Shape->getPosition().x + ClientServerConsts::PLATFORM_WIDTH + 100.f,
		platform5Shape->getPosition().y));
	int movingPlatform5Id = getNextId();

	/*create moving platforms for first "screen" by defining their properties (LocationInSpace, Collision, RepeatedMovement,
	* ServerClientPositionCommunication; no Rendering on server)
	*/
	firstScreenMovingPlatformLocationsInSpace = new LocationInSpace(getNextId(), &propertyMap);
	propertyMap.insert(std::pair<int, Property*>(firstScreenMovingPlatformLocationsInSpace->getId(), 
		firstScreenMovingPlatformLocationsInSpace));
	firstScreenMovingPlatformLocationsInSpace->addObject(movingPlatform1Id, movingPlatform1Shape);
	firstScreenMovingPlatformLocationsInSpace->addObject(movingPlatform2Id, movingPlatform2Shape);
	firstScreenMovingPlatformLocationsInSpace->addObject(movingPlatform3Id, movingPlatform3Shape);
	firstScreenMovingPlatformLocationsInSpace->addObject(movingPlatform4Id, movingPlatform4Shape);
	firstScreenMovingPlatformLocationsInSpace->addObject(movingPlatform5Id, movingPlatform5Shape);
	firstScreenMovingPlatformCollisions = new Collision(getNextId(), &propertyMap);
	propertyMap.insert(std::pair<int, Property*>(firstScreenMovingPlatformCollisions->getId(), firstScreenMovingPlatformCollisions));
	firstScreenMovingPlatformCollisions->addObject(movingPlatform1Id, firstScreenMovingPlatformLocationsInSpace->getId());
	firstScreenMovingPlatformCollisions->addObject(movingPlatform2Id, firstScreenMovingPlatformLocationsInSpace->getId());
	firstScreenMovingPlatformCollisions->addObject(movingPlatform3Id, firstScreenMovingPlatformLocationsInSpace->getId());
	firstScreenMovingPlatformCollisions->addObject(movingPlatform4Id, firstScreenMovingPlatformLocationsInSpace->getId());
	firstScreenMovingPlatformCollisions->addObject(movingPlatform5Id, firstScreenMovingPlatformLocationsInSpace->getId());
	//only need id of rendering to send to client
	firstScreenMovingPlatformRendering = getNextId();
	RealTimeline* msTimeline = new RealTimeline(0.001f);
	GameTimeline* movingPlatformTimeline = new GameTimeline(1.f, msTimeline);
	RepeatedMovement* firstScreenMovingPlatformRepeatedMovements = new RepeatedMovement(getNextId(), &propertyMap, movingPlatformTimeline);
	propertyMap.insert(std::pair<int, Property*>(firstScreenMovingPlatformRepeatedMovements->getId(), 
		firstScreenMovingPlatformRepeatedMovements));
	firstScreenMovingPlatformRepeatedMovements->addObject(movingPlatform1Id, firstScreenMovingPlatformLocationsInSpace->getId(),
		{ RepeatedMovement::RepeatedMovementPosition(movingPlatform1Shape->getPosition().x, movingPlatform1Shape->getPosition().y, true),
		RepeatedMovement::RepeatedMovementPosition(movingPlatform1Shape->getPosition().x, movingPlatform1Shape->getPosition().y + 100.f, true) },
		ClientServerConsts::PLATFORM_PAUSE_DURATION, ClientServerConsts::PLATFORM_VELOCITY);
	firstScreenMovingPlatformRepeatedMovements->addObject(movingPlatform2Id, firstScreenMovingPlatformLocationsInSpace->getId(),
		{ RepeatedMovement::RepeatedMovementPosition(movingPlatform2Shape->getPosition().x, movingPlatform2Shape->getPosition().y, true),
		RepeatedMovement::RepeatedMovementPosition(movingPlatform2Shape->getPosition().x - 100.f, movingPlatform2Shape->getPosition().y, false),
		RepeatedMovement::RepeatedMovementPosition(movingPlatform2Shape->getPosition().x - 100.f, movingPlatform2Shape->getPosition().y + 100.f, true) },
		ClientServerConsts::PLATFORM_PAUSE_DURATION, ClientServerConsts::PLATFORM_VELOCITY);
	firstScreenMovingPlatformRepeatedMovements->addObject(movingPlatform3Id, firstScreenMovingPlatformLocationsInSpace->getId(),
		{ RepeatedMovement::RepeatedMovementPosition(movingPlatform3Shape->getPosition().x, movingPlatform3Shape->getPosition().y, true),
		RepeatedMovement::RepeatedMovementPosition(movingPlatform3Shape->getPosition().x, movingPlatform3Shape->getPosition().y - 50.f, true) },
		ClientServerConsts::PLATFORM_PAUSE_DURATION, ClientServerConsts::PLATFORM_VELOCITY);
	firstScreenMovingPlatformRepeatedMovements->addObject(movingPlatform4Id, firstScreenMovingPlatformLocationsInSpace->getId(),
		{ RepeatedMovement::RepeatedMovementPosition(movingPlatform4Shape->getPosition().x, movingPlatform4Shape->getPosition().y, false),
		RepeatedMovement::RepeatedMovementPosition(movingPlatform4Shape->getPosition().x + 50.f, movingPlatform4Shape->getPosition().y - 50.f, true) },
		ClientServerConsts::PLATFORM_PAUSE_DURATION, ClientServerConsts::PLATFORM_VELOCITY);
	firstScreenMovingPlatformRepeatedMovements->addObject(movingPlatform5Id, firstScreenMovingPlatformLocationsInSpace->getId(),
		{ RepeatedMovement::RepeatedMovementPosition(movingPlatform5Shape->getPosition().x, movingPlatform5Shape->getPosition().y, true),
		RepeatedMovement::RepeatedMovementPosition(movingPlatform5Shape->getPosition().x + 200.f, movingPlatform5Shape->getPosition().y, true) },
		ClientServerConsts::PLATFORM_PAUSE_DURATION, ClientServerConsts::PLATFORM_VELOCITY);
	ServerClientPositionCommunication* firstScreenMovingPlatformServerClientPositionCommunications =
		new ServerClientPositionCommunication(getNextId(), &propertyMap);
	propertyMap.insert(std::pair<int, Property*>(firstScreenMovingPlatformServerClientPositionCommunications->getId(), 
		firstScreenMovingPlatformServerClientPositionCommunications));
	firstScreenMovingPlatformServerClientPositionCommunications->addObject(movingPlatform1Id,
		firstScreenMovingPlatformLocationsInSpace->getId());
	firstScreenMovingPlatformServerClientPositionCommunications->addObject(movingPlatform2Id,
		firstScreenMovingPlatformLocationsInSpace->getId());
	firstScreenMovingPlatformServerClientPositionCommunications->addObject(movingPlatform3Id,
		firstScreenMovingPlatformLocationsInSpace->getId());
	firstScreenMovingPlatformServerClientPositionCommunications->addObject(movingPlatform4Id,
		firstScreenMovingPlatformLocationsInSpace->getId());
	firstScreenMovingPlatformServerClientPositionCommunications->addObject(movingPlatform5Id,
		firstScreenMovingPlatformLocationsInSpace->getId());

	/*make spawn point for character*/
	spawnPointLocationsInSpace = new LocationInSpace(getNextId(), &propertyMap);
	propertyMap.insert(std::pair<int, Property*>(spawnPointLocationsInSpace->getId(), spawnPointLocationsInSpace));
	sf::CircleShape* spawnPoint1Shape = new sf::CircleShape(ClientServerConsts::CHARACTER_RADIUS, ClientServerConsts::CHARACTER_NUM_OF_POINTS);
	spawnPoint1Id = getNextId();
	spawnPoint1Shape->setPosition(50.f, 0.f);
	spawnPointLocationsInSpace->addObject(spawnPoint1Id, spawnPoint1Shape);

	/*make death zone*/
	deathZoneLocationsInSpace = new LocationInSpace(getNextId(), &propertyMap);
	propertyMap.insert(std::pair<int, Property*>(deathZoneLocationsInSpace->getId(), deathZoneLocationsInSpace));
	deathZoneCollisions = new Collision(getNextId(), &propertyMap);
	propertyMap.insert(std::pair<int, Property*>(deathZoneCollisions->getId(), deathZoneCollisions));
	sf::RectangleShape* deathZone1Shape = new sf::RectangleShape(sf::Vector2f(1600.f, 600.f));
	int deathZone1Id = getNextId();
	deathZone1Shape->setPosition(-400.f, 650.f);
	deathZoneLocationsInSpace->addObject(deathZone1Id, deathZone1Shape);
	deathZoneCollisions->addObject(deathZone1Id, deathZoneLocationsInSpace->getId());

	/* make character properties (LocationInSpace, Collision, Gravity, Respawning,
	ServerClientPositionCommunication, PlayerDirectedMovement; no Rendering on server)*/
	characterLocationsInSpace = new LocationInSpace(getNextId(), &propertyMap);
	propertyMap.insert(std::pair<int, Property*>(characterLocationsInSpace->getId(), characterLocationsInSpace));
	characterCollisions = new Collision(getNextId(), &propertyMap);
	propertyMap.insert(std::pair<int, Property*>(characterCollisions->getId(), characterCollisions));
	//only need id of rendering to send to client
	characterRendering = getNextId();
	characterServerClientPositionCommunications = new ServerClientPositionCommunication(getNextId(),
		&propertyMap);
	propertyMap.insert(std::pair<int, Property*>(characterServerClientPositionCommunications->getId(), 
		characterServerClientPositionCommunications));
	GameTimeline* characterTimeline = new GameTimeline(1.f, msTimeline);
	characterPlayerDirectedMovements = new PlayerDirectedMovement(getNextId(), &propertyMap, characterTimeline);
	propertyMap.insert(std::pair<int, Property*>(characterPlayerDirectedMovements->getId(), characterPlayerDirectedMovements));
	characterGravity = new Gravity(getNextId(), &propertyMap, characterTimeline);
	propertyMap.insert(std::pair<int, Property*>(characterGravity->getId(), characterGravity));
	characterRespawning = new Respawning(getNextId(), &propertyMap);
	propertyMap.insert(std::pair<int, Property*>(characterRespawning->getId(), characterRespawning));

	//prepare event manager
	eventManager = EventManager::getManager();
	RealTimeline* msReplayTimeline = new RealTimeline(0.001f);
	GameTimeline* replayTimeline = new GameTimeline(1.f, msReplayTimeline);
	eventManager->setTimeline(msTimeline);
	eventManager->setReplayTimeline(replayTimeline);
	eventManager->setEventTypes({ ClientServerConsts::PLATFORM_MOVED_EVENT, ClientServerConsts::CHARACTER_JUMP_START_EVENT,
		ClientServerConsts::CHARACTER_JUMP_END_EVENT, ClientServerConsts::CHARACTER_MOVED_EVENT,
		ClientServerConsts::CHARACTER_STILL_JUMPING_EVENT,
		ClientServerConsts::USER_INPUT_EVENT, ClientServerConsts::CHARACTER_MOVED_BY_GRAVITY_EVENT,
		ClientServerConsts::CHARACTER_FALL_START_EVENT, ClientServerConsts::CHARACTER_STILL_FALLING_EVENT,
		ClientServerConsts::CHARACTER_FALL_END_EVENT, ClientServerConsts::CHARACTER_COLLISION_EVENT,
		ClientServerConsts::CHARACTER_DEATH_EVENT, ClientServerConsts::CHARACTER_SPAWN_EVENT,
		ClientServerConsts::CHARACTER_MOVED_BY_PLATFORM_EVENT, ClientServerConsts::REPLAY_RECORDING_START_EVENT,
		ClientServerConsts::REPLAY_RECORDING_STOP_EVENT});

	//create handler for positional updates to client
	PositionalUpdateHandler* positionalUpdateHandler = new PositionalUpdateHandler(&propertyMap, true, movementUpdatePubSubSocket);
	eventManager->registerForEvent(ClientServerConsts::PLATFORM_MOVED_EVENT, positionalUpdateHandler);
	eventManager->registerForEvent(ClientServerConsts::CHARACTER_MOVED_EVENT, positionalUpdateHandler);
	eventManager->registerForEvent(ClientServerConsts::CHARACTER_MOVED_BY_GRAVITY_EVENT, positionalUpdateHandler);
	eventManager->registerForEvent(ClientServerConsts::CHARACTER_SPAWN_EVENT, positionalUpdateHandler);
	eventManager->registerForEvent(ClientServerConsts::CHARACTER_MOVED_BY_PLATFORM_EVENT, positionalUpdateHandler);

	//create handler for user inputs from client
	UserInputHandler* userInputHandler = new UserInputHandler(&propertyMap, false, characterPlayerDirectedMovements->getId(),
		characterLocationsInSpace->getId());
	eventManager->registerForEvent(ClientServerConsts::USER_INPUT_EVENT, userInputHandler);
	eventManager->registerForEvent(ClientServerConsts::CHARACTER_STILL_JUMPING_EVENT, userInputHandler);
	eventManager->registerForEvent(ClientServerConsts::CHARACTER_FALL_START_EVENT, userInputHandler);
	eventManager->registerForEvent(ClientServerConsts::CHARACTER_FALL_END_EVENT, userInputHandler);

	//create handler for gravity
	GravityHandler* gravityHandler = new GravityHandler(&propertyMap, false, characterGravity->getId(), characterLocationsInSpace->getId());
	eventManager->registerForEvent(ClientServerConsts::CHARACTER_MOVED_EVENT, gravityHandler);
	eventManager->registerForEvent(ClientServerConsts::CHARACTER_STILL_FALLING_EVENT, gravityHandler);
	eventManager->registerForEvent(ClientServerConsts::CHARACTER_JUMP_START_EVENT, gravityHandler);
	eventManager->registerForEvent(ClientServerConsts::CHARACTER_JUMP_END_EVENT, gravityHandler);
	eventManager->registerForEvent(ClientServerConsts::CHARACTER_SPAWN_EVENT, gravityHandler);
	eventManager->registerForEvent(ClientServerConsts::CHARACTER_MOVED_BY_PLATFORM_EVENT, gravityHandler);

	//create handler for character collision
	CharacterCollisionHandler* characterCollisionHandler = new CharacterCollisionHandler(&propertyMap, false, characterCollisions->getId(),
		{ deathZoneCollisions->getId() });
	eventManager->registerForEvent(ClientServerConsts::CHARACTER_MOVED_EVENT, characterCollisionHandler);
	eventManager->registerForEvent(ClientServerConsts::CHARACTER_MOVED_BY_GRAVITY_EVENT, characterCollisionHandler);
	eventManager->registerForEvent(ClientServerConsts::CHARACTER_MOVED_BY_PLATFORM_EVENT, characterCollisionHandler);

	//create handler for character death
	CharacterDeathHandler* characterDeathHandler = new CharacterDeathHandler(&propertyMap, false, deathZoneCollisions->getId());
	eventManager->registerForEvent(ClientServerConsts::CHARACTER_COLLISION_EVENT, characterDeathHandler);

	//create handler for character spawn
	CharacterSpawnHandler* characterSpawnHandler = new CharacterSpawnHandler(&propertyMap, false, characterLocationsInSpace->getId(),
		characterRespawning->getId());
	eventManager->registerForEvent(ClientServerConsts::CHARACTER_DEATH_EVENT, characterSpawnHandler);

	//create handler for character being moved by platform
	PlatformMovingCharacterHandler* platformMovingCharacterHandler = new PlatformMovingCharacterHandler(&propertyMap, false,
		{ firstScreenMovingPlatformCollisions->getId() },
		characterCollisions->getId(), characterGravity->getId(), characterLocationsInSpace->getId(),
		{ firstScreenStaticPlatformCollisions->getId(), firstScreenMovingPlatformCollisions->getId() });
	eventManager->registerForEvent(ClientServerConsts::PLATFORM_MOVED_EVENT, platformMovingCharacterHandler);

	//create handler for managing replays
	ReplayHandler* replayHandler = new ReplayHandler(&propertyMap, true, { ClientServerConsts::PLATFORM_MOVED_EVENT,
		ClientServerConsts::CHARACTER_MOVED_EVENT, ClientServerConsts::CHARACTER_MOVED_BY_GRAVITY_EVENT,
		ClientServerConsts::CHARACTER_MOVED_BY_PLATFORM_EVENT, ClientServerConsts::CHARACTER_SPAWN_EVENT });
	eventManager->registerForEvent(ClientServerConsts::REPLAY_RECORDING_START_EVENT, replayHandler);
	eventManager->registerForEvent(ClientServerConsts::REPLAY_RECORDING_STOP_EVENT, replayHandler);

	/****MAIN LOOP START*****/
	while (true)
	{

		/*CHECK FOR CONNECTIONS/DISCONNECTIONS*/
		//process several updates
		for (int i = 0; i < 5; i++)
		{
			//if client connected, create thread to handle their event requests and update port modifier
			if (checkForConnectionUpdate())
			{
				std::thread* clientEventThread = new std::thread(handleClientEventRequests,
					ClientServerConsts::EVENT_RAISING_REQ_REP_NUM_START + eventPortModifier);
				eventPortModifier++;
			}
		}

		//sleep for a bit
		sleep(15);

		/*UPDATE PLATFORM POSITIONS/POSSIBLY CHARACTERS STANDING ON THEM AND PUBLISH UPDATES ACCORDINGLY*/
		if (!eventManager->isPlayingReplay())
		{
			//update moving platform positions
			std::vector<int> movingPlatformIds = firstScreenMovingPlatformLocationsInSpace->getObjects();
			int movingPlatformsNum = movingPlatformIds.size();
			for (int i = 0; i < movingPlatformsNum; i++)
			{
				//record previous position
				sf::Shape* movingPlatformShape = firstScreenMovingPlatformLocationsInSpace->getObjectShape(movingPlatformIds[i]);
				float previousX = movingPlatformShape->getPosition().x;
				float previousY = movingPlatformShape->getPosition().y;

				//update position
				firstScreenMovingPlatformRepeatedMovements->updatePosition(movingPlatformIds[i]);

				//record new position
				float newX = movingPlatformShape->getPosition().x;
				float newY = movingPlatformShape->getPosition().y;

				//create and raise event for platform movement
				struct Event::ArgumentVariant objectIdArg = { Event::ArgumentType::TYPE_INTEGER, movingPlatformIds[i] };
				struct Event::ArgumentVariant locationInSpaceIdArg = { Event::ArgumentType::TYPE_INTEGER,
					firstScreenMovingPlatformLocationsInSpace->getId() };
				struct Event::ArgumentVariant xArg;
				xArg.argType = Event::ArgumentType::TYPE_FLOAT;
				xArg.argValue.argAsFloat = newX - previousX;
				struct Event::ArgumentVariant yArg;
				yArg.argType = Event::ArgumentType::TYPE_FLOAT;
				yArg.argValue.argAsFloat = newY - previousY;
				struct Event::ArgumentVariant absoluteXArg;
				absoluteXArg.argType = Event::ArgumentType::TYPE_FLOAT;
				absoluteXArg.argValue.argAsFloat = newX;
				struct Event::ArgumentVariant absoluteYArg;
				absoluteYArg.argType = Event::ArgumentType::TYPE_FLOAT;
				absoluteYArg.argValue.argAsFloat = newY;
				//platform movements should be processed before anything already in queue (so that characters being moved by
				//platform can be processed correctly)
				Event* platformMovementEvent = new Event(eventManager->getCurrentTime() - 1000.f, ClientServerConsts::PLATFORM_MOVED_EVENT,
					{ objectIdArg, locationInSpaceIdArg, xArg, yArg, absoluteXArg, absoluteYArg });

				try
				{
					std::lock_guard<std::mutex> lock(queueLock);
					eventManager->raise(platformMovementEvent);
				}
				catch (...)
				{
					std::cerr << "Error while raising platform move event" << std::endl;
				}
			}
		}

		//handle scheduled events
		try
		{
			std::lock_guard<std::mutex> lock(queueLock);
			eventManager->handleEvents();
		}
		catch (...)
		{
			std::cerr << "Error while handling events" << std::endl;
		}
		
		//sleep for a bit
		sleep(20);
	}
	/****MAIN LOOP END****/

	/*DESTROY STATE*/
	//delete sockets
	delete(connectDisconnectReqRepSocket);
	delete(connectDisconnectPubSubSocket);
	delete(movementUpdatePubSubSocket);
	delete(context);
	//delete properties
	while (propertyMap.size() > 0)
	{
		int currentPropertyId = propertyMap.begin()->first;
		Property* currentProperty = propertyMap.begin()->second;
		propertyMap.erase(currentPropertyId);
		delete(currentProperty);
	}
	//delete static platform shapes
	delete(platform1Shape);
	delete(platform2Shape);
	delete(platform3Shape);
	delete(platform4Shape);
	delete(platform5Shape);
	//delete moving platform shapes
	delete(movingPlatform1Shape);
	delete(movingPlatform2Shape);
	delete(movingPlatform3Shape);
	delete(movingPlatform4Shape);
	delete(movingPlatform5Shape);
	return 0;
}