/*
** This is a program to handle a multi-user poker game. It uses sockets in
** the AF_INET family, so it can handle several players even on different
** machines. The game is 5-card draw poker.
*/

# include	<signal.h>
# include	"cards.h"
# include	"players.h"
# include	"betting.h"
# include	"draw.h"
# include	"whowon.h"
# include	"payoff.h"
# include	"showcards.h"
# include	"util.h"
# include	"tellall.h"
# include	"limits.h"

# define	TRUE		1
# define	FALSE		0
# define	ERROR		-1

main( argc, argv )

int	argc;
char	**argv;

{
int	i,c,d;
int	deck[52];		/* the cards */
int	pot;			/* size of current pot	*/
int	winner;			/* which player won this hand	*/
int	first=0;		/* first player to bet, draw, etc. */
# ifdef MASSCOMP
extern	void	srand();
# else
extern	void	srandom();	/* starts random number generator */
# endif
extern	long	time();		/* unique parameter to above */
extern	void	crash();	/* if broken pipe, release port (I hope) */
extern	int	strcmp();
extern	int	getpid();	/* more random stuff */

signal(SIGPIPE,crash);
signal(SIGHUP,crash);
signal(SIGXCPU,SIG_IGN);
# ifdef MASSCOMP
	srand((int) time(0) ^ getpid() );
# else
	srandom((int) time(0) ^ getpid() );
# endif
player[0].name = strsave( "Computer III" );
player[0].cash = 200;
player[0].sittingout = FALSE;
player[0].wantsout = FALSE;
for(;;)
	{
	if (n_players < 2)
		new_players( player, &n_players, TRUE );	/* wait for someone to play with */
	pot = 0;
	for( i=0; i<n_players; i++)
		{
		if ( player[i].sittingout == FALSE )
			{
			player[i].in = TRUE;
			player[i].bet = ANTE;	/* ante */
			player[i].cash -= ANTE;
			pot += ANTE;
			}
		else
			{
			player[i].in = FALSE;
			player[i].bet = (-1);	/* keep tellall() quiet */
			} 
		if ( n_players == 2 )
			player[i].lonehands += 1;
		}
	tellall( player, n_players, "X" );	/* clear all screens */
	tellall( player, n_players, "U" );	/* clear all screens */
	shuffle( deck );
	d = 0;		/* next card to draw from deck */
	for( c=0; c<5; c++ )
		for( i=0; i<n_players; i++ )
			if ( player[i].in )
				player[i].cards[c] = deck[d++];
	/* show cards to each player ... */
	show_cards( player, n_players, FALSE, FALSE );
	if ( ++first >= n_players )
		first = 0;
	if ((winner = betting( player, n_players, &pot, first )) < 0)
		{
		draw( player, n_players, deck, d, first );	
		if ((winner = betting( player, n_players, &pot, first )) < 0)
			winner = whowon( player, n_players, argc > 1 && strcmp( argv[1], "-test" ) == 0 );
		else if ( argc > 1 && strcmp( argv[1], "-test" ) == 0 )
			whowon( player, n_players, TRUE );
		}
	else if ( argc > 1 && strcmp( argv[1], "-test" ) == 0 )
		whowon( player, n_players, TRUE );
	payoff( player, n_players, winner, pot );
	leave( player, &n_players );
	new_players( player, &n_players, FALSE );
	}
}
