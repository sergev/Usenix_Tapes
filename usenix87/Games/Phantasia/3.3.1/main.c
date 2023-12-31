/*
 * Phantasia 3.3.1 -- Interterminal fantasy game
 *
 * Edward A. Estes
 * AT&T, August 28, 1985
 */

/* DISCLAIMER:
 *
 * This game is distributed for free as is.  It is not guaranteed to work
 * in every conceivable environment.  It is not even guaranteed to work
 * in ANY environment.
 *
 * This game is distributed without notice of copyright, therefore it
 * may be used in any manner the recipient sees fit.  However, the
 * author assumes no responsibility for maintaining or revising this
 * game, in its original form, or any derivitives thereof.
 *
 * The author shall not be responsible for any loss, cost, or damage,
 * including consequential damage, caused by reliance on this material.
 *
 * The author makes no warranties, express or implied, including warranties
 * of merchantability or fitness for a particular purpose or use.
 *
 * AT&T is in no way connected with this game.
 */

/*
 * This is the program which drives the whole mess.  Hopefully, you will be
 * able to wade through the garbage if you have to fix something.
 * several undocumented items exist.  The program checks uid and sets the
 * boolean flag 'su' (super user) if the person is allowed special powers.
 * This only happens if the game is invoked with '-S' as an option.
 * The 'su' may execute any of the valar/council options.  Also,
 * a 'vaporize' option exists to kill anybody at will.	The 'su' can select
 * character type 7, which starts out with the maximum possible in each
 * category.  (The resulting character is an experimento.)  The 'su' may
 * also change the stats of other characters with the -x option.
 */

/*
 * The program allocates as much file space as it needs to store characters,
 * so the possibility exists for the character file to grow without bound.
 * The file is purged upon normal entry to try to avoid that problem.
 * A similar problem exists for energy voids.  To alleviate the problem here,
 * the void file is cleared with every new king.
 */

/*
 * The support functions are split between various files with no apparent
 * order.  Use of 'ctags' is recommended to find a particular function.
 */

/*
 * Put one line of text into the file 'motd' for announcements, etc.
 */

/*
 * If ENEMY is defined, a list of restricted login names is checked
 * in the file 'enemy'.  These names are listed, one per line, with
 * no trailing blanks.
 */

/*
 * The scoreboard file is updated when someone dies, and keeps track
 * of the highest character to date for that login.
 * Being purged from the character file does not cause the scoreboard
 * to be updated.
 */

#include "phant.h"

double	strength, speed;
bool	beyond, marsh, throne, valhala, changed, fghting, su, wmhl, timeout;
int fileloc, users;
jmp_buf fightenv, mainenv;
long	secs;

#ifndef SMALL
/*
 * worm hole map -- This table is carefully set up so that one can always
 * return the way he/she came by inverting the initial path.
 */
struct	worm_hole   w_h[] =
    {
    0,0,0,0,	35,22,2,0,  2,2,0,1,	59,34,64,0,
    54,47,0,60, 50,62,0,56, 19,31,25,0, 0,35,41,41,
    0,46,40,23, 24,0,29,30, 44,57,56,0, 0,44,39,40,
    61,41,0,42, 32,0,17,18, 57,0,63,64, 0,33,26,34,
    48,0,54,55, 28,23,22,13,	63,25,13,19,	34,6,18,20,
    27,26,19,21,    15,27,20,27,    1,28,34,17, 17,29,8,24,
    29,9,23,25, 18,30,24,6, 20,32,27,15,    21,20,21,26,
    22,17,46,29,    23,24,28,9, 25,38,9,31, 6,39,30,32,
    26,13,31,33,    15,40,32,35,    3,19,15,22, 7,1,33,36,
    37,37,35,37,    36,36,36,38,    30,42,37,39,    31,43,38,11,
    33,45,11,8, 12,48,7,7,  38,49,12,43,    39,51,42,44,
    11,10,43,45,    40,52,44,46,    8,53,45,28, 4,54,51,48,
    41,16,47,49,    42,55,48,50,    62,5,49,51, 43,56,50,47,
    45,58,53,53,    46,59,52,52,    47,4,55,16, 49,61,16,54,
    51,63,5,10, 10,14,59,58,	52,64,57,59,	53,3,58,57,
    60,60,4,61, 55,12,60,62,	5,50,61,63, 56,18,62,14,
    58,33,14,3
    };
