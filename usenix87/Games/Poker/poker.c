# include	"util.h"
# include	"port.h"
# include	"limits.h"
# include	<sys/types.h>
# include	<sys/socket.h>
# ifdef MASSCOMP
# 	include		<net/in.h>
	typedef	unsigned long	u_long;
#	include		<net/misc.h>
# else
# 	include		<netinet/in.h>
# endif
# include	<netdb.h>
# include	<stdio.h>
# include	<curses.h>
# include	<ctype.h>
# include	<signal.h>

int	s;	/* the socket */

main( argc, argv, envp )

int	argc;
char	**argv;
char	**envp;

{
struct	sockaddr_in	sockaddr;
struct	hostent		*host;
char	*getenv();
char	*name;
char	temp[80];
extern	int	getuid();
void	closem();

# ifdef MASSCOMP
	s = socket( SOCK_STREAM, 0, 0, 0 );
# else
	s = socket( AF_INET, SOCK_STREAM, 0 );
# endif
if (s < 0) 
	{
	perror( "socket" );
	exit(1);
	}
if ((host = gethostbyname( (argc > 1) ? argv[1] : HOST )) == NULL)
	{
	perror( "gethostbyname" );
	exit(1);
	}
sockaddr.sin_family = AF_INET;
sockaddr.sin_port = htons( PORT );
sockaddr.sin_addr.s_addr = *(u_long *) host->h_addr;
printf( "Waiting for last hand to finish ... " );
fflush(stdout);
signal( SIGINT, closem );
signal( SIGQUIT, closem );
# ifdef MASSCOMP
if ( connect( s, &sockaddr ) < 0 )
# else
if ( connect( s, &sockaddr, sizeof(sockaddr) ) < 0)
# endif
	{
	perror( "connect" );
	exit(1);
	}
readln( s, temp );
if ( strcmp( temp, "TOOMANY\n" ) == 0 )
	printf( "Table Full!\n" );
else
	{
	if ( strcmp( temp, "OK\n" ) != 0 )	/* must go to another socket */
		{
		close( s );
		sockaddr.sin_port = htons( atoi( temp ) );
# ifdef MASSCOMP
		s = socket( SOCK_STREAM, 0, 0, 0 );
		if ( s < 0 )
			perror("damn socket");
		if ( connect( s, &sockaddr ) < 0 )
# else
		s = socket( AF_INET, SOCK_STREAM, 0 );
		if ( connect( s, &sockaddr, sizeof(sockaddr) ) < 0 )
# endif
			{
			perror( "second connect" );
			exit( 1 );
			}
		}
	signal( SIGINT, SIG_IGN );
	signal( SIGQUIT, SIG_IGN );
	if ( ( name = getenv( "POKER" ) ) == NULL )
		if ( ( name = getenv( "NAME" ) ) == NULL )
			name = getenv( "USER" );
	writeln( s, name );
	sprintf( temp, "%d", getuid() );
	writeln( s, temp );
	play( s );
	readln( s, temp );
	while( temp[0] != ':' )
		{
		printf( "%s", temp );
		readln( s, temp ); 
		}
	}
close(s);
exit(0);
}


/* 
** Listen to poker daemon and update the screen as it says. These are the
** commands from the daemon to this program:
**
**	Nn	name of player n
**	Cn	cards for player n
**	P	pot value
**	U	update screen
**	X	clear screen
**	Hn	action for player n (fold, raise, call, etc.)
**	Rn	result for player n (straight, two pair, --fold--, etc.)
**	Bn	total amount bet by player n this hand
**	$n	amount of cash player n has
**	D	get discards from this player
**	?	get bet from this player - amount to call follows
** 	Q	player has quit; cleanup & terminate
**	W	player name following has won this hand
**	T	tells which player's turn it is; cursor is put next to his name
**	M	message to print on bottom of screen
**
**
** These are the commands from this program to the daemon:
**
**	Q	quit after this hand is finished
**	F	fold
**	Dxxxxx	discard these cards (k=keep,d=discard)
**	C	call
**	Rxx	raise by this amount
**	P	pass - not allowed if player has to call someone else's raise
**	S	toggles sitting out switch
**
*/

play( s )

int	s;		/* socket we communicate upon */

