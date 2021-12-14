#pragma once
#include "Property.h"

/*
* Property defining an object's ability to collide with other objects.
* 
* Property Dependencies: LocationInSpace
*/
class Collision :
    public Property
{
    private:
        /* LocationInSpace corresponding to each object */
        std::map<int, int> locationsInSpace;

    public:
        /*
        * Constructs a Collision property with the given values.
        * 
        * id: id of property
        * propertyMap: map of all properties
        */
        Collision(int id, std::map<int, Property*>* propertyMap);

        /*
        * Adds an object to the property with the given values.
        * 
        * objectId: id of object
        * locationInSpaceId: id of LocationInSpace corresponding to object
        */
        void addObject(int objectId, int locationInSpaceId);

        /*
        * Removes the given object from the property.
        * 
        * objectId: id of object
        */
        void removeObject(int objectId);

        /*
        * Returns a pointer to the property's map of object locations in space. Allows different Collision instances to check for
        * collisions between their sets of objects.
        * 
        * returns: pointer to map of locations in space
        */
        std::map<int, int>* getLocationsInSpace();

        /*
        * Returns property's list of objects. Allows different Collision instances to check for
        * collisions between their sets of objects.
        * 
        * returns: list of objects
        */
        std::vector<int> getObjects();

        /*
        * Returns indicating whether the given object is colliding with any of the objects defined in the given collision property.
        * 
        * objectId: id of object
        * collision: id of Collision defining objects to check for collision with
        * 
        * returns: true if the object is colliding with any of the given objects, and false otherwise
        */
        bool isCollidingWithAny(int objectId, int collision);

        /*
        * Returns indicating whether the first object is colliding with the second object. The second object must be defined
        * in the given collision property.
        * 
        * objectId: id of first object
        * otherObjectId: id of second object
        * collision: id of Collision defining second object
        * 
        * returns: true if the two objects are colliding, and false otherwise
        */
        bool isCollidingWith(int objectId, int otherObjectId, int collision);
};

