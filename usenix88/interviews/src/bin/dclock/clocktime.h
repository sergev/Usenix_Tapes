/*
 * dclock timing information
 */

#ifndef time_h
#define time_h

struct TimeVal {
    long sec;
    long usec;
};

class Clock {
    struct TimeVal gmt;
    long nextMinute;	    // next minute in gmt
public:
    Clock();
    int NextTick();	    // seconds until next minute
    void GetTime(char *date, int& hours, int &mins, int &secs);
};

#endif
