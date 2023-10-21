/*****************************************************
 * Calculator RPN                                    *
 * by Jerome M. Lang   17 January 1986               *
 *****************************************************/
/*
 * 1986-02-18:fixed display, it would get shrunk to where CR was pressed.
 * 1986-02-22:add window support, and make a desk accessory
 * 1986-02-22:split in two files
 */
#include <obdefs.h>
#include <gemdefs.h>
#include <define.h>
#include <osbind.h>
#include <stdio.h>

/* Choose one or the other */
#define ACCESSORY
/* #undef ACCESSORY */

#define NO_WINDOW (-1)
#define W_KIND	(NAME | CLOSER | FULLER | MOVER)
#define WINDNAME	"Calculator"

#ifdef ACCESSORY
#define MENUNAME	"  Calculator"
#endif

#define DEPTH		10	/* depth of the stack */
#define LEN_BUFF	31
#define OUT_LEN		15 /* num of char to be displayed  in window */
#define DISP_DEPTH	4 /* how many numbers in stack to display in window */
#define OUTPREC		6		/* six digits after dec. for output */

/*& extern	do_calc(), show_calc(), update();
/*& &*/
extern	int	gl_apid;

int	Window;
int	Handle;

#ifdef	ACCESSORY
int	MenuId;
#endif
int	Ox, Oy, Ow, Oh; /* outer size of window */
int	Ix, Iy, Iw, Ih; /* inner size of window */

int	Xdesk, Ydesk, Wdesk, Hdesk;
int	CharW, CharH;	/* character cell width and height */
int	Baseline;	/* Bottom line relative to baseline */
char	ressource[] = "CALCULAT.RSC";

double	stk[DEPTH+1];
char	Disp[DISP_DEPTH][OUT_LEN+1];
int	__atab[50]; /* try this for ftoa, worked so keep it there */

int	contrl[12];
int	intin[64];
int	ptsin[64];
int	intout[64];
int	ptsout[64];

/*
 * Object flag utility
 */

desel_obj( tree, which )
register OBJECT	*tree;
register int	which;
{

	tree[which].ob_state &= ~SELECTED;
}

/*
 * Stack manipulation
 */

double
pop()
{
	register int	i;
	double		ret;

	ret = stk[0];
	for( i=0; i<DEPTH; i++ ) stk[i]=stk[(long)(i+1)];
	return( ret );
}

push(item)
double item;
{
	register int	i;

	for( i=DEPTH; i>0; --i) stk[i]=stk[(long)(i-1)];
	stk[0] = item;
}


/* closeWindow - 
 * close calculator display window
 */
closeWindow()
{
	if (Window != NO_WINDOW)
	{
		wind_close(Window);
		wind_delete(Window);
		Window = NO_WINDOW;
	}
}

/* show_calc - 
 * 	create window and show the four last entry in the stack in a window 
 */
show_calc()
{
	int	ret;

	/* does it already exist */
	if( NO_WINDOW != Window )
		return;

	/* create that window */

	ret = wind_calc(0, W_KIND, Ix, Iy, Iw, Ih, &Ox, &Oy, &Ow, &Oh );
	Window = wind_create(W_KIND, Ox, Oy, Ow, Oh);
	if( 0 >= Window)
	{
		Window = NO_WINDOW;
		return;
	}
	ret = wind_set(Window, WF_NAME, WINDNAME);
	ret = wind_open(Window, Ox, Oy, Ow, Oh);

	show();
}

/*
 * main processing
 */

events()
{
	int	events;
	int	msgbuf[8];
	int	ret;
#ifndef ACCESSORY
	int	done = FALSE;
#endif

#ifdef ACCESSORY
	for(;;) /* desk accessory never return */
#else
	while(!done)
#endif
	{
		ret = evnt_mesag(msgbuf);
		switch (msgbuf[0])
		{
#ifdef ACCESSORY
		case AC_OPEN: 
			if (msgbuf[4] == MenuId)
			{
				closeWindow();
				do_calc();
				show_calc();
			}
			break;
		case AC_CLOSE: /* for future usage */
			if (msgbuf[3] == MenuId)
			{
				Window = NO_WINDOW;
			}
			break;
#endif
		case WM_CLOSE:
			if (msgbuf[3] == Window)
			{
				closeWindow();
#ifndef ACCESSORY
				done=TRUE;
#endif
			}
			break;
		case WM_MOVED:
			if (msgbuf[3] == Window)
			{
				Ox = msgbuf[4];
				Oy = msgbuf[5];
				Ow = msgbuf[6];
				Oh = msgbuf[7];

				wind_set( Window, WF_CURRXYWH, Ox, Oy, Ow, Oh);
				wind_calc(1, W_KIND, Ox, Oy, Ow, Oh,
						&Ix, &Iy, &Iw, &Ih);
				show();
			}
			break;
		case WM_NEWTOP:
		case WM_TOPPED:
			if (msgbuf[3] ==  Window)
				wind_set( Window, WF_TOP, 0, 0, 0, 0);
			break;
		case WM_FULLED:
			if (msgbuf[3] == Window)
			{
				closeWindow();
				do_calc();
				show_calc();
			}
			break;
		case WM_REDRAW:
			if (msgbuf[3] == Window)
			{
				wshow(msgbuf[4], msgbuf[5], 
					msgbuf[6], msgbuf[7]);
			}
			break;
		}
	}
}

