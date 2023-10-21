/*
 * Dungeon - open UP dungeon
 */

int users[] {
	522,    /* sa */
	164,    /* Leiby */
	229,    /* richards */
	264,    /* marshall */
	1099,   /* wizard */
	425,    /* clm */
	15,     /* mowle */
	32,     /* ghg */
	27,	/* qtip (zager) */
	530,	/* mike */
	16,	/* bc */
	333,	/* pdh */
	230,	/* wa1yyn */
	19,	/* joe
	43,	/* bruner */
	308,	/* gedeon (watch him closely!) */
	429,	/* mayhew */
	743,	/* alicia */
	367,	/* feather */
	85,	/* clark bar */
	382,	/* malcolm */
	99,	/* jones */
	636,    /* gfg */
	0 };

int fout;
main()
{

	register int *up;
	register uid;
	int fd3, fd4, fd5;

	goto ok;        /* allow anyone */

	uid = getuid();
	for (up=users; *up; up++)
		if (*up == uid)
			goto ok;
	printf("You are not a Wizard!\n");
	exit();
ok:
	/*
	 * open up files needed by program
	 * look in current directory first, then try default names
	 * The following files must be as follows:
	 * "dtext.dat" open read-only on fd 3
	 * "dindex.dat open read-only on fd 4 (maybe this file isn't used)
	 * "doverlay" open read-only on fd 5 (put this file on fast disk)
	 */
	close(3);
	close(4);
	close(5);
	if ((fd3 = open("dtext.dat", 0)) < 0)
		if ((fd3 = open("/v/ghg/dtext.dat", 0)) < 0)
			error("Can't open dtext.dat\n");

	if ((fd4 = open("dindex.dat", 0)) < 0)
		if ((fd4 = open("/v/ghg/dindex.dat", 0)) < 0)
			error("Can' open dindex.dat\n");

	if ((fd5 = open("doverlay", 0)) < 0)
		if ((fd5 = open("/tmp/nedtmp/doverlay", 0)) < 0)
			if ((fd5 = open("/v/ghg/doverlay", 0)) < 0)
				if ((fd5 = open("/v/ghg/ovr", 0)) < 0)
					error("Can't open doverlay\n");

	if (fd3 != 3 || fd4 != 4 || fd5 != 5)
		error("Files opened on wrong descriptors\n");

	fout = dup(1);
	printf("UNIX (patched up RT-11) Dungeon\n");
	printf("This game loads overlays like crazy and loads down the\n");
	printf("system quite a lot, please limit to nights and weekends!!\n");
	printf("The TIME, SAVE, and RESTORE commands will cause\n");
	printf("Dungeon to \"EMT trap -- core dumped\" and bomb out.\n");
	printf("If you get a core dump, from other error, save it for ghg.\n\n\n");
	printf("New experimental non debugged commands (all start in col 1)\n");
	printf(">                       Saves game on file dungeon.dat\n");
	printf("<                       Restors game from file dungeon.dat\n");
	printf("!UNIX-COMMAND           Execute UNIX-COMMAND from Dungeon\n");
	printf("                        This trashes next dungeon command\n");
	printf("\n\n\n");

	printf("Welcome to Dungeon.\t\tThis version created 10-SEP-78.\n");
	printf("You are in an open field west of a big white house with a boarded\n");
	printf("front door.\n");
	printf("There is a small mailbox here.\n>");
	flush();
	signal(2,1);
	execl("dung","DUNGEON", 0);
	execl("/v/ghg/dung/dung","DUNGEON",0);
	execl("/usr/bin/dung","DUNGEON", 0);
	printf("Can't start dungeons.\n");
	flush();
	exit();
}
error(s)
char *s;
{
	printf("%s", s);
	flush();
	exit(1);
}

