/*
** Handle betting and drawing for the computer's hand.
*/

# include	"computer.h"
# include	"players.h"
# include	"cards.h"
# include	"whowon.h"
# include	"limits.h"

# define	min(x,y)	((x) > (y) ? (y) : (x))

# define	TRUE	1
# define	FALSE	0

static	int	bluffing=FALSE;		/* bluffing this hand? */
static	int	bluffmore=FALSE;	/* more bluff bets this hand? */

void	computer_bet( player, n_players, n_active, n_raises, pot, total, temp )

PLAYER	player[];		/* player information & hands */
int	n_players;		/* number of players */
int	n_active;		/* number of players still in current hand */
int	n_raises;		/* # raises so far this betting session */
int	pot;			/* size of pot */
int	total;			/* current total bet */
char	temp[];			/* return betting command here */

{
int	raise;			/* amount to raise by */
int	chance;			/* rating of hand */
int	x;			/* random compared to chance to determine action */
int	top=(-1);		/* highest card hand has most of */
int	i;
static	int	lastpot=99999;	/* trick to only get random once per hand */
static	int	random_factor=0;

evaluate( player, 0 );
for( i = 12; i > 0 ; i-- )
	if ( top == -1 || player[0].count[i] > player[0].count[top] )
		top = i;
if ( pot < lastpot )		/* things to be done once per hand */
	{
	random_factor = n_players * crandom( 10 );
	if ( crandom( 20 ) < 1 )	/* bluff sometimes	*/
		bluffing = TRUE;	/* 1 out of 20 chance 	*/
	else
		bluffing = FALSE;
	bluffmore = bluffing;
	}
lastpot = pot;
chance = player[0].score * player[0].score * 10 + top + 1 + random_factor ;
if ( bluffmore )
	chance += n_players * 50;
if ( ( ( total < chance && crandom(10) < 6 ) || chance > total + 80 ) && n_raises < n_players )
	{
	raise = min( RAISE_LIMIT, (player[0].score + crandom(3) - 1) * 10 );
	if ( raise <= 5 ) raise = 5;
	if ( bluffing )
		{
		raise = RAISE_LIMIT - crandom( 2 ) * 10;
		bluffmore = FALSE;
		}
	sprintf( temp, "R%d", min( RAISE_LIMIT, raise ) );
	}
else if ( total <= player[0].bet )
	sprintf( temp, "P" );
else if ( player[0].score >= THREE	/* 3 of a kind or better never folds */
	|| ( n_active == 2 && player[0].score > NOTHING )
	|| total < chance + 80 - 100 / n_players )	/* comparatively good hand */
		{
		sprintf( temp, "C" );
		}
else
	sprintf( temp, "F" );
}

void	computer_draw( player, temp )

PLAYER	player[];		/* computer's hand is in here */
char	temp[];			/* return command here */

{
int	i,j;

/* hand has already been evaluated by computer_bet() at least once. */
strcpy( temp, "Dkkkkk" );
if ( player[0].score < STRAIGHT )	/* higher hands require no discards */
	for( i=0; i<5; i++ )
		{
		for( j=0; j<5; j++ )
			if ( i != j && card(player[0].cards[i]) == card(player[0].cards[j]) )
				break;
		if ( j == 5 )
			{
			temp[i+1] = 'd';	/* discard it */
			}
		}
bluffmore = bluffing;
}
