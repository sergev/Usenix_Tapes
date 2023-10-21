/*
** Figure out who wins.
*/

# include	<stdio.h>
# include	"whowon.h"
# include	"players.h"
# include	"cards.h"
# include	"showcards.h"
# include	"tellall.h"

# define	TRUE		1
# define	FALSE		0

static	char	*scores[] = {
	"Nothing", "Pair", "Two Pair", "Three of a kind", "Straight", "Flush", 
	"Full House", "Four of a kind", "Straight Flush", "Royal Flush" };

int	whowon( player, n_players, test )

PLAYER	player[];		/* all player information */
int	n_players;		/* number of players */
int	test;			/* show all cards regardless */

{
int	i;
int	max=(-1);		/* highest score */
int	winner=0;		/* which player won */
char	temp[80];		/* tell everybody what everybody has */
void	evaluate(),tie();

show_cards( player, n_players, TRUE, test );	/* show all hands everywhere */
for( i=0; i<n_players; i++ )
	if ( test || player[i].in )
		{
		evaluate( player, i );
		sprintf( temp, "R%d%s", i, scores[player[i].score] );
		tellall( player, n_players, temp );
		if ( player[i].in && player[i].score > max )
			{
			max = player[i].score;
			winner = i;
			}
		else if ( player[i].in && player[i].score == max )
			{
			tie( player, &winner, i );
			}
		}
return( winner );
}

static	void	tie( player, winner, which )

PLAYER	player[];		/* all hands on deck */
int	*winner;		/* may have to set new winner */
int	which;			/* which player is challenging the winner */

{
int	top1,top2;
int	i;
int	done=FALSE;
int	oldwinner;

oldwinner = *winner;
while( !done )
	{
	top1 = top2 = -1;
	for( i=12; i>=0; i-- )
		{
		if ( top1 == -1 || player[which].count[i] > player[which].count[top1] )
			top1 = i;
		if ( top2 == -1 || player[*winner].count[i] > player[*winner].count[top2] )
			top2 = i;
		}
	if ( top1 != top2 )
		{
		if ( top1 > top2 )
			*winner = which;
		done = TRUE;
		}
	else if ( player[which].count[top1] == 0 )	/* OH, NO! */
		{
		printf( "tie() failed to resolve - details follow\n" );
		for( i=0; i<5; i++ )
			printf( "%c%c ", rank(player[which].cards[i]), color(player[which].cards[i]) );
		for( i=0; i<5; i++ )
			printf( "%c%c ", rank(player[*winner].cards[i]), color(player[*winner].cards[i]) );
		fflush( stdout );
		done = TRUE;
		}
	else
		{
		player[which].count[top1] = 0;
		player[*winner].count[top2] = 0;
		}
	}
/* re-evaluate hands because counts have been messed ... */
evaluate( player, oldwinner );
evaluate( player, which );
}

void	evaluate( player, which )

PLAYER	player[];
int	which;			/* which player's hand to evaluate */

{
int	kind[6];
int	i,j;

for( i=0; i<13; i++ )
	player[which].count[i] = 0;
for( i=0; i<5; i++ )
	player[which].count[card(player[which].cards[i])] ++ ;
for( i=0; i<6; i++ )
	kind[i] = 0;
for( i=0; i<13; i++ )
	kind[player[which].count[i]] ++ ;
if ( kind[4] )
	player[which].score = FOUR;
else if ( kind[3] && kind[2] )
	player[which].score = FULL;
else if ( kind[3] )
	player[which].score = THREE;
else if ( kind[2] == 2 )
	player[which].score = TWOPAIR;
else if ( kind[2] )
	player[which].score = PAIR;
else 
	player[which].score = NOTHING;
if ( player[which].score == NOTHING )
	{
	for( i=1; i<5; i++ )
		if ( suit(player[which].cards[i]) != suit(player[which].cards[0] ) )
			break;
	if ( i == 5 )
		player[which].score = FLUSH;
	i = -1;
	while( player[which].count[++i] == 0 ) ;
	j = i;
	while( ++i < 13 && player[which].count[i] == 1 ) ;
	/* trash below is for A-5 straights */
	if ( i - j == 5 || ((i == 4) && (j == 0) && (player[which].count[12] == 1)) )
		if ( player[which].score == FLUSH )
			player[which].score = STRAIGHT_FLUSH;
		else
			player[which].score = STRAIGHT;
	if ( player[which].score == STRAIGHT_FLUSH && i == 13 )
		player[which].score = ROYAL_FLUSH;
	}
}
