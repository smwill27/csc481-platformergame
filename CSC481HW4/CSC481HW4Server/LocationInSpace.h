#pragma once
#include "Property.h"
#include <SFML/Graphics.hpp>
#include <map>

/*
* Property defining an object's location in space (i.e. that an object has a particular location, size, etc.). The object's
* location in space is represented using an sf::Shape. New objects can be added and their shapes retrieved.
* 
* Property Dependencies: none
*/
class LocationInSpace :
    public Property
{
    private:
        /* sfml shapes pertaining to each object with this property */
        std::map<int, sf::Shape*> objectShapes;

    public:
        /*
        * Constructs a LocationInSpace property instance with the given values.
        * 
        * id: id of the property
        * propertyMap: map of all properties
        */
        LocationInSpace(int id, std::map<int, Property*>* propertyMap);

        /*
        * Adds an object to this property using the given shape to indicate its location in space.
        * 
        * id: id of object to add
        * shape: shape used to represent object's location in space (size, position, etc.)
        */
        void addObject(int id, sf::Shape* shape);

        /*
        * Removes an object from this property to undefine its location in space.
        * 
        * id: id of object to remove
        */
        void removeObject(int id);

        /*
        * Returns a pointer to the shape corresponding to the given object.
        * 
        * id: id of object to retrieve shape of
        * 
        * returns: pointer to shape representing object's location in space
        */
        sf::Shape* getObjectShape(int id);

        /*
        * Returns property's list of objects. Allows for obtaining ids/locations of individual objects.
        *
        * returns: list of objects
        */
        std::vector<int> getObjects();
};

