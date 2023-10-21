/* system include files */
#include <stdio.h>
#include <pwd.h>
#include <sys/utsname.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <ctype.h>


/* local parameters */
#ifdef DEBUG
#define SMAIL "../smail"
#else
#define SMAIL "/bin/smail"
#endif
#define FULLHOSTNAME "Icom Systems, Inc."
#define HOSTDOMAIN "UUCP"


/* external routines */
char *mktemp();
int umask();
int getpid();
int getuid();
struct passwd *getpwuid();
int uname();
void endpwent();
long time();
struct tm *gmtime();
char *malloc();
void free();
void exit();


/* global variables */
char *tfn = "/tmp/cmXXXXXX";	/* temporary file name (filled in with
				   mktemp(3)) */
FILE *ofile;			/* temp. file in which the mail is stored */
int pid;			/* process id for use in message-id header */
char *name;			/* user name for use in from or sender header
				   */
char *fname;			/* full user name for use in from or sender
				   header */
char *hname;			/* host name for use in from or sender header
				   */
char *hdom;			/* host domain for use in from or sender
				   header */
char *fhname;			/* full host name for use in from or sender
				   header */
long now;			/* current date and time (numeric) */
struct tm *nows;		/* broken out date and time (GMT) for use in
				   date header */
int forged = 0;			/* this flag will be set if from line is
				   encountered in the input */
char *cvec[100] = {		/* command to be executed, addresses will */
    SMAIL			/* be filled in as they are parsed */
#ifdef DEBUG
    , "-d"			/* run smail in debugging mode */
#endif
};
#ifdef DEBUG
int nextvec = 2;		/* next location in cvec to be filled */
#else
int nextvec = 1;		/* next location in cvec to be filled */
#endif


/* forward declarations */
int hparse();			/* parse header & determine type */
void address();			/* parse and save addresses */
void doheads();			/* print system supplied headers */
void error();			/* print error message and exit */
void warn();			/* print warning message */


main()

{
    int cmask;			/* holds umask while we change it */
    int uid;			/* user id field to find password entry */
    struct passwd *pwent;	/* password entry, for info to use in
				   from or sender header */
    struct utsname uts;		/* uname structure to get host name */
    int inhead;			/* flag, non-zero if still in headers */
    static char buf[4096];	/* line buffer */
    char *field;		/* points to field parsed by hparse (just
				   past :[ \t]+) */
#ifdef DEBUG
    char **cpp;			/* used to display the command used */
#endif

    /* create the temporary file */
    (void) mktemp(tfn);		/* create temporary file name */
    cmask = umask(0077);	/* no one else should be able to read it */
    ofile = fopen(tfn, "w");	/* open it for writing */
    (void) umask(cmask);	/* restore old mask */

    /* get relevant system info */
    pid = getpid();		/* process id for use in the message id */
    uid = getuid();		/* now get the users name */
    if ((pwent = getpwuid(uid)) == NULL)
	error("unable to determine user name");
    endpwent();			/* close password file */
    name = pwent->pw_name;	/* save user name */
    fname = pwent->pw_gecos;	/* this may be his long name */
    if (*fname == '\0' || strpbrk(fname, "()="))
	fname = name;		/* use short name, if it is empty or gecos */
    (void) uname(&uts);		/* get uname structure */
    hname = uts.nodename;	/* save hostname */
    fhname = FULLHOSTNAME;	/* sadly, there is no system location */
    hdom = HOSTDOMAIN;		/* for these two fields */
    (void) time(&now);		/* get current numeric time for mesg id */
    nows = gmtime(&now);	/* get structure with greenwich mean time */

    /* copy the mail to the temp. file, fixing up the headers
	and keeping track of the addresses */
    inhead = 1;			/* yes, we are reading headers now */
    while (fgets(buf, sizeof(buf), stdin)) {

	/* handle headers */
	if (inhead) {
	    switch (hparse(buf, &field)) {
		case 'T':	/* To: and Cc: fields, parse addresses */
		case 'C':	/* and copy to output */
		    address(field);
		    break;
		case 'B':	/* Bcc: field, parse addr., do not copy */
		    address(field);
		    continue;	/* skip the copy at end of while */
		case 'F':	/* From:, set forged flag & copy */
		    ++forged;
		    break;
		case 'M':	/* Message-ID:, Date:, Sender:, must */
		case 'D':	/* never bes specified by the user */
		case 'S':	/* ignore them */
		    continue;
		case 'X':	/* any other header will be copied */
		    break;
		case '?':	/* This is returned when something that
				   doesnt look like a header is found. We
				   print a warning, and output a newline to
				   seperate the headers from what we assume
				   is the body */
		    warn("non-header line in header, end of headers assumed");
		    inhead = 0;	/* mark it, and copy the line */
		    doheads();	/* generate the remaining headers */
		    putc('\n', ofile);
		    break;	/* print the first body line after the extra
				   newline */
		case 'Z':	/* we have found the end of the headers */
		    inhead = 0;	/* mark it, and copy the line */
		    doheads();	/* generate the remaining headers */
		    break;	/* don't forget to print the first body line */
	    }
	}
	/* copy line to temp. file */
	fputs(buf, ofile);
    }

    /* pass the resulting file to smail */
    fclose(ofile);		/* close it */
    if (nextvec == 1)		/* if no addresses, give error mesg. */
	error("no addresses specified");
#ifdef DEBUG
    cpp = cvec;			/* echo the command we will use */
    do
	printf("%s%s", *cpp ? *cpp : "\n", *cpp ? " " : "-------\n");
    while (*cpp++);
#endif
    close(0);			/* close std. input, so we can redirect */
    (void) open(tfn, O_RDONLY);	/* now temp. file is standard input */
    (void) unlink(tfn);		/* unlink, file will be removed when closed */
    cvec[nextvec] = NULL;	/* finish off the argv list for exec */
    execv(cvec[0], cvec);	/* exec smail, with temp. file as stdin */
    error("unable to exec smail");
    /* NOTREACHED */
}


