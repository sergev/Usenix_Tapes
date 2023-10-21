/*
 *	score.c
 *
 *	score a board position
 */

# include	"reversi.h"
#ifdef SDEBUG
extern int sdebug;
#endif

boardT	base = {
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	  0,   0, -40, -10,  -5,  -5, -10, -40,   0,   0,
	  0,   0, -10,   3,   1,   1,   3, -10,   0,   0,
	  0,   0,  -5,   1,   0,   0,   1,  -5,   0,   0,
	  0,   0,  -5,   1,   0,   0,   1,  -5,   0,   0,
	  0,   0, -10,   3,   1,   1,   3, -10,   0,   0,
	  0,   0, -40, -10,  -5,  -5, -10, -40,   0,   0,
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
};

short edgemod[10][3] = {
	0,	0,	0,
	0,	0,	0,
	0,	0,	0,
	5,	-10,	5,
	5,	-5,	5,
	5,	-5,	5,
	5,	-10,	5,
	0,	0,	0,
	0,	0,	0,
	0,	0,	0,
};

extern short	cornerscores[3][3][3][3];

extern short	edgescores[6561];

# define edgesc(a,b,c,d,e,f,g,h)	edgescores[a*2187 + b*729 + c*243 + \
					d*81 + e*27 + f*9 + g*3 + h + 3280]

score (board, player)
register boardT	board;
int	player;
{
	register char	*j, *b;
	char	*l;
	register int	score;
	int	i;
	int	n, m;

#ifdef SDEBUG
	if (sdebug) {
		display (board);
	}
#endif
	score = 0;
	for (i = 3; i < SIZE - 1; i++) {
		j = & board[i][3];
		b = & base [i][3];
		l = & board[i][SIZE-1];
		while (j != l) {
			n = *j++;
			score += n * *b++;
		}
		score += board[2][i]      * edgemod[i][board[1][i]+1];
		score += board[SIZE-1][i] * edgemod[i][board[SIZE][i]+1];
		score += board[i][2]      * edgemod[i][board[i][1]+1];
		score += board[i][SIZE-1] * edgemod[i][board[i][SIZE]+1];
	}
	score +=
		cornerscores[board[1][1] + 1]
			[board[1][2] + 1]
			[board[2][1] + 1]
			[board[2][2] + 1] +
		cornerscores[board[1][8] + 1]
			[board[1][7] + 1]
			[board[2][8] + 1]
			[board[2][7] + 1] +
		cornerscores[board[8][1] + 1]
			[board[8][2] + 1]
			[board[7][1] + 1]
			[board[7][2] + 1] +
		cornerscores[board[8][8] + 1]
			[board[8][7] + 1]
			[board[7][8] + 1]
			[board[7][7] + 1];
	score += edgesc (board[1][1], board[1][2], board[1][3], board[1][4],
			 board[1][5], board[1][6], board[1][7], board[1][8]) +
		 edgesc (board[8][1], board[8][2], board[8][3], board[8][4],
			 board[8][5], board[8][6], board[8][7], board[8][8]);
	score += edgesc (board[1][1], board[2][1], board[3][1], board[4][1],
			 board[5][1], board[6][1], board[7][1], board[8][1]) +
		 edgesc (board[1][8], board[2][8], board[3][8], board[4][8],
			 board[5][8], board[6][8], board[7][8], board[8][8]);
#ifdef SDEBUG
	if (sdebug)
		printf ("score: %d\n", score);
#endif
	return score * player;
}
