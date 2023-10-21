#include "con.h"

int xnow, ynow;
int ITTY[3], PTTY[3], OUTF;
float HEIGHT 6.0, WIDTH 6.0, OFFSET 0.0;
int xscale, xoffset, yscale;
float botx 0., boty 0., obotx 0., oboty 0.;
float scalex 1., scaley 1.;

openpl ()
{
	int reset();
		xnow = ynow = 0;
		OUTF = fileno(stdout);
		printf("\r");
		gtty(OUTF, ITTY);
		signal (2, &reset);
		PTTY[0] = ITTY[0];
		PTTY[1] = ITTY[1];
		PTTY[2] = ITTY[2] & (~ 020);  /* don't map lf */
		PTTY[2] =| 001; /* no delays */
		/* initialize constants */
		xscale  = 4096./(HORZRESP * WIDTH);
		yscale = 4096 /(VERTRESP * HEIGHT);
		xoffset = OFFSET * HORZRESP;
		return;
}

openvt(){
openpl();
}
