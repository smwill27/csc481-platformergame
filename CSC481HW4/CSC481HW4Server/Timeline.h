#pragma once
/*
* An abstract class that defines shared functionality for timelines with different representations of time.
* A timeline is an explicit representation of time that enables performing actions based on changes in time and is able
* to be paused and unpaused. The timeline's tic size can also be changed to speed up or slow down the timeline.
*/
class Timeline
{
private:
	/* number of time units of the measurement used that correspond to one time unit for this timeline */
	float tic;
	/* whether the timeline is currently paused */
	bool paused;
	/* the last time at which the timeline was set to paused */
	float lastPausedTime;
	/* the total elapsed time that the timeline has spent paused */
	float elapsedPauseTime;

public:
	/*
	* Constructs a timeline with the given tic size that is unpaused to start.
	*/
	Timeline(float tic);

	/*
	* Function to be defined by subclasses that returns the total time elapsed based on the timeline's measurement
	* and tic size (minus any time spent paused). If the timeline is currently paused, a negative value is
	* returned to indicate no updates based on the timeline should be made.
	*
	* returns: total elapsed time, or a negative value if paused
	*/
	virtual float getTime() = 0;

	/*
	* Pauses the timeline.
	*/
	void pause();

	/*
	* Unpauses the timeline.
	*/
	void unpause();

	/*
	* Sets the tic size to the indicated value.
	*
	* tic: tic size to set
	*/
	void setTic(float tic);

	/*
	* Returns the current tic size.
	*
	* returns: current tic size
	*/
	float getTic();

	/*
	* Returns whether the timeline is currently paused.
	*
	* returns: true if timeline is paused, false otherwise
	*/
	bool isPaused();

	/*
	* Returns the total elapsed time that the timeline has spent paused.
	*
	* returns: total elapsed time spent paused
	*/
	float getElapsedPauseTime();
};
