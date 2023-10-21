#include	"ffdef.h"
/*
**	NEW -- Add a new player to the fast food game
** Compile: cc -O -q new.c glb.o fflib.a -lP -lS -o new
*/

main(argc, argv)
char **argv;
{
	register int i;
	int uid;
	long now;

	if (argc != 3) {
	    printf("Usage: %s player logname\n", argv[0]);
	    exit(2);
	}
	time(&now);
	srand((int) (now >> 2 & 0177777));
	for (pnum = 0; pnum < num_players; pnum++)
	    if (getplyr(pnum, SAFE) == -1 || plyr.p_name[0] == '\0')
		break;
	if (pnum >= num_players) {
	    printf("Sorry, the game is full\n");
	    done(1);
	}
	i = copy(argv[1], plyr.p_name);
	if (i > &plyr.p_name[31]) {
	    printf("Name '%s' is too long\n", argv[1]);
	    exit(2);
	}
	for (i = 0; i < num_plots; i++)
	    getplot(&plyr.p_plot[i]);
	time(&plyr.p_time);
	plyr.p_money = initial_cash;
	uid = namuid(argv[2]);
	if (uid == -1) {
	    printf("Bad logname : '%s'\n", argv[2]);
	    exit(2);
	}
	plyr.p_uid = uid;
	for (i = 0; i <= LIM_CHAINS; i++)
	    plyr.p_shares[i] = 0;
	plyr.p_trans = 0;
	putplyr(pnum);
	done(0);
}
