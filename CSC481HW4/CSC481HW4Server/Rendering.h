#pragma once
#include "Property.h"
#include <SFML/Graphics.hpp>

/*
* Property defining an object's ability to be rendered to the window.
* 
* Property Dependencies: LocationInSpace
*/
class Rendering :
    public Property
{
    private:
        /* window to draw objects to */
        sf::RenderWindow* window;

        /* location in space corresponding to each object (used to retrieve object's shape) */
        std::map<int, int> locationsInSpace;

    public:
        /*
        * Constructs a Rendering property instance with the given values.
        * 
        * id: id of the property
        * propertyMap: map of all properties
        * window: window to draw to
        */
        Rendering(int id, std::map<int, Property*>* propertyMap, sf::RenderWindow* window);

        /*
        * Adds an object to this property to indicate it can be rendered. Object must have corresponding LocationInSpace to
        * retrieve its shape from.
        * 
        * objectId: id of object to add
        * locationInSpaceId: id of LocationInSpace corresponding to object
        */
        void addObject(int objectId, int locationInSpaceId);

        /*
        * Removes an object from this property.
        * 
        * id: id of object to remove
        */
        void removeObject(int id);

        /*
        * Draws the object with the given id to the window. Object must have a defined LocationInSpace used to retrieve object's
        * shape.
        * 
        * objectId: id of object to draw
        */
        void drawObject(int objectId);

        /*
        * Draws all objects to the window. Objects must have defined LocationInSpace instances used to retrieve their shapes.
        */
        void drawAllObjects();
};

