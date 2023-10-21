#
#include <stdio.h>
/* NEC Spinwriter 5520 plotting output routines */
# define DOWN "\012"
# define UP "\0339"
# define LEFT "\010"
# define RIGHT "\040"
# define BEL "\007"
# define ESC "\033"
# define ACK "\006"
#define PLOTIN "\033]a\033]p"
#define PLOTOUT "\033]l\033]w"
# define CR 015
# define FF 014
# define VERTRESP 48
# define HORZRESP 120.
# define VERTRES 8.
# define HORZRES 6.
/* down is line feed, up is escape 9,
   left is backspace, right is space.  48 points per inch
   vertically, 120 horizontally */

extern int xnow, ynow;
extern int ITTY[3], PTTY[3], OUTF;
extern float HEIGHT, WIDTH, OFFSET;
extern int xscale, xoffset, yscale;
extern float botx, boty, obotx, oboty, scalex,scaley;


#define	spewchar(x)	putchar(x)
#define	spew(x)	printf(x)
