# define cmove(x,y) rmove((float)(x * charw),(float)(y * charh))
# define STRLEN 256
# define TICS 03
# define TIC 0
# define TIC1 01
# define TIC2 02
# define TIC3 03
# define LABS 014
# define LAB 0
# define LAB1 04
# define LAB2 010
# define LAB3 014
# define NAME 020
# define GRID 0140
# define GRID0 0
# define GRID1 040
# define GRID2 0100
# define GRID3 0140
# define MINSET 010000
# define MAXSET 020000
# define MINMAX 040000
# define DATATYPE 01600
# define FLOAT 0
# define DOUBLE 0200
# define INTEGER 0600
# define LONG 01200
# define SHORT 01400
# define UNSIGNED 0400
# define NODATA 01600
# define SHIFT 02000
# define SHIFT1 04000
# define LOGAXIS 0100000
# define X 0
# define Y 1

double lincon[9];
# define LOG10 2.30258509299404568402
# define INFINITY  1.e+35
# define EPSILON 1.e-14

/*
 *	union used for handleing various types of data passed to graph routine
 */
union ptr
  {
	double *dp;
	float *fp;
	int *ip;
	long *lp;
	short *hp;
	unsigned *up;
	char *cp;
  };
struct
  {
	double dv;
  };

/*
 *	scanp points to current char in string passed to graph
 */
char *scanp;
/*
 *	strbeg points to beginning of string before it is alterted to aid in
 *	error printout
 */
char *strbeg;


/*
 *	One of these structures are created for each array of data passed 
 *	to graph routine.
 */
struct _data
  {
	double *dptr; /* the data */
	char symble; /* the data symble */
	int npts; /* number of data points */
	int flgs;
	int blubber; /* line fatness */
	float *ddash; /* line dash */
	char *labl;
  };

struct _data *_xdata[10],*_ydata[10];
int nary[2];

double min[2];
double max[2];
double omin[2];
double omax[2];

struct axtype
  {
	int flags;
	float x,y,len;
	int ntics,mtics;
	float ticinc;
	char type;
	char *lab;
  } _ax[2];

char xlabbuf[50],ylabbuf[50];
union ptr *argp;
int ngraphs;
int ychar;
int first;