int hparse(line, field)

char *line;
char **field;

{
    static char lasthead = '?';	/* last header seen, return assumed end of
				   header if first line starts with white */
    static struct ht {		/* table of recognized headers */
	int hlen;		/* length of header including ':' */
	char *hstr;		/* header string, e.g. "From:" */
    } htable[] = {
	 3, "To:",
	 3, "Cc:",
	 4, "Bcc:",
	 5, "From:",
	11, "Message-ID:",
	 5, "Date:",
	 7, "Sender:",
	 0, NULL
    };
    struct ht *p;		/* pointer to cycle through the above table */
    char *cp;			/* aux. char pointer for guessing about
				   unknown headers */

    /* if line starts with white space, return last header type */
    if (*line == ' ' || *line == '\t')
	return lasthead;

    /* cycle through table looking for a match */
    for (p = htable; p->hlen != 0; ++p) {
	if (strncmp(p->hstr, line, p->hlen) != 0)
	    continue;		/* no match, try next */

	/* we have matched headers, return a pointer to first non-space
	   past ':' in field */
	line += p->hlen;	/* point past ':' */
	*field = line+strspn(line, " \t");
	return lasthead = *p->hstr;
    }

    /* if no match was found, see if it looks like a header, note that
       we need not set field in this case */
    if ((cp = strpbrk(line, " \t:")) != NULL && *cp == ':')
	return lasthead = 'X';	/* yep, it appears to be a header */

    /* otherwise, return whether or not we are sure this is end of headers */
    return (*line == '\n') ? 'Z' : '?';
}


void address(alist)

char *alist;

{
    char *p;			/* pointer used to cycle through addresses */
    char *cp;			/* auxilliary pointer used to further parse
				   the indivual addresses */

    /* make a copy of the address list so we can split it with strtok(3) */
    p = malloc((unsigned) (strlen(alist)+1));
    alist = strcpy(p, alist);	/* copy the address list */

    /* cycle through the list, splitting on commas */
    for (p = strtok(alist, ",\n"); p != NULL; p = strtok(NULL, ",\n")) {

	/* addresses take two general forms:
		addr [ (comment) ], and
		[ comment ] < addr >
	   The following code deletes the comments.  It makes the
	   simplifying assumption that there will be nothing valid
	   following a '(' even outside the corresponding ')' */
	if ((cp = strchr(p, '(')) != NULL)
	    *cp = '\0';		/* ignore comment */
	else if ((cp = strchr(p, '<')) != NULL) {
	    p = cp+1;		/* change starting point to after '<' */
	    if ((cp = strchr(p, '>')) != NULL)
		*cp = '\0';	/* change end to before '>' */
	}

	/* now strip leading and trailing blanks, and turn all others into
	   '.'s */
	while (isspace(*p))
	    ++p;		/* skip leading space */
	for (cp = p; *cp != '\0'; ++cp) {
	    if (isspace(*cp))
		*cp = '.';	/* convert spaces to dots */
	}
	while (*--cp == '.') ;	/* back up to first no-dot */
	*++cp = '\0';		/* and there is the end of the address */

	/* now make a copy of the string using malloc and store it in
	   list */
	if ((cp = malloc((unsigned) (cp-p+1))) == NULL)
	    error("out of memory");
	cvec[nextvec++] = strcpy(cp, p);
    }
    free(alist);		/* return copied alist to heap */
}


void doheads()

{
    static char *month[] = {
	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };
    static char *day[] = {
	"Sun", "Mon", "Tue", "Wed",
	"Thu", "Fri", "Sat"
    };

    /* print the from or sender header (depending on the forged flag) */
    fprintf(ofile, "%s: %s@%s.%s (%s at %s)\n", forged ? "Sender" : "From",
		name, hname, hdom, fname, fhname);

    /* print the date header */
    fprintf(ofile, "Date: %.2d %s %.2d %.2d:%.2d:%.2d GMT (%s)\n",
		nows->tm_mday, month[nows->tm_mon], nows->tm_year%100,
		nows->tm_hour, nows->tm_min, nows->tm_sec,
		day[nows->tm_wday]);

    /* print the message ID header */
    fprintf(ofile, "Message-ID: <%lu%.5u@%s.%s>\n", now, pid, hname, hdom);
}


void error(s)

char *s;

{
    fprintf(stderr, "cmail: %s\n", s);
    (void) unlink(tfn);		/* unlink the temporary file */
    exit(1);
}


void warn(s)

char *s;

{
    fprintf(stderr, "cmail: warning: %s\n", s);
}
