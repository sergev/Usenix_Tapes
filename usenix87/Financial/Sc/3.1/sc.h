/*	VC	A Table Calculator
 *		Common definitions
 *
 *		original by James Gosling, September 1982
 *		modified by Mark Weiser and Bruce Israel,
 *			University of Maryland
 *
 */



#define MAXROWS 200
#define MAXCOLS 40
#define error move(1,0), clrtoeol(), printw

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

#define ACOS 0
#define ASIN 1
#define ATAN 2
#define CEIL 3
#define COS 4
#define EXP 5 
#define FABS 6 
#define FLOOR 7
#define HYPOT 8
#define LOG 9
#define LOG10 10
#define POW 11
#define SIN 12
#define SQRT 13
#define TAN 14
#define DTR 15
#define RTD 16
#define MIN 17
#define MAX 18

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
struct enode *new_const();
struct enode *new_var();
struct ent *lookat();
struct enode *copye();
char *coltoa();

int modflg;
