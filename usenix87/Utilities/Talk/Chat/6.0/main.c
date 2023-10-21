/*			     Chat ver 6.0				*/
/*	      Written by Sanford L. Barr  (slb\@BNL44.ARPA)		*/
/* misc.c :=: 9/25/85 */
#define	VERSION	"6.0 by SLB"

/****************************
**	    CHAT           **
**			   **
**	A multi-user	   **
**  communication system   **
****************************/

#include "chat.h"

int cptty(a, b)		/* Compare routine for qsort */
struct logs  *a, *b;
{
	return(strcmp(a->l_line, b->l_line));
}

main()
{
	char	inp,
		*getlogin(),
		*getenv(),
		*ttyname(),
		*temptty = "\0xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx", 
		*handle	= "(\0xxxxxxxx",
		login[8],		/* Time	entered	chat */
		jline[128],
		*junk;			/* Used	for handle and read */

	int	i = 0,
		s = 0,
		l;

	long	ti;


	cinit();			/* Initalize cursor routines */
	temptty = ttyname(2);				/* Get tty */
#ifdef INDEX
	temptty = rindex(temptty,'/') + 1;
#else
	temptty = strrchr(temptty,'/') + 1;
#endif
	sprintf(mytty, "%5.5s", temptty);		/* Truncate */
	length = strlen(mytty);
	strcpy(myname, getlogin());			/* Get username	*/
	junk = &jline[0];

	ti = time(0);
	prtime(ti,login);			/* Set login time */

	if ((wfd=fopen(SYSTEMFILE,"r"))	!= FERROR) {	/* Squelch users */
		for(;;)	{
			if (fgets(junk,	32, wfd) == OK)
				i = 1;
			if (!strncmp(junk, "system-down", 11)) {
				puts("The chat system is down at the moment.  It should	be up shortly.");
				exit(0);
			}
#ifdef ENEMY
			if (!strncmp(junk, "squelch ", 8)) {
				junk = junk + 8;
				if (!strncmp(myname, junk, strlen(myname))) {
				puts("\
You have been disallowed system	access.\n\
If you have any	questions, talk	to your	system administrator.");
				exit(1);
				}
			}
#endif
			if (i)
				break;
		}
		if (fclose(wfd)	== ERR)	{
			perror(SYSTEMFILE);
			exit(1);
		}
	}
	junk = &jline[0];
	strcpy(mixname, myname);		/* Get handle and */
	if ((junk=getenv("HANDLE")) != FERROR) {/* Add it to mixname */
		strncat(handle, junk, 10);
		strcat(handle,")");
		strncat(mixname, handle, strlen(handle));
	}

	s = 3;
	while (s++ < 19)			/* Catch signals and */
		if ((s != 18) && (s != SIGALRM))/* ignore them.      */
			signal(s, SIG_IGN);
			
	signal(SIGHUP, quit);			/* These signals get */
	signal(SIGINT, quit);			/* sent to quit      */
	signal(SIGQUIT,quit);

	if (access(LOGFILE, 00) != OK) {
		close(creat(LOGFILE, 0644)); /* Create logfile */
		chmod(LOGFILE, 0644);
	}

	if((lfile=open(LOGFILE, 0)) == FERROR) {  /* Open logfile */
		perror(LOGFILE);
		exit(1);
	}

	if (ridname() == ERR)			/* Clear out dead users	*/
		exit(1);			/*	  if any	*/

	who(ON, OFF);				/* Get who's logged in.	*/
	if (users > MAXUSERS) {
		puts("Sorry, no	available slots	left at	the moment.  Try later");
		exit(1);
	}

	sprintf(names[users].l_line, "%5.5s", mytty);
	sprintf(names[users].l_name, "%-20.20s", mixname);
	sprintf(names[users].l_time, "%7.7s", login);

	qsort((char *) names, users+1, sizeof(*names), cptty);	/* Sort tty's */

	if ((wfd=fopen(LOGFILE,"w")) ==	FERROR) {
		error("Couldn\'t open LOGFILE", ON);
		exit(1);
	}

					/* Add name, tty, time to log */
	for (l = 0; l <	users+1; l++)
	fprintf(wfd, "%-5.5s%-20.20s%-7.7s", names[l].l_line,
					 names[l].l_name, names[l].l_time);

	if (fclose(wfd)	== ERR)	{
		perror(LOGFILE);
		quit();
	}

	strcpy(myfile, PREFIX);		/* Set up name for my message file */
	strncat(myfile, mytty, 5);
	if (close(creat(myfile,	0)) == ERR);	/* Create message file */
	chmod(myfile, 0644);

	send(1);			/* Say hello to	everyone */

	tstore();			/* Save terminal settings */

	ttyset(ON,OFF);			/* Set cbreak mode - no	echo */

	clear();			/* Clear Screen */
	printf("\r\nChat %s - press '?' for help.\r\n", VERSION);
	who(OFF, OFF);			/* Show	who's logged in	Chat */

	while(ON)
	{

		inp = get();			/* Get a key */

		switch(inp)			/* Check it */
		{
		case 'w':
			who(OFF, ON);
			break;
		case 'p':
			if (page(ON) == ERR)
				fputs("Page(ON) ML: unsucessful.\r\n", stderr);
			break;
		case 's':
			send(0);
			break;
		case 'q':
			quit();
		case 'l':
			if (page(OFF) == ERR)
				fputs("Page(OFF): unsucessful.\r\n", stderr); 
			break;
		case 'v':
			printf("\r\nVer %s\r\n",VERSION);
			break;
		case '!':
			shell();
			break;
		case '?':
			cmds();
			break;
	       default:
			readmsg();
			break;
		}
	}
}

get()	/* get char routine for	chat,  waits 1 sec for input */
	/*     (There's	probably a better way to do it.)     */
{
	char ch;

	signal(SIGALRM, bye);

	alarm(ON);
	ch = getchar() & 127;
	alarm(OFF);

	if (!isprint(ch))
		ch = 0;
	else
		if (isupper(ch))
			ch = tolower(ch);

	return(ch);
}

bye()	/* called if the alarm signal is caught	*/
{
	return;
}

