#pragma once
#include "Event.h"
#include <map>
#include "Property.h"

/*
* Abstract class defining general behavior required for handling different event types.
*/
class EventHandler
{
	protected:
		/* map of all properties */
		std::map<int, Property*>* propertyMap;

		/* whether the EventManager should notify this handler of events when a replay is being played */
		bool notifyWhileReplaying;

	public:
		/*
		* Constructs an EventHandler with the given property map.
		* 
		* propertyMap: map of all properties
		* notifyWhileReplaying: whether to notify this handler of events when a replay is being played
		*/
		EventHandler(std::map<int, Property*>* propertyMap, bool notifyWhileReplaying);

		/*
		* Function to be defined in subclasses that performs different actions based on the type of the given event.
		* 
		* e: event to handle
		*/
		virtual void onEvent(Event* e) = 0;

		/*
		* Returns indicating whether the handler should be notified of events while a replay is being played.
		* 
		* returns: true if handler should be notified of events while replay is playing, false otherwise
		*/
		bool isSetToNotifyWhileReplaying();
};

