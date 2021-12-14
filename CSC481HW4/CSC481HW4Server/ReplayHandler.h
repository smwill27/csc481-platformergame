#pragma once
#include "EventHandler.h"
#include "EventManager.h"
#include "ClientServerConsts.h"

/*
* An EventHandler that handles recording and playing replays.
*/
class ReplayHandler :
    public EventHandler
{
    private:
        /* list of event types to record when recording replay */
        std::vector<std::string> eventsToRecord;

        /* whether the handler is currently recording a replay */
        bool recordingReplay;

        /* list of events recorded for replaying */
        std::vector<Event*> recordedEvents;

        /* time at which we started recording (used to adjust times of events while replaying) */
        float timeAtRecordingStart;

    public:
        /*
        * Constructs a ReplayHandler with the given values.
        * 
        * propertyMap: map of all properties
        * notifyWhileReplaying: whether to notify this handler of events when a replay is being played
        * eventsToRecord: list of event types to record when recording replay
        */
        ReplayHandler(std::map<int, Property*>* propertyMap, bool notifyWhileReplaying, std::vector<std::string> eventsToRecord);

        /*
        * Handles managing recording and playing replays based on the given event.
        * 
        * e: event indicating actions to take regarding replays
        */
        void onEvent(Event* e);
};

