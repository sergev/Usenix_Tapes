#include <stdio.h>
#include <signal.h>
#include "ascii.h"

FILE	*XMODEMIN;

static int xinstate = 0;

#define	XMINIT		0

#define	XMISOHI0	1
#define	XMISOHI1	2
#define	XMISOH		3
#define	XMIBLKN		4
#define	XMICBLKN	5
#define	XMIDATA		6
#define	XMICHECK	7
#define	XMIACK		8

static int xinrec = 0;
static int xinbyte = 0;
static int xincheck = 0;
static int xinerr = 0;
static char xindata[129];
static int xmastal = 0;
static int xmascnt = 0;		/* # of NAK's sent */

main(ac,av)
int ac;
char *av[];
{
	int xmast();
	int w;
	int j;
	char ccc;

	if(ac!=2) {
		fprintf(stderr,"Usage: xmget filename\n");
		exit(1);
	}

	XMODEMIN = fopen(av[1],"w");
	if(XMODEMIN==NULL) {
		perror("xmget");
		exit(1);
	}

	ttybinary();
	signal(SIGALRM,xmast);

	xinrec = xinbyte = xincheck = 0;
	xinstate = XMISOHI0;
	xmastal = 0;
	xmascnt = 0;
	ttyw(NAK);
	alarm(10);
	for(;;) {
		j = ttyr();
		switch(xinstate) {
		case XMISOHI0:
		case XMISOHI1:
			if(j==SOH) {
				alarm(0);
				xinstate = XMIBLKN;
				continue;
			}
			if(j==EOT) {
				alarm(0);
				fclose(XMODEMIN);
				ttynormal();
				exit(0);
			}
			if(xinstate == XMISOHI0 || xmastal!=0) {
				if(xmascnt>5) {
					alarm(0);
					fclose(XMODEMIN);
					ttynormal();
					fprintf(stderr,"Timeout\n");
					exit(1);
				}
				ttyw(NAK);
				xmastal = 0;
				alarm(10);
				xinstate = XMISOHI1;
			}
		case XMISOH:
			if(j==SOH) {
				xinstate = XMIBLKN;
				continue;
			}
			if(j==EOT) {
				ttyw(ACK);
				fclose(XMODEMIN);
				alarm(0);
				ttynormal();
				exit(0);
			}
			continue;
		case XMIBLKN:
			xinerr = 0;
			xinrec = j;
			xinstate = XMICBLKN;
			continue;
		case XMICBLKN:
			xincheck = 0;
			j = (~j)&255;
			if(j!=xinrec) xinerr = 1;
			xinstate = XMIDATA;
			xinbyte = 0;
			continue;
		case XMIDATA:
			xincheck = (xincheck + j)&255;
			xindata[xinbyte++] = j;
			if(xinbyte==128)
				xinstate = XMICHECK;
			continue;
		case XMICHECK:
			if((j&255)!=(xincheck&255)) xinerr = -1;
			if(xinerr) {
				ttyw(NAK);
			} else {
				ttyw(ACK);
				xmwrite(XMODEMIN,xindata);
			}
			xinstate = XMISOH;
			continue;
		}
	}
}

xmwrite(fn,buf)
FILE *fn;
char *buf;
{
	int i;
	for(i=0; i<128; i++) {
		if(buf[i]==CR) continue;
		if(buf[i]==LF) {
			putc('\n',fn);
			continue;
		}
		if(buf[i]==SUB) return;
		putc(buf[i],fn);
	}
}

xmast(i)
{
	int xmast();
	xmastal = 1;
	++xmascnt;
	signal(i,xmast);
}
