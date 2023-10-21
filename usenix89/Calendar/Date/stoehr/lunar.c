#include <curses.h>
#include <math.h>
#include <sys/types.h>
#include <time.h>

/* Globals. */
double		 Fraction;

/* Linked in later. */
char		*rindex();
time_t		 time();
struct tm	*localtime();


#define LINES		24
#define WIDTH		80
#define CENTER		((WIDTH - 2 * LINES) / 2)
#define BRIGHT		'@'
#define LEDGE		'('
#define REDGE		')'
#define FULL		0.5
#define TwoPi		(2 * 3.14159)
#define ZERO		0.03

extern short month, day, year;

lunar()
{
		long yday;

		yday = (long) (days_since_jan1(month, day, year) + 1);
		Calculate(yday, ((long) (year - 1900)), 11L, 0L);
		Draw();
		refresh();
		get_char();
		clear();
		print_screen();
}

long
Calculate(julian, year, hour, minute)
long julian, year, hour, minute;
{
	static struct tm *tm;
	register long	 Length;
	register long	 Phase;
	register long	 DeltaH;
	register long	 Delta;
	register long	 offset;
	/*time_t		 tick;

	static short called_before = 0;

	if (!called_before) {
		tick	= time((time_t *)0);
		tm		= localtime(&tick);
		called_before = 1;
	} else {
		tm->tm_yday++;
	}
	julian	= tm->tm_yday + 1;
	year	= tm->tm_year - 78;
	hour	= tm->tm_hour;
	minute	= tm->tm_min;*/

	year -= 78;
	Length	= (double)2551 / 60 * 1000 + (double)443 / 60;
	offset	= ((year * 365L + julian) * 24L + hour) * 60L + minute;
	Delta	= offset - (273L * 24L + 13L) * 60L + 23L;
	Phase	= Delta - (Delta / Length) * Length;

	Fraction	= (double)Phase / Length;
	return(Phase);
}

int
CharPos(x)
	double		x;
{
	register int	i;

	i = x * LINES + 0.5;
	if ((i += LINES + CENTER) < 1)
	i = 1;
	return(i);
}


Draw()
{
	register char	*p;
	register int	 i;
	register int	 end;
	register double	 y;
	register double	 cht;
	register double	 squisher;
	register double	 horizon;
	register double	 terminator;
	char		 Buffer[256];
	int line = 1;

	/* Clear screen? */
	clear();

	if (Fraction < FULL)
	squisher = cos(TwoPi * Fraction);
	else
	squisher = cos(TwoPi * (Fraction - FULL));

	cht = (double)2.0 / (LINES - 6.0);
	for (y = 0.93; y > -1.0; y -= cht)
	{
	for (i = sizeof Buffer, p = Buffer; --i >= 0; )
		*p++ = ' ';
	horizon = sqrt(1.0 - y * y);
	Buffer[    CharPos(-horizon)]	= LEDGE;
	Buffer[i = CharPos( horizon)]	= REDGE;
	Buffer[++i]			= '\0';
	terminator = horizon * squisher;
	if (Fraction > ZERO && Fraction < (1.0 - ZERO))
	{
		if (Fraction < FULL)
		{
		i   = CharPos( terminator);
		end = CharPos( horizon);
		}
		else
		{
		i   = CharPos(-horizon);
		end = CharPos( terminator);
		}
		while (i <= end)
		Buffer[i++] = BRIGHT;
	}
	mvaddstr(line++, 1, Buffer);
	}
	move(LINES-1, 0);
	refresh();
}
