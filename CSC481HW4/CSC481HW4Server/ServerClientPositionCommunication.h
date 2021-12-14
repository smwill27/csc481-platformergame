#pragma once
#include "Property.h"
#include <zmq.hpp>

/*
* Namespace defining code values used in communication between client and server for this property.
*/
namespace ServerClientPositionCommunicationCodes
{
    const int ABSOLUTE_POSITION_UPDATE_CODE = 1; //code for an absolute update to an object's position
    const int RELATIVE_POSITION_UPDATE_CODE = 2; //code for an update to an object's position relative to its current position
    const int ERROR_CODE = 3; //code indicating an error in communication
    const int SUCCESS_CODE = 4; //code indicating a success in communication
}

/*
* Property defining an object's ability to have updates on its position be communicated between a server and clients.
* 
* Property Dependencies: LocationInSpace
*/
class ServerClientPositionCommunication :
    public Property
{
    private:
        /* LocationInSpace corresponding to each object */
        std::map<int, int> locationsInSpace;

    public:
        /*
        * Constructs a ServerClientPositionCommunicationProperty with the given values.
        * 
        * id: id of the property
        * propertyMap: map of all properties
        */
        ServerClientPositionCommunication(int id, std::map<int, Property*>* propertyMap);

        /*
        * Adds an object to the property with the given values.
        * 
        * objectId: id of the object
        * locationInSpaceId: id of the corresponding LocationInSpace
        */
        void addObject(int objectId, int locationInSpaceId);

        /*
        * Removes the given object from the property.
        */
        void removeObject(int objectId);

        /*
        * Sends a position update for the given objects using the given values and given socket, and processes the reply.
        * 
        * updateCode: code indicating type of position update (absolute or regular)
        * objectId: id of object
        * x: x value of positional update
        * y: y value of positional update
        * socket: req/rep socket to use for sending/receiving messages
        */
        void sendPositionUpdateRequest(int updateCode, int objectId, float x, float y, zmq::socket_t* socket);

        /*
        * Receives a position update with the given socket and processes it.
        * 
        * socket: req/rep socket to receive update with
        * 
        * returns: id of object that had its position updated (-1 if none)
        */
        int receivePositionUpdateRequest(zmq::socket_t* socket);

        /*
        * Publishes a position update with the given values using the given socket.
        * 
        * updateCode: code indicating type of position update (absolute or regular)
        * objectId: id of object
        * x: x value of positional update
        * y: y value of positional update
        * socket: pub/sub socket to use for sending message
        */
        void publishPositionUpdate(int updateCode, int objectId, float x, float y, zmq::socket_t* socket);

        /*
        * Receives and processes a published position update using the given socket.
        * 
        * socket: pub/sub socket to use for receiving message
        */
        void receivePublishedPositionUpdate(zmq::socket_t* socket);
};

