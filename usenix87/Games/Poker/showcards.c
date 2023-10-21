/*
** Tell each player everybody's name. Show each player only his own cards.
*/

# include	"showcards.h"
# include	"players.h"
# include	"cards.h"
# include	"util.h"

void	show_cards( player, n_players, all, test )

PLAYER	player[];		/* all player info */
int	n_players;		/* # active players */
int	all;			/* TRUE=show everybody's cards who is still in
				   FALSE=show only player's cards to himself */
int	test;			/* show all cards whether in or out */
				/* only used if 'all' is TRUE */

{
int	i,j,k;
char	temp[10];
void	show();

for(i=1; i<n_players; i++)	/* don't show cards to computer */
	if ( player[i].bet >= 0 )	/* if sitting out, be quiet */
		{
		for( j=0; j<n_players; j++ )	/* tell player everyone's name */
			{
			sprintf( temp, "N%d", j );
			write( player[i].socket, temp, 2 );
			writeln( player[i].socket, player[j].name );
			}
		if ( all )
			{
			for( k=0; k<n_players; k++ )
				if ( test || player[k].in )
					show( player, temp, k, i );
			}
		else
			show( player, temp, i, i );
		writeln( player[i].socket, "U" );	/* update screen command */
		}
}

void show( player, temp, i, j )

PLAYER	player[];
char	temp[];
int	i;		/* whose cards to show */
int	j;		/* who to show them to */

{
int	c;

sprintf( temp, "C%d", i );		/* cards for player i */
write( player[j].socket, temp, 2 );
for(c=0; c<5; c++)
	{
	sprintf( temp, "%c%c ", rank(player[i].cards[c]), color(player[i].cards[c]));
	write( player[j].socket, temp, 3 );
	}
write( player[j].socket, "\n", 1 );
}
