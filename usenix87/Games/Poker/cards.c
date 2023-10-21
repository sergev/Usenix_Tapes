
/*
** Basic card playing routines like shuffling, etc.
*/

# include	"cards.h"
# ifdef MASSCOMP
#	define		MAXRAND		32767
# else
# 	define		MAXRAND		2147483648
# endif
# define	ERROR	-1

char	ranks[]="23456789TJQKA";
char	suits[]="DCHS";

void	shuffle (deck)

int	deck[];		/* return deck of shuffled cards here */

{
int	temp[52];
int	i,x;
int	crandom();

for(i=0; i<52; i++)
	temp[i] = i;
for(i=0; i<52; i++)
	{
	while (temp[x = crandom(52)] == ERROR) ;
	deck[i] = temp[x];
	temp[x] = ERROR;
	}
}

int	crandom( limit )

int	limit;

{
# ifdef MASSCOMP
# define	random	rand
# endif
unsigned long	random();
unsigned long	scale;
int		x;

scale = (unsigned long) MAXRAND/limit;
x = random()/scale;
if ( x < 0 ) x = 0;	/* weird things do happen occasionally */
return(x);
}
