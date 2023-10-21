/*
**      Defines for CURSOR -- X Y positioning routine for ITEM OWLs
**                (also misc other operations...)
**      psl 8/78
*/

#define CLEAR       -65         /* code for screen clear */
#define OPEN_LINE   -66         /* code for insert line */
#define CLOSE_LINE  -67         /* code for delete line */
#define CHAR_DEL    -68         /* code for delete character */
#define CLEAR_LINE  -75         /* code to clear rest of line and <CR> */
#define FIELD       -76         /* code to indicate attribute setting */
#define DIM         0004        /*      half intensity attribute */
#define REVERSE     0010        /*      reverse video attribute */
#define INVISIBLE   0020        /*      non-display attribute */
#define NORMAL      0040        /*      normal attribute */
#define BLINK       0100        /*      blink attribute */
#define GRAPHICS    -77         /* code to get alt. graphics character set */

