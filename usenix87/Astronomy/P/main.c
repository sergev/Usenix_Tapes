#include	<fcntl.h>
#include	<signal.h>
#include	"p.h"

main(argc, argv)
char	**argv;
	{
	register int	wanderer;
	register struct	planet	*planp;
	register int	bump;
	int	die();

	while (argc-- > 1) {
		if (strcmp(argv[1], "-p") == NULL)
			argv++, print++;

		if (strncmp(argv[1], "-l", 2) == NULL) {
			/*
			 *  Look for the loop increment.
			 */
			deltat = parsint(&argv[1][2]);
			if (deltat == 0.0)
				deltat = 1.0;
			argv++, loop = 1;
		}
		if (strcmp(argv[1], "-n") == NULL)
			argv++, ritenow++;

		if (strcmp(argv[1], "-e") == NULL)
			argv++, eltime++;
	}
	/*
	 *  Who want's to sleep this long anyway?
	 */
	if (eltime)
		if ((long) (deltat * 1440.0 * 60.0) > 999)
			eltime = 0;

	if (ritenow)
		settime();
	else
		gettime();

	if ( ! print) {
		initscr();
		erase();
		sleep(1);
		refresh();
		signal(SIGINT, die);
		signal(SIGHUP, die);
		signal(SIGBUS, die);
		raw();
		noecho();
		flags = fcntl(0, F_GETFL, O_NDELAY);
		fcntl(0, F_SETFL, O_NDELAY);
		clear();
		sleep(1);
		move(23, 0);
		printw(">>> ");
		prthdr();
		prtshdr();
		for (wanderer = MERCURY; wanderer <= PLUTO; wanderer++)
			prtname(wanderer);

		prtname(SUN);
		prtname(MOON);
		prtname(TIME);
		move(0, 0);
		refresh();
	}
	do {
		/*
		 *  Process user requests only in interactive mode.
		 */
		if ( ! print)
			bump = dorequest();

		gestalt();			/* do everything */

		for (wanderer = MERCURY; wanderer <= PLUTO; wanderer++) {
			if ( ! print)
				bump = dorequest();

			if (print)
				prtname(wanderer);
			/*
			 *  Compute everything about a planet.
			 */
			planp = plandata(wanderer);
			/*
			 *  Save equatorial coordinates for angular
			 *  separation calculations.
			 */
			wandeq[wanderer] = planp->equat;
			/*
			 *  Print the planetary data.
			 */
			prtplan(planp, wanderer);
			/*
			 *  Scan for user input.
			 */
			scan();
		}
		if (print)
			prtname(SUN);
		sun.equat = *ectoeq(&sun.eclipt);
		sun.hzn = *eqtohzn(&sun.equat);
		sun.rs = *sunriseset();
		prtsun();

		if (print)
			prtname(MOON);
		moondata();
		prtmoon();

		if (print)
			prtname(TIME);
		prtimes();

		eclipse();

		prtsep();

		scan();

		if ( ! print) {
			move(pt[SEPMAT].row+7, 0);
			printw("%#-9.3g", deltat);
			refresh();
		}

		if ( ! print) {
			if (bump != 0)
				ticktock();
		} else
			ticktock();

	} while (loop);

	if (print)
		putchar('\n');
	else {
		move(23, 0);
		refresh();
	}
	if ( ! print)
		die();
	else
		exit(0);
}

/*
 *  Parse the interval pointed to by arg.
 */
REAL
parsint(sptr)
register char	*sptr;
	{
	register char	chr;
	register indx;
	REAL	dt;

	dt = atof(sptr);
	indx = strlen(sptr) - 1;
	chr = sptr[indx];
	/*
	 *  Delta interval is in days; modify based
	 *  on optional units.
	 */
	switch (chr) {
	case 's':
		if (dt < 1.0)
			dt = 1.0;
		dt /= 1440.0 * 60.0;
		break;

	case 'm':
		dt /= 1440.0; break;

	case 'h':
		dt /= 24.0; break;

	case 'w':
		dt *= 7.0; break;

	case 'y':
		dt *= 365.2422; break;

	default:
		dt = 0.0; break;
	}
	return(dt);
}

die() 
	{
	signal(SIGINT, SIG_IGN);
	signal(SIGHUP, SIG_IGN);
	signal(SIGBUS, SIG_IGN);
	move(COLS - 1, LINES - 1);
	echo();
	noraw();
	fcntl(0, F_SETFL, flags);
	endwin();
	exit(0);
}
