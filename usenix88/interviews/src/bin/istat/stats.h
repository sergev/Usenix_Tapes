/*
 *  IStat
 */

#ifndef stat_h
#define stat_h

#include <InterViews/frame.h>

class Stats {
    char * hostname;

    long oneMinLoad;
    long fiveMinLoad;
    int userCPU;
    int niceCPU;
    int systemCPU;
    int idleCPU;
    int diskTransfers;
    int netInPackets;
    int netOutPackets;
    long timestamp;
public:
    Stats( const char * host );

    float load1Av;
    float load5Av;
    float userCPUFract;
    float niceCPUFract;
    float systemCPUFract;
    float diskIORate;
    float netIORate;

    void NewStats();		    
};

class Log;
class Pattern;
class IStat : public Frame {
    char bannersample[ 256 ];
    char bannerformat[ 256 ];
    Stats * st;
    Log * Loads;
    Log * CPU;
    Log * IO;
    Pattern * gray;
public:
    IStat( const char * host, int delay, int width=0, int height=0 );
    void Delete();
    void Handle(Event&);
    void Run();
    void Tick();
};

#endif
