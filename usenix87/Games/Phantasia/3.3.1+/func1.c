/*
 * func1.c  Phantasia support routines
 */

#include "phant.h"

int
findname(name,stat) 	    		/* return location of character 'name' */
reg char    *name;		    		/* return -1 if not found, fill structure */
reg struct stats *stat; 	    	/* if pointer is non-null */
{
	struct	stats	sbuf;
	int		loc = 0;

	if (stat == NULL)
		stat = &sbuf;
	fseek(read_pfile,0L,0);
	while (fread((char *) stat,sizeof(sbuf),1,read_pfile))
	if (!strcmp(stat->name,name))
		return (loc);
	else
		++loc;
	return (-1);
}

/****************************************************************/

int
findspace() 	    /* allocate space for a character in peoplefile */
{
	struct	stats	buf;
	reg int loc;

	loc = 0;
	fseek(read_pfile,0L,0);
	while (fread((char *) &buf,sizeof(buf),1,read_pfile))
	{
		if (!strcmp(buf.name,"<null>"))
			return (loc);
		else
			++loc;
	}
	fseek(access_pfile,(long) (loc * sizeof(buf)),0);
	initchar(&buf);
	strcpy(buf.name,"inuse");
	fwrite((char *) &buf,sizeof(buf),1,access_pfile);
	fflush(access_pfile);
	return (loc);
}

/****************************************************************/

int
findchar(stat)		/* retrieve a character from file */

reg struct  stats   *stat;
{
	reg int loc = 0, loop;
	char	name[21];

	clear();
	mvprintw(10,0,"Enter your character's name: ");
	getstring(name,21);
	trunc(name);
	if ((loc = findname(name, stat)) >= 0)
	{
		move(11,0);
		refresh();
		nocrmode();
		for (loop = 0; loop < 2; ++loop)
			if (!strcmp(getpass("Password: "),stat->pswd))
			{
				crmode();
				return (loc);
			}
			else
				printf("No good.\n");
		exit1();
		/*NOTREACHED*/
	}
	else
		addstr("\n\nNot found.\n");
	exit1();
	/*NOTREACHED*/
}

/****************************************************************/

void
leave(stat)		/* save character in file */

reg struct  stats   *stat;
{
	long	ltemp;

	if (!stat->lvl)
		strcpy(stat->name,"<null>");
	stat->status = OFF;
	time(&ltemp);
	stat->age += ltemp - secs;
	update(stat,fileloc);
	exit1();
	/*NOTREACHED*/
}

/****************************************************************/

void
death(stat, how)		/* remove a player after dying */

