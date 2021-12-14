#include "EventHandler.h"

EventHandler::EventHandler(std::map<int, Property*>* propertyMap, bool notifyWhileReplaying)
{
	this->propertyMap = propertyMap;
	this->notifyWhileReplaying = notifyWhileReplaying;
}

bool EventHandler::isSetToNotifyWhileReplaying()
{
	return notifyWhileReplaying;
}
