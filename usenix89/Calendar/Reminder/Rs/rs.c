static char rcsid[] = "$Header: rs.c,v 3.3 86/07/03 14:24:34 don Exp $";

/* $Log:	rs.c,v $
 * Revision 3.3  86/07/03  14:24:34  don
 * 1.  MAX_LINE (# columns on a line) now defined by termcap entry for TERM.
 * 2.  Misc. Optimizations.
 * 3.  Reminders can now be "marked", by appending ^G to reminder, to have
 *     reminder highlighted during display.  "Highlight" options can be defined
 *     in the RSINIT environment variable.
 *     U - Underscore,  B - Bold,  S - Asterisk. (Default is standout)
 *     I - Ignore termcap entry.
 * 
 * Revision 3.2  85/08/26  14:55:42  don
 * Adding Mike's enhancements.  Thanks Mike! :-)
 * Revision 3.2 & 3.3 by Mike Spitzer @ (pur-ee!mjs)
 * 
 * Revision 3.2  85/08/05  13:10:52  mjs
 * -added new print_line which doesn't barf on long words
 * -added #define for place to break up lines(MAX_LINE)
 * -print_line is always called now, test for strlen(line) < MAX_LINE
 *  is now made there.
 * 
 * 	MIke Spitzer
 * 	pur-ee!mjs
 * 
 * Revision 3.1  85/07/26  20:48:06  mjs
 * "rs" now recognizes the $EDITOR variable.  "vi" will be used if 
 * $EDITOR is undefined.
 * 
 * added "-e" flag... does the same thing as "-v"
 * 
 * -mjs
 * 
 * Revision 2.2  85/02/25  10:35:48  don
 * 1.  Optimized kill and line wrap routines.
 * 2.  Added defines for LPR & VI.
 * 3.  Added more RSINIT options. (A & D)
 * 4.  Deleted add_calendar routine, now uses add_reminders.
 * 5.  Added comments.
 * 
 * Revision 2.1  85/01/28  17:03:56  don
 * 1. Added new notes file - .rsnotes | command rs -n
 * 2. Added new alternate file - user defined via setenv RSALT | command rs -A
 * 3. Added environment variable RSINIT.
 *    setenv RSINIT 'ad' | a = display after add | d = display after delete.
 * 4. Lines may now be 256 char. long.
 * 5. Long lines will automatically wrap like 'vi'.
 * 6. Added search command. | rs -s 'search string'
 * 7. Help deleted, replaced with 'usage' message.
 * 8. Misc. optimizations.
 * 
 * Revision 1.3  85/01/15  09:17:57  don
 * 1. Added $Log line for RCS.
 * 2. Fixed help message.
 * 3. Coded in range deletion.
 *  */

/*	Program	written by:	Don Joslyn
 *				Nova University Computer Center
 *				3301 College Avenue
 *				Fort Lauderdale, Florida 33314
 *				(305) 475-7630 (novavax!don)
 *
 *	Notice:  No Copyright. 
 *
*/

#include	<stdio.h>
#include	<ctype.h>
#ifdef	BSD
#include	<strings.h>
#else
#include	<string.h>
#define		rindex	strnchr
#endif	BSD

#define		MAXLEN   256	/* Max. Length of a single line     */
#define		MAXLINES 500	/* Max. number of lines (reminders) */

#define		CALENDAR "/calendar"	/* calendar file */
#define		RSFILE	 "/.reminders"	/* reminder file */
#define		RSNOTES  "/.rsnotes"	/* notes    file */

#define		LPR	"/usr/ucb/lpr"	/* location of lpr */
#define		EDITOR	"/usr/ucb/vi" 	/* default editor  */

/*#define		BIGHELP	/* Define if you want verbose help message */

FILE	*fp,
	*fptmp;

char	fname[128],		/* Filename tmp */
	on[32],			/* string used to turn "mark" on  */
	off[32],		/* string used to turn "mark" off */
	buff[1024],		/* Holds termcap entry for TERM */
	s[MAXLEN],		/* Tmp string and line storage */
	sstr[80],		/* Search string */
	cmd[256];		/* cmd for system call */

