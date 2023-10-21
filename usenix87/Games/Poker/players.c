/*
** Routines to handle players joining game and quitting.
*/

# include	"players.h"
# include	"util.h"
# include	"port.h"
# include	"scores.h"
# include	<errno.h>
# include	<sys/types.h>
# include	<sys/socket.h>
# ifdef MASSCOMP
#	include		<net/in.h>
	typedef	unsigned long	u_long;
# 	include		<net/misc.h>
# else
# 	include		<netinet/in.h>
#	include		<sys/time.h>
# endif
# include	<stdio.h>
# include	<netdb.h>
# ifdef MASSCOMP
	extern	char	*index();
# else
# 	include		<strings.h>
# endif

# define	TRUE		1
# define	FALSE		0
# define	ERROR		-1

# define	START_CASH	200	/* amount new player starts with */
# define	MAX_PLAYERS	5	/* max # concurrent players 	*/

PLAYER	player[MAX_PLAYERS];		/* player information	*/
int		n_players=1;		/* computer is playing	*/
static	int	main_socket=ERROR;	/* socket to accept connections on */

void	new_players( player, n_players, block )

PLAYER	player[];	/* array of player info structures	*/
int	*n_players;	/* number of players (incl comp) 	*/
int	block;		/* if nobody wants to join, wait on socket
			   until somebody does. */

{
int	readfds;		/* for select() call 	*/
char	temp[80];		/* to read player's name*/
int	s;			/* temp holder for new socket	*/
int	rc;			/* # ready selected sockets */
int	open_sock();		/* to open main socket */
# ifdef MASSCOMP
int	free_socket();		/* gets a free socket */
int	new_socket;		/* file descriptor of socket gotten */
# endif
struct	sockaddr_in	from;	/* connection acceptor */
int	fromlen = sizeof(from);
# ifndef MASSCOMP
struct	timeval	t;		/* don't let select() run forever */
# endif
int	i;

if (main_socket == ERROR)
	main_socket = open_sock();
/* set up 1 second timeout for use on all select() calls */
# ifndef MASSCOMP
t.tv_sec = 1L;
t.tv_usec = 0L;
# endif
/* see if people who were sitting out want to rejoin yet */
for( i=1; i<*n_players; i++ )
	{
	if ( player[i].sittingout )
		{
		readfds = 1 << player[i].socket;
# ifdef MASSCOMP
		if ( select( 32, &readfds, 0, 1000 ) > 0 )	/* 1000 ms */
# else
		if ( select( 32, &readfds, 0, 0, &t ) > 0 )
# endif
			{
			readln( player[i].socket, temp );
			if ( temp[0] == 'S' )
				player[i].sittingout = FALSE;
			}
		}
	}
for (;;)
	{
	readfds = 1 << main_socket;
# ifdef MASSCOMP
	rc = select( 32, &readfds, 0, 1000 );
# else
	rc = select( 32, &readfds, 0, 0, &t );
# endif
	if (rc > 0 || block)
		{
# ifdef MASSCOMP
		if ((s = accept( main_socket, &from )) < 0 )
# else
		if ((s = accept( main_socket, &from, &fromlen)) < 0)
# endif
			{
			perror("accept");
fflush(stdout);
			break;
			}
		if ( *n_players >= MAX_PLAYERS )
			{
			writeln( s, "TOOMANY" );
			close( s );
			}
		else
			{
			block = FALSE;
			printf("accepted connection from ");
# ifdef MASSCOMP
			new_socket = free_socket();
			socketaddr( new_socket, &from );
			sprintf( temp, "%d", ntohs( from.sin_port ) );
			writeln( main_socket, temp );
			accept( new_socket, &from );
			read( main_socket, temp, 1 );	/* eat the EOF ????? */
			close( main_socket );
			sleep(5);
			main_socket = open_sock();
			s = new_socket;
# else
			writeln( s, "OK" );
# endif
			player[*n_players].socket = s;
			player[*n_players].cash = START_CASH;
			player[*n_players].in = FALSE;
			player[*n_players].wantsout = FALSE;
			player[*n_players].sittingout = FALSE;
			player[*n_players].lonehands = 0;
			readln( s, temp );
			temp[strlen( temp ) - 1] = NULL;
			/* don't allow colons in name because of format of score file */
			if ( index( temp, ':' ) != NULL )	
				*index( temp, ':' ) = NULL;
			if ( strlen( temp ) > 25 )
				temp[25] = NULL;
			player[*n_players].name = strsave( temp );
			printf("%s\n",temp);
			fflush(stdout);
			readln( s, temp );
			player[*n_players].userid = atoi( temp );
			if ( played_before( player[*n_players].name, player[*n_players].userid ) )
				player[*n_players].cash = get_cash();
			(*n_players)++;
			break;
			}
		}
	else
		break;
	}
}

void	leave( player, n_players )

PLAYER	player[];
int	*n_players;

{
register int	i;

for( i=1; i<*n_players; i++)
	if (player[i].wantsout)
		{
		writeln( player[i].socket, "Q" );
		put_cash( player[i].name, player[i].userid, player[i].cash );
		high_score_list( player[i].socket );
		close( player[i].socket );
		free( player[i].name );
		player[i] = player[*n_players - 1];
		(*n_players)--;
		--i;		/* since this entry is new, test again */
		}
}
/* Open the main socket. */

int	open_sock()

{
int	s;
struct	sockaddr_in	sockaddr;
struct	hostent		*host;

# ifdef MASSCOMP
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port = htons( PORT );
	if ((host = gethostbyname( HOST )) == NULL)
		{
		perror( "gethostbyname" );
		exit(1);
		}
	sockaddr.sin_addr.s_addr = *(u_long *) host->h_addr;
	s = socket( SOCK_STREAM, 0, &sockaddr, SO_ACCEPTCONN );
# else
	s = socket( AF_INET, SOCK_STREAM, 0 );
# endif
if (s < 0)
	{
	perror("open_sock");
	exit(1);
	}
# ifndef MASSCOMP
sockaddr.sin_family = AF_INET;
sockaddr.sin_port = htons( PORT );
if (bind( s, &sockaddr, sizeof(sockaddr)) < 0)
	{
	perror("bind");
	exit(1);
	}
if (listen( s, 0 ) < 0)
	{
	perror("listen");
	exit(1);
	}
# endif
return( s );
}

void	crash()

{
int	i;

for(i=0; i<32; i++)
	close(i);
exit(9);
}

# ifdef MASSCOMP

/* accept() does not allocate a new socket on the Masscomps. Therefore we
** have to grab our own new socket and tell the client where it is.
*/

free_socket()

{
int	s;

if ( ( s = socket( SOCK_STREAM, 0, 0, SO_ACCEPTCONN ) ) < 0 )
	{
	perror( "free_socket" );
	crash();
	}
return( s );
}

# endif
