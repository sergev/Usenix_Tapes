#include <stdio.h>
#include "intdefs.h"

/* endlin - end a line and watch for screen overflow */

static int lc = 0;                                        /* line counter */

endlin()
{
extern int tsc;                             /* true if output is to screen */
register int c;

if(tsc && ++lc >= SCRSIZ) {        /* pause if output is to console screen */
    fputs("\n\033[7m--More--", stdout);     /* and we've shown a screenful */
    c = GETC;
    fputs("\033[0m\r\033[K ", stdout);
    switch(c) {
        case '\r':                          /* <RETURN> - show 1 more line */
            lc = SCRSIZ - 1;
            break;
        case 'q':                             /* quit with "q" or "ctrl-C" */
        case '\003':
            exit(0);
        default:
            lc = 0;                         /* else show another screenful */
            break;
        }
    return('\b');
    }   
else
    return('\n');
}
