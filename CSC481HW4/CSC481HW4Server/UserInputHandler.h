#pragma once
#include "EventHandler.h"
#include "ClientServerConsts.h"
#include "PlayerDirectedMovement.h"
#include "LocationInSpace.h"
#include "EventManager.h"
/*
* Event handler that handles inputs from the user.
*/
class UserInputHandler :
    public EventHandler
{
    private:
        /* id of PlayerDirectedMovement property to use for performing character movements */
        int playerDirectedMovementId;

        /* id of LocationInSpace property to use in checking character positions*/
        int locationInSpaceId;

    public:
        /*
        * Constructs a UserInputHandler with the given values.
        * 
        * propertyMap: map of all properties
        * notifyWhileReplaying: whether to notify this handler of events while a replay is being played
        * playerDirectedMovementId: id of PlayerDirectedMovement property to use in performing character movements
        * locationInSpaceId: id of LocationInSpace property to use in checking character positions
        */
        UserInputHandler(std::map<int, Property*>* propertyMap, bool notifyWhileReplaying,
            int playerDirectedMovementId, int locationInSpaceId);

        /*
        * Handles the specific player input in the given event.
        * 
        * e: event being handled
        */
        void onEvent(Event* e);
};