reg struct  stats   *stat;
char *how;
{
	FILE	*fp;
	char	aline[100];
	int 	ch;
	reg int loop;
	long	ltemp;
	float	temp;
	float	allowed_gold;

	clear();
	if (stat->typ == 99)
		if (stat->rng.duration)
		{
			addstr("Valar should be more cautious.  You've been killed.\n");
			printw("You only have %d more chance(s).\n",--stat->rng.duration);
			more(3);
			stat->energy = stat->mxn;
			return;
		}
		else
		{
			addstr("You had your chances, but Valar aren't totally\n");
			addstr("immortal.  You are now left to wither and die . . .\n");
			more(3);
			stat->brn = stat->lvl /25;
			stat->energy = stat->mxn;
			stat->quks = stat->swd = 0;
			stat->typ = 90;
			return;
		}

	if (stat->lvl > 9999)
	{
		addstr("Characters greater than level 10K must be retired.  Sorry.");
		how = "Old age";
	}

	switch(stat->rng.type)
	{
		case -DLREG:
		case -NAZREG:
			mvaddstr(4,0,"Your ring saved you from death!\n");
			refresh();
			stat->rng.type = NONE;
			stat->energy = stat->mxn/12+1;
			stat->crn -= (stat->crn > 0);
			return;
		case DLBAD:
		case -DLBAD:
		case NAZBAD:
		case -NAZBAD:
		case -SPOILED:
		case SPOILED:
			mvaddstr(4,0,"Your ring has taken control of you and turned you into a monster!\n");
			fseek(mfile,0L,0);
			for (loop = 0; loop <= 13; ++loop)
				fgets(aline,100,mfile);
			ltemp = ftell(mfile);
			fp = fopen(monsterfile,ACCESS);
			fseek(fp,ltemp,0);
			fprintf(fp,"%-20s",stat->name);
			fclose(fp);
	}
	clear();
	move(4,0);
	switch ((int) roll(1,5))
	{
		case 1:
			addstr("Aaackk! You're dead!  ");
			break;
		case 2:
			addstr("You have been disemboweled.  ");
			break;
		case 3:
			addstr("You've been mashed, mauled, and spat upon.  (You're dead.)\n");
			break;
		case 4:
			addstr("You died!  ");
			break;
		case 5:
			addstr("You're a complete failure -- you've died!!\n");
	}
	refresh();

	/* max 3 chances to live again if good */

	if (stat->lvl >= 10 && stat->sin < 1.0 && stat->lives < 3)
	{
		sleep(3);
		timeout = FALSE;
		clear();
		mvaddstr(6,0,"Your soul floats in vast, misty space, lit by shafts of pale sunlight.");
		mvaddstr(8,0,"Time passes slowly.  You cannot distinguish seconds from days.");
		refresh();
		sleep(4);
		mvaddstr(10,0,"...suddenly, a Valkyrie in full armor appears before you!");
		mvprintw(12,0,"\"Greetings, %s\", she says, in a clear, ringing voice.",stat->name);
		mvaddstr(13,0,"\"Since your sins are few, you will have a chance to live once more!\"");
		more(15);
		move(15,0);
		clrtoeol();
		if (stat->vrg == TRUE)
		{
			mvaddstr(15,0,"\"But I must have a soul to take with me\", she continues. \"Shall I take the soul");
			mvaddstr(16,0,"of the innocent virgin who accompanies you instead of your own?\" ");
			ch = getans("NY", FALSE);
			if (ch == 'Y')
			{
				mvaddstr(18,0,"The Valkyrie frowns.  \"Valhalla has no place for those who sacrifice the");
				mvaddstr(19,0,"innocent to save themselves!\", she says sternly. \"You are dammned.\"");
				refresh();
				more(20);
				clear();
			}
			else
			{
				mvaddstr(18,0,"\"Your sacrifice has won you your soul!\", she cries. \"Live and prosper.\"");
				refresh();
				more(20);
				clear();
				stat->energy = stat->mxn;
				stat->lives++;
				return;
			}
		}
		else
		{
			mvaddstr(16,0,"\"You must live a godly life and give generously to the poor\", she warns.");
			refresh();
			more(18);
			stat->energy = stat->mxn;
			stat->lives++;
			clear();
			mvaddstr(8,0,"You blink, and open your eyes on the real world again.");
			mvaddstr(10,0,"As you stand dazed, you realize a holy friar is approaching you.");
			mvprintw(11,0,"\"Will you help the needy, %s?\" he asks, holding out",stat->name);
			mvaddstr(12,0,"a large wooden box with a slot in the top.");
			mvprintw(14,0,"You have %-9.0f gold pieces.",stat->gld);
			mvaddstr(16,0,"How much gold will you give him? ");
			temp = inflt();
			loop = 0;
			allowed_gold = stat->gld/10;
			while (loop < 4)
			{
				loop++;
				stat->gld = max(0,stat->gld - floor(temp));
				if (stat->gld > allowed_gold)
				{
					move(16,0);
					clrtoeol();
					mvprintw(14,0,"You have %-9.0f gold pieces.",stat->gld);
					mvaddstr(17,0,"The ghostly form of the Valkyrie appears, ready to take your soul away!");
					mvaddstr(18,0,"Hastily, you give the friar some more gold: ");
					temp = inflt();
					move(17,0);
					clrtoeol();
					move(18,0);
					clrtoeol();
					refresh();
					sleep(1);
				}
				else
				{
					if (stat->gem > 0.5)
					{
						move(16,0);
						clrtoeol();
						move(17,0);
						clrtoeol();
						move(18,0);
						clrtoeol();
						mvaddstr(15,0,"He still looks expectant.  You give him your gems as well.");
						stat->gem = 0;
					}
					move(16,0);
					clrtoeol();
					mvaddstr(16,0,"The Valkyrie fades away...");
					refresh();
					more(18);
					clear();
					stat->energy = stat->mxn;
					stat->lives++;
					return;
				}
			}
			mvaddstr(17,0,"The Valkyrie solidifies. \"Greed is a deadly sin,\" she says, and pulls your");
			mvaddstr(18,0,"soul from your body...");
			more(19);
			clear();
		}
	}
	scoreboard(stat);
	fp = fopen(lastdead,"w");
	if (*how == 'A')
		*how = tolower(*how);
	fprintf(fp,"%s (%s, run by %s, level %d, killed by %s)", stat->name,
		ptype('l',stat->typ),stat->login,stat->lvl,how);
	fclose(fp);
	fp = fopen(messfile,"w");
	fprintf(fp,"%s was killed by %s.", stat->name,how);
	fclose(fp);
	strcpy(stat->name,"<null>");
	update(stat,fileloc);
	mvaddstr(18,0,"Care to give it another try? ");
	ch = getans("NY", FALSE);
	if (ch == 'Y')
	{
		endwin();
		execl(gameprog, "phantasia", "-s", (su ? "-S": (char *) NULL), 0);
	}
	exit1();
	/*NOTREACHED*/
}

/****************************************************************/

void
update(stat,place)	    /* update charac file */

reg struct  stats   *stat;
reg int place;
{
	fseek(access_pfile,(long) (place*sizeof(*stat)),0);
	fwrite((char *) stat,sizeof(*stat),1,access_pfile);
	fflush(access_pfile);
}

