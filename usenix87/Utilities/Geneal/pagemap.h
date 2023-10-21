/* pagemap.h - definition for the pagemap functions */
/* last edit 15-Sep-84 21:52:47 by jimmc (Jim McBeath) */
/* last edit 09-Sept-85 22:54:00 by tlr (Terry L. Ridder) */

extern char *xalloc();

#define XALLOC(item, count, msg) (item *)xalloc(sizeof(item)*count,msg)

struct pagem {
    int rows;		/* number of rows (lines) on the page */
    int cols;		/* number of columns on the page */
    char **data;	/* pointer to an array of char pointers */
    };

typedef struct pagem Pagem, *Pagemp;

/* data is stored in row major order; given the data pointer from the
 * above structure, data[x] points to row x, and data[x][y] points to
 * character y in row x.
 *
 * The number of columns is actually greater by two than the number
 * x->cols indicates; this is to leave space for a newline and a null
 * at the end.
 */

/* access macros */
#define pagePutc(pp,r,c,ch) ((pp)->data[r][c] = (ch))
#define pageGetc(pp,r,c)    ((pp)->data[r][c])

extern Pagemp pageInit();

/* end */
