/*
** A structure is maintained with information on each player.
*/

typedef struct _player {
	char	*name;		/* Player's name 	*/
	int	userid;		/* player's user id number	*/
	int	cash;		/* Amount of money player currently has */
	int	cards[5];	/* Player's current hand (indexes into deck[]) */
	int	score;		/* PAIR, THREE, FLUSH, etc.	*/
	int	count[13];	/* counters used to determine score */
	int	in;		/* Still in current hand?	*/
	int	wantsout;	/* Quit after this hand?	*/
	int	sittingout;	/* Sit out after this hand?	*/
	int	bet;		/* How much bet into current pot*/
	int	lonehands;	/* # hands played against only computer */
	int	socket;		/* To communicate with player on*/
	} PLAYER;

extern	PLAYER	player[];

extern	int	n_players;	/* current number of players (including computer) */

extern	void	new_players(),leave(),crash();
