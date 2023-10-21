/*  :ts=8 bk=0
 *
 * robotroff.c:		Insidious display hack.
 *
 * Leo L. Schwab			8703.17		(415)-456-6565
 *
 * Public Domain.  Reproduce at will.  However, the author would greatly
 * appreciate it if:
 *	1) All parts of the program are made available free of charge
 * 	   (or for cost of media),
 *	2) You don't use this in a commercial product unless you at least
 *	   tell me about it,
 *	3) This notice and the author's name are preserved, even across
 * 	   subsequent revisions (I want to be famous, you see).
 *
 * Naturally, you don't *have* to abide by these terms.  Just remember
 * that if you do violate them, it means you are morally bankrupt.  You
 * wouldn't want people to think that about you, would you?
 *
*************************************************************************
			A few notes on the code:

	Bits of it are extremely messy.  Some of it looks unnecessary or
wrong.  As it turns out, this program works as written.  Hope you can
understand the code; some of it is instructive.  Watch out for ternary
operators; I've used quite a few of them.

	Since this is supposed to be a background display hack, it was
important to me to make it as small as I could and still do interesting
things.  This meant leaving out a couple of features I would have liked.
It would have been nice to have an explosion noise when the grunt or
enforcer ran into the mouse.  I was also thinking about arranging things
so that, if you plugged in a joystick at port 2 and tilted it around, you
could shoot the enemy sprites.  These are ideas that the more ambitious
among you might want to add in.  Some of you may have even more insidious
ideas.  Go right ahead.

	I encountered some odd behavior in the system I never noticed
before.  For example, if you AbortIO() an I/O request, the reply gets posted
to your reply port, and the signal you might Wait() on gets set.  WaitIO()
does not clear this signal apparently; I had to do it myself.

	Also, when moving the mouse pointer with false InputEvents, I
noticed that the mouse pointer, even in a 200-line screen, assumes it's in
a 400-line screen, and therefore a "movement" of 2 results in an actual
pointer movement of 1.  Odd.  Does anyone have an explanation for this?

	I tried to write the code in a such a way that it *should* work on
an interlaced screen, as well as one that's been 'morerows'ed.  However,
since 'morerows' causes overscan, which tromps sprite DMA, this may not
work on an overscan screen.  Oddly enough, it also seems to work with any
combination of screens, no matter their arrangement.  

	I had fun writing this program.  I hope you like it at least as
much as you liked Oing.

	"If you've enjoyed this program just half as much as I've enjoyed
writing it, then I'll have enjoyed it twice as much as you."

					Have fun,
					Leo L. Schwab

*************************************************************************/

#include <ctype.h>
#include <exec/types.h>
#include <exec/alerts.h>
#include <intuition/intuition.h>
#include <graphics/sprite.h>
#include <devices/input.h>
#include "robotroff.h"


/*  We're just opening this to get a pointer to the WorkBench ViewPort  */
struct NewWindow windef = {
	0, 0, 3, 10,
	-1, -1,
	NULL,
	NULL,
	NULL, NULL,
	NULL,
	NULL, NULL, 0, 0, 0, 0,
	WBENCHSCREEN
};

struct MsgPort		*ioport, *timeport, *cmdport;
struct IOStdReq		*ioreq;
struct timerequest	*timereq;
struct control		*ctl;
struct InputEvent	mouse;
struct SimpleSprite	spr;
struct Preferences	prefs;
struct Screen		*wbs;
struct Window		*win;
long			sprnum = -1;
int			wide, high;
void			*vp;
void			*IntuitionBase, *GfxBase;
/*  I take so many liberties with void *'s, it's scary.  */


main (ac, av)
char **av;
{
	void *daemon;

	if (daemon = FindPort (PORTNAME))
		reconfigure (ac, av, daemon);
	else {
		dodaemon (ac, av);
		postmessage ("Robotroff daemon terminated.");
	}

	closestuff ();
}