/****************************************************************/

void
printplayers(stat)	    /* show users */

reg struct  stats   *stat;
{
	struct	stats	buf;
	reg int loop = 0;
	double	mloc;
	long	ltmp;
	int 	ch;

	if (stat->blind)
	{
		mvaddstr(8,0,"You can't see anyone.\n");
		return;
	}
	mloc = circ(stat->x,stat->y);
	mvaddstr(8,0,"Name                             X         Y     Lvl Type Login    Status\n\n");
	fseek(read_pfile,0L,0);
	while (fread((char *) &buf,sizeof(buf),1,read_pfile))
	{
		if (strcmp(buf.name, "<null>") == 0)
			continue;
		if (buf.status)
		{
			ch = (buf.status == CLOAKED) ? '?' : 'W';
			if (stat->typ > 10 || buf.typ > 10 || 
				mloc >= circ(buf.x,buf.y) || stat->pal)
			{
				if (buf.status != CLOAKED || (stat->typ == 99 && stat->pal))
					if (buf.typ == 99)
						addstr("The Valar is watching you...\n");
					else if (buf.wormhole)
						printw("%-20s               %c         %c %6u ",
									buf.name,ch,ch,buf.lvl);
					else
						printw("%-20s       %8.0f  %8.0f %6u ",
									buf.name,buf.x,buf.y,buf.lvl);
			}
			else
				printw("%-20s %24.24s %6u ",
							buf.name,printloc('s',buf.x,buf.y),buf.lvl);
		}
		else if (buf.typ == 99)
			--loop;
		else
			printw("%-20s %24.24s %6u ",
							buf.name,printloc('s',buf.x,buf.y),buf.lvl);
		printw (ptype('s', buf.typ));
		printw ("  %-8.8s ", buf.login);
		switch (buf.status)
		{
			case PLAYING:
				printw("In Game\n");
				break;
			case INBATTLE:
				printw("In Battle\n");
				break;
			case OFF:
				printw("Off\n");
				break;
			default:
				printw("\n");
				break;
		}
		++loop;
	}
	time(&ltmp);
	printw("\nTotal users = %d    %s\n",loop,ctime(&ltmp));
	refresh();
}

/****************************************************************/

void
titlestuff()		    /* print out a header */
{
	FILE	*fp;
	bool	cowfound = FALSE, kingfound = FALSE;
	struct	stats	buf;
	double	hiexp, nxtexp;
	int		hilvl, nxtlvl;
	reg int loop;
	char	instr[120], hiname[21], nxtname[21];

	mvaddstr(0,14,"W e l c o m e   t o   P h a n t a s i a (vers. 3.3.1+)!");
	mvaddstr(1,20,"Modifed by Chris Robertson, September 1985");
	if ((fp = fopen(motd,"r")) != NULL && fgets(instr,80,fp))
	{
		mvaddstr(3,40 - strlen(instr)/2,instr);
		fclose(fp);
	}
	fseek(read_pfile,0L,0);
	while (fread((char *) &buf,sizeof(buf),1,read_pfile))
		if (buf.typ > 10 && buf.typ < 20)
		{
			sprintf(instr,"The present ruler is %s  Level:%d",buf.name,buf.lvl);
			mvaddstr(5,40 - strlen(instr)/2,instr);
			kingfound = TRUE;
			break;
		}
	if (!kingfound)
		mvaddstr(5,24,"There is no ruler at this time.");
	fseek(read_pfile,0L,0);
	while (fread((char *) &buf,sizeof(buf),1,read_pfile))
		if (buf.typ == 99)
		{
			sprintf(instr,"The Valar is %s   Login:  %s",buf.name,buf.login);
			mvaddstr(7,40 - strlen(instr)/2,instr);
			break;
		}
	fseek(read_pfile,0L,0);
	while (fread((char *) &buf,sizeof(buf),1,read_pfile))
		if (buf.typ > 20 && buf.typ < 90)
		{
			if (!cowfound)
			{
				mvaddstr(9,30,"Council of the Wise:");
				loop = 10;
				cowfound = TRUE;
			}

			/* This assumes a finite (<=5) number of C.O.W.: */

			sprintf(instr,"%s   Login:  %s",buf.name,buf.login);
			mvaddstr(loop++,40 - strlen(instr)/2,instr);
		}
	fseek(read_pfile,0L,0);
	nxtname[0] = hiname[0] = '\0';
	hiexp = 0.0;
	nxtlvl = hilvl = 0;
	while (fread((char *) &buf,sizeof(buf),1,read_pfile))
		if (buf.exp > hiexp && buf.typ < 20)
		{
			if (strcmp(buf.name, "<null>") == 0)
				continue;
			nxtexp = hiexp;
			hiexp = buf.exp;
			nxtlvl = hilvl;
			hilvl = buf.lvl;
			strcpy(nxtname,hiname);
			strcpy(hiname,buf.name);
		}
		else if (buf.exp > nxtexp && buf.typ < 20)
		{
			nxtexp = buf.exp;
			nxtlvl = buf.lvl;
			strcpy(nxtname,buf.name);
		}
	if (*hiname)
		if (*nxtname)
		{
			mvaddstr(16,28,"Highest characters are:");
			sprintf(instr,"%s (level %d),  and  %s (level %d)",
				hiname,hilvl,nxtname,nxtlvl);
		}
		else
		{
			mvaddstr(16,28,"Highest character is:");
			sprintf(instr,"%s (level %d)", hiname,hilvl);
		}
		else
			strcpy(instr, "No current highest characters");
	mvaddstr(18,40 - strlen(instr)/2,instr);
	if ((fp = fopen(lastdead,"r")) != NULL)
	{
		if (fgets(instr,80,fp) != NULL && *instr)
		{
			mvaddstr(20,25,"The last character to die was:");
			mvaddstr(21,40 - strlen(instr)/2,instr);
		}
		fclose(fp);
	}
	refresh();
}

