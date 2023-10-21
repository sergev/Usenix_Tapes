extern	char	ranks[];	/* card ranks 2-A */
extern	char	suits[];	/* card suits D,C,H,S */

# define	card(x)		((x) % 13)
# define	suit(x)		((x) / 13)
# define	rank(x)		(ranks[card(x)])
# define	color(x)	(suits[suit(x)])

extern	void	shuffle();	/* shuffle a deck of cards */
extern	int	crandom();	/* return random number 0 .. limit-1 */