#endif

main(argc,argv) 	    /* Phantasia main routine */
int	argc;
char	*argv[];
{
static	struct	stats	charac;
char	aline[200], *login = NULL;
double	x = 0.0, y = 0.0;
int	ch, ch2, loop, temp;
FILE	*fp;
bool	shrt = FALSE, examine = FALSE, header = FALSE;

    if ((login = getlogin()) == NULL)
	login = getpwuid(getuid())->pw_name;
#ifdef ENEMY
    /* check hit list of restricted accounts */
    if ((fp = fopen(enemyfile, "r")) != NULL)
	{
	char	enemy[25];

	while (fscanf(fp, "%s", enemy) != EOF)
	if (!strcmp(login,enemy))
	    {
	    printf ("The Phantasia privileges for the account \"%s\" have been revoked.\n", login);
	    printf ("Mail comments to %s.\n", WIZARD);
	    exit (0);
	    }
	fclose (fp);
	}
#endif
    setbuf(stdin, (char *) NULL);   /* this may or may not be necessary */
    su = FALSE;
    fghting = FALSE;
    timeout = FALSE;
    users = 0;
    while (--argc && (*++argv)[0] == '-')
	switch ((*argv)[1])
	    {
	    case 'h':	    /* help */
		if ((fp = fopen(helpfile, "r")) != NULL)
		    {
		    while (fgets(aline, 200, fp))
		    fputs(aline, stdout);
		    fclose(fp);
		    }
		exit(0);
		/*NOTREACHED*/
		case 's':	    /* short */
		    shrt = TRUE;
		    break;
		case 'x':	    /* examine */
		    examine = TRUE;
		    break;
		case 'H':	    /* Header */
		    header = TRUE;
		    break;
		case 'm':	    /* monsters */
		    printmonster();
		    exit(0);
		    /*NOTREACHED*/
		case 'a':	    /* all users */
		    showusers(FALSE);
		    exit(0);
		    /*NOTREACHED*/
		case 'p':	    /* purge old players */
		    purge();
		    exit(0);
		    /*NOTREACHED*/
		case 'S':
		    su = (getuid() == UID);
		    break;
		case 'b':
		    show_sb();
		    exit(0);
		    /*NOTREACHED*/
		}
    if (!isatty(0)) /* don't let non-tty's play */
	exit(0);
    init1();	/* set up for screen stuff */
    if (examine)
	{
	cstat();
	exit1();
	/*NOTREACHED*/
	}
    if (!shrt)
	{
	titlestuff();
	purge();    /* clean up old characters */
	}
    if (header)
	{
	exit1();
	/*NOTREACHED*/
	}
#ifdef OK_TO_PLAY
    if (!ok_to_play())
	{
	mvaddstr(23,27,"Sorry, you can't play now.\n");
	exit1();
	/*NOTREACHED*/
	}
#endif
    mvaddstr(22,24,"Do you have a character to run? ");
    ch = getans("NY", FALSE);
    if (ch == 'Y')
	fileloc = findchar(&charac);
    else
	{
	initchar(&charac);
	clear();
	mvaddstr(4,21,"Which type of character do you want:");
	mvaddstr(8,4,"1:Magic User  2:Fighter  3:Elf  4:Dwarf  5:Halfling  6:Experimento  ? ");
	ch = getans("2134567", FALSE);
	do
	    {
	    genchar(&charac,ch);
	    mvprintw(12,14,"Strength:  %2.0f  Mana       : %3.0f  Quickness   :  %2d",
	    charac.str,charac.man,charac.quk);
	    mvprintw(13,14,"Brains  :  %2.0f  Magic Level:  %2.0f  Energy Level:  %2.0f",
	    charac.brn,charac.mag,charac.nrg);
	    if (charac.typ != 6)
		{
		mvaddstr(14,14,"Type '1' to keep >");
		ch2 = getans(" ", TRUE);
		}
	    else
		break;
	    }
	while (ch2 != '1');
	if (charac.typ == 6)
	    for (;;)
		{
		mvaddstr(16,0,"Enter the X Y coordinates of your experimento ? ");
		getstring(aline,80);
		sscanf(aline,"%F %F",&charac.x,&charac.y);
		if (abs(charac.x) > 1.2e6 || abs(charac.y) > 1.2e6)
		    mvaddstr(17,0,"Invalid coordinates.  Try again.\n");
		else
		    break;
		}
	for (;;)
	    {
	    mvaddstr(18,0,"Give your character a name [up to 20 characters] ?  ");
	    getstring(aline,80);
	    strncpy(charac.name,aline,20);
	    charac.name[20] = '\0';
	    trunc(charac.name);
	    if (charac.name[0] == '\0')
		mvaddstr(19,0,"Invalid name.");
	    else if (findname(charac.name, (struct stats *) NULL) >= 0)
		mvaddstr(19,0,"Name already in use.");
	    else
		break;
	    addstr("  Pick another.\n");
	    }
	putchar('\n');
	fflush(stdout);
	nocrmode();
	do
	    {
	    strcpy(charac.pswd,getpass("Give your character a password [up to 8 characters] ? "));
	    putchar('\n');
	    strcpy(aline,getpass("One more time to verify ? "));
	    }
	while (strcmp(charac.pswd,aline));
	fileloc = findspace();
	}
    crmode();
    if (charac.status)
	{
	clear();
	addstr("Your character did not exit normally last time.\n");
	addstr("If you think you have good cause to have your character saved,\n");
	printw("you may quit and mail your reason to '%s'.\n",WIZARD);
	addstr("Otherwise, continuing spells certain death.\n");
	addstr("Do you want to quit ? ");
	ch = getans("YN", FALSE);
	if (ch== 'Y')
	    {
	    charac.tampered = charac.quk;	/* store this away */
	    charac.quk = -100;
	    leave(&charac);
	    /*NOTREACHED*/
	    }
	death(&charac, "Stupidity");
	}
    charac.status = PLAYING;
    if (charac.lvl > 5)
	timeout = TRUE;
    strcpy(charac.login,login);
    time(&secs);
    charac.lastused = localtime(&secs)->tm_yday;
    update(&charac,fileloc);
    clear();
#ifdef	BSD41
    sigset(SIGINT,interrupt);
#endif
#ifdef	BSD42
    signal(SIGINT,interrupt);
#endif
#ifdef	SYS3
    signal(SIGINT,interrupt);
#endif
#ifdef	SYS5
    signal(SIGINT,interrupt);
#endif

    /* all set for now */

TOP:
    switch (setjmp(mainenv))
	{
	case QUIT:
#ifdef	BSD41
	    sigrelse(SIGINT);
#endif
#ifdef	BSD42
	    signal(SIGINT,interrupt);
#endif
#ifdef	SYS3
	    signal(SIGINT,interrupt);
#endif
#ifdef	SYS5
	    signal(SIGINT,interrupt);
#endif
	    leave(&charac);
	    /*NOTREACHED*/
	case DIE:
#ifdef	BSD41
	    sigrelse(SIGINT);
#endif
#ifdef	BSD42
	    signal(SIGINT,interrupt);
#endif
#ifdef	SYS3
	    signal(SIGINT,interrupt);
#endif
#ifdef	SYS5
	    signal(SIGINT,interrupt);
#endif
	    death(&charac, "Bailing out");
	    break;
	}
#ifdef OK_TO_PLAY
    if (!ok_to_play())
	{
	mvaddstr(6,0,"Whoops!  Can't play now.\n");
	leave(&charac);
	/*NOTREACHED*/
	}
#endif
    fghting = FALSE;
    adjuststats(&charac);
    if (throne && !charac.crn && (charac.typ < 10 || charac.typ > 20))
	{
	mvaddstr(5,0,"You're not allowed in the Lord's Chamber without a crown.\n");
	changed = TRUE;
	charac.x = charac.y = 10 + roll(8, 5);
	}
    if (charac.status != CLOAKED && abs(charac.x) == abs(charac.y)
    && floor(sqrt(fabs(charac.x/100.0))) == sqrt(fabs(charac.x/100.0)) && !throne)
	{
	trade(&charac);
	clear();
	}
    checktampered(&charac);
    checkinterm(&charac);
    if (charac.nrg < 0 || (charac.lvl >= 10000 && charac.typ != 99))
	death(&charac, "Interterminal battle");
    neatstuff(&charac);
    if (changed)
	{
	update(&charac,fileloc);
	changed = FALSE;
	goto TOP;
	}
    mesg();
    printstats(&charac);
    move(6,0);
#ifndef SMALL
    if (!wmhl)
#endif
	{
	if (throne)
	    kingstuff(&charac);
	addstr("NSEW1:Move  2:Players  3:Talk  4:Stats  5:Quit  ");
	if (charac.lvl >= 7 && charac.mag >= 20)
	    addstr("6:Cloak  ");
	if (charac.lvl >= 12 && charac.mag >= 40)
	    addstr("7:Teleport  ");
	if (charac.typ > 20)
	    addstr("8:Intervene");
	ch = gch(charac.rng.type);
	mvaddstr(4,0,"\n\n");
	if (charac.status == CLOAKED)
	    if (charac.man > 3.0)
		charac.man -= 3;
	    else
		{
		charac.status = PLAYING;
		changed = TRUE;
		}
	move(7,0);
	clrtobot();
	if (charac.typ == 99 && (ch == '1' || ch == '7'))
	    ch = ' ';
	switch (ch2 = ch)
	    {
	    case 'N':
		charac.y += maxmove;
		break;
	    case 'S':
		charac.y -= maxmove;
		break;
	    case 'E':
		charac.x += maxmove;
		break;
	    case 'W':
		charac.x -= maxmove;
		break;
	    default:    /* rest */
		charac.nrg += (charac.mxn+charac.shd)/15+charac.lvl/3+2;
		charac.nrg = min(charac.nrg,charac.mxn + charac.shd);
		if (charac.status != CLOAKED)
		    {
		    charac.man += circ(charac.x,charac.y)/4;
		    charac.man += charac.lvl/6;
		    rndattack();
		    }
		break;
	    case '1':   /* move */
		for (loop = 3; loop; --loop)
		    {
		    mvaddstr(4,0,"X Y Coordinates ? ");
		    getstring(aline,80);
		    if (sscanf(aline,"%F %F",&x,&y) != 2)
			mvaddstr(5,0,"Try again\n");
		    else if (dist(charac.x, x, charac.y, y) > maxmove)
			illmove();
		    else
			{
			charac.x = x;
			charac.y = y;
			break;
			}
		    }
		break;
	    case '2':   /* players */
		printplayers(&charac);
		break;
	    case '3':   /* message */
		mvaddstr(4,0,"Message ? ");
		getstring(aline, 80);
		fp = fopen(messfile, "w");
		if (*aline)
		    fprintf(fp, "%s: %s", charac.name, aline);
		fclose(fp);
		break;
	    case '4':	/* stats */
		showall(&charac);
		break;
	    case '5':	/* good-bye */
		leave(&charac);
		/*NOTREACHED*/
	    case '6':	/* cloak */
		if (charac.lvl < 7 || charac.mag < 20)
		    illcmd();
		else if (charac.status == CLOAKED)
		    charac.status = PLAYING;
		else if (charac.man < 35)
		    {
		    mvaddstr(5,0,"No power left.\n");
		    refresh();
		    }
		else
		    {
		    changed = TRUE;
		    charac.man -= 35;
		    charac.status = CLOAKED;
		    }
		break;
	    case '7':	/* teleport */
		if (charac.lvl < 12 || charac.mag < 40)
		    illcmd();
		else 
		    for (loop = 3; loop; --loop)
			{
			mvaddstr(4,0,"X Y Coordinates ? ");
			getstring(aline,80);
			if (sscanf(aline,"%F %F",&x,&y) == 2)
			    if ((temp = dist(charac.x,x,charac.y,y))
				> (charac.lvl+charac.mag)*20+((charac.typ > 30) ? 1e+6 : 0)
				&& !throne)
				    illmove();
			    else if ((temp = (temp/75+1)*20) > charac.man && !throne)
				mvaddstr(5,0,"Not enough power for that distance.\n");
			    else
				{
				charac.x = x;
				charac.y = y;
				if (!throne)
				charac.man -= temp;
				break;
				}
			}
		break;
	    case '9':	/* monster */
		if (throne)
		    mvaddstr(5,0,"No monsters in the chamber!\n");
		else if (charac.typ != 99)
		    {
		    charac.status = PLAYING;
		    changed = TRUE;
		    charac.sin += 1e-6;
		    fight(&charac,-1);
		    }
		break;
	    case '0':	/* decree */
		if (su || charac.typ > 10 && charac.typ < 20 && throne)
		    decree(&charac);
		else
		    illcmd();
		break;
	    case '8':	/* intervention */
		if (su || charac.typ > 20)
		    valarstuff(&charac);
		else
		    illcmd();
		break;
	    case '\014':    /* redo screen */
		clear();
	    }
	if (ch2 == 'E' || ch2 == 'W' || ch2 == 'N' || ch2 == 'S'
	    || ch2 == '1' || ch2 == '7')
	    {
	    checkmov(&charac);
	    rndattack();
	    changed = TRUE;
	    }
	}
#ifndef SMALL
    else
	{
	addstr("F:Forward  B:Back  R:Right  L:Left  Q:Quit  T:Talk  P:Players  S:Stats  ");
	ch = getans(" ", TRUE);
	move(6,0);
	clrtobot();
	if (charac.status == CLOAKED)
	    if (charac.man > 3.0)
		charac.man -= 3;
	    else
		{
		charac.status = PLAYING;
		changed = TRUE;
		}
	switch (ch)
	    {
	    default:	/* rest */
		charac.nrg += (charac.mxn+charac.shd)/15+charac.lvl/3+2;
		charac.nrg = min(charac.nrg,charac.mxn + charac.shd);
		if (charac.status != CLOAKED)
		    charac.man += charac.lvl/5;
		break;
	    case 'F':
		temp = w_h[charac.wormhole].f;
		goto CHKMOVE;
	    case 'B':
		temp = w_h[charac.wormhole].b;
		goto CHKMOVE;
	    case 'R':
		temp = w_h[charac.wormhole].r;
		goto CHKMOVE;
	    case 'L':
		temp = w_h[charac.wormhole].l;
		goto CHKMOVE;
	    case 'Q':
		leave(&charac);
		/*NOTREACHED*/
	    case 'T':
		mvaddstr(4,0,"Message ? ");
		getstring(aline, 80);
		fp = fopen(messfile, "w");
		if (*aline)
		    fprintf(fp, "%s: %s", charac.name, aline);
		fclose(fp);
		break;
	    case 'P':
		printplayers(&charac);
		break;
	    case 'S':
		showall(&charac);
		break;
	    case '\014':    /* redo screen */
		clear();
	    }
	goto TOP;
CHKMOVE:
	if (!temp)
	    {
	    charac.y = 0.0;
	    charac.x = pow(-1.0,(double) charac.wormhole) * charac.wormhole * 400 - 1.0;
	    charac.wormhole = 0;
	    changed = TRUE;
	    }
	else
	    charac.wormhole = temp;
	}
#endif
    goto TOP;
}

