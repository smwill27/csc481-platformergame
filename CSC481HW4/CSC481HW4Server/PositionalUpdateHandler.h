#pragma once
#include "EventHandler.h"
#include "ClientServerConsts.h"
#include <zmq.hpp>
/*
* Event handler that handles publishing positional updates to the client.
*/
class PositionalUpdateHandler :
    public EventHandler
{
    private:
        /* socket used for publishing position updates */
        zmq::socket_t* socket;

    public:
        /*
        * Constructs a PositionalUpdateHandler with the given values.
        * 
        * propertyMap: map of all properties
        * notifyWhileReplaying: whether to notify this handler of events while a replay is being played
        * socket: pub/sub socket to use in publishing positional updates
        */
        PositionalUpdateHandler(std::map<int, Property*>* propertyMap, bool notifyWhileReplaying, zmq::socket_t* socket);

        /*
        * Publishes a position update for the given event.
        * 
        * e: event being handled
        */
        void onEvent(Event* e);
};