/****************************************************************/

void
printmonster()		    /* do a monster list on the terminal */
{
	FILE	*fp;
	reg int count = 0;
	char	instr[100];

	puts(" #  Name              Str     Brains  Quick   Hits    Exp    Treas Type   Flk%\n");
	fseek(mfile,0L,0);
	while (fgets(instr,100,mfile))
		printf("%2d  %s",count++,instr);
}

/****************************************************************/

void
exit1() 		/* exit, but cleanup */
{
	move(23,0);
	refresh();
	nocrmode();
	endwin();
	exit(0);
	/*NOTREACHED*/
}

/****************************************************************/

void
init1() 		/* set up for screen updating */
{
	/* catch/ingnore signals */
#ifdef	BSD41
	sigignore(SIGQUIT);
	sigignore(SIGALRM);
	sigignore(SIGTERM);
	sigignore(SIGTSTP);
	sigignore(SIGTTIN);
	sigignore(SIGTTOU);
	sigset(SIGINT,exit1);		/* allow interrupts in startup stuff */
	sigset(SIGHUP,ill_sig);
	sigset(SIGTRAP,ill_sig);
	sigset(SIGIOT,ill_sig);
	sigset(SIGEMT,ill_sig);
	sigset(SIGFPE,ill_sig);
	sigset(SIGBUS,ill_sig);
	sigset(SIGSEGV,ill_sig);
	sigset(SIGSYS,ill_sig);
	sigset(SIGPIPE,ill_sig);
#endif
#ifdef	BSD42
	signal(SIGQUIT,SIG_IGN);
	signal(SIGALRM,SIG_IGN);
	signal(SIGTERM,SIG_IGN);
	/* signal(SIGTSTP,SIG_IGN); 		commented out for V7; leave in for BSD
	signal(SIGTTIN,SIG_IGN);
	signal(SIGTTOU,SIG_IGN); */
	signal(SIGINT,exit1);		/* allow interrupts in startup stuff */
	signal(SIGHUP,ill_sig);
	signal(SIGTRAP,ill_sig);
	signal(SIGIOT,ill_sig);
	signal(SIGEMT,ill_sig);
	signal(SIGFPE,ill_sig);
	signal(SIGBUS,ill_sig);
	signal(SIGSEGV,ill_sig);
	signal(SIGSYS,ill_sig);
	signal(SIGPIPE,ill_sig);
#endif
#ifdef	SYS3
	signal(SIGINT,exit1);		/* allow interrupts in startup stuff */
	signal(SIGQUIT,SIG_IGN);
	signal(SIGTERM,SIG_IGN);
	signal(SIGALRM,SIG_IGN);
	signal(SIGHUP,ill_sig);
	signal(SIGTRAP,ill_sig);
	signal(SIGIOT,ill_sig);
	signal(SIGEMT,ill_sig);
	signal(SIGFPE,ill_sig);
	signal(SIGBUS,ill_sig);
	signal(SIGSEGV,ill_sig);
	signal(SIGSYS,ill_sig);
	signal(SIGPIPE,ill_sig);
#endif
#ifdef	SYS5
	signal(SIGINT,exit1);		/* allow interrupts in startup stuff */
	signal(SIGQUIT,SIG_IGN);
	signal(SIGTERM,SIG_IGN);
	signal(SIGALRM,SIG_IGN);
	signal(SIGHUP,ill_sig);
	signal(SIGTRAP,ill_sig);
	signal(SIGIOT,ill_sig);
	signal(SIGEMT,ill_sig);
	signal(SIGFPE,ill_sig);
	signal(SIGBUS,ill_sig);
	signal(SIGSEGV,ill_sig);
	signal(SIGSYS,ill_sig);
	signal(SIGPIPE,ill_sig);
#endif
	srand((unsigned) time((long *) NULL));  /* prime random numbers */
	initscr();
	noecho();
	crmode();
	clear();
	refresh();
}

