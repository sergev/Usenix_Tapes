/*
 * istat : a workstation statistics display for InterViews
 */

#include <InterViews/world.h>
#include <InterViews/frame.h>
#include <InterViews/sensor.h>

#include <CC/stdio.h>
#include "stats.h"

extern void gethostname(char*, int);
extern void exit(int);

World * world;
IStat * istat;

char hostname[64];
int delay = 5;
int xpos = 0;
int ypos = 0;
int width = 0;
int height = 0;

main(int argc, char* argv[]) {

    int p1, p2;
    char * curarg;
    for (int i = 1; i < argc; i++) {
	curarg = argv[i];
	if (sscanf(curarg, "delay=%d", &p1) == 1) {
	    delay = p1;
	} else if (sscanf(curarg, "pos=%d,%d", &p1, &p2) == 2) {
	    xpos = p1;
	    ypos = p2;
	} else if (sscanf(curarg, "size=%d,%d", &p1, &p2) == 2) {
	    width = p1;
	    height = p2;
	} else if (sscanf(curarg, "host=%s", hostname) == 1) {
	    /* nothing else to do */
	} else {
	    fprintf(stderr, "%s: unexpected argument '%s'\n", argv[0], curarg);
	    fprintf(stderr, "usage: %s %s\n",
		argv[0], "[pos=#,#] [size=#,#] [delay=#] [host=name]"
	    );
	    exit(1);
	}
    }
    if (hostname[0] == '\0') {
	gethostname(hostname, sizeof(hostname));
    }

    world = new World;

    if ( width==0 && height==0 ) {
	istat = new IStat( hostname, delay );
    } else {
	istat = new IStat( hostname, delay, width, height );
    }
    if ( xpos == 0 && ypos == 0 ) {
	world->Insert( istat );
    } else {
	world->Insert( istat, xpos, ypos );
    }

    istat->Run();

    world->Remove( istat );
    delete istat;
    delete world;
}
