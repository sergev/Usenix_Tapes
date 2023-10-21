#include "desktop.h"
#include "dispatch.h"
#include <InterViews/world.h>
#include <InterViews/Xdefs.h>
#include <stdio.h>

extern int fork();
extern void close(int);
extern boolean Equal(const char*, const char*);
extern int strcmp(const char*, const char*);

char* program_name;
World* world;

void GiveHelp()
{
printf(".Xdefaults options (and their value if not specified in .Xdefaults):\n\n");

printf("iwm.logo:52-4+4       // sizes which are multiples of 13 work best\n");
printf("iwm.inverse:false     // inverts the window manager's output\n");
printf("iwm.lock:false        // whether screensaver works when Lock'ed\n");
printf("iwm.font:6x13p        // default is actually .InterViews.stdfont\n");
printf("iwm.ignorecaps:false  // if true, iwm will ignore the position of\n");
printf("                      // shift-lock (aka Caps) key\n");
printf("iwm.fork:true         // if one of your user-defined operations\n");
printf("                      // sends output to the terminal, set the\n");
printf("                      // value of fork to false\n\n");

printf("See the file /usr/local/lib/iwmrc.default for a description and\n");
printf("example of the ~/.iwmrc file which controls which operations are\n");
printf("activated by which buttons/menu entries.  If you do not have a\n");
printf("~/.iwmrc file, then /usr/local/lib/iwmrc.default will be used.\n");
}

void main (int argc, char* argv[]) {
    int i, pid;
    Desktop* v;
    boolean dofork = true;
    char* s;

    program_name = "iwm";

    world = new World(program_name);
    s = world->GetDefault("fork");
    if (s != nil) {
	if (Equal(s,"false")) {
	    dofork = false;
	} else if (!Equal(s,"true")) {
	    fprintf(stderr,"%s: .Xdefaults: fork must be true or false, `%s' is invalid\n", program_name, s);
	}
    }

    for (i = 1; i < argc; i++) {
	if (!strcmp(argv[i], "help")) {
	    GiveHelp();
	    exit(0);
	} else {
	    fprintf(stderr, "Usage: %s [help]\n", program_name);
	    exit(1);
	}
    }

    v = new Desktop(world);

    if (dofork) {
	pid = fork();
	if (pid < 0) {
	    fprintf(stderr, "iwm: cannot fork\n"); exit(1);
	} else if (pid == 0) {
	    /* child closes fds to let parent exit */
	    close(0); close(1); close(2);
	} else {
	    /* parent just exits and lets child takeover */
	    exit(0);
	}
    }

    v->Run();
    delete v;
}
