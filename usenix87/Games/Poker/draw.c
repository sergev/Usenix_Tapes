/*
** Let each player who is still in draw as many as 5 cards to replace
** those he already has in his hand. This routines expects a command
** of the form Dxxxxx  from each player program, where x is either
** 'k'  to keep the card in that position, or
** 'd'  to discard the card in that position.
*/

# include	"draw.h"
# include	"players.h"
# include	"showcards.h"
# include	"tellall.h"

# define	TRUE		1
# define	FALSE		0

void	draw( player, n_players, deck, d, first )

PLAYER	player[];		/* hands are in here */
int	n_players;		/* # of players */
int	deck[];			/* shuffled deck of cards */
int	d;			/* next card to take from deck */
int	first;			/* player who gets to draw first */

{
int	i,c;
char	temp[80];
char	temp2[20];
int	n;
int	cycle=FALSE;

i = first;
while ( i != first || !cycle )
	{
	if ( player[i].in )
		{
		sprintf( temp, "T%d", i );
		tellall( player, n_players, temp );
		tellall( player, n_players, "U" );
		if ( i == 0 )
			computer_draw( player, temp );
		else
			{
			writeln( player[i].socket, "D" );
			readln( player[i].socket, temp );
			}
		if ( temp[0] != 'D' )	/* what else could it be ?? */
			{
			--i;		/* do this man again ! */
			continue;
			}
		n = 0;
		for( c=0; c<5; c++ )
			if ( temp[c+1] == 'd' )
				{
				player[i].cards[c] = deck[d++];
				n++;
				}
		if ( n == 0 )
			strcpy( temp2, "Draws none" );
		else
			sprintf( temp2, "Draws %d", n );
		sprintf( temp, "H%d%s", i, temp2 );
		tellall( player, n_players, temp );
		tellall( player, n_players, "U" );
		if ( i != 0 )
			{
			show( player, temp, i, i );
			writeln( player[i].socket, "U" );
			}
		}
	if ( ++i == n_players )
		{
		cycle = TRUE;
		i = 0;
		}
	}
}
