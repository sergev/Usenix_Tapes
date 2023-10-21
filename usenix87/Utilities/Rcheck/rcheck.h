#include <stdio.h>
#include <pwd.h>
#include <utmp.h>
#include <sys/types.h>
#include <sys/dir.h>
#define MAXUSERS 64
#define MAXHOSTS 20
#define NAMELEN 8
char hostname [32], allperson[MAXUSERS][10], allhost[MAXHOSTS][32],
	alltty[MAXUSERS][10];
