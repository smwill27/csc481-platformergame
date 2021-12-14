#pragma once
#include "EventHandler.h"
#include "EventManager.h"
#include "ClientServerConsts.h"
#include "Collision.h"

/*
* An EventHandler that handles checking whether a character has died.
*/
class CharacterDeathHandler :
    public EventHandler
{
    private:
        /* id of Collision property for death zones */
        int deathZoneCollisionId;

    public:
        /*
        * Constructs a CharacterDeathHandler with the given values.
        * 
        * propertyMap: map of all properties
        * notifyWhileReplaying: whether to notify this handler of events while a replay is being played
        * deathZoneCollisionId: id of Collision property for death zones
        */
        CharacterDeathHandler(std::map<int, Property*>* propertyMap, bool notifyWhileReplaying, int deathZoneCollisionId);

        /*
        * Handles checking whether a character death has occurred.
        * 
        * e: event that could have led to a character death
        */
        void onEvent(Event* e);
};

