#pragma once
#include "Property.h"
#include "Timeline.h"
#include <SFML/Graphics.hpp>

/*
* Property defining an object's ability to be moved by player input. An object with this property is assigned movements tied to
* key inputs. These can either be ordinary movements, or jumps which proceed gradually after being activated. Values can be set
* to check whether a movement would cause collision with other objects before making the movement.
* 
* Property Dependencies: LocationInSpace, Collision
*/
class PlayerDirectedMovement :
    public Property
{
    public:
        /*
        * Class representing a movement to occur for an object when a certain input key is pressed. This movement may happen all
        * at once or be a jump that happens gradually.
        */
        class MovementOnInput : public sf::Vector2f
        {
            private:
                /* input key that triggers this movement */
                sf::Keyboard::Key key;

                /* whether this movement is considered a jump (and so should be handled differently) */
                bool isJump;

            public:
                /*
                * Default constructor to allow use in map. Sets values arbitrarily.
                */
                MovementOnInput();

                /*
                * Constructs a MovementOnInput with the given values.
                *
                * x: amount to move in x direction
                * y: amount to move in y direction
                * key: input key that triggers movement
                * isJump: whether to treat this movement as a jump
                */
                MovementOnInput(float x, float y, sf::Keyboard::Key key, bool isJump);

                /*
                * Sets the input key to the given value.
                * 
                * key: input key that triggers movement
                */
                void setKey(sf::Keyboard::Key key);

                /*
                * Returns the input key.
                * 
                * returns: input key
                */
                sf::Keyboard::Key getKey();

                /*
                * Sets whether this movement is considered a jump.
                * 
                * isJump: whether movement is a jump
                */
                void setIsJump(bool isJump);

                /*
                * Returns indicating whether movement is a jump.
                * 
                * returns: whether movement is a jump
                */
                bool getIsJump();
        };

    private:
        /* timeline used to time movements*/
        Timeline* timeline;

        /* set of movements for objects based on certain input */
        std::map<int, std::vector<MovementOnInput>> movementsOnInput;

        /* values indicating whether objects are currently jumping up*/
        std::map<int, bool> jumpingUpValues;

        /* values indicating whether objects are currently falling down (used to help coordinate Gravity) */
        std::map<int, bool> fallingDownValues;

        /* velocity of each object (time units spent on each movement) */
        std::map<int, float> velocities;

        /* LocationInSpace corresponding to each object */
        std::map<int, int> locationsInSpace;

        /* Collision corresponding to each object */
        std::map<int, int> collisions;

        /* whether to check for collision before moving on each object */
        std::map<int, bool> collisionStatuses;

        /* list of collision properties to check for collision with before making a movement on each object */
        std::map<int, std::vector<int>> collisionsToCheck;

        /* jumps currently being performed by each object (if any) */
        std::map<int, MovementOnInput> jumpsBeingPerformed;

        /* amount in y direction of jump already travelled by each object (if performing jump) */
        std::map<int, float> amountsYOfJumpPerformed;

        /* amount in x direction of jump already travelled by each object (if performing jump) */
        std::map<int, float> amountsXOfJumpPerformed;

    public:
        /*
        * Constructs a PlayerDirectedMovement with the given values.
        * 
        * id: id of property
        * propertyMap: map of all properties
        * timeline: timeline to use in timing movements
        */
        PlayerDirectedMovement(int id, std::map<int, Property*>* propertyMap, Timeline* timeline);

        /*
        * Adds an object to this property with the given values. The object must have a corresponding LocationInSpace and Collision. If
        * set to check for collisions, any corresponding Collisions should also be indicated.
        * 
        * objectId: id of object
        * locationInSpaceId: id of corresponding LocationInSpace
        * collisionId: id of corresponding Collision
        * movements: movements tied to key inputs for object
        * velocity: amount of time spent on each movement
        * collisionStatus: whether to check for collisions before moving
        * collisions: list of Collisions used in checking for collisions
        */
        void addObject(int objectId, int locationInSpaceId, int collisionId, std::vector<MovementOnInput> movements, float velocity,
            bool collisionStatus, std::vector<int> checkedCollisions);

        /*
        * Removes the object with the given id from the property.
        * 
        * objectId: id of object
        */
        void removeObject(int objectId);

        /*
        * Returns indicating whether object is jumping up.
        * 
        * objectId: id of object
        * 
        * returns: true if object is jumping, false otherwise
        */
        bool isJumpingUp(int objectId);

        /*
        * Returns indicating whether object is falling down.
        * 
        * objectId: id of object
        * 
        * returns: true if object is falling down, false otherwise
        */
        bool isFallingDown(int objectId);

        /*
        * Sets whether the given object is falling down. Helps coordinate operations with Gravity.
        * 
        * objectId: id of object
        * fallingDown: whether object is falling down
        */
        void setFallingDown(int objectId, bool fallingDown);

        /*
        * Processes any movement for the object based on the given input key or the current status of the object.
        * 
        * objectId: id of object
        * input: key pressed
        */
        void processMovement(int objectId, sf::Keyboard::Key input);

        /*
        * Processes any movement for all objects based on the given input key or the current status of the objects.
        * 
        * input: key pressed
        */
        void processMovementForAll(sf::Keyboard::Key input);

        /*
        * Processes movement for the object based on its current status.
        * 
        * objectId: id of object to process movement for
        */
        void processMovement(int objectId);

        /*
        * Process movement for all objects based on their current statuses.
        */
        void processMovementForAll();
};

