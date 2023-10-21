/*	VC	A Table Calculator
 *		Common definitions
 *
 *		original by James Gosling, September 1982
 *		modified by Mark Weiser and Bruce Israel,
 *			University of Maryland
 *
 */

#include<stdio.h>

#ifdef MCH_AMIGA
#define ROWS	23
#define COLS	79
#else
#define ROWS	24
#define COLS	80
#endif

#define MAXROWS 200
#define MAXCOLS 40

struct ent {
    double v;
    char *label;
    struct enode *expr;
    short flags;
    short row, col;
    struct ent *next;
};


struct enode {
    int op;
    union {
	double k;
	struct ent *v;
	struct {
	    struct enode *left, *right;
	} o;
    } e;
};

/* op values */
#define O_VAR 'v'
#define O_CONST 'k'
#define O_REDUCE(c) (c+0200)

/* flag values */
#define is_valid     0001
#define is_changed   0002
#define is_lchanged  0004
#define is_leftflush 0010
#define is_deleted   0020

#define ctl(c) ('c'&037)

struct ent *tbl[MAXROWS][MAXCOLS];

int strow, stcol;
int currow, curcol;
int savedrow, savedcol;
int FullUpdate;
int maxrow, maxcol;
int fwidth[MAXCOLS];
int precision[MAXCOLS];
char hidden_col[MAXCOLS];
char hidden_row[MAXROWS];
char line[1000];
int linelim;
int changed;
struct ent *to_fix;
struct enode *new();
struct ent *lookat();
struct enode *copye();
char *coltoa();

int modflg;
