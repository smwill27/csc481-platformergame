#include "PositionalUpdateHandler.h"
#include "LocationInSpace.h"
#include <iostream>

PositionalUpdateHandler::PositionalUpdateHandler(std::map<int, Property*>* propertyMap, bool notifyWhileReplaying, zmq::socket_t* socket)
	: EventHandler(propertyMap, notifyWhileReplaying)
{
	this->socket = socket;
}

void PositionalUpdateHandler::onEvent(Event* e)
{
	if (e->getType() == ClientServerConsts::PLATFORM_MOVED_EVENT || e->getType() == ClientServerConsts::CHARACTER_MOVED_EVENT
		|| e->getType() == ClientServerConsts::CHARACTER_MOVED_BY_GRAVITY_EVENT 
		|| e->getType() == ClientServerConsts::CHARACTER_SPAWN_EVENT
		|| e->getType() == ClientServerConsts::CHARACTER_MOVED_BY_PLATFORM_EVENT)
	{
		//get ids of moved object and its location in space, as well as its new absolute position
		int objectId = e->getArgument(0).argValue.argAsInt;
		int locationInSpaceId = e->getArgument(1).argValue.argAsInt;
		float absoluteX = 0.f;
		float absoluteY = 0.f;

		if (e->getType() == ClientServerConsts::CHARACTER_SPAWN_EVENT)
		{
			absoluteX = e->getArgument(2).argValue.argAsFloat;
			absoluteY = e->getArgument(3).argValue.argAsFloat;
		}
		else
		{
			absoluteX = e->getArgument(4).argValue.argAsFloat;
			absoluteY = e->getArgument(5).argValue.argAsFloat;
		}

		//construct message to send to clients
		std::string msgString = std::to_string(objectId) + " " + std::to_string(locationInSpaceId)
			+ " " + std::to_string(absoluteX) + " " + std::to_string(absoluteY);
		zmq::message_t msg(msgString.length() + 1);
		const char* msgChars = msgString.c_str();
		memcpy(msg.data(), msgChars, msgString.length() + 1);

		//std::cout << "Sending positional update" << std::endl;
		//send update to clients
		socket->send(msg, zmq::send_flags::none);
	}
}
