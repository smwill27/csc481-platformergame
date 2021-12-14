#include <stdlib.h>
#include <vector>
#include <string>
#include <map>

/*
* An abstract class representing a property used to define game objects. A property contains a list of game objects with the instance of the
* property (as ids), a unique id, and a pointer to the map for all property instances.
*/
#pragma once
class Property
{
	private:
		/* id of this property */
		int id;

	protected:

		/* ids of game objects exhibiting this property */
		std::vector<int> objects;

		/* map of all properties (used in coordinating property operations) */
		std::map<int, Property*>* propertyMap;

		/*
		* Constructs a Property with the given id and map of all properties.
		* 
		* id: id for this property
		* propertyMap: map of all properties (used in coordinating operations between properties)
		*/
		Property(int id, std::map<int, Property*>* propertyMap);

	public:
		/*
		* Returns indicating whether the given object has this property.
		* 
		* id: id of object to check for
		* 
		* returns: true if object has this property, false otherwise
		*/
		bool hasObject(int id);

		/*
		* Returns this property's id.
		* 
		* returns: this property's id
		*/
		int getId();
};

