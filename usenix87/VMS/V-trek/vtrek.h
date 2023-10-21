/* vtrek.h -- header file for visual star trek */


#include stdio.h
#include math.h
#include ctype.h

#define VTREKINS	"[richs.games.vtrek]vtrek.doc"	/* instructions */
#define ERROR		-1
#define PI		3.1415926
#define Toupper(x)	(islower(x) ? toupper(x) : x)
#define abs(x)		(x < 0 ? -x : x)

/* used for damaging klingon */
#define AUTOKILL	9999

/* used for fixing/damaging devices */
#define ABS		0		/* absolute fix */
#define REL		1		/* relative fix */
#define RND		-1		/* pick a random device */

/* status of a ship */
#define ALIVE		0		/* ship is still here */
#define DEAD		1		/* ship has been destroyed */

/* used for replot calls */
#define TEXT		1		/* replot text portion */
#define INFO		2		/* replot information portion */
#define ELEMENT		4		/* replot individual element */
#define ALL		(TEXT | INFO)	/* replot all */

/* legal condition values */
#define GREEN		0		/* everything OK */
#define YELLOW		1		/* reason to be cautious */
#define RED		2		/* eminent danger */
#define DOCKED		3		/* docked at base (no danger) */

/* legal quadrant values (used in s.r.s) */
#define EMPTY		0		/* sector is empty */
#define KLINGON		1		/* sector contains klingon */
#define STARBASE	2		/* sector contains star base */
#define STAR		3		/* sector contains star */
#define PLAYER		4		/* sector contains player */
#define TBLAST		5		/* torpedo */

/* legal damage control indices */
#define WARP		0		/* warp drive */
#define SRS		1		/* short range sensors */
#define LRS		2		/* long range sensors */
#define PHASER		3		/* phaser control */
#define DAMAGE		4		/* damage control */
#define DEFENSE		5		/* defense control (shields) */
#define COMPUTER	6		/* computer (galaxy map, calculations) */
#define TUBES		7		/* torpedo tubes */

/* legal status items */
#define STARDATE	0
#define CONDITION	1
#define QUADRANT	2
#define SECTOR		3
#define ENERGY		4
#define TORPS		5
#define SHIELDS		6

/* used for readout calls */
#define CLEAR		0		/* clear readout */
#define ADDLINE		1		/* add line to readout */

/* sructure used to store quandrant information */
typedef struct {
	char nkling;	/* number of klingons in quadrant */
	char nbase;	/* number of bases in quadrant */
	char nstar;	/* number of stars in quadrant */
	char known;	/* tells if info is known to player */
} QUADINFO;

/* structure used to store klingon information */
struct {
	int xs, ys;	/* sector coordinates */
	int sh;		/* shield level */
} klingon[3];

QUADINFO galaxy[8][8];	/* galaxy */
int numbases;		/* number of bases in galaxy */
int numkling;		/* number of klingons in galaxy */
int begkling;		/* beginning number of klingons */
int condition;		/* current condition (GREEN,YELLOW,RED,DOCKED) */
int xquad, yquad;	/* current quadrant */
int xsect, ysect;	/* current sector */
int old_xquad, old_yquad; /* quadrant before last movement */
int old_xsect, old_ysect; /* sector before last movement */
int energy;		/* energy level */
int shields;		/* shield level */
int torps;		/* number of torps left */
int quadrant[8][8];	/* current quadrant picture (EMPTY,KLINGON,STARBASE,STAR,PLAYER) */
extern int rolines;	/* number of lines used in current readout */
int damage[8];		/* % effectiveness of devices, normal range is 0-100 */
			/* if < 0 then device is damaged beyond use */
extern char playership[]; /* image of player's ship for s.r.s */
char captain[11];	/* captain's name */
char shipname[11];	/* ship's name */
float stardate;		/* current star date */
float lastdate;		/* last star date before federation is conquered */
float begdate;		/* beginning star date */
int base_xsect, base_ysect; /* starbase sector, if one is present */
int numkmove;		/* number of klingon moves allowed */
int skill;		/* skill level */
