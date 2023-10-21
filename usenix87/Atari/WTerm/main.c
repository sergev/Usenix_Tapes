/*
 * Terminal emulator (with TEK emulation) using VDI routines
 *
 * This code is placed in the public domain.  Any section of this
 * may be copied and used as long as it is not for a commercial
 * product and reference is made to this program.  The author assumes
 * no liability for anything...  Best of luck and enjoy.
 *
 * Eric S Fraga
 * 1986 March 31
 *
 * Based on a window demo:
 *
 *****************************************************
 * Window demo					     *
 * by Jerome M. Lang   21 February 1986	             *
 * PD						     *
 *****************************************************/

#include <obdefs.h>
#include <gemdefs.h>
#include <define.h>
#include <osbind.h>
#include "wterm.h"

/*
 * main processing
 */

events()
{
	long lx;
	int events, event, keyhit, msgbuf[8], ret, done = FALSE;
	register int x, Curscnt, n;
	register char c;

	Cursor();
	Curscnt = CURSCNT;
	while(!done)
	{
		/*
		 * Check keyboard for characters.  Using event seems
		 * to miss lots of chars unfortunately.
		 */
		
		x = Cconis();	/* check status */
		if( 0 != x ) {	/* some chars waiting */
			lx = (long) Crawcin();		/* read from console */
			c = (char) (lx & 0x7fL);	/* actual character */
			if( NIL == c )
				done = special(lx);
			else {
				Cauxout(c);	/* send to serial port */
				if( TRUE == Half )
					proc( (char) c );
			}
		}
		
		/*
		 * Finally, check the serial port for stuff.
		 */
		
		x = Cauxis();
		if( -1 == x ) {		/* there's something there */
		  	if( Curscnt <= 0 )
				Cursor();	/* Disable cursor */
			n = 10;			/* read up to 10 chars */
			do {
				c = Cauxin();
				c = c & 0x7f;	/* get rid of parity */
				proc( (char) c );
			} while( n-->0 && (x=Cauxis()) == -1);
			Curscnt = CURSCNT;	/* reset count */
		} else if( Curscnt > 0 && ALPHA == T_mode ) {
			Curscnt--;
			if( Curscnt <= 0 )
				Cursor();	/* Enable cursor */
		}
	}
}

main()
{
	appl_init();

	initMain();

	/*
	 * Init Screen stuff, then show the window, and finally
	 * allow user to interact with it (ha ha).
	 */
	
	Init_Scr();
	events();

	/*
	 * close the work station (could be cause of previous
	 * bug...)
	 */
	v_clsvwk( Handle );
	
	appl_exit();
}

initMain()
{
	int	attrib[10];
	int	distances[5],effects[3];
	int	workIn[11], workOut[57];
	auto	int	dummy;
	int	i;

	/* find size of desk */
/*	wind_get( 0, WF_WORKXYWH, &Xdesk, &Ydesk, &Wdesk, &Hdesk );*/
	Ix = 0;
	Iy = 0;
	Iw = 639;
	Ih = 399;

	Handle=graf_handle(&dummy,&dummy,&dummy,&dummy);
	for( i=0; i<10; i++)
		workIn[i]=1;
	workIn[10]=2;
	v_opnvwk(workIn, &Handle, workOut);

	/* how big is character cell */
	vqt_attribute( Handle, attrib );
	CharW = attrib[8];
	CharH = attrib[9];

	vqt_fontinfo( Handle, &dummy, &dummy, distances, &dummy, effects);
	Baseline = distances[0];

	/* set fill pattern & color in order to clear window */
	vsf_color( Handle, 0 );

	/*
	 * Initialize some other stuff
	 */
	graf_mouse(M_OFF, 0L);
	Clear();
	Clear_Sysl();
	command = FALSE;
	comd_pos = 0;
	Half = FALSE;
	T_mode = ALPHA;
	T_x = 0;
	T_y = 0;
	Sysline = FALSE;
}

/*
 * Special key has been hit
 * (ie. function keys, arrow keys, HELP, UNDO, etc)
 */

special(x)
long x;
{
	register char c;

	c = (char) ( (x >> 16) & 0x7f );
	switch( c ) {

	case HELP:
		do_help();
		break;

	case UNDO:
		return( TRUE );
		break;

	case HOME:
		Cauxout( ESC );
		Cauxout( HO );
		break;

	case UARROW:
		Cauxout( ESC );
		Cauxout( UP );
		break;

	case LARROW:
		Cauxout( ESC );
		Cauxout( LE );
		break;

	case DARROW:
		Cauxout( ESC );
		Cauxout( DO );
		break;

	case RARROW:
		Cauxout( ESC );
		Cauxout( ND );
		break;
	}
	return( FALSE );
}

/*
 * do_help
 *
 * User hit HELP key.  This key gives short description (ie. help) and
 * allows user to set some variables (like baud rate and such).
 */

do_help()
{
	register char c;
	do {
		Cconout( ESC );
		Cconout( CL );
		Cconws( "WTerm -- UNDO to quit, HELP for help\r\n" );
		Cconws( "(c) 1986 Eric S. Fraga\r\n\n" );
		Cconws( "Hit:\r\n" );
		Cconws( "\t1\t300 baud\r\n" );
		Cconws( "\t2\t1200 baud\r\n" );
		Cconws( "\t3\t2400 baud\r\n" );
		Cconws( "\t4\t4800 baud\r\n" );
		Cconws( "\t5\t9600 baud\r\n\n" );
		Cconws( "\tf\tFull duplex\r\n" );
		Cconws( "\th\tHalf duplex\r\n\n" );
		Cconws( "\tspace\tto return\r\n\n" );
		Cconws( "-->" );

		/*
		 * read in char from user
		 */
	
		c = Cconin();
		switch( c ) {
		case 'q':
		case ' ':	wshow( Ix, Iy, Iw, Ih );
			break;
		
		case '1':	Rsconf( B300, -1, -1, -1, -1, -1 ); break;
		case '2':	Rsconf( B1200, -1, -1, -1, -1, -1 ); break;
		case '3':	Rsconf( B2400, -1, -1, -1, -1, -1 ); break;
		case '4':	Rsconf( B4800, -1, -1, -1, -1, -1 ); break;
		case '5':	Rsconf( B9600, -1, -1, -1, -1, -1 ); break;
		case 'f':	Half = FALSE;			break;
		case 'h':	Half = TRUE;			break;
		default:	Cconout(7);			break;
		}
	} while( c != ' ' && c != 'q' );
	T_mode = ALPHA;		/* just to have some way to reset for now */
}
