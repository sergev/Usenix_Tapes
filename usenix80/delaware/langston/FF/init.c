#include	"ffdef.h"
/*
**	INIT -- Setup for game of fast food
**		(see "Usage" below)
**	Copyright (c) P. Langston , 1977, N.Y.C., N.Y.
*/

char	*names[] {
	"Zeroth Chain",
	"Arturo's House of Pizza",
	"Ben's Big Burgers",
	"Chicken Licken",
	"Dizzy Dogs",
	"E-Z Eats",
	"Fu Health Bar",
	"Gyro Guru",
	"Home Brew Haven",
	"Instant Lunch",
	"Jiffy's Ribs",
	"Kalorie Kitchen",
	"Lite Bite Lunch",
	"Mama's Home Cookin'",
	"Nena's Noshery",
	"Oh Fred's",
	"P. J.'s Pantry",
	"Quik Snak",
	"Red Banjo Emporium",
	"Scarf 'n' Barf",
	"Travelers Three",
	"Uncle Jerry's Place",
	"Vita-Vittles",
	"Wine & Wiggle Disco",
	"Xaviera's Frogurt Farm",
	"Yellow Pearl Coffeeshop",
	"Zesty Zoup & Zalad",
};

main(argc, argv)
char **argv;
{
	register int i, j, fh;
	char *cp;
	int k, *ip;
	long now;
	struct plot plts[LIM_X_SIZE * LIM_Y_SIZE];

	if (argc == 1) {
	    printf("Usage: %s [-satsun] uphr:upmin [ ... ]", argv[0]);
	    printf("  where uphr:upmin is hour & minute of update(s)\n");
	    exit(2);
	}
	time(&now);
	srand((int) (now >> 2 & 0177777));
	if (argv[1][0] == '-') {
	    misc.m_satsun = 1;
	    argv++;
	    --argc;
	} else
	    misc.m_satsun = 0;
	k = argc - 1;
	for (i = 0; i < k && i < LIM_UPDATES; i++) {
	    cp = argv[i + 1];
	    misc.m_time[i].t_hr = atoip(&cp);
	    if (*cp++ != ':') {
		printf("Format is hr:min\n");
		exit(2);
	    }
	    misc.m_time[i].t_min = atoip(&cp);
	}
	for (; i < LIM_UPDATES; i++)
	    misc.m_time[i].t_hr = misc.m_time[i].t_min = 0;
	misc.m_nextup = 0;
	misc.m_lstchn = 0;
	k = 0;
	for (j = 0; j < y_size; j++) {
	    for (i = 0; i < x_size; i++) {
		plts[k].p_x = i;
		plts[k].p_y = j;
		k++;
	    }
	}
	for (j = x_size * y_size; --j > 0; ) {            /* shuffle plots */
	    i = (rand() >> 2) % j;
	    k = plts[i].p_xy;
	    plts[i].p_xy = plts[j].p_xy;
	    plts[j].p_xy = k;
	}
	misc.m_lstplt = x_size * y_size;
	fh = creat(miscfil, 0600);
	if (fh < 0)
	    perror(miscfil);
	write(fh, &misc, sizeof misc);
	write(fh, &plts, sizeof plts);
	close(fh);
	printf("Misc file, `%s', created\n", miscfil);

	fh = creat(boardfil, 0600);
	if (fh < 0)
		perror(boardfil);
	for (j = 0; j < y_size; j++)
	    for (i = 0; i < x_size; i++)
		board[i][j] = 0;
	write(fh, board, sizeof board);
	close(fh);
	printf("Board file, `%s', created\n", boardfil);

	fh = creat(plyrfil, 0600);
	if (fh < 0)
	    perror(plyrfil);
	plyr.p_name[0] = '\0';
	for (i = 0; i < num_players; i++)
	    write(fh, &plyr, sizeof plyr);
	close(fh);
	printf("Player file, `%s', created\n", plyrfil);

	fh = creat(chainfil, 0600);
	if (fh < 0)
	    perror(chainfil);
	chain.c_size = 0;
	for (i = 0; i <= LIM_CHAINS; i++) {
	    if (length(names[i]) >= CNAME_LENGTH) {
		printf("The name \"%s\" is too long.\n", names[i]);
		names[i][CNAME_LENGTH - 1] = '\0';
	    }
	    copy(names[i], chain.c_name);
	    write(fh, &chain, sizeof chain);
	}
	close(fh);
	printf("Chain file, `%s', created\n", chainfil);

	if (close(creat(newsfil, 0600)) == -1)
	    perror(newsfil);
	printf("News file, `%s', created\n", newsfil);

	if (close(creat(onwsfil, 0600)) == -1)
	    perror(newsfil);
	printf("Old news file, `%s', created\n", onwsfil);

	if (close(creat(histfil, 0600)) == -1)
	    perror(histfil);
	printf("History file, `%s', created\n", histfil);

	if (close(creat(locknode, 000)) == -1)
	    perror(locknode);
	printf("Locknode, `%s', created\n", locknode);

	exit(0);
}

length(string)
char	*string;
{
	register char *cp;

	for (cp = string; *cp++; );
	return(cp - string - 1);
}