main()
{

	appl_init();

#ifdef ACCESSORY
	MenuId = -1;
	MenuId = menu_register( gl_apid, MENUNAME );
#endif
	Window = NO_WINDOW;

	rsrc_load( ressource );

#ifndef ACCESSORY
	graf_mouse( ARROW, 0L);
#endif

	initMain();

	InitStack();
#ifndef ACCESSORY
	closeWindow();
	do_calc();
	show_calc();
#endif
	events();

#ifndef ACCESSORY
	appl_exit();
#endif
}

initMain()
{
	int	attrib[10];
	int	distances[5],effects[3];
	int	workIn[11], workOut[57];
	auto	int	dummy;
	int	i;

	/* find size of desk */
	wind_get( 0, WF_WORKXYWH, &Xdesk, &Ydesk, &Wdesk, &Hdesk );

	Handle=graf_handle(&dummy,&dummy,&dummy,&dummy);
	for( i=0; i<10; i++)
		workIn[i]=1;
	workIn[10]=0;
	v_opnvwk(workIn, &Handle, workOut);

	/* how big is character cell */
	vqt_attribute( Handle, attrib );
	CharW = attrib[8];
	CharH = attrib[9];

	vqt_fontinfo( Handle, &dummy, &dummy, distances, &dummy, effects);
	Baseline = distances[0];

	/* set fill pattern & color in order to clear window */
	vsf_color( Handle, 0 );

	/* initial position of the window */
	Ix = Xdesk+Wdesk/15;
	Iy = Ydesk+Hdesk/15;
	Iw = CharW*OUT_LEN;
	Ih = CharH*DISP_DEPTH;
}


	/* update one display */
update1(tree, display, stack, xdial, ydial, wdial, hdial)
register OBJECT *tree;
register int	display, stack;
int	xdial, ydial, wdial, hdial;
{
	char	buff[LEN_BUFF];

	etoa(stk[stack],buff,OUTPREC);
	wput(buff,stack);
	rstrcpy( ((TEDINFO *)tree[display].ob_spec)->te_ptext, buff);
	objc_draw(tree, display, 0, xdial, ydial, wdial, hdial);
}

/* wput - put in window buffer */
wput(str, number)
int	number;
char	*str;
{
	int	i;
	for (i=0; i<OUT_LEN; i++)
	{
		if( *str )
			Disp[number][i] = *str++;
		else
			break;
	}
	Disp[number][i] = EOS;
}

InitStack()
{
	register int i;

	for( i=0; i<=DEPTH; stk[i++] = 0.0 );
}

rstrcpy(x,y) /* copy y into x, formatted */
register char	*x,*y;
{
	register char *i;
	int	pos;

	for( i=x; *i!='\0'; *i++ =' ')
		;		/* blank field first */

	pos = strlen(x) - strlen(y); /* insert position */
	if (pos>=0) strcpy(&x[pos],y); /* y shorter, right just. in x */
		else strcpy(x,&y[(long)(-pos)]);
				/* destination shorter, trunc left part of src*/
}

strcpy(to,from)
register	char *to, *from;
{
	while( *to++ = *from++ )
		;
}

int
strlen(str)
register char	*str;
{
	register count;
	for(count=0;*str++;count++)
		;
	return(count);
}

cleartext(tree,which)
register	OBJECT	*tree;
int	which;
{
	register	int	i;
	register	int	len;
	register	char	*where;

	len = ((TEDINFO *)tree[which].ob_spec)->te_txtlen;
	where = ((TEDINFO *)tree[which].ob_spec)->te_ptext;
 	for( i = 0; i<len; i++)
		*where++ = ' ';
	*--where = EOS; /* length contains the EOS character */
}

clip( handle, x, y, w, h)
int	x,y,w,h;
int	handle;
{
	int	pxy[4];

	pxy[0] = x;
	pxy[1] = y;
	pxy[2] = x+w;
	pxy[3] = y+h;

	vs_clip( handle, 1, pxy );
}

eraseWindow( handle, x, y, w, h)
int	x,y,w,h;
int	handle;
{
	int	pxy[4];
	pxy[0] = x;
	pxy[1] = y;
	pxy[2] = w+x;
	pxy[3] = h+y;

	v_bar( handle, pxy );
}

/* show -
 * walk through the rectangle list of the window & show last four entry of
 * display 
 */
show()
{
	auto	int	x, y, w, h;

	if( NO_WINDOW == Window)
		return;

	graf_mouse(M_OFF, 0L);
	wind_update( BEG_UPDATE );

	wind_get(Window, WF_FIRSTXYWH, &x, &y, &w, &h);

	while( w!=0 && h!=0 )
	{
		wshow(x,y,w,h);
		wind_get(Window, WF_NEXTXYWH, &x, &y, &w, &h);
	}
	wind_update( END_UPDATE );
	graf_mouse(M_ON, 0L);
}

/* wshow - 
 * 	actual display of the stack
 */
wshow(x,y,w,h)
int	x,y,w,h;
{
	int	i;

	clip(Handle,x,y,w,h);
	eraseWindow(Handle, Ix, Iy, Iw, Ih);
	for( i = 0; i<DISP_DEPTH; i++)
	{
		v_gtext(Handle, Ix, Iy-Baseline-1+(i+1)*CharH, Disp[i]);
	}
}