char	*a,
	*editor,		/* Hold for EDITOR environment variable */
	*entity = "Reminders",	/* Entity working on */
	*env,			/* Hold for RSINIT environment variable */
	*fun,			/* Used in arg processing */
	*mark_on  = "so",	/* Default mark is Standout mode */
	*mark_off = "se";	/* Standout end */
	

char	*getenv(),
	*malloc(),
	*mktemp();

char	rstmp[]= "/tmp/rsXXXXXX";	/* name of temp. file during delete */

int	adisplay = 0,	/* display reminders after additions (default = no) */
	ddisplay = 0,	/* display reminders after deletions (default = no) */
	Adisplay = 0,	/* display notes/alt after additions (default = no) */
	Ddisplay = 0,	/* display notes/alt after deletions (default = no) */
	columns = 80,	/* Number of columns in a line. */
	len      = 0,	/* Length of line */
	mark     = 0,	/* Mark the line (High Priority) */
	tcap     = 1,	/* Read Termcap stuff. */
	tcapok   = 0,	/* TERM found in Termcap file. */
	rem_work = 1,	/* working on reminder file by default */
	search   = 0,	/* search option seen on command line */
	status;		/* Hold status returned by sys calls and such */

main(argc, argv)
int	argc;
char	*argv[];
{ 

/* Get RSINIT environment variable to find options desired */

env = getenv("RSINIT");

while (*env) {
	switch(*env++) {
		case 'a' : adisplay++; break;
		case 'd' : ddisplay++; break;
		case 'A' : Adisplay++; break;
		case 'D' : Ddisplay++; break;
		case 'I' : tcap = 0;   break;
		case 'U' :
			mark_on  = "us";
			mark_off = "ue";
			break;
		case 'B' :
			mark_on  = "bo";
			mark_off = "be";
			break;
		case 'S' :
			mark_on  = "ss";
			mark_off = "ss";
			break;
	}
}


/* Get users HOME directory and construct default name for reminder file */

sprintf(fname, "%s%s", getenv("HOME"), RSFILE);

/* Find out what the user wants to do */

while ( --argc > 0 && (*++argv)[0] == '-')
	for (fun = argv[0]+1; *fun != '\0'; fun++)
		switch(*fun) {
		case 'A' :	/* Work on Alternate reminder file */
			env = getenv("RSALT");
			if (!env)
				env = "phone_numbers";

			sprintf(fname, "%s/.%s", getenv("HOME"), env);
			entity = env;
			if (islower(entity[0]))
				entity[0] = toupper(entity[0]);
			rem_work = 0;
			break;

		case 'a' :	/* Add reminder(s)/note(s)/alternate(s) */
			add_reminder();
			exit(0);

		case 'c' :	/* Add entries to calendar file */
			sprintf(fname, "%s%s", getenv("HOME"), CALENDAR);
			entity = "Calendar commands";
			rem_work = 0;
			add_reminder();
			exit(0);

		case 'd' :	/* Delete reminder(s)/note(s)/alternate(s) */
			delete_reminder(argc, argv);
			exit(0);

		case 'k' :	/* Kill all reminders/notes/alternates */
			if ( unlink(fname) == 0) {
				printf("%s killed.\n", entity);
			}
			exit(0);

		case 'n' :	/* Work on notes file */
			sprintf(fname, "%s%s", getenv("HOME"), RSNOTES);
			entity = "Notes";
			rem_work = 0;
			break;

		case 'p' :	/* print reminders/notes/alternates */
			sprintf(cmd, "%s %s", LPR, fname);
			status = system(cmd);

			if (status == 0) {
				printf("%s will be printed.\n", entity);
				exit(0);
			}
			else {
				printf("%s can't be printed, sorry\n", entity);
				exit(1);
			}

		case 's' :	/* search for string in reminders/notes/alts */
			argc--;
			argv++;
			sprintf(sstr, "%s", argv[0]);
			search++;
			break;
		case 'e' : /* -e and -v are equivilent */
		case 'v' : /* Edit reminders/notes/alternates with editor */
			editor = getenv("EDITOR");
			if (!editor)
				editor = EDITOR;
			sprintf(cmd, "%s %s",editor,fname);
			system(cmd);
			exit(0);
		default:	/* Give em some help */
#ifndef	BIGHELP
			printf("Usage: rs [ -Aaceknpv ] [ -s search_string ] [ -d # ] [ -d #-# ]\n");
			printf("For additional information type:  man rs\n");
#else
			/* You can enable this stuff if you want! */
			printf("Reminder Service commands:\n\n");
			printf("-A\tWork on Alternate reminder file.\n");
			printf("-a\tAdd reminder(s)/note(s)/Alt(s).\n");
			printf("-c\tAdd line(s) to the calendar file.\n");
			printf("-d\tDelete reminder(s)/note(s)/Alt(s).\n");
			printf("-k\tKill all reminders/notes/Alternate.\n");
			printf("-n\tWork on notes.\n");
			printf("-p\tPrint reminders/notes/Alternates.\n");
			printf("-s\tSearch for string.\n");
			printf("-e\tEdit reminders/notes/Alt.\n\n");
			printf("rs with no option will display reminders.\n");
			printf("SEE ALSO rs(1)\n");
#endif	BIGHELP
			exit(0);
		}

print_reminders();

}


