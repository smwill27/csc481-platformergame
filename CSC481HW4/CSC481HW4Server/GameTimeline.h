#pragma once
#include "Timeline.h"
class GameTimeline :
    public Timeline
{
private:
    /* time of the anchor when this timeline first starts */
    float startTime;

    /* timeline whose measurement of time anchors this timeline */
    Timeline* anchor;

public:
    /*
    * Constructs the timeline initially unpaused with the given tic size and anchor.
    */
    GameTimeline(float tic, Timeline* anchor);

    /*
    * Function that returns the total time elapsed based on the measurement of the timeline's anchor
    * and tic size (minus any time spent paused). If the timeline is currently paused, a negative value is
    * returned to indicate no updates based on the timeline should be made.
    *
    * returns: total elapsed time, or a negative value if paused
    */
    float getTime();
};



