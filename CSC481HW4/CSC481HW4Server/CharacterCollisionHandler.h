#pragma once
#include "EventHandler.h"
#include "Collision.h"
#include "ClientServerConsts.h"
#include "EventManager.h"

/*
* An EventHandler that handles identifying whether a character is colliding with certain objects (such as a death zone).
*/
class CharacterCollisionHandler :
    public EventHandler
{
    private:
        /* id of Collision property to use for characters */
        int collisionId;

        /* list of Collision properties to check when determining if a character collision has occurred*/
        std::vector<int> collisionsToCheck;

    public:
        /*
        * Constructs a CharacterCollisionHandler with the given values.
        * 
        * propertyMap: map of all properties
        * notifyWhileReplaying: whether to notify this handler of events while a replay is being played
        * collisionId: id of Collision property to use for characters
        * collisonsToCheck: list of Collision properties to check when determining if a character collision has occurred
        */
        CharacterCollisionHandler(std::map<int, Property*>* propertyMap, bool notifyWhileReplaying,
            int collisionId, std::vector<int> collisionsToCheck);

        /*
        * Handles checking for character collisions.
        * 
        * e: event possibly leading to character collision
        */
        void onEvent(Event* e);
};

