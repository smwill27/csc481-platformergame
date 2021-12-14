#include "RealTimeline.h"
#include <chrono>

RealTimeline::RealTimeline(float tic) : Timeline(tic)
{
    startTime = std::chrono::steady_clock::now();
}

float RealTimeline::getTime()
{
    //if timeline is paused, return negative value
    if (isPaused())
    {
        return -1.f;
    }

    //get current time
    std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();

    //calculate total elapsed time (subtracting out paused time)
    float elapsedTime = std::chrono::duration<float>(currentTime - startTime).count();
    elapsedTime -= getElapsedPauseTime();

    //return total elapsed time divided by current tic size
    return elapsedTime / getTic();
}