dodaemon (ac, av)
char **av;
{
	/*
	 * cmdport in this context means the port at which commands arrive
	 * for the daemon to act on.
	 */
	register struct control *msg;
	long csig, tsig, signals;
	int hw = DEFAULTHIGHWATER, lw = DEFAULTLOWATER, now = 0;
	void *me, *oldms;

	openstuff ();
	rnd ((short) -VBeamPos());	/*  Seed generator  */
	me = FindTask (NULL);

	/*  Parse command args  */
	if (ac)
		while (++av, --ac) {
			if (!strncmp (*av, "-h", 2)) {
				hw = atoi (&(*av)[2]);
				if (hw <= 0)
					hw = DEFAULTHIGHWATER;
			} else if (!strncmp (*av, "-l", 2)) {
				lw = atoi (&(*av)[2]);
				if (lw <= 0)
					lw = DEFAULTLOWATER;
			} else if (!strncmp (*av, "-p", 2))
				SetTaskPri (me, (long) atoi (&(*av)[2]));
			else if (!strcmp (*av, "-i"))
				now = 1;
		}
	if (hw < lw)	/*  hw time must not be shorter than lw time  */
		lw = hw;

	tsig = TIMESIG;
	csig = CMDSIG;
	while (1) {
		if (now) {
			now = 0;
			goto dosomething;
		}

		timereq -> tr_time.tv_secs = rnd (hw-lw+1) + lw;
		timereq -> tr_time.tv_micro = 0;

		SendIO (timereq);
		signals = Wait (tsig | csig);

		if (signals & csig) {
			/*  Command from another task (user)  */
			msg = GetMsg (cmdport);
			AbortIO (timereq);
			WaitIO (timereq);
			/*
			 * The I/O reply causes a signal to be 
			 * raised, which screws things up.  So
			 * we clear it.
			 */
			Wait (tsig);

			if (msg -> actions & SUICIDE) {
				ReplyMsg (msg);
				break;
			}
			if (msg -> actions & SETHIGH)
				hw = msg -> highwater;
			if (msg -> actions & SETLOW)
				lw = msg -> lowater;
			if (hw < lw)
				lw = hw;
			if (msg -> actions & SETPRI)
				SetTaskPri (me, (long) msg -> newpri);
			if (msg -> actions & DOSOMETHING)
				now = 1;
			ReplyMsg (msg);
		}

		if (signals & tsig) {
			/*  Timer went off, time to do something weird  */
			WaitIO (timereq);
dosomething:
			switch (rnd (5)) {
			case 0:  case 1:
				dohulk ();
				break;
			case 2:  case 3:
				dogrunt ();
				break;
			case 4:
				enforce ();
			}
		}
	}
}

reconfigure (ac, av, daemonport)
char **av;
void *daemonport;
{
	/*
	 * cmdport in this context means the reply port for commands to
	 * the daemon.
	 */

	if (ac < 2)	/*  Nothing to do  */
		return;

	/*  In order to use die(), we have to open Intuition.  */
	if (!(IntuitionBase = OpenLibrary ("intuition.library", 0L))) {
		Alert (AG_OpenLib | AO_Intuition, 0L);
		closestuff ();
		exit (20);
	}
	GetPrefs (&prefs, (long) sizeof (prefs));

	if (!(cmdport = CreatePort (NULL, NULL)))
		die ("Can't create command reply port.");

	if (!(ctl = CreateExtIO (cmdport, (long) sizeof (*ctl))))
		die ("Can't create command block.");

	/*  Parse command line args  */
	while (++av, --ac) {
		if (!strncmp (*av, "-h", 2)) {
			ctl -> highwater = atoi (&(*av)[2]);
			if (ctl -> highwater <= 0)
				ctl -> highwater = DEFAULTHIGHWATER;
			ctl -> actions |= SETHIGH;
		} else if (!strncmp (*av, "-l", 2)) {
			ctl -> lowater = atoi (&(*av)[2]);
			if (ctl -> lowater <= 0)
				ctl -> lowater = DEFAULTLOWATER;
			ctl -> actions |= SETLOW;
		} else if (!strncmp (*av, "-p", 2)) {
			ctl -> newpri = atoi (&(*av)[2]);
			ctl -> actions |= SETPRI;
		} else if (!strcmp (*av, "-i"))
			ctl -> actions |= DOSOMETHING;
		else if (!strcmp (*av, "-kill"))
			ctl -> actions |= SUICIDE;
	}

	/*  Send command to daemon  */
	PutMsg (daemonport, ctl);
	WaitPort (cmdport);
	GetMsg (cmdport);
}

