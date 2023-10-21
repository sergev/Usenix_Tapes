/*
** Send everyone a message.
*/

# include	"tellall.h"
# include	"util.h"
# include	"players.h"

void	tellall( player, n_players, msg )

PLAYER	player[];		/* all player info */
int	n_players;		/* # of players */
char	*msg;			/* message to send */

{
int	i;

for( i=1; i<n_players; i++ )	/* don't send any messages to the computer */
	if ( player[i].bet != -1 )	/* no messages to anyone sitting out */
		writeln( player[i].socket, msg );
}
