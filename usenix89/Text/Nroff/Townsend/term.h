#ifndef	INCH
#define	INCH	240
#endif

struct t {
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
	char *iton;
	char *itoff;
	char *ploton;
	char *plotoff;
	char *up;
	char *down;
	char *right;
	char *left;
	char *codetab[256-32];
	char *zzz;
	};

struct t_stor {		/* This structure will be stored in the tab file */
	int bset;
	int breset;
	int Hor;
	int Vert;
	int Newline;
	int Char;
	int Em;
	int Halfline;
	int Adj;
	int twinit;
	int twrest;
	int twnl;
	int hlr;
	int hlf;
	int flr;
	int bdon;
	int bdoff;
	int iton;
	int itoff;
	int ploton;
	int plotoff;
	int up;
	int down;
	int right;
	int left;
	int codetab[256-32];
	int zzz;
};
