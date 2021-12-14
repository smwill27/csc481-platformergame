#pragma once
#include "EventHandler.h"
#include "EventManager.h"
#include "Respawning.h"
#include "ClientServerConsts.h"
#include "LocationInSpace.h"
class CharacterSpawnHandler :
    public EventHandler
{
    private:
        /* id of LocationInSpace property for characters */
        int locationInSpaceId;
        
        /* id of Respawning property for characters */
        int respawningId;

    public:
        /*
        * Constructs a CharacterSpawnHandler with the given values.
        * 
        * propertyMap: map of all properties
        * notifyWhileReplaying: whether to notify this handler of events while a replay is being played
        * locationInSpaceId: id of LocationInSpace property for characters
        * respawningId: id of Respawning property for characters
        */
        CharacterSpawnHandler(std::map<int, Property*>* propertyMap, bool notifyWhileReplaying,
            int locationInSpaceId, int respawningId);

        /*
        * Handles respawning a character upon death.
        * 
        * e: event indicating character death
        */
        void onEvent(Event* e);
};

