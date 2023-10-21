/*	quickdraw.h	1.0	04/23/84	*/

/*
 * Quickdraw definitions.
 *
 * Copyright (C) 1983 Apple Computer Inc.
 *
 * C language version (C) 1984, Stanford Univ. SUMEX project.
 * May be used but not sold without permission.
 */

/*  Written by Bill Atkinson   */

/*
 * history
 * 04/23/84	Croft	Created C version.
 */

#define	srcCopy      0  /* the 16 transfer modes */
#define	srcOr        1
#define	srcXor       2
#define	srcBic       3
#define	notSrcCopy   4
#define	notSrcOr     5
#define	notSrcXor    6
#define	notSrcBic    7
#define	patCopy      8
#define	patOr        9
#define	patXor       10
#define	patBic       11
#define	notPatCopy   12
#define	notPatOr     13
#define	notPatXor    14
#define	notPatBic    15

/* QuickDraw color separation constants */

#define	normalBit    0       /* normal screen mapping   */
#define	inverseBit   1       /* inverse screen mapping  */
#define	redBit       4       /* RGB additive mapping */
#define	greenBit     3
#define	blueBit      2
#define	cyanBit      8       /* CMYBk subtractive mapping */
#define	magentaBit   7
#define	yellowBit    6
#define	blackBit     5

#define	blackColor   33      /* colors expressed in these mappings */
#define	whiteColor   30
#define	redColor     205
#define	greenColor   341
#define	blueColor    409
#define	cyanColor    273
#define	magentaColor 137
#define	yellowColor  69

#define	picLParen    0       /* standard picture comments */
#define	picRParen    1


typedef	char *	Ptr;		/* blind pointer */
typedef	char **	Handle;		/* blind handle */

typedef	int	(*ProcPtr)();	/* pointer to a procedure */

typedef struct {
	char	s[256];
} Str255;

typedef struct {
	char	s[8];
} Pattern;

typedef struct {
	short	s[16];
} Bits16;

typedef	short	GrafVerb;

#define	frameMode	0
#define	paintMode	1
#define	eraseMode	2
#define	invertMode	3
#define	fillMode	4

typedef	short	Style;

#define	boldStyle	1
#define	italicStyle	2
#define	underlineStyle	4
#define	outlineStyle	8
#define	shadowStyle	16
#define	condenseStyle	32
#define	extendStyle	64

typedef struct {
	short	ascent;
	short	descent;
	short	widMax;
	short	leading;
} FontInfo;

typedef	int	VHSelect;

#define	V	0
#define	H	1

typedef union {
	struct {
		short	P0v;
		short	P0h;
	} P0;
	short	vh[2];
} Point;

#define	v	P0.P0v
#define	h	P0.P0h

typedef union {
	struct {
		short	R0top;
		short	R0left;
		short	R0bottom;
		short	R0right;
	} R0;
	struct {
		Point	R1topLeft;
		Point	R1botRight;
	} R1;
} Rect;

#define	top	R0.R0top
#define	left	R0.R0left
#define	bottom	R0.R0bottom
#define	right	R0.R0right
#define	topLeft	R1.R1topLeft
#define	botRight R1.R1botRight

typedef struct {
	Ptr	baseAddr;
	short	rowBytes;
	Rect	bounds;
} BitMap;

typedef struct {
	Bits16	data;
	Bits16	mask;
	Point	hotSpot;
} Cursor;

typedef struct {
	Point	pnLoc;
	Point	pnSize;
	short	pnMode;
	Pattern	pnPat;
} PenState;

typedef struct {
	short	polySize;
	Rect	polyBBox;
	Point	polyPoints[1];
} Polygon;

typedef	Polygon * PolyPtr;
typedef	PolyPtr * PolyHandle;

typedef struct {
	short	rgnSize;	/* = 10 for rectangular */
	Rect	rgnBBox;
				/* plus more data if not rectangular */
} Region;

typedef	Region * RgnPtr;
typedef	RgnPtr * RgnHandle;

typedef struct {
	short	picSize;
	Rect	picFrame;
				/* plus byte codes for picture content */
} Picture;

typedef Picture * PicPtr;
typedef PicPtr * PicHandle;

typedef struct {
	int	(*textProc)();
	int	(*lineProc)();
	int	(*rectProc)();
	int	(*rRectProc)();
	int	(*ovalProc)();
	int	(*arcProc)();
	int	(*polyProc)();
	int	(*rgnProc)();
	int	(*bitsProc)();
	int	(*commentProc)();
	int	(*txMeasProc)();
	int	(*getPicProc)();
	int	(*putPicProc)();
} QDProcs;

typedef QDProcs * QDProcsPtr;

typedef struct {
	short	device;
	BitMap	portBits;
	Rect	portRect;
	RgnHandle visRgn;
	RgnHandle clipRgn;
	Pattern	bkPat;
	Pattern	fillPat;
	Point	pnLoc;
	Point	pnSize;
	short	pnMode;
	Pattern	pnPat;
	short	pnVis;
	short	txFont;
	Style	txFace;
	short	txMode;
	short	txSize;
	long	spExtra;
	long	fgColor;
	long	bkColor;
	short	colrBit;
	short	patStretch;
	Handle	picSave;
	Handle	rgnSave;
	Handle	polySave;
	QDProcsPtr grafProcs;
} GrafPort;

typedef GrafPort * GrafPtr;

typedef struct {
	short	errNum;
	Handle	fontHandle;
	char	bold;
	char	italic;
	char	ulOffset;
	char	ulShadow;
	char	ulThick;
	char	shadow;
	char	extra;
	char	ascent;
	char	descent;
	char	widMax;
	char	leading;
	char	unused;
	Point	numer;
	Point	denom;
} FMOutRec;

typedef FMOutRec * FMOutPtr;

struct QDVar {
	FMOutRec fontData;	/* font manager output record */
	FMOutPtr fontPtr;	/* last font used; used by DrawText */
	long	fontAdj;	/* fixed point to adjust for font subst */
	Point	patAlign;	/* to align pattern during DrawPicture */
	short	polyMax;	/* max bytes allocated so far to thePoly */
	PolyHandle thePoly;	/* the current polygon being defined */
	short	playIndex;	/* used by StdGetPic */
	PicHandle playPic;
	short	rgnMax;		/* max bytes allocated so far to rgnBuf */
	short	rgnIndex;	/* current bytes used in rgnBuf */
	Handle	rgnBuf;		/* buffer for OpenRgn */
	Region	wideData;	/* a dummy rectangular region, readonly */
	RgnPtr	wideMaster;
	RgnHandle wideOpen;
	long	randSeed;	/* random number generator seed */
	BitMap	screenBits;	/* mac screen */
	Cursor	arrow;		/* pointing arrow cursor */
	Pattern	dkGray;		/* standard grey patterns */
	Pattern	ltGray;
	Pattern	gray;
	Pattern	black;
	Pattern	white;
	GrafPtr	QDthePort;	/* current pointer to grafPort */
};

struct QDVar *QD;		/* master global area pointer */
#define	thePort	(QD->QDthePort)	/* shorthand, use "QD->mumble" for others */


/* C to Pascal string conversion utilities. */

char	*c2pstr();		/* convert C string to Pascal string */
char	*p2cstr();		/* convert Pascal string into C string */
char	*isapstr();		/* "declares" argument is a pascal string */


/* Functions returning other than integer */

PolyHandle OpenPoly();
RgnHandle NewRgn();
PicHandle OpenPicture();
