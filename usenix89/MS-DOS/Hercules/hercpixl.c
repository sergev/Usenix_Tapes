/*
 *	HERCPIXL.C
 *	Dave Tutelman  -  last modified 8/86
 *
 *	This is a set of basic graphic routines for the Hercules
 *	Board (or at least the SuperComputer version of it; I assume
 * 	that they work with the real thing).
 *		alfa ()		puts us in alphanumeric mode.
 *		grafix (page)	puts us in graphics mode in page 0 or 1
 *		pixel (x,y,val)	puts a pixel at <x,y>. Bright dot if
 *				val=1, dark dot if val=0.
 * 		dchar (x,y,c)	puts character "c" at <x,y>. Note that
 *				character raster is 90 x 29.
 *		swpage (page)	switches to a different page.
 *	        waitkey ()	just waits till a key is pressed.
 *
 *	Actually, the routines should work with any board, since the
 *	BIOS calls are used throughout.  It's Hercules-specific only
 *	because I've defined the graphics and alpha modes for my
 *	Hercules BIOS.
 *
 *	Compile with deSmet C.
 */

#define VIDI    0x10		/* video interrupt, normally 10H  */
#define	KBD	0x16		/* keyboard interrupt */
#define ALFA_MODE	7	/* monochrome alpha mode */
#define GRAF_MODE	8	/* Hercules graphics mode */

int	page = 0;
extern unsigned _rax,_rbx,_rcx,_rdx;

/*
 * This puts us back in alphanumeric mode 
 */

alfa (dummy)
	unsigned int dummy;
{
	_rax = ALFA_MODE;	/* mono 80-col mode */
	_doint ( VIDI );	/* set mode */
}

/*
 *	This one switches us to hercules graphics mode
 *		in page 0 or 1
 */

grafix (newpage)
	int newpage;
{

	_rax = GRAF_MODE;			/* herc grafix mode */
	_doint ( VIDI );	/* set mode */

	/*  now set the page */
	swpage (newpage);
}

/*
 *   This writes a pixel at (x,y), where (0,0) is the upper-left
 *   corner of the screen.  If val = 0 then the pixel is erased.
 */

pixel (x, y, val)
	int x, y, val;
{

	_rax = 0x0C00 + val;	/* function 12      */	
	_rcx = x;
	_rdx = y;
	_doint ( VIDI );	/* set mode */

}


/*
 *	dchar (x,y,c)	puts character "c" at <x,y>. Note that
 *			character raster is 90 x 25.
 */

dchar (x,y,c)
	int x,y;
	char c;
{

	_rax = 2*256;		/* AH=Fn#2 */
	_rdx = 256*y + x;		/* DH=row, DX=col */
	_rbx = page * 256;		/* BH=page  */
	_doint (VIDI);		/* set cursor */

	_rax = 10*256 + (int) c;	/* AH=Fn#10, AL=char */
	_rbx = page * 256;		/* BH=page */
	_rcx = 1;			/* CX=count */
	_doint (VIDI);		/* write character */
}


/*
 *	This one switches us to a different page, without changing
 *	the contents of that page.
 */

swpage (newpage)
	int 	newpage;
{
	page = newpage;
	_rax = 0x500 + page;		/* new page function */
	_doint ( VIDI );	/* interrupt call */
}


waitkey ()
{
	_rax = 0;		/* keyboard blocking read function */
	_doint (KBD);
}
