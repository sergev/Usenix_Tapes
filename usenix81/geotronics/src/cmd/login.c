/*
	login -- sign on

	last edit: 06-Jun-1981	D A Gwyn

Features:
	Brought up to 7th Edition level (using V6 file formats), plus:
	Erase, kill chars changed to backspace, Ctrl/U.
	"Login incorrect" is deferred until the password has been tested.
	Login with argument exits on error to work better with "getty".
	Bad login names and passwords are logged in "/etc/xtmp".
	Games are not permitted to log in via dialups.
	A single argument can now be passed to the shell program.

Feature missing from V6 shell, implemented here instead:
	If ".profile" exists, it is passed to "/bin/sh" to execute.
*/

#include	<ctype.h>
#include	<pwd.h>
#include	<sgtty.h>
#include	<signal.h>
#include	<stdio.h>
#include	<utmp.h>
#include	<sys/types.h>
#include	<sys/stat.h>

extern char		*crypt(), *index(), *ttyname();
extern int		chdir(), fork(), stat(), wait();
extern int		isatty(), strcmp(), strlen(), strncmp();
extern struct passwd	*getpwnam();

#define	UTMP	"/etc/utmp"
#define	WTMP	"/usr/adm/wtmp"
#define	XTMP	"/etc/xtmp"
#define	MOTD	"/etc/motd"
#define	GAMES	"/usr/games"
#define	SHELL	"/bin/sh"
#define	CMDFILE	".profile"

static char		*shell = SHELL ;
static char		*gamedir = GAMES ;

static struct utmp	utmpb ;
#define	ut_tty	ut_line[0]

static struct sgttyb	ttyb ;

static struct stat	statb ;

static char		name[9] , password[9] ;