/* ADD REMINDERS */
/* This routine uses cat to add lines to the reminder/note/alternate file */
/* If you don't like it, add your own getline routine! */

add_reminder()
{
	printf("Enter %s then ^d to exit:\n", entity);

	sprintf(cmd, "%s %s","/bin/cat >>",fname);
	system(cmd);
	if ( (adisplay && rem_work) || (Adisplay && !rem_work) )
		print_reminders();
}


/* DELETE REMINDERS/NOTES/ALTERNATES */

delete_reminder(argc, argv)
	int	argc;
	char	*argv[];
{
	int	begin,
		end,
		delete_line[MAXLINES];

	char	a[4], b[4];

	register int	i	    = 0,
			j	    = 0,
			deleted     = 0,
			line_number = 0,
			printed     = 0;

	argc--, argv++;
	for (;argc > 0; argc--, argv++) {
		if (alldigits(*argv)) {
			delete_line[atoi(*argv)] = 1;
		}
		else {
			i = rindex(*argv, '-');
			sscanf(i+1, "%s", b );
			j = ( strlen(*argv) - strlen(b) ) - 1;
			strncpy(a, *argv, j);
			a[j]='\0';
			begin = atoi(a);
			end   = atoi(b);

			if (begin < end) {
				for (j = begin ; j < end+1 ; j++)
					delete_line[j] = 1;
			}
			else {
				printf("Illegal range specified.\n");
				exit(1);
			}
		}
	}

	mktemp(rstmp);
	if ((fptmp = fopen(rstmp,"w")) == NULL) {
		print_open_error_exit(rstmp);
	}

	if ((fp = fopen(fname,"r")) == NULL) {
		print_open_error_exit(fname);
	}

	while (getline(s, MAXLEN) > 0) {
		if (delete_line[++line_number] != 1) {
			fprintf(fptmp,"%s",s);
			printed++;
		}
		else {
			deleted++;
		}
	}

	fclose(fp);
	fclose(fptmp);

	status = unlink(fname);
	if (status != 0) {
		printf("Can't remove file - %s", fname);
		exit(1);
	}

	if (printed) {
		if ((fp = fopen(fname,"w")) == NULL) {
			print_open_error_exit(fname);
		}

		if ((fptmp = fopen(rstmp,"r")) == NULL) {
			print_open_error_exit(rstmp);
		}

		while (fgets(s, MAXLEN, fptmp) != NULL) {
			fputs(s, fp);
		}
	}

	fclose(fp);
	fclose(fptmp);
	status = unlink(rstmp);

	if (deleted)
		printf("%d %s deleted.\n", deleted, entity );

	if ( (ddisplay && rem_work) || (Ddisplay && !rem_work) )
		print_reminders();
}


/* PRINT REMINDERS/NOTES/ALTERNATES */
/* This routine will print reminders/notes/alternates */
/* print_line is called to do the printing with */
/* auto wrap.  If a search argument was given, only lines that contain the */
/* search string will be printed. */