/****************************************************************/

void
getstring(cp,mx)		/* get a string from the stdscr at current y,x */

reg char    *cp;
reg int 	mx;
{
	reg int loop = 0, x, y, xorig;
	int ch;

	getyx(stdscr,y,xorig);
	clrtoeol();
	refresh();
	while((ch = getch()) != '\n' && loop < mx - 1)
		switch (ch)
		{
			case '\033':    /* escape */
			case '\010':    /* backspace */
				if (loop)
				{
					--loop;
					getyx(stdscr,y,x);
					mvaddch(y,x-1,' ');
					move(y,x-1);
					refresh();
				}
				break;
			case '\030':    /* ctrl-x */
				loop = 0;
				move(y,xorig);
				clrtoeol();
				refresh();
				break;
#ifdef SHELL
			case '!' :	/* shell escape at start of string, normal otherwise */
				if (! loop)
				{
					shellcmd();
					refresh();
					continue;
				}
				/* FALLTHROUGH */
#endif
			default:
				if (ch >= ' ') /* printing char */
				{
					addch(ch);
					cp[loop++] = ch;
					refresh();
				}
		}
	cp[loop] = '\0';
}

/****************************************************************/

void
shellcmd()		/* do a shell escape */
{
#ifdef SHELL
		char	command[80];
		char	*shell;

		clear();
		refresh();
		inshell = TRUE;
		alarm(0);
		echo();
		nocrmode();
		putchar('!');
		gets(command);
		if (*command)
			system(command);
		else
		{
			if((shell = getenv("SHELL")) == NULL)
				shell = "/bin/sh";
			printf("[Type ^D to exit shell]\n");
			system(shell);
		}
		printf("\n[Press RETURN twice to continue]");
		getch();
		crmode();
		noecho();
		inshell = FALSE;
		clear();
#endif
}

/****************************************************************/

void
showusers(screen)	    /* print a list of all characters */

bool	screen;
{
	struct	stats	buf;

	if (screen)
	{
		clear();
		refresh();
	}
	fseek(read_pfile,0L,0);
	{
		puts("Current characters on file are:\n");
		while (fread((char *) &buf,sizeof(buf),1,read_pfile))
			if (strcmp("<null>",buf.name))
				printf("%-20s   Login: %-9s  Level: %6d  Type: %3s\n",
						buf.name,buf.login,buf.lvl,ptype('s',buf.typ));
	}
	if (screen)
	{
		putchar('\n');
		putchar('\n');
		more(22);
		clear();
	}
}

/****************************************************************/

void
kingstuff(stat) 	    /* stuff upon entering throne */

reg struct  stats   *stat;
{
	FILE	*fp;
	struct	stats	buf;
	struct	energyvoid vbuf;
	reg int loc = 0;

	if (stat->typ < 10) /* check to see if king -- assumes crown */
	{
		fseek(read_pfile,0L,0);
		while (fread((char *) &buf,sizeof(buf),1,read_pfile))
		{
			if (buf.typ > 10 && buf.typ < 20)	/* found old king */
			{
				if (buf.x == 0.0 && buf.y == 0.0)
				{
					mvaddstr(4,0,"The king is on his throne, so you cannot steal it right now!");
					stat->x = stat->y = 9;
					move(6,0);
					return;
				}
				else if (buf.status != OFF)
				{
					mvaddstr(4,0,"The king is playing, so you cannot steal his throne\n");
					stat->x = stat->y = 9;
					move(6,0);
					return;
				}
				else
				{
					buf.typ -= 10;
					if (buf.crn)
					--buf.crn;
					update(&buf,loc);
KING:				stat->typ = stat->typ + 10;
					mvaddstr(4,0,"You have become king!\n");
					fp = fopen(messfile,"w");
					fprintf(fp,"All hail the new king, %s!",stat->name);
					fclose(fp);

					/* clear all energy voids */

					fp = fopen(voidfile,"r");
					fread((char *) &vbuf,sizeof(vbuf),1,fp);
					fclose(fp);
					fp = fopen(voidfile,"w");
					fwrite((char *) &vbuf,sizeof(vbuf),1,fp);
					fclose(fp);
					goto EXIT;
				}
			}
			else
				++loc;
		}
		goto KING;  /* old king not found -- install new one */
	}
EXIT:	mvaddstr(6,0,"0:Decree  ");
}

/****************************************************************/

void
more(where)		/* wait for input to continue */

int where;
{
	mvaddstr(where,0,"-- More --");
	getans(" \n", FALSE);
}

/****************************************************************/

void
cstat(stat) 		/* examine/change stats of a character */

