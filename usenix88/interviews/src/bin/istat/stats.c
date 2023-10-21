/*
 * IStat and Stats classes
 */

#include "stats.h"
#include "log.h"
#include "indicator.h"

#include <InterViews/box.h>
#include <InterViews/glue.h>
#include <InterViews/frame.h>
#include <InterViews/border.h>
#include <InterViews/shape.h>
#include <InterViews/sensor.h>
#include <InterViews/paint.h>
#include <InterViews/painter.h>

#ifdef vax
#   include "rstat.h"
#else
#   include <rpcsvc/rstat.h>
#endif

extern int rstat( const char *, statstime * );
extern void sprintf( char *, const char * ... );

Stats::Stats( const char * host ) {
    hostname = (char*)host;
    load1Av = load5Av = 0.0;
    userCPUFract = niceCPUFract = systemCPUFract = 0.0;
    diskIORate = netIORate = 0.0;
    timestamp = 0;
}

void Stats::NewStats() {
    struct statstime st;
    (void) rstat(hostname, &st);

    int diskXfer = 0;
    for (int i = 0; i<DK_NDRIVE; ++i ) {
	diskXfer += st.dk_xfer[i];
    }
    if ( timestamp != 0 && timestamp != st.curtime.tv_sec ) {
	load1Av = float(st.avenrun[0]) / float(1<<8);
	load5Av = float(st.avenrun[1]) / float(1<<8);
	int totalCPU =
	    (st.cp_time[0] - userCPU)	    // want the *difference*
	    +(st.cp_time[1] - niceCPU)
	    +(st.cp_time[2] - systemCPU)
	    +(st.cp_time[3] - idleCPU);
	userCPUFract = float(st.cp_time[0] - userCPU) / float(totalCPU);
	niceCPUFract =  float(st.cp_time[1] - niceCPU) / float(totalCPU);
	systemCPUFract = float(st.cp_time[2] - systemCPU) / float(totalCPU);
	diskIORate =
	    float(diskXfer - diskTransfers)
	    / float(st.curtime.tv_sec - timestamp);
	netIORate =
	    float(st.if_ipackets-netInPackets + st.if_opackets-netOutPackets)
	    / float(st.curtime.tv_sec - timestamp);
    }
    oneMinLoad = st.avenrun[0];		    // save for next call
    fiveMinLoad = st.avenrun[1];
    userCPU = st.cp_time[0];
    niceCPU = st.cp_time[1];
    systemCPU = st.cp_time[2];
    idleCPU = st.cp_time[3];
    diskTransfers = diskXfer;
    netInPackets = st.if_ipackets;
    netOutPackets = st.if_opackets;
    timestamp = st.curtime.tv_sec;
}

boolean IOScaleDown(Indicator*i) {
    Trace * t1 = i->GetLog()->GetTrace(1);
    Trace * t2 = i->GetLog()->GetTrace(2);
    if ( t2 != nil && t2->Last() > i->GetScale() ) {
	return true;
    } else if ( t1 != nil && t1->Last() > i->GetScale() ) {
	return true;
    } else {
	return false;
    }
}

boolean IOScaleUp(Indicator*i) {
    Trace * t1 = i->GetLog()->GetTrace(1);
    Trace * t2 = i->GetLog()->GetTrace(2);
    if ( t2 != nil && t2->Last() < i->GetScale()/10.0 ) {
	return true;
    } else if ( t1 != nil && t1->Last() < i->GetScale()/10.0 ) {
	return true;
    } else {
	return false;
    }
}

float LoadScales[] = {
    0.1, 0.2, 0.5, 1.0, 2.0, 5.0, 10.0, 20.0, 50.0, 100.0
};
int LoadScaleCount = sizeof(LoadScales)/sizeof(float);

float IOScales[] = {
    20, 40, 60, 80, 100
};
int IOScaleCount = sizeof(IOScales)/sizeof(float);

IStat::IStat( const char * host, int delay, int width, int height )
: ( new Painter(stdpaint), new Interactor, 2) {

    input = new Sensor(onoffEvents);
    input->Catch(KeyEvent);
    input->CatchTimer( delay,0 );

    gray = new Pattern( 0x5a5a );
    output->SetPattern( gray );

    sprintf( bannersample, "%s 00.0", host );
    sprintf( bannerformat, "%s %%4.1f", host );
    st = new Stats( host );

    Loads = new Log( new Trace );
    CPU = new Log( new Trace, new Trace, new Trace );
    IO = new Log( new Trace, new Trace );

    Readout * CurrentLoad = new Readout( Loads, bannersample, bannerformat );
    Graph * LoadHistory = new Graph( Loads );
    LoadHistory->Scaling( LoadScaleCount, 0, LoadScales );
    Bar * CurrentCPU = new Bar( CPU, 8 );
    Pointer * CurrentIO = new Pointer( IO );
    CurrentIO->Scaling( IOScaleCount, 0, IOScales );
    CurrentIO->Scalers( &IOScaleUp, &IOScaleDown );

    HBox * inner = new HBox;
    inner->Insert( new HGlue( output, 1, 0, 0 ) );
    inner->Insert( CurrentIO );
    inner->Insert( new HGlue( output, 1, 0, 0 ) );
    inner->Insert( LoadHistory );
    inner->Insert( new HGlue( output, 1, 0, 0 ) );
    inner->Insert( CurrentCPU );
    inner->Insert( new HGlue( output, 1, 0, 0 ) );

    HBox * banner = new HBox;
    banner->Insert( new HGlue( output, 1, 0, 0 ) );
    banner->Insert( CurrentLoad );
    banner->Insert( new HGlue( output, 1, 0, hfil ) );

    VBox * contents = new VBox;
    contents->Insert( banner );
    contents->Insert( new VGlue( output, 1, 0, 0 ) );
    contents->Insert( inner );
    contents->Insert( new VGlue( output, 1, 0, 0 ) );

    Insert( contents );
    if ( width!=0 && height!=0 ) {
	shape->Rect(width,height);
    }
}

void IStat::Delete() {
    delete Loads;
    delete CPU;
    delete IO;
    delete gray;
}

void IStat::Run() {
    Event e;
    for (;;) {
	Read(e);
	Handle(e);
	if ( e.target == nil ) {
	    break;
	}
    }
}

void IStat::Handle(Event&e) {
    switch( e.eventType ) {
    case TimerEvent:
	Tick(); break;
    case OnEvent:
	output->SetPattern( solid );
	Redraw( 0, 0, xmax, ymax );
	break;
    case OffEvent:
	output->SetPattern( gray );
	Redraw( 0, 0, xmax, ymax );
	break;
    case KeyEvent:
	if ( e.target == this && e.len > 0 && e.keystring[0] == 'q' ) {
	    e.target = nil;
	}
	break;
    default:
	if ( e.target != nil ) {
	    e.target->Handle(e);
	}
	break;
    }
}

void IStat::Tick() {
    st->NewStats();

    Loads->GetTrace(1)->Data( st->load1Av );
    Loads->Tick();

    CPU->GetTrace(1)->Data( st->userCPUFract );
    CPU->GetTrace(2)->Data( st->niceCPUFract );
    CPU->GetTrace(3)->Data( st->systemCPUFract );
    CPU->Tick();

    IO->GetTrace(1)->Data( st->diskIORate );
    IO->GetTrace(2)->Data( st->netIORate );
    IO->Tick();
}
