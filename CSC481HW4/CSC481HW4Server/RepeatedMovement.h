#pragma once
#include "Property.h"
#include <SFML/Graphics.hpp>
#include <map>
#include "Timeline.h"

/*
* Property defining an object's repeated pattern of movement. An object with this property will move from position to position in
* a regular list, possibly pausing at some positions. The speed of the object will depend on the object's defined velocity and
* the speed of the timeline set for this property.
* 
* Property Dependencies: LocationInSpace
*/
class RepeatedMovement :
    public Property
{
    public:
        /*
        * Class representing a position assigned to an object to help define its pattern of movement.
        * Contains designated x and y values, as well as a value to indicate whether the position is one that
        * should be paused at.
        */
        class RepeatedMovementPosition : public sf::Vector2f
        {
        private:
            /* value indicating whether the object assigned to this position should pause upon reaching it */
            bool pauseHere;

        public:
            /*
            * Constructs a RepeatedMovementPosition with the given values.
            *
            * x: the horizontal value of the position (in pixels)
            * y: the vertical value of the position (in pixels)
            * pauseHere: whether the position is one that should be paused at
            */
            RepeatedMovementPosition(float x, float y, bool pauseHere);

            /*
            * Sets whether the position should be paused at.
            *
            * pauseHere: whether to pause at the position
            */
            void setPauseHere(bool pauseHere);

            /*
            * Returns whether the position should be paused at.
            *
            * returns: true if the position should be paused at, false otherwise
            */
            bool getPauseHere();
        };

    private:
        /* LocationInSpace corresponding to each object */
        std::map<int, int> locationsInSpace;

        /* timeline to use in timing movements and pauses */
        Timeline* timeline;

        /* set of positions for each object defining its pattern of movement */
        std::map<int, std::vector<RepeatedMovement::RepeatedMovementPosition>> objectMovementPatterns;

        /* time for each object's last movement (not relative to tic size) */
        std::map<int, float> timesAtLastMovement;

        /* pause durations for each object (in units of timeline) */
        std::map<int, float> pauseDurations;

        /* whether each object is paused */
        std::map<int, bool> paused;

        /* time elapsed for each object since it was first paused */
        std::map<int, float> timesSincePause;

        /* velocity of each object (amount to move per time unit) */
        std::map<int, float> velocities;

        /* target index in list of positions for each object */
        std::map<int, int> targetIndices;

        /* last change in x for each object */
        std::map<int, float> lastXChanges;

        /* last change in y for each object */
        std::map<int, float> lastYChanges;

    public:
        /*
        * Constructs a RepeatedMovement property instance with the given values.
        * 
        * id: id of the property
        * propertyMap: map of all properties
        * timeline: timeline to use in timing movements and pauses
        */
        RepeatedMovement(int id, std::map<int, Property*>* propertyMap, Timeline* timeline);

        /*
        * Adds an object to this property with the given values. The object must have a corresponding LocationInSpace used
        * to retrieve its shape. The object will start unpaused and heading toward the second position in the list 
        * (assumed to be starting at the first). 
        * 
        * objectId: id of object
        * locationInSpaceId: id of LocationInSpace corresponding to object
        * movementPattern: pattern of movement defined by list of positions
        * pauseDuration: number of time units to spend on each pause
        * velocity: amount to move per time unit
        */
        void addObject(int objectId, int locationInSpaceId, std::vector<RepeatedMovementPosition> movementPattern, 
            float pauseDuration, float velocity);

        /*
        * Removes the object with the given id from this property.
        * 
        * id: id of object to remove
        */
        void removeObject(int id);

        /*
        * Returns the last change in x for the object with the given id.
        * 
        * id: id of object
        * 
        * returns: last change in x for object
        */
        float getLastXChange(int id);

        /*
        * Returns the last change in y for the object with the given id.
        * 
        * id: id of object
        * 
        * returns: last change in y for object
        */
        float getLastYChange(int id);

        /*
        * Updates the position of the object with the given id based on the current time.
        * 
        * id: id of object
        */
        void updatePosition(int id);

        /*
        * Updates the position of all objects in this property based on the current time.
        */
        void updatePositionOfAll();
};