reg	struct	stats *stat;
{
	struct	stats charac;
	char	s[60], flag[2];
	reg int loc = 0;
	int 	c, temp;
	long	today, ltemp;
	double	dtemp;

	flag[0] = 'F', flag[1] = 'T';
	for (;;)
	{
		clear();
		if (stat != NULL)
			charac = *stat;			/* copy in su's in-memory stats */
		else
		{
			addstr("Current characters on file are:\n");
			fseek(read_pfile,0L,0);
			while (fread((char *) &charac,sizeof(charac),1,read_pfile))
				if (strcmp("<null>",charac.name))
					printw("%-20s   Login: %-9s\n",charac.name,charac.login);
			addstr("\n\nWhich character do you want to look at? ");
			refresh();
			getstring(s,60);
			trunc(s);
			if ((loc = findname(s, &charac)) < 0)
			{
				mvaddstr(11,0,"Not found.");
				exit1();
				/*NOTREACHED*/
			}
		}

		time(&ltemp);
		today = localtime(&ltemp)->tm_yday;
		if (!su)
			strcpy(charac.pswd,"XXXXXXXX");
		clear();
	TOP:	mvprintw(0,0,"a: Name         %s\n",charac.name);
		printw		("b: Password     %s\n",charac.pswd);
		printw		(" : Login        %s\n",charac.login);
		temp = today - charac.lastused;
		if (temp < 0)
			temp += 365;
		printw		("c: Used         %d\n",temp);
		mvprintw	(5,0,"d: Experience   %.0f\n",charac.exp);
		printw		("e: Level        %d\n",charac.lvl);
		printw		("f: Strength     %.0f\n",charac.str);
		printw		("g: Sword        %.0f\n",charac.swd);
		printw		("h: Quickness    %d",charac.quk);
		if (charac.quk < 0)
			printw("(%d)\n",charac.tampered);
		else
			addch('\n');
		printw		("i: Quicksilver  %d\n",charac.quks);
		printw		("j: Energy       %.0f\n",charac.energy);
		printw		("k: Max-Energy   %.0f\n",charac.mxn);
		printw		("l: Shield       %.0f\n",charac.shd);
		printw		("m: Magic        %.0f\n",charac.mag);
		printw		("n: Mana         %.0f\n",charac.man);
		printw		("o: Brains       %.0f\n",charac.brn);
		mvprintw	(0,40,"p: X-coord      %.0f\n",charac.x);
		mvprintw	(1,40,"q: Y-coord      %.0f\n",charac.y);
		if (su)
			mvprintw(2,40,"r: Wormhole     %d\n",charac.wormhole);
		else
			mvprintw(2,40,"r: Wormhole     %c\n",flag[charac.wormhole != 0]);
		mvprintw	(3,40,"s: Type         %d (%s)\n",charac.typ,
									ptype('l',charac.typ));
		mvprintw	(5,40,"t: Sin          %0.3f\n",charac.sin);
		mvprintw	(6,40,"u: Poison       %0.3f\n",charac.psn);
		mvprintw	(7,40,"v: Gold         %.0f\n",charac.gld);
		mvprintw	(8,40,"w: Gem          %.0f\n",charac.gem);
		mvprintw	(9,40,"x: Holy Water   %d\n",charac.hw);
		mvprintw	(10,40,"y: Charms       %d\n",charac.chm);
		mvprintw	(11,40,"z: Crowns       %d\n",charac.crn);
		mvprintw	(12,40,"1: Amulets      %d\n",charac.amu);
		mvprintw	(13,40,"2: Age          %ld\n",charac.age);
		mvprintw	(14,40,"S: Status       %d\n",charac.status);
		mvprintw	(18,3,"3: Virgin %c    4: Blessed %c    5: Ring %c  6: Blind %c    7: Palantir %c",
		flag[charac.vrg],flag[charac.bls],flag[charac.rng.type != 0],
				flag[charac.blind],flag[charac.pal]);
		if (!su)
			exit1();
		if (stat == NULL)
		{
			mvaddstr(16,40,"U: Update character");
			mvaddstr(16,62,"D: Delete");
		}
		mvprintw(19,3, "9: Lives: %d",charac.lives);
		mvaddstr(19,34,"8: Duration");
		if (stat == NULL)
			mvaddstr(21,0,"What would you like to change? (RUBOUT to quit) ");
		else
			mvaddstr(21,0,"What would you like to change? ('Q' to quit) ");
		refresh();
		c = getch();
		switch(c)
		{
			case 'p':   /* change x coord */
				mvprintw(23,0,"X = %f;  X = ",charac.x);
				dtemp = inflt();
				if (dtemp != 0.0)
					charac.x = dtemp;
				break;
			case 'q':   /* change y coord */
				mvprintw(23,0,"Y = %f;  Y = ",charac.y);
				dtemp = inflt();
				if (dtemp != 0.0)
				charac.y = dtemp;
				break;
			case 'd':   /* change Experience */
				mvprintw(23,0,"Exp = %f;  Exp = ",charac.exp);
				dtemp = inflt();
				if (dtemp != 0.0)
					charac.exp = dtemp;
				break;
			case 'e':   /* change level */
				mvprintw(23,0,"Level = %d;  Level = ",charac.lvl);
				dtemp = inflt();
				if (dtemp != 0.0)
					charac.lvl = dtemp;
				break;
			case 'h':   /* change quickness */
				mvprintw(23,0,"Quickness = %d;  Quickness= ",charac.quk);
				dtemp = inflt();
				if (dtemp != 0.0)
					charac.quk = dtemp;
				break;
			case 'f':   /* change strength */
				mvprintw(23,0,"Strength = %f;  Strength = ",charac.str);
				dtemp = inflt();
				if (dtemp != 0.0)
					charac.str = dtemp;
				break;
			case 't':   /* change Sin */
				mvprintw(23,0,"Sin = %f;  Sin = ",charac.sin);
				dtemp = inflt();
				if (dtemp != 0.0)
					charac.sin = dtemp;
				break;
			case 'n':   /* change mana */
				mvprintw(23,0,"Mana = %f;  Mana = ",charac.man);
				dtemp = inflt();
				if (dtemp != 0.0)
					charac.man = dtemp;
				break;
			case 'v':   /* change gold */
				mvprintw(23,0,"Gold = %f;  Gold = ",charac.gld);
				dtemp = inflt();
				if (dtemp != 0.0)
					charac.gld = dtemp;
				break;
			case 'j':   /* change energy */
				mvprintw(23,0,"Energy = %f;  Energy = ",charac.energy);
				dtemp = inflt();
				if (dtemp != 0.0)
					charac.energy = dtemp;
				break;
			case 'k':   /* change Maximum energy */
				mvprintw(23,0,"Max energy = %f;  Max energy = ",charac.mxn);
				dtemp = inflt();
				if (dtemp != 0.0)
					charac.mxn = dtemp;
				break;
			case 'm':   /* change magic */
				mvprintw(23,0,"Magic level = %f;  Magic level = ",charac.mag);
				dtemp = inflt();
				if (dtemp != 0.0)
					charac.mag = dtemp;
				break;
			case 'o':   /* change brains */
				mvprintw(23,0,"Brains = %f;  Brains = ",charac.brn);
				dtemp = inflt();
				if (dtemp != 0.0)
					charac.brn = dtemp;
				break;
			case 'z':   /* change crowns */
				mvprintw(23,0,"Crown = %d;  Crown = ",charac.crn);
				dtemp = inflt();
				if (dtemp != 0.0)
					charac.crn = dtemp;
				break;
			case '5':   /* change ring type */
				mvprintw(23,0,"Ring type = %d;  Ring type = ",charac.rng.type);
				dtemp = inflt();
				if (dtemp != 0.0)
					charac.rng.type = dtemp;
				break;
			case '8':   /* change ring duration */
				mvprintw(23,0,"Ring duration = %d;  Ring duration = ",
							charac.rng.duration);
				dtemp = inflt();
				if (dtemp != 0.0)
					charac.rng.duration = dtemp;
				break;
			case '7':   /* change palantir */
				mvprintw(23,0,"Palantir = %d;  Palantir = ",charac.pal);
				dtemp = inflt();
				if (dtemp != 0.0)
				{
					charac.pal = dtemp;
					charac.pal = (charac.pal != 0);
				}
				break;
			case '9':   /* change lives */
				mvprintw(23,0,"Lives = %d;  Lives = ",charac.lives);
				dtemp = inflt();
				if (dtemp >= 0.0)
					charac.lives = dtemp;
					break;
			case 'S':   /* change status */
				mvprintw(23,0,"Status = %d;  Status = ",charac.status);
				dtemp = inflt();
				if (dtemp >= 0.0)
					charac.status = dtemp;
				break;
			case 'u':   /* change poison */
				mvprintw(23,0,"Posion = %f;  Posion = ",charac.psn);
				dtemp = inflt();
				if (dtemp != 0.0)
					charac.psn = dtemp;
				break;
			case 'x':   /* change holy water */
				mvprintw(23,0,"Holy Water = %d;  Holy Water = ",charac.hw);
				dtemp = inflt();
				if (dtemp != 0.0)
					charac.hw = dtemp;
				break;
			case '1':   /* change amulet */
				mvprintw(23,0,"Amulet = %d;  Amulet = ",charac.amu);
				dtemp = inflt();
				if (dtemp != 0.0)
					charac.amu = dtemp;
				break;
			case '4':   /* change Blessing */
				mvprintw(23,0,"Blessing = %d;  Blessing = ",charac.bls);
				dtemp = inflt();
				if (dtemp != 0.0)
				{
					charac.bls = dtemp;
					charac.bls = (charac.bls != 0);
				}
				break;
			case 'y':   /* change Charm */
				mvprintw(23,0,"Charm = %d;  Charm = ",charac.chm);
				dtemp = inflt();
				if (dtemp != 0.0)
					charac.chm = dtemp;
				break;
			case 'w':   /* change Gems */
				mvprintw(23,0,"Gem = %f;  Gem = ",charac.gem);
				dtemp = inflt();
				if (dtemp != 0.0)
					charac.gem = dtemp;
				break;
			case 'i':   /* change Quicksilver */
				mvprintw(23,0,"Quicksilver = %d;  Quicksilver = ",charac.quks);
				dtemp = inflt();
				if (dtemp != 0.0)
					charac.quks = dtemp;
				break;
			case 'g':   /* change swords */
				mvprintw(23,0,"Sword = %f;  Sword = ",charac.swd);
				dtemp = inflt();
				if (dtemp != 0.0)
					charac.swd = dtemp;
				break;
			case 'l':   /* change shields */
				mvprintw(23,0,"Shield = %f;  Shield = ",charac.shd);
				dtemp = inflt();
				if (dtemp != 0.0)
					charac.shd = dtemp;
				break;
			case 's':   /* change type */
				mvprintw(23,0,"Type = %d;  Type = ",charac.typ);
				dtemp = inflt();
				if (dtemp != 0.0)
					charac.typ = dtemp;
				break;
			case '3':   /* change virgin */
				mvprintw(23,0,"Virgin = %d;  Virgin = ",charac.vrg);
				dtemp = inflt();
				if (dtemp > 0.0)
					charac.vrg = TRUE;
				else
					charac.vrg = FALSE;
				break;
			case 'c':   /* change last-used */
				mvprintw(23,0,"Last used = %d;  Last used = ",charac.lastused);
				dtemp = inflt();
				if (dtemp != 0.0)
					charac.lastused = dtemp;
				break;
			case 'b':	/* change password */
				mvaddstr(23,0,"New password: ");
				getstring(s,60);
				if (*s)
					strcpy(charac.pswd,s);
				break;
			case 'a':	/* change name */
				mvaddstr(23,0,"New name: ");
				getstring(s,60);
				if (*s)
					strcpy(charac.name,s);
				break;
			case 'r':   /* change wormhole */
				mvprintw(23,0,"Wormhole = %d;  Wormhole = ",charac.wormhole);
				dtemp = inflt();
				if (dtemp != 0.0)
					charac.wormhole = dtemp;
				break;
			case '2':   /* change age */
				mvprintw(23,0,"Age = %d;  Age = ",charac.age);
				dtemp = inflt();
				if (dtemp != 0.0)
					charac.age = dtemp;
				break;
			case '6':   /* change blindness */
				mvprintw(23,0,"Blind = %d;  Blind = ",charac.blind);
				dtemp = inflt();
				if (dtemp != 0.0)
				{
					charac.blind = dtemp;
					charac.blind = (charac.blind != 0);
				}
				break;
			case 'U':   /* update buffer stats */
				if (stat == NULL)
					goto LEAVE;
			case 'D':   /* delete char */
				strcpy(charac.name,"<null>");
				initchar(&charac);
				goto LEAVE;
			case 'Q':	/* return to play for su altering own stats */
				*stat = charac;
				clear();
				return;
		}
		goto TOP;
LEAVE:	charac.status = OFF;
		update(&charac,loc);
	}
}

