#pragma once
#include "Timeline.h"
#include <chrono>
class RealTimeline :
	public Timeline
{
private:
	/* Time at which the timeline started. */
	std::chrono::steady_clock::time_point startTime;

public:
	/*
	* Constructs the timeline with the given tic size and initially unpaused.
	*/
	RealTimeline(float tic);

	/*
	* Function that returns the total time elapsed based on the timeline's measurement of real time in seconds
	* and tic size (minus any time spent paused). If the timeline is currently paused, a negative value is
	* returned to indicate no updates based on the timeline should be made.
	*
	* returns: total elapsed time, or a negative value if paused
	*/
	float getTime();
};

