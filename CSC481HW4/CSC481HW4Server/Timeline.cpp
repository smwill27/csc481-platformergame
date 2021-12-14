#include "Timeline.h"

Timeline::Timeline(float tic)
{
	this->tic = tic;
	paused = false;
	lastPausedTime = 0.f;
	elapsedPauseTime = 0.f;
}

void Timeline::pause()
{
	lastPausedTime = getTime();
	paused = true;
}

void Timeline::unpause()
{
	paused = false;
	elapsedPauseTime += (getTime() - lastPausedTime);
}

void Timeline::setTic(float tic)
{
	this->tic = tic;
}

float Timeline::getTic()
{
	return tic;
}

bool Timeline::isPaused()
{
	return paused;
}

float Timeline::getElapsedPauseTime()
{
	return elapsedPauseTime;
}
