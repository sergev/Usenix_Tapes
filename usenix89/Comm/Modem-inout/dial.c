/* dial.c -- "dial in|out [ttyxx]" for bidirectional line use
 *
 * originally written (I believe) by Gertjan Vinkesteyn
 * 30-Oct-86 Steve McConnel	add option to specify tty line, and use of
 *				/etc/dial-lines
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>

extern char *strcpy(), *strcat(), *fgets();

#define ROOT_UID 0
#define UUCP_UID 5

char *documentation[] = {
"Usage:  /etc/dial in|out [ttyxx]",
"",
"    /etc/dial in         changes /dev/ttyxx to an incoming (login) line",
"    /etc/dial out        changes /dev/ttyxx to an outgoing line",
"    /etc/dial in ttyi3   changes /dev/ttyi3 to an incoming (login) line",
"    /etc/dial out ttyh4  changes /dev/ttyh4 to an outgoing line",
NULL };

/************************************************************************
 * NAME
 *    usage
 * DESCRIPTION
 *    Print the online documentation on how to use this program.
 */
usage()
{
register char **p;
for (p = documentation ; *p ; ++p)
    fprintf(stderr,"%s\n", *p);
exit(0);
}

/************************************************************************
 * NAME
 *    fgetss
 * DESCRIPTION
 *    Read a line from a file like fgets(), but strip the trailing '\n'
 *    like gets().
 * ARGUMENTS
 *    buf  - address of input buffer
 *    size - size of input buffer
 *    fp   - input FILE pointer
 * RETURN VALUE
 *    original value of `buf' if successful, NULL if failure (such as EOF)
 */
char *fgetss(buf, size, fp)
char *buf;
int size;
FILE *fp;
{
register char *p;

if (fgets(buf,size,fp) == NULL)
    return(NULL);
p = buf + strlen(buf);
if (*--p == '\n')
    *p = '\0';
return(buf);
}

/************************************************************************
 * NAME
 *    fputss
 * DESCRIPTION
 *    Write a line to a file like fputs(), but add a trailing '\n' like
 *    puts().
 * ARGUMENTS
 *    buf  - address of output buffer
 *    fp   - output FILE pointer
 */
fputss(buf, fp)
char *buf;
FILE *fp;
{
fputs(buf,fp);
fputs("\n",fp);
}

/************************************************************************
 * NAME
 *    Abort
 * DESCRIPTION
 *    print an error message and die.
 * RETURN VALUE
 *    does not return to program, returns to shell with -1 value
 */
Abort(msg)
char *msg;
{
fprintf(stderr, "dial abort:  %s\n", msg);
exit(-1);
}

/************************************************************************
 * NAME
 *    main
 * DESCRIPTION
 *    This program switches a tty line between incoming and outgoing.
 *    See dial(8L) for details (or read the code below!).
 * ARGUMENTS
 *    argc - number of command line arguments
 *    argv - array of command line arguments
 */
main(argc, argv)
int argc;
char *argv[];
{
register char *p;
char line[100];		/* working buffer large enough for any need */
char ttyline[12];	/* tty name (excluding "/dev/") */
char lck[32];		/* constructed name of lock file */
char tty[16];		/* constructed tty filename (includes "/dev/") */
FILE *Ti;		/* for input from /etc/dial-lines and /etc/ttys */
FILE *To;		/* for output to /etc/ttysnew */
struct stat statbuf;	/* for checking owner of /dev/ttyxx */
/*
 *  check for valid command line arguments
 */
if ((argc < 2) || (argc > 3))
    usage();
if ((strcmp(argv[1], "in") != 0) && (strcmp(argv[1],"out") != 0))
    usage();
if (argc==3)
    {
    if (strncmp(argv[2],"tty",3) == 0)
	strcpy(ttyline,argv[2]);
    else
	usage();
    }
else
    strcpy(ttyline,"");
/*
 *  get/check a valid tty line from /etc/dial-lines
 */
if ((Ti = fopen("/etc/dial-lines", "r")) == NULL)
    Abort("cannot open `/etc/dial-lines'");
if (fgetss(line,sizeof(line),Ti) == NULL)
    Abort("/etc/dial-lines is empty");
if (ttyline[0] == '\0')
    strcpy(ttyline,line);
else
    {
    while (strcmp(ttyline,line) != 0)
	{
	if (fgetss(line,sizeof(line),Ti) == NULL)
	    Abort("invalid tty line requested");
	}
    }
fclose(Ti);
strcat( strcpy(lck,"/usr/spool/uucp/LCK.."), ttyline );
strcat( strcpy(tty,"/dev/"), ttyline );
/*
 *  check that the line isn't already being used
 */
if (access(lck,F_OK) == 0)
    Abort( strcat(strcpy(line,lck)," already exists") );
if (stat(tty, &statbuf) < 0)
    Abort("stat failed");
if ((statbuf.st_uid != ROOT_UID) && (statbuf.st_uid != UUCP_UID))
    Abort( strcat(strcat(strcpy(line,"root or uucp does not own "), tty),
				     ", somebody might be logged on") );
/*
 *  get set to modify /etc/ttys to change the tty line's mode
 */
if ((Ti = fopen("/etc/ttys", "r")) == NULL)
    Abort("cannot open `/etc/ttys'");
if ((To = fopen("/etc/ttysnew", "w")) == NULL)
    Abort("cannot create `/etc/ttysnew'");
/*
 *  scan through /etc/ttys to find the desired line, writing to /etc/ttysnew
 */
while ((p = fgetss(line, sizeof(line), Ti)) != NULL)
    {
    if (strcmp(&line[2], ttyline) == 0)
	{
	if (strcmp(argv[1],"in") == 0)
	    *p = '1';
	else
	    {
	    *p = '0';
	    if (chmod(tty, 0666) != 0)
		fprintf(stderr, "dial: cannot chmod `%s'\n", tty);
	    else if (chown(tty, UUCP_UID, 1) != 0)
		fprintf(stderr, "dial: cannot chown `%s'\n", tty);
	    }
	}
    fputss(line, To);
    }
fclose(To);
fclose(Ti);
/*
 *  replace /etc/ttys with /etc/ttysnew
 */
if (unlink("/etc/ttys") == -1)
    Abort("cannot unlink /etc/ttys");
if (link("/etc/ttysnew", "/etc/ttys") == -1)
    {
    fprintf(stderr,
	  "dial abort:  link(\"/etc/ttysnew\",\"/etc/ttys\") failed\n");
    Abort("FATAL--NOW WE DON'T HAVE A /etc/ttys FILE ANYMORE!!");
    }
if (unlink("/etc/ttysnew") == -1)
    Abort("cannot unlink /etc/ttysnew");
/*
 *  get the system to recognize the new /etc/ttys
 */
if (kill (1, 1) != 0)
    Abort("cannot \"kill -1 1\"");
}
