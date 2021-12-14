#include "EventManager.h"
#include <stdexcept>
#include <iostream>

EventManager* EventManager::manager = nullptr;

EventManager::EventManager()
{
	this->timeline = nullptr;
	this->replayTimeline = nullptr;
	this->playingReplay = false;
	this->replayStartTime = 0.f;
}

void EventManager::setTimeline(Timeline* timeline)
{
	this->timeline = timeline;
}

void EventManager::setEventTypes(std::vector<std::string> eventTypes)
{
	int eventTypesNum = eventTypes.size();
	for (int i = 0; i < eventTypesNum; i++)
	{
		eventRegistrations.insert(std::pair<std::string, std::vector<EventHandler*>>(eventTypes[i], {}));
	}
}

void EventManager::registerForEvent(std::string eventType, EventHandler* eventHandler)
{
	try
	{
		eventRegistrations.at(eventType).push_back(eventHandler);
	}
	catch (const std::out_of_range oor)
	{
		std::cerr << "No such event type: " + eventType << std::endl;
	}
}

void EventManager::unregisterForEvent(std::string eventType, EventHandler* eventHandler)
{
	try
	{
		std::vector<EventHandler*> handlers = eventRegistrations.at(eventType);
		int handlersNum = handlers.size();
		for (int i = 0; i < handlersNum; i++)
		{
			if (handlers[i] == eventHandler)
			{
				handlers.erase(handlers.begin() + i);
				break;
			}
		}
	}
	catch (const std::out_of_range oor)
	{
		std::cerr << "No such event type: " + eventType << std::endl;
	}
}

void EventManager::raise(Event* e)
{
	//if playing replay, queue in event queue
	if (playingReplay)
	{
		replayEventQueue.push(e);
	}
	//otherwise, queue in regular queue
	else
	{
		eventQueue.push(e);
	}
}

void EventManager::handleEvents()
{
	if (playingReplay)
	{
		//std::cout << "Handling replay events" << std::endl;
		//std::cout << std::to_string(replayTimeline->getTime()) << std::endl;
		//std::cout << std::to_string(replayEventQueue.top()->getTimestamp()) << std::endl;
		//std::cout << replayEventQueue.top()->getType() << std::endl;

		//retrieve absolute time to use in determining which events to handle
		float currentTime = getReplayTime() * replayTimeline->getTic();

		//while queue is not empty and highest priority event has timestamp less than or equal to current time, continue handling events
		while (!replayEventQueue.empty() && (replayEventQueue.top()->getTimestamp() + (replayStartTime / replayTimeline->getTic()) < (currentTime / replayTimeline->getTic())
			|| fabs((currentTime / replayTimeline->getTic()) - (replayEventQueue.top()->getTimestamp() + (replayStartTime / replayTimeline->getTic()))) < FLT_EPSILON))
		{

			//get list of handlers registered for this event
			std::vector<EventHandler*> registeredHandlers = eventRegistrations.at(replayEventQueue.top()->getType());
			int registeredHandlersNum = registeredHandlers.size();

			//call function for each handler
			for (int i = 0; i < registeredHandlersNum; i++)
			{
				//only notify handler if handler is set to notify while replaying
				if (registeredHandlers[i]->isSetToNotifyWhileReplaying())
				{
					registeredHandlers[i]->onEvent(replayEventQueue.top());
				}
			}

			//remove event from queue
			replayEventQueue.pop();
		}

		//check if replay is finished
		if (replayEventQueue.empty())
		{
			setPlayingReplay(false);

		}
	}
	else
	{
		//retrieve time to use in determining which events to handle
		float currentTime = getCurrentTime();

		//while queue is not empty and highest priority event has timestamp less than or equal to current time, continue handling events
		while (!eventQueue.empty() && (eventQueue.top()->getTimestamp() < currentTime
			|| fabs(currentTime - eventQueue.top()->getTimestamp()) < FLT_EPSILON))
		{
			//get list of handlers registered for this event
			std::vector<EventHandler*> registeredHandlers = eventRegistrations.at(eventQueue.top()->getType());
			int registeredHandlersNum = registeredHandlers.size();

			//call function for each handler
			for (int i = 0; i < registeredHandlersNum; i++)
			{
				registeredHandlers[i]->onEvent(eventQueue.top());
			}

			//remove event from queue
			eventQueue.pop();

		}
	}
}

float EventManager::getCurrentTime()
{
	return timeline->getTime();
}

bool EventManager::isPlayingReplay()
{
	return playingReplay;
}

void EventManager::setPlayingReplay(bool playingReplay)
{
	this->playingReplay = playingReplay;

	//if we are now playing replay, record the replay start time unrelative to the speed
	if (this->playingReplay)
	{
		replayStartTime = replayTimeline->getTime() * replayTimeline->getTic();
	}
}

void EventManager::setReplayTimeline(Timeline* replayTimeline)
{
	this->replayTimeline = replayTimeline;
}

float EventManager::getReplayTime()
{
	return replayTimeline->getTime();
}

void EventManager::setReplaySpeed(float ticSize)
{
	replayTimeline->setTic(ticSize);
}

bool EventManager::isRegistered(std::string eventType, EventHandler* eventHandler)
{
	try
	{
		std::vector<EventHandler*> handlers = eventRegistrations.at(eventType);
		int handlersNum = handlers.size();
		for (int i = 0; i < handlersNum; i++)
		{
			if (handlers[i] == eventHandler)
			{
				return true;
			}
		}
	}
	catch (const std::out_of_range oor)
	{
		std::cerr << "No such event type: " + eventType << std::endl;
	}

	return false;
}
