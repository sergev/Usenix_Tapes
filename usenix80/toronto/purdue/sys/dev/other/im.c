#
#include "../param.h"
#include "../conf.h"
/* kludgy dh11 8-bit output for imlac loads */
#define IMADDR	0160060
#define BUFZ	514

struct  {
	int	imcsr;
	int	imnxch;
	int	imlpr;
	int	imcar;
	int	imbcr;
	int	imbar;
};
	char imbuf[BUFZ];
	char	*imin;
	char	*imout;
	char	*imlim;
	int	imopenf;
	int	imeof;	/* eof set after 10 sec of no input data */
	int	imdly;	/* counter for 10 sec delay */

imopen()
{
	extern imok();
	static int	savcsr;
	imdly = imeof = 0;
	spl6();
	imin = imout = &imbuf[0];
	imlim = &imbuf[BUFZ-1];
	savcsr = IMADDR->imcsr;
	IMADDR->imcsr = 0;
	IMADDR->imlpr = 033503; /* 9600 baud 8 bits, one stop bit */
	IMADDR->imcsr = savcsr&0147677;
	spl0();
	imopenf++;
	timeout(&imok,0,2);
}
imclose()
{
	imopenf = 0;
	IMADDR->imcsr =| 030100;
}
imwrite()
{
	register int savcsr;
	register int i;
	extern imck();
loop:	for(i=0; ((i<BUFZ) && ((savcsr=cpass()) >=0)); i++)
		imbuf[i] = savcsr;
	if(i != 0) {
		spl5();
		savcsr = IMADDR->imcsr;
		IMADDR->imcsr = 0;	/* select line 0, no interrupts */
		IMADDR->imlpr = 033507; /* 9600 baud, 8-bits */
		IMADDR->imcar = &imbuf[0];
		IMADDR->imbcr = -i;
		IMADDR->imbar =| 1;	/* start up output */
		timeout(&imck,0,2);
		while(IMADDR->imbar&01)
			sleep(&imwrite,-3);
		IMADDR->imcsr = savcsr;
		spl0();
		if(i == BUFZ) goto loop;
	}
}
imread()
{
	register c;
	imeof = 0;	/* set after 10 sec of no data */
	while(((c=imgc()) != -1) && ((passc(c)) >=0));
}
imgc()
{
	register c;
	while((imin == imout) && !imeof)
		sleep(&imread, 9);
	if(imeof)
		return(-1);
	c = *imout++;
	if(imout == imlim)
		imout = &imbuf[0];
	imdly = 0;	/* clear eof timeout */
	return(c&0377);
}
imck()
{
	if(IMADDR->imbar&01)
		timeout(&imck,0,2);	/* still busy, try later */
	else
		wakeup(&imwrite);	/* xfer done, wakeup job */
}
imok()
{
	register c;
	if(imopenf) {
		while((c = IMADDR->imnxch) < 0) {
			if(c&077400)
				continue;
			*imin++ = c;
			if(imin == imlim)
				imin = &imbuf[0];
		}
		if(imin != imout)
			wakeup(&imread);
		timeout(&imok,0,2);
		if(imdly++ >= 300) {
			imdly = 0;
			imeof++;
			wakeup(&imread);
		}
	}
	else
		imin = imout = &imbuf[0];
}
