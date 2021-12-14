#include "GameTimeline.h"

GameTimeline::GameTimeline(float tic, Timeline* anchor) : Timeline(tic)
{
    this->anchor = anchor;
    startTime = anchor->getTime();
}

float GameTimeline::getTime()
{
    //if currently paused, return negative value
    if (isPaused())
    {
        return -1.f;
    }

    //get current time and calculate elapsed time (minus paused time)
    float currentTime = anchor->getTime();
    float elapsedTime = currentTime - startTime;
    elapsedTime -= getElapsedPauseTime();

    //return elapsed time divided by tic size
    return elapsedTime / getTic();
}
