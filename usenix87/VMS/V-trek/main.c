/*
 *	main.c
 *
 *	visual star trek
 *
 *	BASIC version written by Tom Goerz and debugged by Dug Patrick
 *		22-Dec-79, 13-Nov-80
 *	C version written by Dug Patrick
 *		05-Aug-84, 11-Mar-85
 *
 */

#include "vtrek.h"

char playership[] = " ? ";
int rolines = 0;
short chan;

main()
{
	int cmd, ch;
	char str[44];


	instructions();
	initvars();
	terminit();
	replot();

	sprintf(str, "You have %.1f stardates to save the", lastdate - stardate);
	readout(ADDLINE, str);
	readout(ADDLINE, "Federation from the Klingon invasion.");

	setcondition();

	while (numkling > 0) {
	    switch (cmd = getcmd()) {

	    case 'H' :		/* hyper-space */
		hyperspace();
		break;

	    case 'S' :		/* short range scan */
		srs();
		break;

	    case 'L' :		/* long range scan */
		lrs();
		break;

	    case 'P' :		/* fire phasers */
		phasers();
		break;

	    case 'T' :		/* fire photon torpedo */
		torpedo();
		break;

	    case 'U' :		/* change shield level */
		defense();
		break;    

	    case 'R' :		/* replot screen */
		replot();
		continue;

	    case 'Q' :		/* move using impulse engines */
	    case 'W' :
	    case 'E' :
	    case 'A' :
	    case 'D' :
	    case 'Z' :
	    case 'X' :
	    case 'C' :
		impulse(cmd);
		break;

	    case 'K' :		/* kill - commit suicide */
		prompt("Quit ? ");
		ch = get_a_char();
		if (Toupper(ch) == 'Y')
		    die();
		break;

	    case 'F' :		/* fix devices */
		repdevices();
		break;

	    case 03 :		/* exit without warning */
	    case 04 :
		die();
		break;

	    case '?' :		/* help */
		help();
		break;

	    default :		/* illegal command */
		readout(ADDLINE, "Type '?' for help.");
		break;
	    }

	    fixdev(REL, RND, 5);
	    setcondition();
	    klingmove();

	    if ((stardate += 0.1) > lastdate)
		timeout();
	    plt_stat(ELEMENT, STARDATE);
	    plt_num(INFO);

	    if (energy <= 0 && shields <= 0)
		dead();
	}

	win();
}

