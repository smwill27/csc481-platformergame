#pragma once
#include "Property.h"

/*
* Property defining an object's ability to be respawned at a designated spawn point.
* 
* Property Dependencies: LocationInSpace
*/
class Respawning :
    public Property
{
    public:
        /*
        * Class representing a SpawnPoint that defines a point at which an object can be respawned. A SpawnPoint corresponds to
        * some game object with a defined LocationInSpace.
        */
        class SpawnPoint
        {
            private:
                /* id of object corresponding to this spawn point */
                int objectId;

                /* id of LocationInSpace corresponding to object */
                int locationInSpaceId;

            public:
                /*
                * Default constructor to allow use in map. Sets values arbitrarily.
                */
                SpawnPoint();

                /*
                * Constructs a SpawnPoint with the given values.
                * 
                * objectId: id of corresponding object
                * locationInSpaceId: id of LocationInSpace corresponding to object
                */
                SpawnPoint(int objectId, int locationInSpaceId);

                /*
                * Returns id of object used to represent this spawn point.
                * 
                * returns: object id
                */
                int getObjectId();

                /*
                * Returns id of LocationInSpace corresponding to object.
                * 
                * returns: LocationInSpace id
                */
                int getLocationInSpaceId();
        };

    private:
        /* LocationInSpace corresponding to each object */
        std::map<int, int> locationsInSpace;

        /* current spawn point set for each object */
        std::map<int, SpawnPoint> spawnPoints;

    public:
        /*
        * Constructs a Respawning property with the given values.
        * 
        * id: id of property
        * propertyMap: map of all properties
        */
        Respawning(int id, std::map<int, Property*>* propertyMap);

        /*
        * Adds an object to the property with the given values.
        * 
        * objectId: id of object
        * locationInSpaceId: id of LocationInSpace corresponding to object
        * spawnPoint: current spawn point for object
        */
        void addObject(int objectId, int locationInSpaceId, SpawnPoint spawnPoint);

        /*
        * Removes the given object from the property.
        * 
        * objectId: id of object to remove
        */
        void removeObject(int objectId);

        /*
        * Sets the object's spawn point to the given value.
        * 
        * objectId: id of object
        * spawnPoint: new spawn point
        */
        void setSpawnPoint(int objectId, SpawnPoint spawnPoint);

        /*
        * Respawns the object at its current spawn point.
        * 
        * objectId: id of object
        */
        void respawn(int objectId);

        /*
        * Respawns all objects at their current spawn points.
        */
        void respawnAll();
};