openstuff ()
{
	if (!(IntuitionBase = OpenLibrary ("intuition.library", 0L))) {
		Alert (AG_OpenLib | AO_Intuition, 0L);
		closestuff ();
		exit (20);
	}
	GetPrefs (&prefs, (long) sizeof (prefs));

	if (!(GfxBase = OpenLibrary ("graphics.library", 0L)))
		die ("Art shop closed.");

	/*  Set up a few environment things based on screen info  */
	if (!(win = OpenWindow (&windef)))
		die ("Window painted shut.");
	vp = ViewPortAddress (win);
	wbs = win -> WScreen;
	CloseWindow (win);  win = 0;
	high = wbs -> Height;
	wide = wbs -> Width;
	if (wbs -> ViewPort.Modes & HIRES)
		wide /= 2;
	if (wbs -> ViewPort.Modes & LACE)
		high /= 2;

	if ((sprnum = GetSprite (&spr, 2L)) < 0)
		die ("Couldn't get sprite.");

	if (!(cmdport = CreatePort (PORTNAME)))
		die ("Can't create command port.");

	if (!(timeport = CreatePort (NULL, NULL)))
		die ("Can'r create time port.");

	if (!(timereq = CreateExtIO (timeport, (long) sizeof (*timereq))))
		die ("Can't make timer I/O.");
	if (OpenDevice (TIMERNAME, UNIT_VBLANK, timereq, 0L)) {
		timereq -> tr_node.io_Device = 0;
		die ("Can't open timer device.");
	}
	timereq -> tr_node.io_Command = TR_ADDREQUEST;

	if (!(ioport = CreatePort (NULL, NULL)))
		die ("Can't create I/O port.");

	if (!(ioreq = CreateStdIO (ioport)))
		die ("Can't make I/O request.");
	if (OpenDevice ("input.device", 0L, ioreq, 0L)) {
		ioreq -> io_Device = 0;
		die ("Can't generate input.");
	}
	ioreq -> io_Command = IND_WRITEEVENT;
	ioreq -> io_Flags = 0;
	ioreq -> io_Length = sizeof (struct InputEvent);
	ioreq -> io_Data = (APTR) &mouse;
}

closestuff ()
{
	if (ioreq) {
		if (ioreq -> io_Device)
			CloseDevice (ioreq);
		DeleteStdIO (ioreq);
	}
	if (timereq) {
		if (timereq -> tr_node.io_Device)
			CloseDevice (timereq);
		DeleteExtIO (timereq, (long) sizeof (*timereq));
	}
	if (ctl)		DeleteExtIO (ctl, (long) sizeof (*ctl));
	if (cmdport)		DeletePort (cmdport);
	if (ioport)		DeletePort (ioport);
	if (timeport)		DeletePort (timeport);
	if (sprnum >= 0)	FreeSprite (sprnum);
	if (win)		CloseWindow (win);
	if (GfxBase)		CloseLibrary (GfxBase);
	if (IntuitionBase)	CloseLibrary (IntuitionBase);
}

die (str)
char *str;
{
	postmessage (str);
	closestuff ();
	exit (10);
}

postmessage (str)
char *str;
{
	if (win)	CloseWindow (win);	/*  Should never happen  */

	windef.TopEdge = 0;
	windef.Height = 10;
	windef.Width = 8 + strlen (str) *
			(prefs.FontHeight == TOPAZ_SIXTY ? 10 : 8);
	windef.LeftEdge = (640 - windef.Width) / 2;
	windef.DetailPen = 1;
	windef.BlockPen = 3;
	windef.Flags |= ACTIVATE;
	windef.Title = (UBYTE *) str;

	if (!(win = OpenWindow (&windef)))
		/*  Crash system and let user worry about it  */
		Alert (AN_OpenWindow, 0L);

	Delay (200L);
	CloseWindow (win);  win = 0;
}