/****************************************************************/

int
level(expr) 	/* calculate level */

double	expr;
{
	if (expr < 1.1e+7)
		return (pow((expr/1000.0), 0.4875));
	else
		return (pow((expr/1250.0), 0.4865));
}

/****************************************************************/

void
trunc(str)		/* remove blank spaces at the end of str[] */

reg char    *str;
{
	reg int loop;

	loop = strlen(str);
	while (str[--loop] == ' ')
		str[loop] = '\0';
}

/****************************************************************/

double
inflt() 	    /* get a floating point # from the terminal */
{
	char	aline[80];
	double	res;

	getstring(aline,80);
	if (sscanf(aline,"%F",&res) < 1)
		res = 0.0;
	return (res);
}

/****************************************************************/

void
checkmov(stat)		    /* see if beyond PONR */

reg struct  stats   *stat;
{
	if (beyond)
	{
		stat->x = sgn(stat->x) * max(abs(stat->x),1.1e+6);
		stat->y = sgn(stat->y) * max(abs(stat->y),1.1e+6);
	}
}

/****************************************************************/

void
scramble(stat)		/* mix up some stats */

reg struct  stats   *stat;
{
	double	buf[6], temp1, temp2;
	reg int first, second;
	reg double  *bp;

	bp = buf;
	*bp++ = stat->str;
	*bp++ = stat->man;
	*bp++ = stat->brn;
	*bp++ = stat->mag;
	*bp++ = stat->energy;
	*bp = stat->sin;

	bp = buf;
	first = roll(0,5);
	second = roll(0,5);
	temp1 = bp[first];

	/* this expression is split to prevent a compiler loop on some compilers */

	temp2 = bp[second];
	bp[first] = temp2;
	bp[second] = temp1;

	stat->str = *bp++;
	stat->man = *bp++;
	stat->brn = *bp++;
	stat->mag = *bp++;
	stat->energy = *bp++;
	stat->sin = *bp;
}
