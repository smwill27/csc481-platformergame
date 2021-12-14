#include "ServerClientPositionCommunication.h"
#include <iostream>
#include "LocationInSpace.h"

ServerClientPositionCommunication::ServerClientPositionCommunication(int id, std::map<int, Property*>* propertyMap)
	: Property(id, propertyMap)
{
	//construction taken care of in Property
}

void ServerClientPositionCommunication::addObject(int objectId, int locationInSpaceId)
{
	if (hasObject(objectId))
	{
		std::cerr << "Cannot add duplicate to ServerClientPositionCommunication" << std::endl;
		return;
	}

	objects.push_back(objectId);
	locationsInSpace.insert(std::pair<int, int>(objectId, locationInSpaceId));
}

void ServerClientPositionCommunication::removeObject(int objectId)
{
	if (!hasObject(objectId))
	{
		std::cerr << "No such object defined in ServerClientPositionCommunication" << std::endl;
		return;
	}

	//remove from objects list
	int objectsNum = objects.size();
	for (int i = 0; i < objectsNum; i++)
	{
		if (objects[i] == objectId)
		{
			objects.erase(objects.begin() + i);
			break;
		}
	}

	//remove from maps
	locationsInSpace.erase(objectId);
}

void ServerClientPositionCommunication::sendPositionUpdateRequest(int updateCode, int objectId, float x, float y, zmq::socket_t* socket)
{
	if (!hasObject(objectId))
	{
		std::cerr << "No such object defined in ServerClientPositionCommunication" << std::endl;
		return;
	}

	std::string updateString;

	//make sure update code is valid and construct string with message to be sent
	if (updateCode == ServerClientPositionCommunicationCodes::ABSOLUTE_POSITION_UPDATE_CODE
		|| updateCode == ServerClientPositionCommunicationCodes::RELATIVE_POSITION_UPDATE_CODE)
	{
		updateString = std::to_string(updateCode) + " " + std::to_string(objectId) + " " + std::to_string(x) +
			" " + std::to_string(y);
	}
	else
	{
		std::cerr << "Incorrect update code in ServerClientPositionCommunication" << std::endl;
		return;
	}

	//send message
	zmq::message_t updateMsg(updateString.length() + 1);
	const char* updateChars = updateString.c_str();
	memcpy(updateMsg.data(), updateChars, updateString.length() + 1);
	socket->send(updateMsg, zmq::send_flags::none);

	//receive reply
	zmq::message_t updateReply;
	socket->recv(updateReply, zmq::recv_flags::none);

	//parse out reply and print error message if appropriate
	std::string updateReplyString = updateReply.to_string();
	int responseCode = -1;
	int scan = sscanf_s(updateReplyString.c_str(), "%d", &responseCode);
	if (scan != 1 || responseCode != ServerClientPositionCommunicationCodes::SUCCESS_CODE)
	{
		std::cerr << "Error in communication while sending position update request" << std::endl;
	}
}

