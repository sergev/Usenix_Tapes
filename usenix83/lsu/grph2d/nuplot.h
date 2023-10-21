#define 	H	700
#define		W	470
#define		CHARH	9
#define		CHARW	8
#define		NPW	3	/* Number of plot windows */
#define		YSIZE	792/NPW	/* dots per plot/number plot windows */
#define		XSIZE	134	/* 792/6+2 : dots per line/bits per char + 2 */
#define		XROTE	20	/* Set rotation about x-axis */
#define		YROTE	-15	/* Set rotation about y-axis */
#define		ZROTE	0	/* Set rotation about z-axis */
#define PERSP		2	/* Set perspective  (%)       */
#define DEPTH		2.25	/* Set depth parameter 	*/
				/* Greater depth = less perspective */
#define VIEWY		0.5	/* Set view height (0-1)	*/
#define VIEWX		0.5	/* Set view lateral (0-1)	*/
#define OFFSET		45	/* Set offset angle (degrees) */
#define XMARGIN		85
#define YMARGIN		200
#define	LINE		0
#define	DOT		1
#define	DASH		2
#define	DITHER		3
#define	DPERCENT	20
#define VYSIZE		250.	/* Yes, routines expect (and need) a      */
				/* floating point value here and below !  */
#define VXSIZE		500.
#define VZSIZE		750.
#define ZSIZE		1000.
#define ZMARGIN		VZSIZE/2
#define LPSCALE		0.83333333	/* Device dependent !! */
#define TICKLEN		10.
#define XYP		0	/* x-y plane */
#define XZP		1	/* x-z plane */
#define YZP		2	/* y-z plane */
#define VERT		0
#define HORZ		1
#define F		ZSIZE
#define D		ZSIZE*PERSP
#define	S		VXSIZE/2
#define LMGN		3*CHARW
#define TMGN		CHARH
