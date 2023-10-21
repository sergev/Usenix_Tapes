#include <stdio.h>
#include <signal.h>
#include "ascii.h"

FILE	*XMODEMOUT;


static int xoutstate = 0;

#define	XMINIT		0

#define	XMONAK		1
#define	XMORREC		2
#define	XMOSOH		3
#define	XMORESP		4
#define	XMOEOT		5
#define	XMOWFACK	6



static int xoutrec = 0;
static int xoutbyte = 0;
static int xoutcheck = 0;
static int xouterr = 0;
static char xoutdata[129];

main(ac,av)
int ac;
char *av[];
{
	int j, bailout();
	char ccc;
	int w;
	if(ac!=2) {
		fprintf(stderr,"usage: xmput filename\n");
		exit(1);
	}

	XMODEMOUT = fopen(av[1],"r");
	if(XMODEMOUT==0) {
		perror("xmput");
		exit(1);
	}

	xoutrec = 0;
	xoutbyte = xoutcheck = 0;
	xoutstate = XMONAK;
	ttybinary();

	signal(SIGALRM, bailout);

        alarm(30);
	for(;;) {
		switch(xoutstate) {
		case XMONAK:
			j = ttyr();
			if(j!=NAK) continue;
		case XMORREC:
			xoutstate = XMOSOH;
			w = xmread(XMODEMOUT,xoutdata);
			if(w==0) xoutstate=XMOEOT;
			xoutrec = xoutrec+1;
			continue;
		case XMOSOH:
			alarm(30);
			ttyw(SOH);
			ttyw(xoutrec&255);
			ttyw((~xoutrec)&255);
			xoutcheck = 0;
			for(w=0; w<128; w++) {
				ttyw(xoutdata[w]);
				xoutcheck =(xoutcheck+xoutdata[w])&255;
			}
			ttyw(xoutcheck);
			xoutstate = XMORESP;
			continue;
		case XMORESP:
			j = ttyr();
			if(j==NAK) {
				xoutstate = XMOSOH;
				continue;
			}
			if(j==ACK) {
				xoutstate = XMORREC;
				continue;
			}
			continue;
		case XMOEOT:
			ttyw(EOT);
			xoutstate = XMOWFACK;
			continue;
		case XMOWFACK:
			j = ttyr();
			if(j==ACK) {
				fclose(XMODEMOUT);
				XMODEMOUT = NULL;
				ttynormal();
				exit(0);
			}
			continue;
		}
	}
}

xmread(fn,buf)
FILE *fn;
char *buf;
{
	int rsiz;
	int rrs;
	static int carryflag = 0;
	static int carrydata = 0;
	if(carryflag == -1) {
		carryflag = 0;
		return 0;
	}

	rrs = 0;
	for(rsiz=0; rsiz<128; rsiz++) {
		if(carryflag) {
			buf[rsiz] = carrydata;
			carryflag = 0;
			continue;
		}
		rrs = getc(fn);
		if(rrs==EOF) {
			buf[rsiz++] = SUB;
			carryflag = -1;
			break;
		}
		if(rrs=='\n') {
			buf[rsiz] = CR;
			carrydata = LF;
			carryflag = 1;
			continue;
		}
		buf[rsiz] = rrs;
	}
	while(rsiz<128) buf[rsiz++] = 0;
	return 1;
}

bailout()
{
  ttynormal();
  exit(0);
}

