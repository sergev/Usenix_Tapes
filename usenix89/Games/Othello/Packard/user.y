%{
/*
 *	ex:set ts=8 sw=8:
 *	user interface
 */

# include	"reversi.h"
# include	<stdio.h>
# include	<signal.h>

boardT	board, saveBoard;
int	saved;
int	savePlayer;
int	atend;
int	atbegin;
int	level;
int	player;
extern int	maxlev, movex, movey;
int	com;
int	gotsignal;
char	sbuf[80];
char	ebuf[80];
int	sdebug = 0, mdebug = 0;
int	record = 0;
FILE	*rfile;
int	first = WHITE;
int	defcom = BLACK;
int	showScore = 1;

struct move {
	int	p, x, y;
};

struct move	saveGame[64];
struct move	*saveP;

%}
%token	MOVE LEVEL COMPUTER UNDO HINT PLAY
%token	RECORD REPLAY SAVE
%token	RESTART NEW GAME QUIT
%token	GRID NOGRID HELP NOHELP SCORE NOSCORE
%token	DEBUG EVAL
%token	FROM INTO TO FILEe NO
%token	NUMBER COMMA NL STRING SEMI EOG ERR
%token	WH BL HUMAN BOTH NEITHER NONE FIRST SECOND
%%
game	:	game commands NL prompt
	|	prompt
	;
prompt	:
	{
		if (!atend) {
			loop:	;
			dispTurn (player);
			if (!hasmove (player, board)) {
				if (!hasmove (-player, board)) {
					fini (board);
					if (com == 0)
						com = BLACK;
					++atend;
					dispTurn (EMPTY);
					goto nomove;
				} else {
					if (player == WHITE)
						dispError ("white has no move");
					else
						dispError ("black has no move");
					player = -player;
				}
			}
			if (com == 0 || com == player) {
				dispError ("thinking...");
				if (computer (player, board, level)) {
					atbegin = 0;
					sprintf (ebuf, "I move to %d, %d\n",
						movex, movey);
					dispError (ebuf);
					saveP->x = movex;
					saveP->y = movey;
					saveP->p = player;
					++saveP;
					if (record)
						fprintf (rfile, "%d: %d,%d\n",
						    player, movex, movey);
					player = -player;
					display (board);
					if (gotsignal && com != 0)
						gotsignal = 0;
				}
				if (gotsignal && com == 0) {
					com = -player;
					gotsignal = 0;
				}
				goto loop;
			}
		}
	nomove:	;
		readLine ();
	}
	;
commands:	commands SEMI command
	|	command
	|	error oerror
		{
			dispHelp ();
		}
	;
command	:	
	|	EOG
		{
			YYACCEPT;
		}
	|	omove NUMBER ocomma NUMBER
		{
			if (1 <= $2 && $2 <= SIZE &&
			    1 <= $4 && $4 <= SIZE &&
			    legal (player, $2, $4, board)) {
				copy (saveBoard, board);
				savePlayer = player;
				++saved;
				move (player, $2, $4, board);
				atbegin = 0;
				if (record)
					fprintf (rfile, "%d: %d,%d\n",
					    player, $2, $4);
				saveP->x = $2;
				saveP->y = $4;
				saveP->p = player;
				++saveP;
				player = -player;
				display (board);
			} else {
				sprintf (ebuf, "illegal move: %d, %d", $2, $4);
				dispError (ebuf);
			}
		}
	|	DEBUG STRING
		{
			register char	*s;
			register int	v;

			v = 1;
			for (s = sbuf; *s; ++s)
				switch (*s) {
				case 'm':
					mdebug = v;
					break;
				case 's':
					sdebug = v;
					break;
				case '!':
					v = !v;
					break;
				}
		}
	|	GRID
		{
			dispGrid ();
		}
	|	NO GRID
		{
			dispNoGrid ();
		}
	|	NOGRID
		{
			dispNoGrid ();
		}
	|	SCORE
		{
			showScore = 1;
			dispScore (board);
		}
	|	NOSCORE
		{
			showScore = 0;
			dispNoScore ();
		}
	|	NO SCORE
		{
			showScore = 0;
			dispNoScore ();
		}
	|	LEVEL NUMBER
		{
			level = $2;
		}
	|	LEVEL oerror
		{
			sprintf (ebuf, "current level is %d", level);
			dispError (ebuf);
		}
	|	PLAY whichp
		{
			if ($2 == WHITE || $2 == BLACK) 
				defcom = $2;
			com = $2;
		}
	|	PLAY oerror
		{
			dispError ("play (white black both none)");
		}
	|	whichp FIRST
		{
			if ($1 == WHITE || $1 == BLACK)
				first = $1;
			if (atbegin)
				player = first;
		}
	|	FIRST oerror
		{
			dispError ("(white black you me) first");
		}
	|	whichp SECOND
		{
			if ($1 == WHITE || $1 == BLACK)
				first = - $1;
			if (atbegin)
				player = first;
		}
	|	SECOND oerror
		{
			dispError ("(white black you me) second");
		}
	|	HELP
		{
			dispHelp ();
		}
	|	NOHELP
		{
			dispNoHelp ();
		}
	|	NO HELP
		{
			dispNoHelp ();
		}
	|	QUIT
		{
			YYACCEPT;
		}
	|	UNDO
		{
			if (saved) {
				copy (board, saveBoard);
				player = savePlayer;
				saved = 0;
				display (board);
			}
		}
	|	NEW ogame eoc
		{
			YYABORT;
		}
	|	RESTART eoc
		{
			YYABORT;
		}
	|	EVAL
		{
			sprintf (ebuf, "score: %d\n", score (board, WHITE));
			dispError (ebuf);
		}
	|	RECORD ointo ofile STRING
		{
			if ((rfile = fopen (sbuf, "w")) == NULL) {
				sprintf (ebuf, "could not open %s", sbuf);
				dispError (ebuf);
				record = 0;
			} else
				++record;
		}
	|	RECORD oerror
		{
			dispError ("record \"file\"");
		}
	|	REPLAY whichp ofrom ofile STRING
		{
			replay ($2, sbuf);
		}
	|	REPLAY oerror
		{
			dispError ("replay (both white black) \"file\"");
		}
	|	SAVE ointo ofile STRING
		{
			struct move	*m;

			if ((rfile = fopen (sbuf, "w")) == NULL) {
				sprintf (ebuf, "could not open %s", sbuf);
				dispError (ebuf);
			} else {
				m = saveGame;
				fprintf (rfile, "%d: -1,-1\n", m->p);
				for (; m != saveP; m++)
					fprintf (rfile, "%d: %d,%d\n",
					    m->p, m->x, m->y);
				fclose (rfile);
				rfile = 0;
			}
		}
	|	SAVE oerror
		{
			dispError ("save \"file\"");
		}
	|	HINT
		{
			if (hasmove (player, board)) {
				char	buf[80];
				hint (player, board, level);
				sprintf (buf, "I suggest %d, %d", movex, movey);
				dispError (buf);
			}
		}
	;
