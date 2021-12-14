#pragma once
#include "EventHandler.h"
#include "EventManager.h"
#include "ClientServerConsts.h"
#include "Collision.h"
#include "Gravity.h"
#include "LocationInSpace.h"

/*
* An EventHandler that handles moving platforms affecting the position of the character (either due to the character standing
* on the platform or being pushed when in the way of the platform).
*/
class PlatformMovingCharacterHandler :
    public EventHandler
{
    private:
        /* ids of Collision properties pertaining to moving platforms */
        std::vector<int> platformCollisionIds;

        /* id of Collision property pertaining to characters*/
        int characterCollisionId;

        /* id of Gravity property pertaining to characters */
        int characterGravityId;

        /* id of LocationInSpace property pertaining to characters */
        int characterLocationInSpaceId;

        /* ids of Collision properties pertaining to objects that Character might collide with when moved*/
        std::vector<int> possibleObjectCollisionIds;

    public:
        /*
        * Constructs a PlatformMovingCharacterHandler with the given values.
        * 
        * propertyMap: map of all properties
        * notifyWhileReplaying: whether to notify this handler of events while a replay is being played
        * platformCollisionIds: ids of Collision properties pertaining to moving platforms
        * characterCollisionId: id of Collision property pertaining to characters
        * characterGravityId: id of Gravity property pertaining to characters
        * characterLocationInSpaceId: id of LocationInSpace property pertaining to characters
        * possibleObjectCollisionIds: ids of Collision properties pertaining to objects that Character might collide with when moved
        */
        PlatformMovingCharacterHandler(std::map<int, Property*>* propertyMap, bool notifyWhileReplaying,
            std::vector<int> platformCollisionIds,
            int characterCollisionId, int characterGravityId, int characterLocationInSpaceId, 
            std::vector<int> possibleObjectCollisionIds);

        /*
        * Handles moving character if necessary based on the given platform movement.
        * 
        * e: platform movement event possibly leading to character movement
        */
        void onEvent(Event* e);
};

