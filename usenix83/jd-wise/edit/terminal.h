#include <sgtty.h>
#define ff '\014'
#define esc '\033'
#define ctl(x) ('x'&037)

#define escup	'A'
#define escdown	'B'
#define escright 'C'
#define escleft	'D'
#define eschome	'H'
#define escdc	'P'
#define escic	'@'
#define escil	'L'
#define escdl	'M'
#define escel	'K'
#define escef	'q'
#define escep	'J'
#define escst	'1'
#define escct	'2'
#define escow	'2'	/* ct <=> set overwrite */
#define esctab	'M'	/* now dl */
#define escbtab	'L'	/* now il */
#define escesc	'\033'

/* special functions */
#define escaccept	'o'	/* accept spelling word */
#define escsfind	's'	/* find misspelled word */
#define esca		'a'	/* append line */
#define escd		'd'	/* delete (line) */
#define esch		'h'	/* scroll to tos */
#define eschat		'^'	/* scroll down */
#define esci		'i'	/* insert line */
#define escj		'j'	/* join */
#define escmatch	'm'	/* match bracket */
#define escminus	'-'	/* same as eschat */

int hastabs;

int curline,curcol;	/* current virtual position */
int Curline,Curcol;	/* current actual position */
int curstack[128],cursptr;


struct sgttyb sgtty;
struct tchars tchars;
int origmode;
int ospeed;		/* output baud rate */
int termtype;

char *tty;