eoc	:	SEMI
	|	NL
	;
omove	:	MOVE
	|
	;
ogame	:	GAME
	|
	;
ocomma	:	COMMA
	|
	;
oerror	:	oerror error
		{
			yyerrok;
		}
	|	oerror ERR
	|
	;
ointo	:	TO
	|	INTO
	|
	;
ofrom	:	FROM
	|
	;
ofile	:	FILEe
	|
	;
whichp	:	WH
		{ $$ = WHITE; }
	|	BL
		{ $$ = BLACK; }
	|	COMPUTER
		{ $$ = com==WHITE?WHITE:BLACK; }
	|	HUMAN
		{ $$ = com==WHITE?BLACK:WHITE; }
	|	BOTH
		{ $$ = 0; }
	|	none
		{ $$ = 2; }
	;
none	:	NONE
	|	NEITHER
	;
%%
yyerror (s)
char	*s;
{
	dispError (s);
}

caught ()
{
	gotsignal++;
	signal (SIGINT, caught);
}

main (argc, argv)
char **argv;
{
	signal (SIGINT, caught);
	level = 2;
	dispInit ();
	srand (getpid());
	while (**++argv == '-') {
		while (*++*argv) {
			switch (**argv) {
			case 'b':
				defcom = BLACK;
				break;
			case 'w':
				defcom = WHITE;
				break;
			case '1':
				if (!*++*argv)
					continue;
				if (**argv == WHITE)
					first = WHITE;
				else
					first = BLACK;
				break;
			case 'g':
				dispGrid ();
				break;
			case 's':
				showScore = 1;
			}
		}
	}
	do {
		if (rfile)
			fclose (rfile);
		rfile = 0;
		player = first;
		com = defcom;
		atend = 0;
		atbegin = 1;
		setup ();
		saved = 0;
		saveP = saveGame;
		display (board);
		if (*argv) {
			replay (0, *argv);
			++argv;
		}
	} while (yyparse ());
	dispEnd ();
}

yywrap ()
{
	return 1;
}

setup ()
{
	register int	i,j;

	for (i = 1; i <= SIZE; i++)
		for (j = 1; j <= SIZE; j++)
			board[i][j] = 0;
	board[4][4] = WHITE;
	board[4][5] = BLACK;
	board[5][4] = BLACK;
	board[5][5] = WHITE;
}

replay (who, file)
char *file;
{
	int	x, y, p;
	if (rfile)
		fclose (rfile);
	if ((rfile = fopen (file, "r")) == NULL) {
		sprintf (ebuf, "could not open %s", file);
		dispError (ebuf);
		return;
	}
	while (fscanf (rfile, "%d: %d, %d\n", &p, &x, &y) == 3) {
		if (x == -1 && y == -1) {
			player = p;
			continue;
		}
		if (!hasmove (player, board)) {
			player = -player;
			if (!hasmove (player, board))
				return;
		}
		if (p != player) {
			sprintf (ebuf, "not %s's turn\n",
			    player == WHITE? "white":"black");
			dispError (ebuf);
			return;
		}
		if (who == 0 || p == who) {
			if (!legal (p, x, y, board)) {
				sprintf(ebuf, "illegal move: %d, %d\n", x, y);
				dispError (ebuf);
				return;
			}
			move (p, x, y, board);
			atbegin = 0;
			player = -player;
			display (board);
		}
		else if (player == com) {
			if (hasmove (player, board)) {
				dispError ("thinking...");
				dispTurn (EMPTY);
				if (computer (player, board, level)) {
					dispError ("");
					atbegin = 0;
					player = -player;
					display (board);
				}
			}
		}
	}
	fclose (rfile);
	rfile = 0;
}
