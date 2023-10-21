#include <stdio.h>
#include <types.h>

typedef int (*PFI)();

struct nlist {
	char *name;
	PFI func;
	struct nlist *next;
};

#define AR_SIZE 2000

	/* types of line */
#define SOLID        1
#define LONG_DASH    2
#define DOT          3
#define DASH_DOT     4
#define MEDIUM_DASH  5
#define DASH_2_DOTS  6

#define MIN_L_TYPE   1
#define MAX_L_TYPE   6

	/* types of points */
#define POINT        1
#define PLUS         2
#define ASTERISK     3
#define CIRCLE       4
#define CROSS        5
#define DIAMOND      6

#define MIN_P_TYPE   1
#define MAX_P_TYPE   6

	/* maximum number of columns of data the program can read from 
	   each line of the input file */
#define MAX_COLUMN  10

	/* the character typed by the user to backspace in the text window */
#define BACKSPACE 	'\b'
#define CR 			'\r'
#define LF			'\n'

/* global variables */

INT16 dev_crt, xscale, yscale, scale;
INT16 MINX, MAXX, MINY, MAXY;
INT16 T_HEIGHT, T_WIDTH;
INT16 cur_p_type, cur_l_type;
int high_line, low_line;
int xc, yc;
double x_text_pos, y_text_pos;			/* pos where text to be drawn next */
double file_x[AR_SIZE], file_y[AR_SIZE];	/* raw coords from file */
double x[AR_SIZE], y[AR_SIZE];				/* same, but translated to NDC */
double minx, maxx, miny, maxy;
int num_read;
FILE *fp;

short graph_w, text_w;
short PRINT, PLOT;

#ifdef PRINTER
INT16 dev_prt;
INT16 PR_MINX, PR_MAXX, PR_MINY, PR_MAXY;
INT16 PR_T_HEIGHT, PR_T_WIDTH;
double pr_x[AR_SIZE], pr_y[AR_SIZE];
#endif