print_reminders()
{
	register int line_number = 0;

	if ((fp = fopen(fname,"r")) == NULL) {
		exit(0);
	}

	if (tcap) {
		if (tgetent(buff, getenv("TERM")) == 1) {
			tcapok++;
			columns = tgetnum("co");
			if (columns <= 0)
				columns = 80;
			columns -= 6;
			a = on;
			tgetstr(mark_on,  &a);
			a = off;
			tgetstr(mark_off, &a);
			if (!strlen(on)) {
				if (tgetflag("bs"))
					strcpy(on, "\010*");
				else {
					a = on;
					tgetstr("bc", &a);
				}
			}
		}
	}
		
	printf("\n%s:\n", entity);

	while ((len = getline(s, MAXLEN)) > 0) {
		if (search) {
			if (instr(s, sstr) != -1) {
				printf("%3d. ", ++line_number);
				if (tcapok)
					find_mark();
				if (mark)
					printf("%s", on);
				print_line(s, columns);
				if (mark) {
					printf("%s", off);
					mark = 0;
				}
			}
			else
				line_number++;
		}
		else {
			printf("%3d. ", ++line_number);
			if (tcapok)
				find_mark();
			if (mark)
				printf("%s", on);
			print_line(s, columns);
			if (mark) {
				printf("%s", off);
				mark = 0;
			}
		}
	}

	printf("\n");

	fclose(fp);
}

getline(s, lim)
char s[];
int  lim;
{
	register int i = 0;
	register int c;

	while (--lim > 0 && (c=getc(fp)) != EOF && c != '\n')
		s[i++] = c;

	if (c == '\n')
		s[i++] = c;

	s[i] = '\0';

	return(i);
}

alldigits(s)
register char *s;
{
	register c = *s++;

	do {
	if (!isdigit(c))
		return(0);
	}
	while (c = *s++);

	return(1);
}

instr(s, t)
char s[], t[];
{
	register int i, j, k;

	for (i = 0; s[i] != '\0' ; i++) {
		for (j=i, k=0 ; t[k] != '\0' && s[j]==t[k] ; j++ , k++)
			;
		if (t[k] == '\0')
			return(i);
	}
	return(-1);
}

print_open_error_exit(file_name)
char	file_name[];
{
	printf("Can't open file - %s\n",file_name);
	exit(1);
}

/* This routine will wrap lines greater than 75 characters long.      */
/* An attempt is made to break the line up after a word.  If a "word" */
/* is greater than 75 characters, the word will be split.             */ 
/* 	pur-ee!mjs						      */

#define SPACE	' '
int print_line (line, maxlen)
char    line[];			/* string to be printed */
unsigned int    maxlen;		/* longest line permitted for output */
{
    register int    index,	/* used in for loops */
                    breaking_pt,/* index in line[] at point to break line 
				*/
                    nospace = 0,/* set to TRUE if no space is found */
                    last_br_pt = 0,/* last breaking point */
                    line_len,	/* number of characters in line[] */
                    done = 0;	/* set to TRUE if finished */

 /* if line[] isn't longer than the longest line permitted, we can just
    print it, and return. */

    line_len = strlen (line);

    if (strlen (line) <= maxlen) {
	printf ("%s", line);
	return;
    }

    do {
    /* set breaking_pt at point in line[] where we should break the line.
       Do this by start looking backwards from maxlen until a space is
       found. */

	for (index = (maxlen + last_br_pt); line[index] != SPACE; --index) {
	    if (index == last_br_pt) {
		nospace = 1;
		break;
	    }
	    if (index >= line_len) {
		index = line_len - 1;   /* 1 gets added to it below */
		done = 1;
		break;
	    }
	}
	breaking_pt = index + 1;

    /* if there is no space found, set breaking point to MAXLEN */

	if (nospace == 1) {
	    breaking_pt = maxlen + last_br_pt;
	    nospace = 0;	/* reset flag */
	}
    /* print line[] up to breaking point */

	for (index = last_br_pt; index < breaking_pt; ++index) 
	    putchar (line[index]);


    	if (last_br_pt == 0) /* this is our first time through */
    		maxlen -= 5; /* for indenting */
    	if (!done) {
		if (mark)
			printf("%s", off);

    		printf("\n     "); /* indent */

		if (mark)
			printf("%s", on);
	}

	last_br_pt = breaking_pt;
    } while (!done);
}
find_mark()
{
	if (s[len-2] == '\07') {
		mark = 1;
		s[len-2] = '\n';
		s[len-1] = '\0';
	}
}
