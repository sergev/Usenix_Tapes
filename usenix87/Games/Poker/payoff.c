/*
** Add current pot to winners available cash. Tell everyone who won.
*/

# include	"players.h"
# include	"tellall.h"

void	payoff( player, n_players, winner, pot )

PLAYER	player[];		/* player information */
int	n_players;		/* number of players */
int	winner;			/* which player won */
int	pot;			/* how much $ was won */

{
char	temp[80];

player[winner].cash += pot;
sprintf( temp, "W%s", player[winner].name );
tellall( player, n_players, temp );
}
