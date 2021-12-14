#pragma once
#include "Property.h"
#include "Timeline.h"

/*
* Property defining the gravity of an object, which means that it will fall down if not standing on another object or jumping up.
* 
* Property Dependencies: LocationInSpace, Collision
*/
class Gravity :
    public Property
{
    private:
        /* timeline used for timing movements due to falling down */
        Timeline* timeline;

        /* LocationInSpace corresponding to each object*/
        std::map<int, int> locationsInSpace;

        /* Collision corresponding to each object */
        std::map<int, int> collisions;

        /* values indicating whether objects are jumping up */
        std::map<int, bool> jumpingUpValues;

        /* values indicating whether objects are falling down */
        std::map<int, bool> fallingDownValues;

        /* values indicating whether objects are currently on an object (and thus neither jumping or falling) */
        std::map<int, bool> standingOnObjectsValues;

        /* list of collisions to check for each object to determine whether object has stopped falling */
        std::map<int, std::vector<int>> collisionsToCheck;

        /* object each object is standing on (if any) */
        std::map<int, int> objectsStoodOn;

        /* time spent for each object on movement (used for falling down) */
        std::map<int, float> velocities;

    public:
        /*
        * Constructs a Gravity property using the given values.
        * 
        * id: id of property
        * propertyMap: map of all properties
        * timeline: timeline to use in timing falling movements
        */
        Gravity(int id, std::map<int, Property*>* propertyMap, Timeline* timeline);

        /*
        * Adds an object to the property with the given values. The object should initially be standing on something and neither
        * jumping up nor falling down.
        * 
        * objectId: id of object
        * locationInSpaceId: id of corresponding LocationInSpace
        * collisionId: id of corresponding Collision
        * collisionsToCheck: list of Collision properties to check for this object when determining whether it is standing
        * objectStandingOn: object that the added object is currently standing/resting on
        * 
        */
        void addObject(int objectId, int locationInSpaceId, int collisionId, 
            std::vector<int> collisionsToCheck, int objectStandingOn, float velocity);

        /*
        * Removes the given object from the property.
        * 
        * objectId: id of object
        */
        void removeObject(int objectId);

        /*
        * Returns indicating whether the object is jumping up.
        * 
        * objectId: id of object
        * 
        * returns: true if object is jumping up, false otherwise
        */
        bool isJumpingUp(int objectId);

        /*
        * Sets whether the object is jumping up.
        * 
        * objectId: id of object
        * jumpingUp: whether object is jumping
        */
        void setJumpingUp(int objectId, bool jumpingUp);

        /*
        * Returns whether the object is falling down.
        * 
        * objectId: id of object
        * 
        * returns: true if object is falling, false otherwise
        */
        bool getFallingDown(int objectId);

        /*
        * Returns whether the object is standing on another object.
        * 
        * objectId: id of object
        * 
        * returns: true if object is standing/resting, false otherwise
        */
        bool isStanding(int objectId);

        /*
        * Returns id of object that the given object is standing on (if any).
        * 
        * objectId: id of object
        * 
        * returns: id of object that given object is standing on (or -1 if none).
        */
        int objectStandingOn(int objectId);

        /*
        * Processes any gravity adjustments for the given object. An object may either be jumping, falling down, starting to fall down,
        * no longer falling down, or standing on another object (and thus at rest).
        * 
        * objectId: id of object
        */
        void processGravity(int objectId);

        /*
        * Processes any gravity adjustments for all objects. An object may either be jumping, falling down, starting to fall down,
        * no longer falling down, or standing on another object (and thus at rest).
        */
        void processGravityForAll();
};

