/*
 *	Logically turn on or off Unix terminals
 *
 *
 * This program allows quick editing of the file /etc/ttys, which
 * contains the information used by "init" to start up new processes
 * at terminals.  It should be linked to all of the following names:
 *
 * on on1 on2 on3 on4 on5 on6 on7 on8 on9 off
 *
 * The links determine what name is used when the program is executed.
 * If executed as "off", the specified terminals except tty8 are
 * turned off.  If executed as "on" or "on1", the specified terminals
 * are turned on for login, "on2" for the shell, etc.  Note that the
 * argument "all" is special:
 *
 * off all -- turns off all terminals except tty8
 * on# all -- turns on terminals mentioned in /etc/onttys with value #
 * on all  -- turns on terminals as specified in /etc/onttys
 *
 * Each line in /etc/onttys corresponds to a different type of
 * process.  The first line represents the index 1, for login.
 * The second represents the index 2, etc.  All terminals designated
 * on these lines are turned on as specified when the "on" command is
 * used.  When any variation of "on" is used, all terminals mentioned
 * in /etc/onttys are turned on with the specified character, regardless
 * of what line they occupy in /etc/onttys.  (For example, "on2 all"
 * turns all terminals mentioned anywhere in /etc/onttys on with 2).
 *
 * Note: this program should be located in /bin so that it can be
 * used when in single-user mode.  Also, it must be executed by
 * the program name, not the pathname (the shell will supply the
 * pathname for the actual "exec" call) so that the first argument
 * is "off", "on", "on1", etc.
 *
 * Modified February, 1978 by J. Bruner for multiple base-level
 * startup by "init."
 */
char buffer[4];
char onttys[128];	/* ttys to be turned on with "all" command */
char *ttyfile           "/etc/ttys";
char *onttysf		"/etc/onttys";
int fildes;
char writechar '1';	/* changed to '0' for off */
main (argc, argv)
int argc; char *argv[];
{
	register int i, oflg;
	register char *ptr;
	
	if((fildes = open(onttysf, 0)) == -1)
		printf ("Can't open %s\n", onttysf);
	read(fildes, &onttys, sizeof onttys);
	if ((fildes = open (ttyfile, 2)) == -1)
	{
		printf ("Can't open %s\n", ttyfile);
		exit(1);
	}

	oflg = 0;               /* = 1 if command "on" was used */

	if  (strcomp(*argv, "off")) writechar = '0';
		else if (argv[0][2]) writechar = argv[0][2];
			else oflg++;

	if (strcomp(*++argv, "all"))  
		if(writechar == '0') {
			update(0); /* turn all off except 8 */
			kill(1,1);
		}
		else
		{
			ptr = &onttys[0];
			while (*ptr)
				if (*ptr == '\n'){
					if (oflg) writechar++;
					ptr++;
				}
					else update(*ptr++);
			kill (1, 1); /* send init a hangup signal */
			if (oflg) writechar = '1';
		}
	else   --argv;

	i = 1;
	for (;--argc;argv[i++])   /* loop on groups of args: abc def gh */
	{       ptr = argv[i];
		while (*ptr) update ( *ptr++);/* loop within group: abcde */
		kill (1, 1); /* send init a hangup signal */
	}
}

update (c)      /* edits /etc/ttys */
char c;
{
	while (read (fildes, buffer, 4) == 4) /* until EOF */
	if (((c > 0 && buffer[1] == c) || c == 0) && buffer[1] != '8')
	{
		seek (fildes, -4, 1);      /* backup to beginning of line */
		buffer[0] = writechar;
		write (fildes, buffer, 4); /* write updated line in file */
		if (c > 0){
			seek(fildes, 0, 0);
			return;
		} /* all done with this char, look no further */
	}
}
strcomp(s,t)
	      /* s,t are pointers to char strings;  returns 1
		 if s == t; 0 otherwise 			  */
   char *s, *t;
     {while (*s++ == *t) if (*t++ == '\0') return(1); return(0);}

