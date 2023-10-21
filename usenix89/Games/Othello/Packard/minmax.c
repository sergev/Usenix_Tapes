/*
 *	minmax.c
 */

# include	"reversi.h"

int	maxlev, movex, movey;

/*
 *	this defines the order in which the board
 *	is searched for the best move.  It is
 *	here to shorten the time to best move,
 *	this increasing the chance of hitting
 *	a good trimming point as well as
 *	increasing the possibility of making
 *	a reasonable move when an interrupt is
 *	caught.
 */
short	morder[64][2] = {
	1,1, 1,8, 8,1, 8,8,
	1,3, 1,6, 3,1, 3,8, 6,1, 6,8, 8,3, 8,6,
	3,3, 3,6, 6,3, 6,6,
	1,4, 1,5, 4,1, 4,8, 5,1, 5,8, 8,4, 8,5,
	3,4, 3,5, 4,3, 4,6, 5,3, 5,6, 6,4, 6,5,
	2,3, 2,6, 3,2, 3,7, 6,2, 6,7, 7,3, 7,6,
	2,4, 2,5, 4,2, 4,7, 5,2, 5,7, 7,4, 7,5,
	1,2, 1,7, 2,1, 2,8, 7,1, 7,8, 8,2, 8,7,
	2,2, 2,7, 7,2, 7,7, 4,4, 4,5, 5,4, 5,5,
};

# define	NOMOVE	(-32760)


# define	USECOPY
# ifdef	USECOPY
struct copyB {
	boardT	data;
};
# define	copyb(next,board)	(*((struct copyB *)next) = *((struct copyB *) board))
# else
# define	copyb(next,board)	copy(next,board)
# endif

copy(next, board)
register int	*next, *board;
{
	register int	count;

	count = sizeof (boardT) / sizeof (int);
	do {
		*next++ = *board++;
	} while (--count);
}

computer (player, board, level)
boardT	board;
{
	int	i;
	extern int	com, defcom;

	maxlev = level;
	movex = movey = -1;
	i = seek (player, board, 0, 1, -NOMOVE);
	if (movex == -1 || movey == -1)
		return 0;
	move (player, movex, movey, board);
	return 1;
}

hint (player, board, level)
boardT board;
{
	int	i;

	maxlev = level;
	i = seek (player, board, 0, 1, -NOMOVE);
	if (movex == -1 || movey == -1)
		return 0;
	return 1;
}

seek (player, board, level, moved, best)
register player;
register boardT	board;
{
	boardT		next;
	register int	x, y;
	register int	s;
	int		max, i;
	int		bestx, besty;
	int		m, j;
	extern int	gotsignal;

	max = NOMOVE;
	m = 0;
	for (j = 0; j < 60; j++) {
		x = morder[j][0];
		y = morder[j][1];
		if (gotsignal)
			return 0;
		if (legal (player, x, y, board)) {
			copyb (next, board);
			if (level == 0 && movex == -1) {
				movex = x;
				movey = y;
			}
			move (player, x, y, next);
			++m;
			if (level >= maxlev)
				s = score (next, player);
			else
				s = seek (-player, next, level+1, 1, -max);
			if (s >= max) {
				/*
				 *	try to make the game appear random
				 *	by choosing among equal moves
				 *	randomly
				 */
				if (s == max && rand()&01)
					goto skip;
				if (s > best)
					return -s;
				bestx = x;
				besty = y;
				if (level == 0) {
					movex = bestx;
					movey = besty;
				}
				max = s;
			}
skip:			;
		}
	}
	if (m == 0)
		if (moved && level)
			return seek (-player, board, level + 1, 0, -best);
		else
			return - count (player, board) * 500;
	return -max;
}
