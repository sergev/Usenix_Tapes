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
#define	PAWS(x)	(unss) ((1000 * x * BPBYTE) / bauds[baud])
#define	TIMEBS(x,y) (((unss) ((x)->time - (y)->time) * 1000) + \
			(x)->millitm - (y)->millitm)
/*
** delay()	- coordinate with tty speed
**     /                1000 ms.                            \
**     | ------------------------------------ * delta(chars) | == delay in ms.
**     \   (baud bits/sec)/(BPBYTE bits/char)               /
*/
void	delay ()
{
	unss	mlen, u;	/* how long it took to print */
	auto	struct	timeb	tp;

	ftime(&tp);
	mlen = TIMEBS(&tp, &_tp);
	if ((u = PAWS(chcnt)) >= mlen)
		msleep(u - mlen);
	chcnt = 0l;
}

/*
** msleep()	- sleep the specified number of milliseconds
*/
void	msleep (u)
reg	unss	u;
{
#ifdef	ITIMER_REAL
	static	struct	itimerval	oitv, itv =
	{
		{	0L, 0L },
		{	0L, 0L }
	};
	extern	int	wakeup();

#ifndef	LINT
	signal(SIGALRM, wakeup);
#endif
	itv.it_value.tv_sec = u / 1000;
	itv.it_value.tv_usec = u % 1000;
	if (setitimer(ITIMER_REAL, &itv, &oitv))
	{
		fprintf(stderr, "%s: setitimer() error\n", argv0);
		perror("setitimer");
		quitit();
	}
	pause();
#else
	auto	struct	timeb	tp, tp2;

	ftime(&tp);
	while (TRUE)
	{
		ftime(&tp2);
		if (TIMEBS(&tp2, &tp) >= u)
			return;
	}
#endif
}

#ifdef	ITIMER_REAL
/*
** wakeup()	- someplace to go when the alarm goes off
*/
static	int	wakeup ()
{
	return(0);
}
#endif

static	int	rates[] =	/* these were `tuned' after much playing */
{
/*        0    1    2    3    4    5    6    7    8    9	*/
	430, 395, 380, 350, 340, 310, 325, 330, 325, 300,
/*       10   11   12   13   14   15   16   17   18   19	*/
	305, 325, 360, 320, 290, 260, 220, 190, 140, 120,
/*       20   21   22   23   24   25   26   27   28   29	*/
	 95,  70,  45,  20, 100, 200,  50,  99, 200, 300,
/*       30   31   32   33   34   35   36   37   38   39	*/
	  5,   0,   0,   0,   0,   0,   0,   0,   0,   0,
/*       40   41   42   43   44   45   46   47   48   49	*/
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
/*       50   51   52   53   54   55   56   57   58   59	*/
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
};

/*
** slow()	- make the game go faster as they go deeper
**		- assuming that they WILL NOT GET DEEPER THAN
**		  50 LEVELS!!!!  if they are, then they have been
**		  tieing up the computer long enough and should stop
**		  playing anyways
*/
void	slow (flag)
reg	int	flag;
{
	reg	int	ms;
	static	struct	timeb	tp;
	auto	struct	timeb	tp2;

	if (flag)
	{
		ftime(&tp);
		return;
	}
	if (level > 59)
		quitit();
	ftime(&tp2);
	ms = (int) TIMEBS(&tp2, &tp);
	if (fast)
	{
		if (ms < rates[level]/2)
			msleep((rates[level]/2) - ms);
	}
	else
	{
		if (ms < rates[level])
			msleep(rates[level] - ms);
	}
	ftime(&tp);
}

/*
** slowness()	- sets delay padding for number of nulls to be printed
**		  each loop
*/
void	slowness ()
{
	auto	char	buf[BUFSIZ];

	_puts(CL);
	echo();
	nocrmode();
	printf("old delay: %d, new delay: ", rates[level]);
	if (!gets(buf))
		msg("EOF in slowness");
	if (buf[0])
		if (sscanf(buf, "%d", &(rates[level])) == EOF)
			msg("EOF2 in slowness");
	noecho();
	crmode();
	redraw();
}

/*
** to_baud()	- convert the given string to an appropriate baud define
*/
char	to_baud (s)
reg	char	*s;
{
	reg	int	i;
	static	char	*sbauds[] =
	{
		"0", "50", "75", "110", "134", "150",
		"200", "300", "600", "1200", "1800", "2400",
		"4800", "9600", "EXTA", "EXTB",
		0
	};

	for (i = 0; sbauds[i]; i++)
		if (!strcmp(sbauds[i], s))
			return((char) i);
	return('\0');
}
