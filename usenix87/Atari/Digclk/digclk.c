/*
 * Digital Clock DA
 *
 * Written and placed in the public domain by Jan Gray, 1986.
 *
 * link: accstart,clock,aesbind,osbind
 */

#include "define.h"
#include "gemdefs.h"
#include "osbind.h"

#define NO_WINDOW	-1			/* no window opened */
#define	NO_POSITION	-1			/* window has no position yet */

typedef struct window {
	int	id;
	int	x;
	int	y;
	int	w;
	int	h;
} Window;

/*
 * Hey Atari and Digital Research!  Notice how many global variables I declared?
 */


main()
{
	int	menuID;
	extern int	gl_apid;

	appl_init();
	menuID = menu_register(gl_apid,  "  Digital Clock");
	events(menuID);
}


/*
 * Loop processing events, and wake up every thirty seconds to update time.
 */
events(menuID)
int	menuID;
{
	Window	wind;
	int	event;
	int	msgbuf[8];
	int	ret;

	wind.id = NO_WINDOW;
	wind.x = NO_POSITION;

	for (;;) {
		event = evnt_multi(MU_MESAG | MU_TIMER,
				   0, 0, 0,
				   0, 0, 0, 0, 0,
				   0, 0, 0, 0, 0,
				   msgbuf, 30000, 0,
				   &ret, &ret, &ret, &ret, &ret, &ret);

		if (event & MU_MESAG) switch (msgbuf[0]) {
		case AC_OPEN:
			if (msgbuf[4] == menuID)
				if (wind.id == NO_WINDOW)
					openWindow(&wind);
				else
					wind_set(wind.id, WF_TOP, 0, 0, 0, 0);
			break;
		case AC_CLOSE:
			if (msgbuf[3] == menuID)
				wind.id = NO_WINDOW;
			break;
		case WM_CLOSE:
			if (msgbuf[3] == wind.id)
				closeWindow(&wind);
			break;
		case WM_MOVED:
			wind_set(wind.id, WF_CURRXYWH, msgbuf[4], msgbuf[5],
				 msgbuf[6], msgbuf[7]);
			wind.x = msgbuf[4]; wind.y = msgbuf[5];
			wind.w = msgbuf[6]; wind.h = msgbuf[7];
			break;
		case WM_NEWTOP:
		case WM_TOPPED:
			if (msgbuf[3] == wind.id)
				wind_set(wind.id, WF_TOP, 0, 0, 0, 0);
			break;
		}
		if (event & MU_TIMER && wind.id != NO_WINDOW)
			update(&wind);
	}
}


/*
 * Extract times from stupid DOS time format
 */
#define	MINS(t)		((t >> 5) & 0x3f)
#define	HRS(t)		(t >> 11)

#define	DIGIT(d)	((d) + '0')

#define	TEMPLATE	"hh:mm AM"
#define	TEMP_LEN	8

update(wp)
Window	*wp;
{
	static char	time[]	= TEMPLATE;
	unsigned	t	= Tgettime();
	unsigned	hrs	= HRS(t);
	unsigned	hrs12	= (hrs % 12 == 0) ? 12 : hrs % 12;
	unsigned	mins	= MINS(t);

	/*
	 * Do things the hard way: sprintf() would spend too much memory.
	 */
	time[0] = (hrs12 >= 10) ? DIGIT(1) : ' ';
	time[1] = DIGIT(hrs12 % 10);
	time[3] = DIGIT(mins / 10);
	time[4] = DIGIT(mins % 10);
	time[6] = (hrs < 12) ? 'A' : 'P';
	wind_set(wp->id, WF_NAME, time, 0, 0);
}


/*
 * Create and open a window just big enough to hold the time on its title bar
 */
openWindow(wp)
Window	*wp;
{
	int	workW;				/* work area width */
	int	workH;				/* work area height */
	int	ret;

	if (wp->id == NO_WINDOW) {
		if (wp->x == NO_POSITION) {
			/*
			 * Position the clock in the centre of the screen.
			 * This is a hack to determine the size and position
			 * of the window.
			 */
			graf_handle(&wp->w, &ret, &ret, &wp->h);
			wp->w *= TEMP_LEN + 3;
			wind_get(0, WF_WORKXYWH, &wp->x, &wp->y, &workW, &workH);
			wp->x += (workW - wp->w) / 2;
			wp->y += (workH - wp->h) / 2;
		}
		wp->id = wind_create(NAME|CLOSER|MOVER, wp->x, wp->y, wp->w, wp->h);
		wind_open(wp->id, wp->x, wp->y, wp->w, wp->h);
		update(wp);
	}
}


closeWindow(wp)
Window	*wp;
{
	if (wp->id != NO_WINDOW) {
		wind_close(wp->id);
		wind_delete(wp->id);
		wp->id = NO_WINDOW;
	}
}