static char		*usrmail = "/usr/mail/XXXXXXXX" ;
main( argc , argv )			/* "login" executive */
	int			argc ;
	char			*argv[];
	{
	extern char		_sobuf[] ;	// in STDIO
	char			*devttyx ;	// gets tty name

	signal( SIGQUIT , SIG_IGN );
	signal( SIGINT , SIG_IGN );

	nice( -40 ); nice( 20 ); nice( 0 );	// V6 compatible

	/* Find the terminal name, "/dev/tty~" */

	if ( ! isatty( 1 ) || (devttyx = ttyname( 0 )) == NULL )
		{
		fputs( "Login via terminal only!\n" , stderr );
		exit( 1 );
		}
	// We have one synonym for a /dev/tty~
	if ( strcmp( &devttyx[5] , "console" ) == 0 )
		strcpy( &devttyx[5] , "tty8" );	// V6 only
	utmpb.ut_tty = devttyx[8] ;	// V6 only

	gtty( 0 , &ttyb );
	ttyb.sg_erase = `\b' ;
	ttyb.sg_kill = `U' - 0100 ;
	ttyb.sg_flags &= ~ RAW ;	// cooked mode
	ttyb.sg_flags |= ECHO ;
	stty( 0 , &ttyb );

	setbuf( stdout , _sobuf );

	for ( ; ; )
		{
		register struct passwd	*pwp ;	// -> user info
		FILE			*fp ;	// general stream ptr
		int			badname = 0 ;	// set on error
		register char		*np = name ;	// general ptr
		/* Get login name */

		if ( argc > 1 )
			if ( strlen( argv[1] ) == 0 )
				exit( 1 );	// back to "getty" etc.
			else
				strncpy( np , argv[1] , 8 );
		else	{
			fputs( "Name: " , stdout ); fflush( stdout );
			if ( fgets( np , 9 , stdin ) == NULL ||
			     toascii( *np ) == `\n' )
				{
				putchar( `\n' );
				exit( 1 );	// back to "getty" etc.
				}
			for ( ; np < &name[8] ; ++ np )	// strip parity
				if ( (*np = toascii( *np )) == `\n' )
					break ;
			*np = 0 ;	// null replaces \n if any
			if ( np == &name[8] )
				{
				register int	c ;

				while ( (c = getchar()) != EOF && c != `\n' )
					;	// eat up rest of input
				}
			}

		/* Get user's data from password file */

		if ( (pwp = getpwnam( name )) == NULL )
			++ badname ;	// name not in password file

		for ( np = &name[0] ; *np != 0 ; ++ np )
			;		// find EOS
		for ( ; np < &name[8] ; ++ np )
			*np = ` ' ;	// pad with blanks
		while ( --np >= &name[0] )
			if ( *np < ` ' )
				*np = `?' ;	// remove ctrls
		strncpy( utmpb.ut_name , name , 8 );

		/* We don't allow logins into /usr/games[/....] on dialups */

		if ( ! badname &&
		     (utmpb.ut_tty == `9' || utmpb.ut_tty == `a') &&
		     strncmp( gamedir , pwp->pw_dir , strlen( gamedir ) )
		     == 0 )
			{
			puts( "No games on dialup!" );
			if ( argc > 1 )
				exit( 1 );	// back to "getty" etc.
			continue ;
			}
		/* Get password from user */

		if ( badname || pwp->pw_passwd != NULL )
			{
			register char	*cp = "Armadillos" ;

			ttyb.sg_flags &= ~ ECHO ;
			stty( 0 , &ttyb );

			fputs( "Password: " , stdout ); fflush( stdout );

			if ( fgets( password , 9 , stdin ) == NULL ||
			     password[0] == `\n' )
				{
				ttyb.sg_flags |= ECHO ;
				stty( 0 , &ttyb );
				putchar( `\n' );
				exit( 1 );	// back to "getty" etc.
				}
			for ( np = &password[0] ; np < &password[8] ; ++ np )
				if ( (*np = toascii( *np )) == `\n' )
					break ;
			*np = 0 ;	// null replaces \n if any
			if ( np == &password[8] )
				{
				register int	c ;

				while ( (c = getchar()) != EOF && c != `\n' )
					;	// eat up rest of input
				}

			ttyb.sg_flags |= ECHO ;
			stty( 0 , &ttyb );
			putchar( `\n' );

			if ( ! badname && pwp->pw_passwd != NULL )
				cp = pwp->pw_passwd ;
			if ( strcmp( crypt( password , cp ) , cp ) != 0 )
				++ badname ;	// failed password test
			}

		time( &utmpb.ut_time );	// time stamp
		/* After both name AND password, check validity */

		if ( badname )
			{
			// record the transgression
			if ( (fp = fopen( XTMP , "r" )) != NULL
			  && freopen( XTMP , "a" , fp ) != NULL )
				{
				fwrite( &utmpb , sizeof utmpb , 1 , fp );
				for ( np = &password[0] ; *np != 0 ; ++ np )
					;	// find EOS
				for ( ; np < &password[8] ; ++ np )
					*np = ` ' ;	// pad with blanks
				while ( --np >= &password[0] )
					if ( *np < ` ' )
						*np = `?' ;	// no ctrls
				fwrite( password , 8 , 1 , fp );
				fclose( fp );
				}

			puts( "Login incorrect." );

			if ( argc > 1 )
				exit( 1 );	// back to "getty" etc.
			continue ;
			}

		/* Set up home directory and "shell" program */

		if ( chdir( pwp->pw_dir ) == -1 )
			{
			puts( "No directory" );
			if ( argc > 1 )
				exit( 1 );	// back to "getty" etc.
			continue ;
			}

		/* Successful login -- update accounting files */

		if ( (fp = fopen( UTMP , "r+w" )) != NULL )
			{
			register int	t = utmpb.ut_tty ;

			if ( t >= `a' )
				t -= `a' - (10 + `0') ;

			fseek( fp , (long)((t - `0') * sizeof utmpb) , 0 );
			fwrite( &utmpb , sizeof utmpb , 1 , fp );
			fclose( fp );
			}

		if ( (fp = fopen( WTMP , "r" )) != NULL
		  && freopen( WTMP , "a" , fp ) != NULL )
			{
			fwrite( &utmpb , sizeof utmpb , 1 , fp );
			fclose( fp );
			}
		/* Display message-of-the-day and tell about mail */

		if ( (fp = fopen( MOTD , "r" )) != NULL )
			{
			register int	c ;

			while ( (c = getc( fp )) != EOF )
				putchar( c );
			fclose( fp );
			}

		strcpy( &usrmail[10] , pwp->pw_name );
		if ( stat( usrmail , &statb ) == 0 )
			puts( "You have mail." );
		fflush( stdout );

		/* Set up user parameters, then start up "shell" program */

		chown( devttyx , pwp->pw_uid , pwp->pw_gid );
		setgid( pwp->pw_gid );	// the order is critical
		setuid( pwp->pw_uid );

		// in 7th edition, the following is done by the shell itself
		if ( stat( CMDFILE , &statb ) == 0 )	// exists
			{
			register int	pid ;

			if ( (pid = fork()) == 0 )	// child
				{
				execl( shell , "sh" , CMDFILE , NULL );
				exit( 1 );	// "can't happen"
				}
			while ( wait( NULL ) != -1 )
				;	// wait for only child
			}

		{	// begin local symbol block
		register char	*ap ;	// -> Shell argument

		if ( strlen( pwp->pw_shell ) == 0 )
			pwp->pw_shell = shell ;	// use default shell

		if ( (ap = index( pwp->pw_shell , ` ' )) == NULL )
			execl( pwp->pw_shell , "-sh" , NULL );
		else	{
			*ap++ = 0 ;	// terminate command name
			execl( pwp->pw_shell , "-sh" , ap , NULL );
			}
		puts( "No Shell" );
		exit( 1 );		// back to "getty" etc.
		}	// end local symbol block
		}
	}
