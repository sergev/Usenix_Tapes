/*
 * xy11 incremental plotter driver
 */

#include "../hd/param.h"
#include "../hd/conf.h"
#include "../hd/user.h"

#define XYADDR  0172554
#define XYPRI   40
#define XYLWAT  50
#define XYHWAT  100
#define DONE    0200
#define IENABLE 0100

#define OPEN    0400
#define ASLEEP  01000
#define PAGE    02000
#define LIMITY  04000
#define POSX    1
#define NEGX    2
#define POSY    4
#define NEGY    010
#define PENDN   020
#define PENUP   040

#define FORM    0200
#define JAML    0201
#define JAMR    0202
#define SETY    0203

int     xypage[2]  { 850, 1000 };       /* 8.5" x 11"  @ 100 steps/in */

struct {
	int cc; 
	int cf; 
	int cl; 
	int flag; 
	int mx;         /* maximum x */
	int cx;         /* current x */
	int cy;         /* relative y */
	int ay;         /* actual y */
	} xy11;

int     xyok;

struct {
	int xycsr;      /* command/status register */
	int xybuf;      /* data buffer */
	};

xyopen(dev,flag){ 
	if(!xyok)
		if(tkword(XYADDR)){ u.u_error= ENXIO; return; }
		else xyok++;
	if(dev.d_minor>1){ u.u_error= ENXIO; return; }
	if(xy11.flag&OPEN){ u.u_error= EIO; return; }
	xy11.mx= 0;
	xy11.cx= 0;
	xy11.cy= 0;
	xy11.ay= 0;
	xy11.flag=| OPEN | (dev.d_minor? 0:PAGE);
	xycanon(PENUP);
	XYADDR->xycsr =| IENABLE; }

xyclose(dev,flag){
	xycanon(PENUP);
	if(xy11.flag&PAGE) xycanon(FORM);
	xy11.flag=& ASLEEP; }

xywrite(){
	register int c;
	while((c=cpass()) >= 0) xycanon(c); }

xycanon(c){
	register int dx, dy;
	switch(c){
		case FORM:
			if(xypage[0]>0 && xy11.mx){
				if(dx= xy11.mx % xypage[0]) xy11.mx=+ xypage[0] -dx;
				dx= xy11.mx - xy11.cx;
				dy= -xy11.cy;
				c= POSY;
				if(dy<0){ dy= -dy; c= NEGY; }
				while(dx-->0) xyoutput(POSX | (dy-->0? c: 0));
				while(dy-->0) xyoutput(c); }
			break;
		case JAML:
			c= POSY; xy11.ay= xypage[1]; goto jam;
		case JAMR:
			c= NEGY; xy11.ay= 0;
		jam:    for(dy=xypage[1]; dy-->0; ) xyoutput(c);
			if((xy11.flag&PAGE) && xypage[1]>0) xy11.flag=| LIMITY;
		case SETY:
			xy11.cy= 0;
			break;
		default:
			if(c & (c>>1) & 025) return;            /* motion conflict */
			if(c & PENUP & xy11.flag) return;       /* pen motion unnecessary */
			if(c & PENDN & xy11.flag) return;
			xy11.flag.lobyte= c;
			if(c & POSX) xy11.cx++;
			else if(c & NEGX) xy11.cx--;
			if(c & POSY){
				xy11.cy++;
				if((xy11.flag&LIMITY) && (xy11.ay<0 || xy11.ay>=xypage[1]))
					c=& ~POSY;
				xy11.ay++; }
			else if(c & NEGY){
				xy11.cy--;
				if((xy11.flag&LIMITY) && (xy11.ay<=0 || xy11.ay>xypage[1]))
					c=& ~NEGY;
				xy11.ay--; }
			if(xy11.cx>xy11.mx) xy11.mx= xy11.cx;
			if(c) xyoutput(c);
	}       } 
		
xyoutput(c){
	if(!xyok) return;
	spl4();
	while(xy11.cc>XYHWAT){
		xy11.flag=| ASLEEP;
		sleep(&xy11,XYPRI); }
	putc(c, &xy11);
	xystart();
	spl0(); }

xystart(){
	register int c;
	while((XYADDR->xycsr&DONE) && (c=getc(&xy11)) >= 0)
		XYADDR->xybuf= c;
	} 

xyint(){
	xystart();
	if(xy11.cc<=XYLWAT && (xy11.flag&ASLEEP)){
		xy11.flag=& ~ASLEEP;
		wakeup(&xy11);
	}       } 