{
char	temp[80];	/* buffer to read command into */
char	temp2[80];	/* outgoing commands are sometimes built here */
int	n;		/* determine which player we're talking about */
int	minimum;	/* minimum bet to stay in */
char	c;		/* player's command */
int	done;		/* get commands until something valid */
int	quit=FALSE;	/* has player quit yet? */
int	out=FALSE;	/* is player sitting out? */
char	cards[20];	/* hold player's current hand */
void	mygetstr();	/* because curses getstr() doesn't work */
int	i,x;
int	oldx=0,oldy=0;	/* markers for current player cursor */

initscr();
noecho(); crmode();
leaveok( stdscr, FALSE );
while ( !quit )
	{
	readln( s, temp );
	n = temp[1] - '0';	/* which player? */
	switch ( temp[0] ) {
		case 'M' :
				mvaddstr( 15, 40, temp+1 );
				refresh();
				break;
		case 'N' :
				mvaddstr( n*2, 4, temp+2 );
				break;
		case 'T' :
				mvaddstr( oldy, oldx, "    " );
				mvaddstr( n*2, 0, "--> " );
				oldy = n * 2;
				oldx = 0;
				refresh();
				break;
		case 'C' :
				mvaddstr( n*2, 35, temp+2 );
				strcpy( cards, temp+2 );
				break;
		case 'U' :
				move( 0, 0 );
				refresh();
				break;
		case 'X' :
				clear();
				mvaddstr( 14, 4, "C) Call last raise" );
				mvaddstr( 15, 4, "F) Fold this hand" );
				mvaddstr( 16, 4, "P) Pass (Check)" );
				mvaddstr( 17, 4, "Q) Quit after this hand" );
				mvaddstr( 18, 4, "R) Raise last bet" );
				mvaddstr( 19, 4, "S) Sit out for a while" );
				break;
		case 'P' :
				move( LINES-1, 4 );
				printw( "Pot: %-6d ", atoi( temp + 1 ) );
				break;
		case 'B' :
				move( n*2+1, 6 );
				printw( "Bet: %-6d ", atoi( temp + 2 ) );
				break;
		case 'H' :
				mvaddstr( n*2+1, 35, temp+2 );
				break;
		case 'R' :
				mvaddstr( n*2, 60, temp+2 );
				break;
		case '$' :
				move( n*2+1, 18 );
				printw( "Cash: %-6d ", atoi( temp + 2 ) );
				break;
		case 'Q' :
				endwin();
				quit = TRUE;
				break;
		case 'W' :
				move( LINES-2, 20 );
				temp[strlen( temp ) - 1] = NULL;	/* strip newline */
				printw( "%s wins this hand! --more-- ", temp+1 );
				refresh();
				while( getch() != ' ' ) ;
				if ( out )
					{
					move( LINES-2, 20 );
					clrtoeol();
					printw( "Hit space to come back--" );
					refresh();
					while( getch() != ' ' ) ;
					printw( "waiting..." );
					refresh();
					writeln( s, "S" );
					out = FALSE;
					}
				break;
		case 'D' :
				/* find out what player wants to discard */
				move( 14, 25 );
				clrtoeol();
				mvaddstr( 14, 40, "Discard: " );
				refresh();
				mygetstr( temp );
				i = 0;
				strcpy( temp2, "Dkkkkk" );
				while( temp[i] != NULL )
					{
					if ( isdigit( temp[i] ) )
						{
						x = temp[i] - '0';
						if ( x > 0 && x < 6 )
							temp2[x] = 'd';
						}
					i++;
					}
				writeln( s, temp2 );
				break;
		case '?' :
				/* time to let player bet on this hand */
				minimum = atoi( temp + 1 );
				done = FALSE;
				move( 14, 25 );
				clrtoeol();
				while ( !done )
					{
					mvaddstr( 14, 25, "Bet: " );
					refresh();
					while( (c = getch()) == '\014' )
						wrefresh( curscr );
					if ( islower( c ) )
						c = toupper( c );
					done = TRUE;
					switch (c)  {
						case 'Q' :
							done = FALSE;
							mvaddstr( LINES-1, 50, "Quitting." );
						case 'C' :
							if ( c == 'C' && minimum == 0 )
								{
								mvaddstr( 14, 35, "No raise to call; pass or raise." );
								refresh();
								done = FALSE;
								break;
								}
						case 'F' :
							sprintf( temp, "%c", c );
							writeln( s, temp );
							break;
						case 'S' :
							done = FALSE;
							out = TRUE;
							writeln( s, "S" );
							mvaddstr( LINES-1, 30, "Sitting out." );
							break;
						case 'P' :
							if ( minimum > 0 )
								{
								mvaddstr( 14, 35, "Can't pass; meet last raise or fold." );
								refresh();
								done = FALSE;
								}
							else
								writeln( s, "P" );
							break;
						case 'R' :
							strcpy( temp, "0" );
							while ( atoi( temp ) <= 0 )
								{
								move( 15, 25 );
								printw( "%15.15s", " " );
								mvaddstr( 15, 25, "Raise: " );
								refresh();
								mygetstr( temp );
								if ( atoi( temp ) > RAISE_LIMIT )
									{
									move( 15, 40 );
									printw( "Raise limit = %d.", RAISE_LIMIT );
									strcpy( temp, "0" );
									}
								}
							minimum = atoi( temp );
							sprintf( temp, "R%d", minimum );
							writeln( s, temp );
							break;
						default:
							done = FALSE;
							/* try again */
							break;
						}
					}
				break;
		default:
			/* ignore unknown commands */
				break;
		}
	}
printf("\n\n\n");
}

void	mygetstr( str )

char	*str;

{
int	i=0,x,y;
char	c;

getyx( stdscr, y, x );
while ( ( c = getch() ) != '\n' )
	if ( c == '\b' && i > 0 )
		{
		move( y, --x );
		refresh();
		--i;
		}
	else if ( c == '\014' )
		wrefresh( curscr );
	else 
		{
		str[i++] = c;
		addch( c );
		refresh();
		x++;
		}
str[i] = NULL;
}

void	closem()

{
close( s );
exit( 1 );
}
