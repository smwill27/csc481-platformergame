#include "ReplayHandler.h"
#include <iostream>

ReplayHandler::ReplayHandler(std::map<int, Property*>* propertyMap, bool notifyWhileReplaying, std::vector<std::string> eventsToRecord)
	: EventHandler(propertyMap, notifyWhileReplaying)
{
	this->eventsToRecord = eventsToRecord;
	this->recordingReplay = false;
	this->timeAtRecordingStart = 0.f;
}

void ReplayHandler::onEvent(Event* e)
{
	//if we get event to start recording a replay and are not already doing so, start recording a replay
	if (e->getType() == ClientServerConsts::REPLAY_RECORDING_START_EVENT && !recordingReplay)
	{
		//set that we are recording
		recordingReplay = true;

		//register for each of the events we need to record
		int eventsToRecordNum = eventsToRecord.size();
		for (int i = 0; i < eventsToRecordNum; i++)
		{
			EventManager::getManager()->registerForEvent(eventsToRecord[i], this);
		}

		//get time at which we started recording
		timeAtRecordingStart = EventManager::getManager()->getCurrentTime();
		//std::cout << "***Time at Recording Start: " + std::to_string(timeAtRecordingStart) << std::endl;
	}
	//if we get event to stop recording a replay and we are recording, stop recording
	else if (e->getType() == ClientServerConsts::REPLAY_RECORDING_STOP_EVENT && recordingReplay)
	{
		//std::cout << "Stopping recording" << std::endl;
		//set that we are no longer recording
		recordingReplay = false;

		//unregister for recording events
		int eventsToRecordNum = eventsToRecord.size();
		for (int i = 0; i < eventsToRecordNum; i++)
		{
			EventManager::getManager()->unregisterForEvent(eventsToRecord[i], this);
		}

		//std::cout << "Unregistered for events" << std::endl;

		//for now, just print out values
		/*
		std::cout << "Time At Recording Start: " + std::to_string(timeAtRecordingStart) << std::endl;
		int recordedEventsNum = recordedEvents.size();
		for (int i = 0; i < recordedEventsNum; i++)
		{
			std::cout << recordedEvents[i]->getType() + ": " + std::to_string(recordedEvents[i]->getTimestamp()) << std::endl;
		}
		*/

		//set initial replay speed
		EventManager::getManager()->setReplaySpeed(e->getArgument(0).argValue.argAsFloat);

		//set event manager to playing replay
		EventManager::getManager()->setPlayingReplay(true);

		//std::cout << "Queueing replay events" << std::endl;
		//queue and then clear all recorded events, adjusting times to fit replay timeline
		while (!recordedEvents.empty())
		{
			recordedEvents[0]->setTimestamp(recordedEvents[0]->getTimestamp() - timeAtRecordingStart);
			EventManager::getManager()->raise(recordedEvents[0]);
			recordedEvents.erase(recordedEvents.begin());
		}

		//std::cout << "Finished queueing replay events" << std::endl;
		//std::cout << std::to_string(recordedEvents.size()) << std::endl;

		//register for user input events so we can handle changes in replay speed
		if (!EventManager::getManager()->isRegistered(ClientServerConsts::USER_INPUT_EVENT, this))
		{
			EventManager::getManager()->registerForEvent(ClientServerConsts::USER_INPUT_EVENT, this);
		}
	}
	//while replaying, we might get an event to change speed of replay
	else if (e->getType() == ClientServerConsts::USER_INPUT_EVENT)
	{
		//if we are no longer playing replay, unregister for this event type
		if (!EventManager::getManager()->isPlayingReplay() 
			&& EventManager::getManager()->isRegistered(ClientServerConsts::USER_INPUT_EVENT, this))
		{
			EventManager::getManager()->unregisterForEvent(ClientServerConsts::USER_INPUT_EVENT, this);
		}
		//otherwise, see if we need to change replay speed
		else
		{
			int keyType = e->getArgument(1).argValue.argAsInt;

			if (keyType == ClientServerConsts::ONE_KEY)
			{
				EventManager::getManager()->setReplaySpeed(2.f);
			}
			else if (keyType == ClientServerConsts::TWO_KEY)
			{
				EventManager::getManager()->setReplaySpeed(1.f);
			}
			else if (keyType == ClientServerConsts::THREE_KEY)
			{
				EventManager::getManager()->setReplaySpeed(0.5f);
			}
		}
	}
	//if we are recording replay, check to see if we need to record this event
	else if (recordingReplay)
	{
		int eventsToRecordNum = eventsToRecord.size();
		for (int i = 0; i < eventsToRecordNum; i++)
		{
			if (eventsToRecord[i] == e->getType())
			{
				//recorded event should have time it was actually being handled so that in replay events are raised in correct order
				Event* recordedEvent = new Event(EventManager::getManager()->getCurrentTime(), e->getType(), e->getArguments());
				recordedEvents.push_back(recordedEvent);
				break;
			}
		}
	}
}
