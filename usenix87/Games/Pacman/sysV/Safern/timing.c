/*
** timing.c -	functions dealing with the "smooth" running of the game
**		it is important not for the game to get ahead of the screen
**		and the necessary `slowing down' of the game is done here
**
**	[pm by Peter Costantinidis, Jr. @ University of California at Davis]
*/

#include <signal.h>
#include "pm.h"

#define	BPBYTE	10	/* bits sent to termnal per character (byte) */
#define PAWS(x)  ((1000 * x * BPBYTE) / baud)
#define TIMEBS(x,y) ((x - y) * 100 / 6)
/*
** delay()	- coordinate with tty speed
**     /                1000 ms.                            \
**     | ------------------------------------ * delta(chars) | == delay in ms.
**     \   (baud bits/sec)/(BPBYTE bits/char)               /
*/
void	delay ()
{
	int     u;
	auto    long tp;

	u = PAWS(chcnt);
	for(;;) {
		tp = times(&garbage);
		if (TIMEBS(tp, _tp) >= u) {
			chcnt = 0L;
			return;
		}
	}
}

static	int	rates[] =	/* these were `tuned' after much playing */
{
/*        0    1    2    3    4    5    6    7    8    9	*/
	320, 265, 220, 210, 200, 190, 180, 170, 160, 150,
/*       10   11   12   13   14   15   16   17   18   19	*/
	140, 130, 120, 110, 100, 100, 100,  90,  90,  80,
/*       20   21   22   23   24   25   26   27   28   29	*/
	 95,  70,  45,  20, 100, 150,  50,  99, 100, 100,
/*       30   31   32   33   34   35   36   37   38   39	*/
	  5,   0,   0,   0,   0,  20,   0,   0,   0,   0,
/*       40   41   42   43   44   45   46   47   48   49	*/
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
/*       50   51   52   53   54   55   56   57   58   59	*/
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
};

/*
** slow()	- make the game go faster as they go deeper
**		- assuming that they WILL NOT GET DEEPER THAN
**                60 LEVELS!!!!  if they are, then they have been
**                tying up the computer long enough and should stop
**                playing anyway
*/
static  long    tp;

void    initslow()
{
	tp = times(&garbage);
}

void    slow()
{
	reg     int     ms, num;
	auto    long    tp2;

	if (level > 59)
		quitit();
	num = (fast ? rates[level]/4 : rates[level]/2);
	for(;;) {
		tp2 = times(&garbage);
		if (TIMEBS(tp2, tp) >= num) {
			tp = tp2;
			return;
		}
	}
}

/*
** slowness()   - sets delay in rates
**
*/
void	slowness ()
{
	auto	char	buf[BUFSIZ];

	doclear();
	nocrmode();
	printf("old delay: %d, new delay: ", rates[level]);
#if SYSV|SYSIII
	fcntl(0, F_SETFL, oldfl);
#endif
	Echo();        /* defined to echo() on machines without bug */
	if (!gets(buf))
		msg("EOF in slowness");
	if (buf[0])
		if (sscanf(buf, "%d", &(rates[level])) == EOF)
			msg("EOF2 in slowness");
#if SYSV|SYSIII
	fcntl(0, F_SETFL, O_NDELAY);
#endif
	noecho();
	crmode();
	redraw();
}