int ServerClientPositionCommunication::receivePositionUpdateRequest(zmq::socket_t* socket)
{
	//receive update request
	zmq::message_t updateRequest;

	if (socket->recv(updateRequest, zmq::recv_flags::dontwait))
	{

		//parse out update request
		std::string updateRequestString = updateRequest.to_string();
		int updateCode = -1;;
		int objectId = -1;
		float x = 0.f;
		float y = 0.f;
		int scan = sscanf_s(updateRequestString.c_str(), "%d %d %f %f", &updateCode, &objectId, &x, &y);

		//make sure request is valid, and if not send back error message
		if (scan != 4 || (updateCode != ServerClientPositionCommunicationCodes::ABSOLUTE_POSITION_UPDATE_CODE &&
			updateCode != ServerClientPositionCommunicationCodes::RELATIVE_POSITION_UPDATE_CODE)
			|| !hasObject(objectId))
		{
			std::string replyString = std::to_string(ServerClientPositionCommunicationCodes::ERROR_CODE);
			zmq::message_t replyMsg(replyString.length() + 1);
			const char* replyChars = replyString.c_str();
			memcpy(replyMsg.data(), replyChars, replyString.length() + 1);
			socket->send(replyMsg, zmq::send_flags::none);
			return -1;
		}
		//if request is valid, perform position update and send back success message
		else
		{
			LocationInSpace* objectLocationInSpace = (LocationInSpace*)propertyMap->at(locationsInSpace.at(objectId));
			sf::Shape* objectShape = objectLocationInSpace->getObjectShape(objectId);

			//if absolute update, set position to given x and y
			if (updateCode == ServerClientPositionCommunicationCodes::ABSOLUTE_POSITION_UPDATE_CODE)
			{
				objectShape->setPosition(x, y);
			}
			//if relative update, alter existing position by x and y
			else
			{
				objectShape->move(x, y);
			}

			//send success message
			std::string replyString = std::to_string(ServerClientPositionCommunicationCodes::SUCCESS_CODE);
			zmq::message_t replyMsg(replyString.length() + 1);
			const char* replyChars = replyString.c_str();
			memcpy(replyMsg.data(), replyChars, replyString.length() + 1);
			socket->send(replyMsg, zmq::send_flags::none);
			return objectId;
		}
	}

	return -1;
}

void ServerClientPositionCommunication::publishPositionUpdate(int updateCode, int objectId, float x, float y, zmq::socket_t* socket)
{
	if (!hasObject(objectId))
	{
		std::cerr << "No such object defined in ServerClientPositionCommunication" << std::endl;
		return;
	}

	std::string updateString;

	//make sure update code is valid and construct string with message to be sent
	if (updateCode == ServerClientPositionCommunicationCodes::ABSOLUTE_POSITION_UPDATE_CODE
		|| updateCode == ServerClientPositionCommunicationCodes::RELATIVE_POSITION_UPDATE_CODE)
	{
		updateString = std::to_string(updateCode) + " " + std::to_string(objectId) + " " + std::to_string(x) +
			" " + std::to_string(y);
	}
	else
	{
		std::cerr << "Incorrect update code in ServerClientPositionCommunication" << std::endl;
		return;
	}

	//publish message
	zmq::message_t updateMsg(updateString.length() + 1);
	const char* updateChars = updateString.c_str();
	memcpy(updateMsg.data(), updateChars, updateString.length() + 1);
	socket->send(updateMsg, zmq::send_flags::none);
}

void ServerClientPositionCommunication::receivePublishedPositionUpdate(zmq::socket_t* socket)
{
	//receive published update message
	zmq::message_t updateMsg;
	if (socket->recv(updateMsg, zmq::recv_flags::dontwait))
	{

		//parse out message
		int updateCode = -1;
		int objectId = -1;
		float x = 0.f;
		float y = 0.f;
		int scan = sscanf_s(updateMsg.to_string().c_str(), "%d %d %f %f", &updateCode, &objectId, &x, &y);

		//make sure update is valid, and if not print error message
		if (scan != 4 || (updateCode != ServerClientPositionCommunicationCodes::ABSOLUTE_POSITION_UPDATE_CODE &&
			updateCode != ServerClientPositionCommunicationCodes::RELATIVE_POSITION_UPDATE_CODE)
			|| !hasObject(objectId))
		{
			std::cerr << "Error in communication while processing published update in ServerClientPositionCommunication" << std::endl;
		}
		//if update is valid, perform position update
		else
		{
			LocationInSpace* objectLocationInSpace = (LocationInSpace*)propertyMap->at(locationsInSpace.at(objectId));
			sf::Shape* objectShape = objectLocationInSpace->getObjectShape(objectId);

			//if absolute update, set position to given x and y
			if (updateCode == ServerClientPositionCommunicationCodes::ABSOLUTE_POSITION_UPDATE_CODE)
			{
				objectShape->setPosition(x, y);
			}
			//if relative update, alter existing position by x and y
			else
			{
				objectShape->move(x, y);
			}
		}
	}
}