/*
 * This function is provided to allow one to restrict access to the game.
 * Tailor this routine as appropriate.
 */

#ifdef	OK_TO_PLAY
#include <sys/types.h>
#include <utmp.h>   /* used for counting users on system */

bool	ok_to_play()	    /* return FALSE if playing is not allowed at this time */
{
#define MAXUSERS    30	/* max. number of people on system */
reg struct  tm	*tp;
reg int numusers = 0;
FILE	*fp;
long	now;
struct	utmp	ubuf;

    if (su)
	return (TRUE);
    /* check time of day */
    time(&now);
    if (((tp = localtime(&now))->tm_hour > 8 && tp->tm_hour < 12)   /* 8-noon */
	|| (tp->tm_hour > 12 && tp->tm_hour < 16))	/* 1-4 pm */
	    return (FALSE);
    /* check # of users */
    if ((fp = fopen("/etc/utmp","r")) != NULL)
	{
	while (fread((char *) &ubuf,sizeof(ubuf),1,fp))
#ifdef	SYS5
	    if (ubuf.ut_type == USER_PROCESS)
#else
	    if (*ubuf.ut_name)
#endif
		++numusers;
	fclose(fp);
	if (numusers > MAXUSERS)
	    return (FALSE);
	}
    return (TRUE);
}
#endif
