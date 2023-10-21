# include	"reversi.h"
extern int offsets[];

legal (player, x, y, board)
register int	player;
int				x, y;
boardT			board;
{
	register char	*b, *m;
	register int	*o, i;

	b = & board[x][y];
	player = -player;
	if (*b == EMPTY) {
		for (o = offsets; i = *o++;) {
			if (b[i] == player) {
				m = b+i;
				while (*m == player)
					m += i;
				if (*m == -player)
					return 1;
			}
		}
	}
	return 0;
}
