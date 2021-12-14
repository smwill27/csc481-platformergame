#pragma once
#include "EventHandler.h"
#include "ClientServerConsts.h"
#include "Gravity.h"
#include "LocationInSpace.h"
#include "EventManager.h"

/*
* An EventHandler that handles changes to a character based on gravity.
*/
class GravityHandler :
    public EventHandler
{
    private:
        /* id of Gravity property to use for performing checks on character */
        int gravityId;

        /* id of LocationInSpace property to use in checking character positions*/
        int locationInSpaceId;

    public:
        /*
        * Constructs a GravityHandler with the given values.
        * 
        * propertyMap: map of all properties
        * notifyWhileReplaying: whether to notify this handler of events while a replay is being played
        * gravityId: id of gravity to use in performing checks on characters
        * locationInSpaceId: id of LocationInSpace to use in checking character positions
        */
        GravityHandler(std::map<int, Property*>* propertyMap, bool notifyWhileReplaying,
            int gravityId, int locationInSpaceId);

        /*
        * Handles processing gravity based on the given event.
        *
        * e: event being handled
        */
        void onEvent(Event* e);
};

