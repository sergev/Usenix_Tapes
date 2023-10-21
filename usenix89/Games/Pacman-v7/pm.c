/*
** pm.c -	main: argument parsing, main control loop and usage messages
**
**	[pm by Peter Costantinidis, Jr. @ University of California at Davis]
*/
#include "pm.h"

main (argc, argv)
reg	int	argc;
reg	char	**argv;
{
	argv0 = *argv++, argc--;
	if (argc == 1)
	{
		if (*((*argv)++) != '-')
			usage();
		switch (**argv)
		{
			case 'p':
				pmers();
				exit(0);
			case 's':
				check_scrs();
				exit(0);
			case 'B':
				baud = to_baud(++*argv);
				break;
			default:
				usage();
		}
	}
	else if (argc)
		usage();
	if (!strcmp("tester", argv0))
		was_wiz = is_wiz = TRUE;
#ifdef	NOFULLPATH
	/*
	** at one time the author was worried about the whole world finding
	** out this games existed.  to keep its location a little private
	** he tried to discourage full pathnames to the game from showing
	** up on a ps(1) output by forcing argv[0] to be "pm".
	** the following code checked for this condition.
	*/
	else if (getuid() != wizard_uid && strcmp("pm", argv0))
	{
		fprintf(stderr, "That is a Big No NO!!!");
		setuid(getuid());
		if (getuid() != 0)
			kill(0, 9);	/* blast them out of the water */
	}
#endif
	init();
	slow(TRUE);
	while (TRUE)
	{
		SLOWER();
		demon++;
		if (make_moves())
			break;
		if (timer > 0)
			timer--;
		/*
		** we know we need a new board drawn when we are
		** out of dots and energizers
		*/
		p_scores();
		if (!e_left && !d_left)
		{	/* no more dots left	*/
			chg_lvl(1);
			continue;
		}
		if (!timer && !pm_run)	/* energizers ran out	*/
		{
			aggressive();
			pm_run = TRUE;
			continue;
		}
		if ((timer < MAX_BLINKS) && !(timer % 4))
			warning();		/* warn every four moves*/
		m_move();			/* move the monsters	*/
	}
	quitit();
	exit(0);
}

/*
** usage()	- print a usage message and exit
*/
void	usage ()
{
	fprintf(stderr, "Usage: %s [-s] [-p] [-Bn]\n", argv0);
	exit(1);
}
