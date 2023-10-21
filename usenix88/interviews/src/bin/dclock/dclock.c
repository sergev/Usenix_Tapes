/*
 * dclock - another (digital) clock for InterViews
 */

char rcsAuth[]	= "$Author: pan $";
char rcsDate[]	= "$Date: 87/11/02 15:12:45 $";
char rcsRev[]	= "$Revision: 1.8 $";
char rcsState[]	= "$State: Exp $";

#include "dclock.h"
#include "dface.h"
#include <stdio.h>

extern void exit(int);
extern int fork();
extern void close( int );
extern int setpriority (int, int, int);
extern int getpid ();
extern int getuid ();

void DoArgs( int argc, char *argv[] );

DFace *TheClock;

World *world;

main( int argc, char * argv[] ) {
    DoArgs( argc, argv );

#ifdef BACKGROUND
    if ( ! Debug ) {
	int pid;
	if ( (pid=fork()) == -1 ) {			// fork off new process
	    fprintf( stderr, "%s: fork failed\n", argv[0] );
	    exit( 1 );
	} else if ( pid != 0 ) {			// parent -- exit
	    exit( 0 );
	}						// else child
	if (Nice > 20) {
	    Nice = 20;
	} else if (Nice < -20) {
	    Nice = -20;
	}
	if (Nice < 0 && getuid() != 0) {
	    fprintf( stderr,
		"%s: can't set nice below 0, if not super user\n", argv[0]
	    );
	    exit(1);
	}
	if (setpriority(0, getpid(), Nice) != 0) {
	    // error, but don't do anything.
	}
	close(0); close(1); close(2);			// so parent can die
    }
#endif
    world = new World();
    InitData();

    switch (CreateMode) {
	case SWEPT:
	    TheClock = new DFace(
		ShowDate, ShowBorder, ShowTime, TimeMode, Invert
	    );
	    world->Insert(TheClock);
	    break;
	case PLACED:
	    TheClock = new DFace(
		ShowDate, ShowBorder, ShowTime, TimeMode, Invert, Width, Height
	    );
	    world->Insert(TheClock);
	    break;
	case DEFINED:
	    TheClock = new DFace(
		ShowDate, ShowBorder, ShowTime, TimeMode, Invert, Width, Height
	    );
	    world->Insert(TheClock, XPos, YPos);
	    break;
    }

    TheClock->Run();

    delete TheClock;
}

void DoArgs( int argc, char * argv[] ) {
    int i, j, p1, p2;
    char* curarg;

    for (i = 1; i < argc; i++) {
	curarg = argv[i];
	if (sscanf(curarg, "size=%d,%d", &p1, &p2) == 2) {
	    if (CreateMode != DEFINED) {
		CreateMode = PLACED;			// size specified
	    }
	    Width = p1;
	    Height = p2;
	} else if (sscanf(curarg, "pos=%d,%d", &p1, &p2) == 2) {
	    CreateMode = DEFINED;			// position specified
	    XPos = p1;
	    YPos = p2;
	} else if (sscanf(curarg, "-s%d", &p1) == 1) {
	    SlantPC = p1;
	} else if (sscanf(curarg, "-t%d", &p1) == 1) {
	    ThickPC = p1;
	} else if (sscanf(curarg, "-f%d", &p1) == 1) {
	    FadeRate = p1;
	} else if (sscanf(curarg, "nice=%d", &p1) == 1) {
	    Nice = p1;
	} else if (curarg[0] == '-') {
	    for (j = 1; curarg[j] != '\0'; j++) {
		switch (curarg[j]) {
		    case 'D':
			Debug = true;
			break;
		    case 'c':
			TimeMode = CIVIL;		// the default
			break;
		    case 'm':
			TimeMode = MIL;			// 24 hour
			break;
		    case 'j':
			JohnsFlag = true;		// tail on 9
			break;
		    case 'i':
			Invert = true;
			break;
		    case 'd':
			ShowDate = false;
			break;
		    case 's':
			SlantPC = 0;
			break;
		    case 'f':
			FadeRate = 0;
			break;
		    case 'b':				//show date/time border
			ShowBorder = true;
			break;
		    case 'T':
			ShowTime = false;		// date only
			break;
		    default:
			fprintf(stderr,
			    "%s: unrecognized flag '%c'\n", argv[0], curarg[j]
			);
			break;
		}
	    }
	} else {
	    fprintf(stderr, "%s: unexpected argument '%s'\n", argv[0], curarg);
	    fprintf(stderr,
		"usage: %s [-cmjidbT] [-s#] [-t#] [-f#] [pos=#,#] [size=#,#]\n",
		argv[0]
	    );
	    exit(1);
	}
    }
}
