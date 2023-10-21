#include "tnt.h"
#include "tdef.h"

/* typewriter driving table structure*/
#ifdef NROFF
struct {
	int bset;
	int breset;
	int Hor;
	int Vert;
	int Newline;
	int Char;
	int Em;
	int Halfline;
	int Adj;
	char *twinit;
	char *twrest;
	char *twnl;
	char *hlr;
	char *hlf;
	char *flr;
	char *bdon;
	char *bdoff;
	char *ploton;
	char *plotoff;
	char *up;
	char *down;
	char *right;
	char *left;
	char *codetab[256-32];
	int zzz;
	} t;
int pipeflg;
#endif

struct {
	int op;	/* output place (tmp file) */
	int os;
	int dnl;
	int dimac;
	int ditrap;
	int ditf;
	int alss;
	int blss;
	int nls;
	int mkline;
	int maxl;
	int hnl;
	int curd;	/* current diversion pointer */
} d[NDI];

struct {
	int pn;
	int nl;
	int yr;
	int hp;
	int ct;
	int dn;
	int mo;
	int dy;
	int dw;
	int ln;
	int dl;
	int st;
	int sb;
	int cd;
	int vxx[NN-NNAMES];
} v;
int *vlist &v;
int *dip &d[0];

int level;
int stdi;
int waitf;
int nofeed;
int quiet;
int stop;
int stopval;
char ibuf[IBUFSZ];
char xbuf[IBUFSZ];
char *ibufp;
char *xbufp;
char *eibuf;
char *xeibuf;
int cbuf[NC];
int *cp;
int nx;
int mflg;
int ch;
int cps;
int suffid;
int sufind[26];
int ibf;
int ibfe;
int ttyod;
int ttys[3];
int iflg;
int ioff;
char *enda;
int rargc;
char **argp;
char trtab[256];
int lgf;
int copyf;
int ch0;
int cwidth;
int ip;
int iseek;
int nlflg;
int *nxf;
int *ap;
int *frame;
int *stk;
int donef;
int nflush;
int nchar;
int rchar;
int nfo;
int ifile;
int padc;
int raw;
int ifl[NSO];
int offl[NSO];
int ipl[NSO];
int ifi;
int flss;
int nonumb;
int trap;
int *litlev;
int tflg;
int ejf;
int *ejl;
int lit;
int gflag;
int dilev;
int tlss;
int offset;
int oseek;
int em;
int ds;
int woff;
int app;
int ndone;
int lead;
int ralss;
int paper;
int nextb;
int *argtop;
int nrbits;
int nform;
int oldmn;
int newmn;
int macerr;
int apptr;
int apseek;
int aplnk;
int diflg;
int roff;
int rseek;
int wbfi;
int inc[NN];
int fmt[NN];
int evi;
int vflag;
int noscale;
int po1;
int nlist[NTRAP];
int mlist[NTRAP];
int evlist[EVLSZ];
int ev;
int tty;
int sfont;
int sv;
int esc;
int cs;
int bd;
int widthp;
int xpts;
int xfont;
int code;
int setwdf;
int ccs;
int xbitf;
int mfont;
int mpts;
int pfont;
int ppts;
int over;
int nhyp;
int **hyp;
int *olinep;
int *pslp;
int back;
int esct;
int mcase;
int psflg;
int verm;
int escm;
int ttysave;
int dotT;
int eqflg;
char *unlkms, *unlkev;
int no_out;
int hflg;
int bug;
int nhcnt, nhzone;
int upid;	/* .U register = process id */
