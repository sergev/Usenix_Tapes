#include <sgtty.h>
#include <sys/file.h>
#include <stdio.h>
#include <sys/param.h>
#include <strings.h>
#include <ctype.h>

/*
 * Jeff Glass, The MITRE Corporation, 5/14/87
 *
 * type - make the system think that a command was entered on
 * another user's terminal.
 *
 * type [-n] <terminal> <command> ...
 *
 *	<terminal> is the line that the command should be entered on.
 *		examples: "/dev/ttyj4" ; "ttyj4" ; "j4"
 *
 *	<command> is the command that should be entered on that terminal.
 *		examples: "logout" ; "jobs > /dev/console"
 *
 * a newline will be appended to <command> unless type is given a "-n"
 * option.  a space will be appended after each command except the last.
 * command can have '\nnn' and '\[tbnr]' and '^[SQC]' escapes in it.
 *
 * type can only be used on the terminal you are on, unless you are the
 * superuser, in which case you can send commands to any terminal.
 */

static int fd;		/* file descriptor for terminal line */
static char *name;	/* argv[0] */
extern int wakeup();	/* routine to wakeup if open of terminal line hangs */

main(argc, argv, environ)
int argc;
char *argv[], *environ[];
{
	int flag_n;		/* do not append newline to <command> */
	int opt;
	extern int optind;

	name = argv[0];
	while ( (opt = getopt(argc,argv,"n"))  != EOF ) {
		switch ( opt ) {
			case 'n' :
				flag_n++;
				break;
			default:
				usage();
				break;
		}
	}
	if ( (optind+2) > argc ) {
		usage();
	}

	if ( index( argv[optind], '/' ) == NULL ) {
		char terminal[MAXPATHLEN+1];

		/*
		 * since a full pathname was not given, assume the
		 * file is in the "/dev" directory, and if only 2
		 * characters were given, assume that means a "tty".
		 */
		strcpy( terminal, "/dev/" );
		if (strlen(argv[optind]) == 2)
			strcat( terminal, "tty" );
		argv[optind] = strcat( terminal, argv[optind] );
	}

	/*
	 * O_NDELAY does not work (?), so set a timer to
	 * wake up if the open hangs.
	 */
	
	signal( SIGALRM, wakeup );
	alarm( 30 );

	if ( (fd = open(argv[optind], O_RDONLY|O_NDELAY, 0)) < 0 ) {
		perror( "open" );
		exit( 2 );
	} else {
		int i;

		alarm( 0 );

		for (i = optind+1; i < argc; i++) {
			if (sendstr(argv[i]) != 0)
				exit(3);
			if (i != (argc-1))
				if (send(' ') != 0)
					exit(3);
		}

		if ( ! flag_n )
			if (send('\n'))
				exit(3);
	}
}

usage()
{
	fprintf( stderr, "%s: usage: %s [-n] <terminal> <command> ...\n",
		 name,
		 name );
	exit( 1 );
}

send( c )
char c;
{
	register int status;

	if ( (status = ioctl(fd,TIOCSTI,&c)) != 0 )
		perror( "ioctl" );

	return status;
}

/*
 * send a string to the terminal, but do some translation (such as '\012'
 * or '\n' for newline, or '^S' for CTRL-S).
 */
sendstr(cp)
char *cp;
{
	short	ch, in_bslash, in_carat;

	ch = in_bslash = in_carat = 0;

	/*
	 * this is a FSM (don't laugh!), where the states are :
	 *	in_bslash == 1
	 *	in_bslash > 1
	 *	in_carat == 1
	 *	in_bslash == 0 && in_carat == 0
	 *
	 * whenever the FSM is in the state (in_bslash == 0 && in_carat == 0),
	 * it prints out the character held in ch.
	 */
	while (*cp != '\0' ) {
		if (in_bslash) {
			if (isascii(*cp) && isdigit(*cp)) {
				ch = ch * 8 + *cp - '0';
				if (in_bslash < 3)
					in_bslash++;
				else
					in_bslash = 0;
			} else if (in_bslash == 1) {
				switch (*cp) {
				case 'b' : ch = '\b'; break;
				case 'f' : ch = '\f'; break;
				case 'n' : ch = '\n'; break;
				case 'r' : ch = '\r'; break;
				case 't' : ch = '\t'; break;
				default  : ch = *cp ; break;
				}
				in_bslash = 0;
			} else {
				/*
				 * this character terminated a '\nn',
				 * so back up cp so this character is
				 * seen again.
				 */
				cp--;
				in_bslash = 0;
			}
		} else if (in_carat) {
			if (*cp == '?')
				ch = '\177';
			else
				ch = *cp & 037;
			in_carat = 0;
		} else {
			switch (*cp) {
			case '\\' :
				in_bslash = 1;
				ch = 0;
				break;
			case '^' :
				in_carat = 1;
				break;
			default :
				ch = *cp;
				break;
			}
		} /*else*/

		if (!(in_bslash || in_carat))
			if (send(ch) != 0)
				return 1;
		cp++;
				
	} /*while*/

	/*
	 * take care of anything left at the end of the string.
	 */
	if (in_bslash) {
		if (send(ch) != 0)
			return 1;
	} else if (in_carat) {
		if (send('^') != 0)
			return 1;
	}
	
	return 0;
}

wakeup()
{
	fprintf( stderr, "open hung\n" );
	exit( 4 );
}
