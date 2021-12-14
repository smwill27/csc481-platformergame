#include <zmq.hpp>
#include <stdlib.h>
#include <string>
#include <stdio.h>
#include <iostream>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <thread>
#include <chrono>
#include "ClientServerConsts.h"
#include "Property.h"
#include "LocationInSpace.h"
#include "Rendering.h"

#ifndef _WIN32
#include <unistd.h>
#else
#include <windows.h>

#define sleep(n)    Sleep(n)
#endif
#include "ServerClientPositionCommunication.h"

/* context for all sockets */
zmq::context_t* context;

/* socket used for sending connection/disconnection requests to server */
zmq::socket_t* connectDisconnectReqRepSocket;

/* socket used for receiving connection/disconnection updates from server */
zmq::socket_t* connectDisconnectPubSubSocket;

/* socket used for receiving moving platform updates from server */
zmq::socket_t* movementUpdatePubSubSocket;

/* socket used for receiving object creation messages from server upon connecting */
zmq::socket_t* objectCreationReqRepSocket;

/* socket used for raising events on server */
zmq::socket_t* eventRaisingReqRepSocket;

/* id corresponding to character controlled by this client */
int characterId;

/* map with all properties */
std::map<int, Property*> propertyMap;

/* list of rendering property ids to use in drawing objects to screen */
std::vector<int> renderingIds;

/* value indicating whether the window is selected (and so input should be registered) */
bool windowInFocus;

