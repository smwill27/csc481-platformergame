#pragma once
#include <map>
#include <queue>
#include "Timeline.h"
#include "Event.h"
#include "EventHandler.h"

/*
* Singleton class used for managing an event system to coordinate functionality. Handlers can register to be notified when particular events
* occur.
*/
class EventManager
{
	protected:
		/*
		* Comparator comparing events by their timestamps.
		*/
		class Compare
		{
			public:
				bool operator() (Event* a, Event* b)
				{
					if (a->getTimestamp() > b->getTimestamp())
					{
						return true;
					}

					return false;
				}
		};

	private:
		/*
		* Constructs an EventManager with the given list of possible event types and timeline.
		*/
		EventManager();

		/* sole instance of EventManager */
		static EventManager* manager;

		/* map matching event types to their registered handlers */
		std::map<std::string, std::vector<EventHandler*>> eventRegistrations;

		/* queue of events organized by priority (lower timestamp = higher priority) */
		std::priority_queue<Event*, std::vector<Event*>, Compare> eventQueue;

		/* timeline used for timing handling of events */
		Timeline* timeline;

		/* whether the manager is currently playing a replay */
		bool playingReplay;

		/* timeline used for timing handling of events during a replay */
		Timeline* replayTimeline;

		/* queue of events for a replay */
		std::priority_queue<Event*, std::vector<Event*>, Compare> replayEventQueue;

		/* time at which replay started (unrelative to tic size so that it can be adjusted by current speed) */
		float replayStartTime;

	public:
		/*
		* Returns the sole instance of EventManager.
		*/
		static EventManager* getManager()
		{
			if (!manager)
			{
				manager = new EventManager();
			}

			return manager;
		}

		/*
		* Sets manager's timeline to the given value.
		* 
		* timeline: timeline to set
		*/
		void setTimeline(Timeline* timeline);

		/*
		* Creates lists for the given event types in event registration.
		* 
		* eventTypes: event types
		*/
		void setEventTypes(std::vector<std::string> eventTypes);

		/*
		* Registers the handler for the given event type.
		* 
		* eventType: type of event being registered for
		* eventHandler: handler registering for event
		*/
		void registerForEvent(std::string eventType, EventHandler* eventHandler);

		/*
		* Unregisters the handler for the given event type.
		* 
		* eventType: type of event being unregistered from
		* eventHandler: handler unregistering from event
		*/
		void unregisterForEvent(std::string eventType, EventHandler* eventHandler);

		/*
		* Raises the given event by adding it to the event queue.
		* 
		* e: event to raise
		*/
		void raise(Event* e);

		/*
		* Handles events based on the current time.
		*/
		void handleEvents();

		/*
		* Retrieves the current time according to the manager's timeline.
		* 
		* returns: current time
		*/
		float getCurrentTime();

		/*
		* Returns indicating whether the manager is currently playing a replay.
		* 
		* returns: true if manager is playing a replay, false otherwise
		*/
		bool isPlayingReplay();

		/*
		* Sets whether the manager is currently playing a replay.
		* 
		* playingReplay: value indicating whether the manager is playing a replay
		*/
		void setPlayingReplay(bool playingReplay);

		/*
		* Sets the manager's replay timeline to the given value.
		* 
		* replayTimeline: timeline to use for replays
		*/
		void setReplayTimeline(Timeline* replayTimeline);

		/*
		* Retrieves the current time according to the replay timeline.
		* 
		* returns: current time according to replay timeline
		*/
		float getReplayTime();

		/*
		* Sets the replay speed by setting the tic size of the replay timeline to the given value.
		* 
		* ticSize: new tic size for replay timeline (controls speed of replay)
		*/
		void setReplaySpeed(float ticSize);

		/*
		* Returns indicating whether the given handler is registered for the given event type.
		* 
		* eventType: type of event
		* eventHandler: handler to check
		* 
		* returns: true if handler is registered for the given event, and false otherwise
		*/
		bool isRegistered(std::string eventType, EventHandler* eventHandler);
};

