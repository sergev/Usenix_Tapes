#include    <cursor.h>
/*
**      CURSOR -- X Y positioning routine for ITEM OWLs
**                (also misc other operations...)
**      psl 8/78
**
** Compile: cc -O -c -q cursor.c; ar r /lib/libP.a cursor.o
*/

#define FF      014                 /* ^L */
#define CURADDR 017                 /* ^O */
#define LOPEN   020                 /* ^P */
#define LCLOSE  021                 /* ^Q */
#define CDEL    022                 /* ^R */

#define MULTC   026

#define LCLEAR  043

cursor(x, y)
int    x, y;
{
	register char *cp;
	static char cursbuf[4][4], next;

	next = (next + 1) & 3;
	cp = &cursbuf[next][4];
	*--cp = '\0';
	if (x < 0) {
	    if (x == CLEAR)
		*--cp = FF;
	    else if (x == OPEN_LINE)
		*--cp = LOPEN;
	    else if (x == CLOSE_LINE)
		*--cp = LCLOSE;
	    else if (x == CHAR_DEL)
		*--cp = CDEL;
	    else if (x == CLEAR_LINE) {
		*--cp = LCLEAR;
		*--cp = MULTC;
	    } else if (x == FIELD) {
		*--cp = y & ~3;
		*--cp = '$';
		*--cp = MULTC;
	    } else if (x == GRAPHICS) {
		*--cp = (y << 2) | 3;
		*--cp = '$';
		*--cp = MULTC;
	    } else
		*--cp = '?';
	} else {
	    *--cp = x + 040;
	    *--cp = y + 040;
	    *--cp = CURADDR;
	}
	return(cp);
}