int main()
{
	/*PREPARE STATE*/

	// Prepare our context and sockets
	context = new zmq::context_t(1);
	connectDisconnectReqRepSocket = new zmq::socket_t(*context, ZMQ_REQ);
	connectDisconnectPubSubSocket = new zmq::socket_t(*context, ZMQ_SUB);
	movementUpdatePubSubSocket = new zmq::socket_t(*context, ZMQ_SUB);
	objectCreationReqRepSocket = new zmq::socket_t(*context, ZMQ_REQ);
	const char* filter1 = "";
	const char* filter2 = "";
	connectDisconnectPubSubSocket->set(zmq::sockopt::subscribe, filter1);
	movementUpdatePubSubSocket->set(zmq::sockopt::subscribe, filter2);

	//std::cout << "prepared context and sockets" << std::endl;

	//connect req/rep socket
	connectDisconnectReqRepSocket->connect(ClientServerConsts::CONNECT_DISCONNECT_REQ_REP_CLIENT_PORT);
	objectCreationReqRepSocket->connect(ClientServerConsts::OBJECT_CREATION_REQ_REP_CLIENT_PORT);

	//std::cout << "connected req/rep socket" << std::endl;

	//create an 800x600 window with a titlebar that can be resized and closed
	sf::RenderWindow window(sf::VideoMode(ClientServerConsts::WINDOW_WIDTH, ClientServerConsts::WINDOW_HEIGHT),
		ClientServerConsts::WINDOW_TITLE, sf::Style::Default);
	windowInFocus = true;

	/*SEND CONNECTION REQUEST*/
	//send request to server to initialize connection
	std::string connectMsgString = std::to_string(ClientServerConsts::CONNECT_CODE);
	zmq::message_t connectMsg(connectMsgString.length() + 1);
	const char* connectMsgChars = connectMsgString.c_str();
	memcpy(connectMsg.data(), connectMsgChars, connectMsgString.length() + 1);
	connectDisconnectReqRepSocket->send(connectMsg, zmq::send_flags::none);

	//std::cout << "sent request to server for connection" << std::endl;

	//get response from server confirming connection
	zmq::message_t connectReply;
	connectDisconnectReqRepSocket->recv(connectReply, zmq::recv_flags::none);
	int connectionId = -1;
	int portNum = -1;
	int scan = sscanf_s(connectReply.to_string().c_str(), "%d %d", &connectionId, &portNum);
	//std::cout << "Received from server: " + connectReply.to_string() << std::endl;

	//if connection failed, close window and exit
	if (scan != 2 || connectionId == ClientServerConsts::CONNECT_ERROR_CODE)
	{
		/*TODO: CLOSE WINDOW AND CLEAN UP*/
		window.close();
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
		std::cerr << "Failed to connect to server" << std::endl;
		return 0;
	}

	//if connection didn't fail, create objects as dictated by server
	//set id of character being controlled by this client
	characterId = connectionId;
	
	/* repeatedly check for object creation messages and create properties/objects indicated */
	bool moreToCreate = true;
	while (moreToCreate)
	{
		//send filler message
		std::string creationMsgString = std::to_string(ClientServerConsts::CREATION_FILLER);
		zmq::message_t creationMsg(creationMsgString.length() + 1);
		const char* creationMsgChars = creationMsgString.c_str();
		memcpy(creationMsg.data(), creationMsgChars, creationMsgString.length() + 1);
		objectCreationReqRepSocket->send(creationMsg, zmq::send_flags::none);

		//receive and parse out response
		zmq::message_t creationReply;
		objectCreationReqRepSocket->recv(creationReply, zmq::recv_flags::none);
		int objectOrPropertyType = -1;
		int objectOrPropertyId = -1;
		int locationInSpaceId = -1;
		int renderingId = -1;
		int colorInt = -1;
		float x = 0.f;
		float y = 0.f;
		int scan = sscanf_s(creationReply.to_string().c_str(), "%d %d %d %d %d %f %f", &objectOrPropertyType, &objectOrPropertyId,
			&locationInSpaceId, &renderingId, &colorInt, &x, &y);
		//std::cout << "Received from server: " + creationReply.to_string() << std::endl;

		//if end of property/object creation, don't send another message
		if (objectOrPropertyType == ClientServerConsts::CREATION_END)
		{
			moreToCreate = false;
		}
		//if location in space property, create it
		else if (objectOrPropertyType == ClientServerConsts::PROPERTY_LOCATION_IN_SPACE)
		{
			LocationInSpace* newLocationInSpace = new LocationInSpace(objectOrPropertyId, &propertyMap);
			propertyMap.insert(std::pair<int, Property*>(newLocationInSpace->getId(), newLocationInSpace));
		}
		//if rendering property, create it
		else if (objectOrPropertyType == ClientServerConsts::PROPERTY_RENDERING)
		{
			Rendering* newRendering = new Rendering(objectOrPropertyId, &propertyMap, &window);
			propertyMap.insert(std::pair<int, Property*>(newRendering->getId(), newRendering));
			renderingIds.push_back(objectOrPropertyId);
		}
		//if static platform, create it
		else if (objectOrPropertyType == ClientServerConsts::STATIC_PLATFORM)
		{
			sf::RectangleShape* newStaticPlatformShape = new sf::RectangleShape(sf::Vector2f(ClientServerConsts::PLATFORM_WIDTH,
				ClientServerConsts::PLATFORM_HEIGHT));
			newStaticPlatformShape->setFillColor(sf::Color(colorInt));
			newStaticPlatformShape->setPosition(sf::Vector2f(x, y));
			LocationInSpace* staticPlatformLocationInSpace = (LocationInSpace*)propertyMap.at(locationInSpaceId);
			staticPlatformLocationInSpace->addObject(objectOrPropertyId, newStaticPlatformShape);
			Rendering* staticPlatformRendering = (Rendering*)propertyMap.at(renderingId);
			staticPlatformRendering->addObject(objectOrPropertyId, locationInSpaceId);
		}
		//if moving platform, create it
		else if (objectOrPropertyType == ClientServerConsts::MOVING_PLATFORM)
		{
			sf::RectangleShape* newMovingPlatformShape = new sf::RectangleShape(sf::Vector2f(ClientServerConsts::PLATFORM_WIDTH,
				ClientServerConsts::PLATFORM_HEIGHT));
			newMovingPlatformShape->setFillColor(sf::Color(colorInt));
			newMovingPlatformShape->setPosition(sf::Vector2f(x, y));
			LocationInSpace* movingPlatformLocationInSpace = (LocationInSpace*)propertyMap.at(locationInSpaceId);
			movingPlatformLocationInSpace->addObject(objectOrPropertyId, newMovingPlatformShape);
			Rendering* movingPlatformRendering = (Rendering*)propertyMap.at(renderingId);
			movingPlatformRendering->addObject(objectOrPropertyId, locationInSpaceId);
		}
		//if character, create it
		else if (objectOrPropertyType == ClientServerConsts::CHARACTER)
		{
			sf::CircleShape* newCharShape = new sf::CircleShape(ClientServerConsts::CHARACTER_RADIUS,
				ClientServerConsts::CHARACTER_NUM_OF_POINTS);
			
			//if this is character of this client, color it magenta
			if (objectOrPropertyId == characterId)
			{
				newCharShape->setFillColor(sf::Color::Magenta);
			}
			//otherwise, color it red
			else
			{
				newCharShape->setFillColor(sf::Color::Red);
			}

			newCharShape->setPosition(sf::Vector2f(x, y));
			LocationInSpace* charLocationInSpace = (LocationInSpace*)propertyMap.at(locationInSpaceId);
			charLocationInSpace->addObject(objectOrPropertyId, newCharShape);
			Rendering* charRendering = (Rendering*)propertyMap.at(renderingId);
			charRendering->addObject(objectOrPropertyId, locationInSpaceId);
		}
	}

	//create and connect event raising socket with given port number
	eventRaisingReqRepSocket = new zmq::socket_t(*context, ZMQ_REQ);
	eventRaisingReqRepSocket->connect(ClientServerConsts::EVENT_RAISING_REQ_REP_CLIENT_PORT_START + std::to_string(portNum));

	//subscribe for updates
	connectDisconnectPubSubSocket->connect(ClientServerConsts::CONNECT_DISCONNECT_PUB_SUB_CLIENT_PORT);
	movementUpdatePubSubSocket->connect(ClientServerConsts::MOVEMENT_UPDATE_PUB_SUB_CLIENT_PORT);

	//values indicating whether a button was just pressed (used to support only one event being sent per key press)
	bool rKeyPressed = false;
	bool oneKeyPressed = false;
	bool twoKeyPressed = false;
	bool threeKeyPressed = false;

	/***MAIN LOOP START***/
	while (true)
	{
		/*CHECK FOR WINDOW EVENTS AND SEND ANY NECESSARY UPDATES*/
		sf::Event event;
		//check for window events
		while (window.pollEvent(event))
		{
			//if a window close is requested, send disconnect request to server and close the window
			if (event.type == sf::Event::Closed)
			{
				//std::cout << "Sending disconnection request" << std::endl;
				//send request to server to disconnect
				std::string connectMsgString = std::to_string(characterId);
				zmq::message_t connectMsg(connectMsgString.length() + 1);
				const char* sChars = connectMsgString.c_str();
				memcpy(connectMsg.data(), sChars, connectMsgString.length() + 1);
				connectDisconnectReqRepSocket->send(connectMsg, zmq::send_flags::none);

				//get response from server confirming disconnection
				zmq::message_t disconnectReply;
				connectDisconnectReqRepSocket->recv(disconnectReply, zmq::recv_flags::none);
				//std::cout << "Received from server: " + disconnectReply.to_string() << std::endl;

				//send disconnect message to event port to let that thread stop
				std::string disconnectEventMsgString = ClientServerConsts::CLIENT_DISCONNECT_EVENT;
				zmq::message_t disconnectEventMsg(disconnectEventMsgString.length() + 1);
				const char* disconnectEventChars = disconnectEventMsgString.c_str();
				memcpy(disconnectEventMsg.data(), disconnectEventChars, disconnectEventMsgString.length() + 1);
				eventRaisingReqRepSocket->send(disconnectEventMsg, zmq::send_flags::none);
				
				//receive response
				zmq::message_t eventDisconnectReply;
				eventRaisingReqRepSocket->recv(eventDisconnectReply, zmq::recv_flags::none);

				//close window and destroy state
				window.close();
				//std::cout << "Deleting properties" << std::endl;
				//delete properties
				while (propertyMap.size() > 0)
				{
					int currentPropertyId = propertyMap.begin()->first;
					Property* currentProperty = propertyMap.begin()->second;
					propertyMap.erase(currentPropertyId);
					delete(currentProperty);
				}
				return 0;
			}

			//if window is no longer active, stop handling keyboard input
			if (event.type == sf::Event::LostFocus)
			{
				windowInFocus = false;
			}

			//if window is now active, resume handling keyboard input
			if (event.type == sf::Event::GainedFocus)
			{
				windowInFocus = true;
			}

			//if a window resize is requested, resize the window
			if (event.type == sf::Event::Resized)
			{
				//set the window to the given size
				window.setSize(sf::Vector2u(event.size.width, event.size.height));
			}
		}

		//check for user input
		if (windowInFocus && sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
		{
			//send message to server to raise event
			std::string userInputString = std::to_string(ClientServerConsts::USER_INPUT_EVENT_CODE) + " "
				+ std::to_string(characterId) + " " + std::to_string(ClientServerConsts::RIGHT_ARROW_KEY);
			zmq::message_t userInputMsg(userInputString.length() + 1);
			const char* userInputChars = userInputString.c_str();
			memcpy(userInputMsg.data(), userInputChars, userInputString.length() + 1);
			eventRaisingReqRepSocket->send(userInputMsg, zmq::send_flags::none);
			
			//receive response
			zmq::message_t userInputReply;
			eventRaisingReqRepSocket->recv(userInputReply, zmq::recv_flags::none);
			//std::cout << "Received from server: " + userInputReply.to_string() << std::endl;
		}
		
		if (windowInFocus && sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
		{
			//send message to server to raise event
			std::string userInputString = std::to_string(ClientServerConsts::USER_INPUT_EVENT_CODE) + " "
				+ std::to_string(characterId) + " " + std::to_string(ClientServerConsts::LEFT_ARROW_KEY);
			zmq::message_t userInputMsg(userInputString.length() + 1);
			const char* userInputChars = userInputString.c_str();
			memcpy(userInputMsg.data(), userInputChars, userInputString.length() + 1);
			eventRaisingReqRepSocket->send(userInputMsg, zmq::send_flags::none);

			//receive response
			zmq::message_t userInputReply;
			eventRaisingReqRepSocket->recv(userInputReply, zmq::recv_flags::none);
			//std::cout << "Received from server: " + userInputReply.to_string() << std::endl;
		}
		
		if (windowInFocus && sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
		{
			//send message to server to raise event
			std::string userInputString = std::to_string(ClientServerConsts::USER_INPUT_EVENT_CODE) + " "
				+ std::to_string(characterId) + " " + std::to_string(ClientServerConsts::UP_ARROW_KEY);
			zmq::message_t userInputMsg(userInputString.length() + 1);
			const char* userInputChars = userInputString.c_str();
			memcpy(userInputMsg.data(), userInputChars, userInputString.length() + 1);
			eventRaisingReqRepSocket->send(userInputMsg, zmq::send_flags::none);

			//receive response
			zmq::message_t userInputReply;
			eventRaisingReqRepSocket->recv(userInputReply, zmq::recv_flags::none);
			//std::cout << "Received from server: " + userInputReply.to_string() << std::endl;
		}

		if (windowInFocus && sf::Keyboard::isKeyPressed(sf::Keyboard::R))
		{
			if (!rKeyPressed)
			{
				rKeyPressed = true;

				//send message to server to raise event
				std::string userInputString = std::to_string(ClientServerConsts::USER_INPUT_EVENT_CODE) + " "
					+ std::to_string(characterId) + " " + std::to_string(ClientServerConsts::R_KEY);
				zmq::message_t userInputMsg(userInputString.length() + 1);
				const char* userInputChars = userInputString.c_str();
				memcpy(userInputMsg.data(), userInputChars, userInputString.length() + 1);
				eventRaisingReqRepSocket->send(userInputMsg, zmq::send_flags::none);

				//receive response
				zmq::message_t userInputReply;
				eventRaisingReqRepSocket->recv(userInputReply, zmq::recv_flags::none);
				//std::cout << "Received from server: " + userInputReply.to_string() << std::endl;
			}
		}
		else
		{
			rKeyPressed = false;
		}

		if (windowInFocus && sf::Keyboard::isKeyPressed(sf::Keyboard::Num1))
		{
			if (!oneKeyPressed)
			{
				oneKeyPressed = true;

				//send message to server to raise event
				std::string userInputString = std::to_string(ClientServerConsts::USER_INPUT_EVENT_CODE) + " "
					+ std::to_string(characterId) + " " + std::to_string(ClientServerConsts::ONE_KEY);
				zmq::message_t userInputMsg(userInputString.length() + 1);
				const char* userInputChars = userInputString.c_str();
				memcpy(userInputMsg.data(), userInputChars, userInputString.length() + 1);
				eventRaisingReqRepSocket->send(userInputMsg, zmq::send_flags::none);

				//receive response
				zmq::message_t userInputReply;
				eventRaisingReqRepSocket->recv(userInputReply, zmq::recv_flags::none);
				//std::cout << "Received from server: " + userInputReply.to_string() << std::endl;
			}
		}
		else
		{
			oneKeyPressed = false;
		}

		if (windowInFocus && sf::Keyboard::isKeyPressed(sf::Keyboard::Num2))
		{
			if (!twoKeyPressed)
			{
				twoKeyPressed = true;

				//send message to server to raise event
				std::string userInputString = std::to_string(ClientServerConsts::USER_INPUT_EVENT_CODE) + " "
					+ std::to_string(characterId) + " " + std::to_string(ClientServerConsts::TWO_KEY);
				zmq::message_t userInputMsg(userInputString.length() + 1);
				const char* userInputChars = userInputString.c_str();
				memcpy(userInputMsg.data(), userInputChars, userInputString.length() + 1);
				eventRaisingReqRepSocket->send(userInputMsg, zmq::send_flags::none);

				//receive response
				zmq::message_t userInputReply;
				eventRaisingReqRepSocket->recv(userInputReply, zmq::recv_flags::none);
				//std::cout << "Received from server: " + userInputReply.to_string() << std::endl;
			}
		}
		else
		{
			twoKeyPressed = false;
		}

		if (windowInFocus && sf::Keyboard::isKeyPressed(sf::Keyboard::Num3))
		{
			if (!threeKeyPressed)
			{
				threeKeyPressed = true;

				//send message to server to raise event
				std::string userInputString = std::to_string(ClientServerConsts::USER_INPUT_EVENT_CODE) + " "
					+ std::to_string(characterId) + " " + std::to_string(ClientServerConsts::THREE_KEY);
				zmq::message_t userInputMsg(userInputString.length() + 1);
				const char* userInputChars = userInputString.c_str();
				memcpy(userInputMsg.data(), userInputChars, userInputString.length() + 1);
				eventRaisingReqRepSocket->send(userInputMsg, zmq::send_flags::none);

				//receive response
				zmq::message_t userInputReply;
				eventRaisingReqRepSocket->recv(userInputReply, zmq::recv_flags::none);
				//std::cout << "Received from server: " + userInputReply.to_string() << std::endl;
			}
		}
		else
		{
			threeKeyPressed = false;
		}

		/*CHECK FOR SEVERAL CONNECTION UPDATES FROM SERVER AND PROCESS*/
		for (int i = 0; i < 5; i++)
		{
			zmq::message_t connectionUpdate;
			if (connectDisconnectPubSubSocket->recv(connectionUpdate, zmq::recv_flags::dontwait))
			{
				//parse out update
				int connectUpdateCode = 0;
				int connectUpdateId = 0;
				int locationInSpaceId = 0;
				int renderingId = 0;
				float x = 0.f;
				float y = 0.f;
				int scan = sscanf_s(connectionUpdate.to_string().c_str(), "%d %d %d %d %f %f", &connectUpdateCode, &connectUpdateId,
					&locationInSpaceId, &renderingId, &x, &y);
				//std::cout << "Received from server: " + connectionUpdate.to_string() << std::endl;

				//if message not as expected, print error message
				if (scan != 6
					|| (connectUpdateCode != ClientServerConsts::CONNECT_CODE && connectUpdateCode != ClientServerConsts::DISCONNECT_CODE))
				{
					std::cerr << "Error in communication when processing connection update from server" << std::endl;
				}
				//if update is connection, add character with given id
				else if (connectUpdateCode == ClientServerConsts::CONNECT_CODE && connectUpdateId != characterId)
				{
					//std::cout << "*********ADDING CHARACTER FOR NEW CLIENT*********" << std::endl;

					//create character shape
					sf::CircleShape* nextCharacterShape = new sf::CircleShape(ClientServerConsts::CHARACTER_RADIUS, ClientServerConsts::CHARACTER_NUM_OF_POINTS);
					nextCharacterShape->setFillColor(sf::Color::Red);
					nextCharacterShape->setPosition(sf::Vector2f(x, y));

					//add character to properties
					LocationInSpace* charLocationInSpace = (LocationInSpace*)propertyMap.at(locationInSpaceId);
					charLocationInSpace->addObject(connectUpdateId, nextCharacterShape);
					Rendering* charRendering = (Rendering*)propertyMap.at(renderingId);
					charRendering->addObject(connectUpdateId, locationInSpaceId);
				}
				//if update is disconnection, remove character with given id from all properties where it exists
				else
				{
					//std::cout << "******REMOVING CHARACTER FOR DISCONNECTING CLIENT*********" << std::endl;
					LocationInSpace* charLocationInSpace = (LocationInSpace*)propertyMap.at(locationInSpaceId);
					charLocationInSpace->removeObject(connectUpdateId);
					Rendering* charRendering = (Rendering*)propertyMap.at(renderingId);
					charRendering->removeObject(connectUpdateId);
				}
			}
		}

		/*CHECK FOR SEVERAL POSITION UPDATES FROM SERVER AND PROCESS*/
		for (int i = 0; i < 10; i++)
		{
			zmq::message_t positionUpdate;
			if (movementUpdatePubSubSocket->recv(positionUpdate, zmq::recv_flags::dontwait))
			{
				//parse out update
				int objectId = 0;
				int locationInSpaceId = 0;
				float x = 0.f;
				float y = 0.f;
				int scan = sscanf_s(positionUpdate.to_string().c_str(), "%d %d %f %f", &objectId, &locationInSpaceId, &x, &y);
				//std::cout << "Received from server: " + positionUpdate.to_string() << std::endl;

				//if message not as expected, print error message
				if (scan != 4)
				{
					std::cerr << "Error in communication when processing position update from server" << std::endl;
				}
				//otherwise, process position update
				else
				{
					LocationInSpace* locationInSpace = (LocationInSpace*)propertyMap.at(locationInSpaceId);
					//make sure it has object (could be lingering update from removed character)
					if (locationInSpace->hasObject(objectId))
					{
						sf::Shape* objectShape = locationInSpace->getObjectShape(objectId);
						objectShape->setPosition(sf::Vector2f(x, y));
					}
				}
			}
		}

		/*DRAW TO WINDOW*/
		//clear the window with blue color
		window.clear(sf::Color::Blue);

		//draw all objects
		int renderingNum = renderingIds.size();
		for (int i = 0; i < renderingNum; i++)
		{
			Rendering* nextRendering = (Rendering*) propertyMap.at(renderingIds[i]);
			nextRendering->drawAllObjects();
		}

		//end current frame
		window.display();


	}
	/***MAIN LOOP END***/

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

	return 0;
}