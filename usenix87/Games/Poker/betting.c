/*
** Handle all the betting being done.
*/

# include	<stdio.h>
# include	"betting.h"
# include	"players.h"
# include	"computer.h"

# define	TRUE	1
# define	FALSE	0

int	betting( player, n_players, pot, first )

PLAYER	player[];		/* all player information */
int	n_players;		/* # of players */
int	*pot;			/* current pot */
int	first;			/* player who gets to bet first */

{
int	last_raiser=(-1);	/* player who made last raise */
int	n_active=0;		/* # players active in betting (haven't folded) */
int	n_raises=0;		/* # of raises this hand so far */
int	j=(-1);			/* player currently betting */
int	i;
int	total_bet;		/* total amount bet each player this hand */
char	temp[80];
int	done;			/* has player entered something valid yet? */
int	amount;			/* how much more is player donating to pot? */
int	cycle=FALSE;		/* has bet been around circle once yet? */
int	x;
int	rounds=0;

i = first;
while ( i != first || !cycle )
	{
	if ( player[i].in )
		{
		if ( j == -1 )
			{
			total_bet = player[i].bet;
			j = i;
			}
		sprintf( temp, "B%d%d", i, player[i].bet );
		tellall( player, n_players, temp );
		sprintf( temp, "$%d%d", i, player[i].cash );
		tellall( player, n_players, temp );
		n_active++;
		}
	if ( ++i == n_players )
		{
		cycle = TRUE;
		i = 0;
		}
	}
tellall( player, n_players, "U" );
while( n_active > 1 && j != last_raiser )
	{
	sprintf( temp, "T%d", j );
	tellall( player, n_players, temp );
	sprintf( temp, "P%d", *pot );
	tellall( player, n_players, temp );
	sprintf( temp, "B%d%d", j, player[j].bet );
	tellall( player, n_players, temp );
	tellall( player, n_players, "U" );
	sprintf( temp, "$%d%d", j, player[j].cash );
	tellall( player, n_players, temp );
	if ( j > 0 )		/* if not computer */
		{
		sprintf( temp, "?%d", total_bet - player[j].bet );
		writeln( player[j].socket, temp );
		}
	tellall( player, n_players, "U" );	/* update displays */

	/* get bet ... */

	done = FALSE;
	while ( !done )
		{
		if ( j > 0 )
			readln( player[j].socket, temp );
		else
			computer_bet( player, n_players, n_active, n_raises, *pot, total_bet, temp );
		done = TRUE;
		switch ( temp[0] )  {
			case 'Q' :
					player[j].wantsout = TRUE;
					done = FALSE;
					break;
			case 'S' :
					player[j].sittingout = TRUE;
					done = FALSE;
					break;
			case 'F' :
					player[j].in = FALSE;
					sprintf( temp, "H%d--fold--", j );
					tellall( player, n_players, temp );
					n_active--;
					break;
			case 'C' :
					amount = total_bet - player[j].bet;
					*pot += amount;
					player[j].cash -= amount;
					player[j].bet = total_bet;
					sprintf( temp, "B%d%d", j, total_bet );
					tellall( player, n_players, temp );
					sprintf( temp, "H%dCall", j );
					tellall( player, n_players, temp );
					break;
			case 'R' :
					if ( ++n_raises > n_players )
						{
						sprintf( temp, "MMax # of raises; Call or fold." );
						writeln( player[j].socket, temp );
						--j;
						break;
						}
					last_raiser = j;
					amount = (total_bet += (x = atoi(temp+1))) - player[j].bet;
					*pot += amount;
					player[j].cash -= amount;
					player[j].bet = total_bet;
					sprintf( temp, "B%d%d", j, total_bet );
					tellall( player, n_players, temp );
					sprintf( temp, "H%dRaise %d", j, x);
					tellall( player, n_players, temp );
					break;
			case 'P' :
					sprintf( temp, "H%dPass", j );
					tellall( player, n_players, temp );
					break;
			}
		}
	sprintf( temp, "$%d%d", j, player[j].cash );
	tellall( player, n_players, temp );

	sprintf( temp, "P%d", *pot );
	tellall( player, n_players, temp );
	tellall( player, n_players, "U" );
	if ( ++j >= n_players )
		{
		++rounds;
		j = 0;
		}
	while( !player[j].in )
		if ( ++j >= n_players )
			{
			++rounds;
			j = 0;
			}
	if ( last_raiser == -1 && ( rounds > 1 || ( rounds == 1 && j >= first ) ) )
		break;		/* all passed or folded */
	}
if ( n_active <= 1 )
	return( j );
else
	return( -1 );
}
