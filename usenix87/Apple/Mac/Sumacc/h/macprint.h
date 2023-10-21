/*	macprint.h	1.0	84/06/28	*/

/*
 * Print interface.
 */

/*
 * history
 * 06/28/84	Schuster	Created.
 */

#define iPrRelease 2		/* code version */
#define iPrPgFract 120		/* page scale factor */

#define iPrPgFst 1		/* page range constants */
#define iPrPgMax 9999

/* spare integers in print record */
#define iPrintX 19

typedef struct {
	short iDev;		/* font manager device code */
	short iVRes;		/* v resolution of device */
	short iHRes;		/* h resolution of device */
	Rect rPage;		/* the printable page rectangle */
} TPrInfo;

typedef TPrInfo * TPPrInfo;

typedef struct {
	short wDev;		/* the device driver number */
	short iPageV;		/* page size in units of 1/iPrPgFract */
	short iPageH;		
	char bPort;		/* io port number */
	char feed;		/* paper feeder types */
} TPrStl;

typedef TPrStl * TPPrStl;

typedef struct {
	short iRowBytes;	/* band's rowbytes */
	short iBandV;		/* size of band, in device coords */
	short iBandH;
	short iDevBytes;	/* size for allocation */
	short iBands;		/* number of bands per page */
	char bPatScale;		/* pattern scaling */
	char bULThick;		/* 3 underscoring params */
	char bULOffset;
	char bULShadow;
	char scan;		/* band scan direction */
	char bXInfoX;		/* extra byte */
} TPrXInfo;

typedef TPrXInfo * TPPrXInfo;

/* band scan direction */
#define scanTB 0
#define scanBT 1
#define scanLR 2
#define scanRL 3

typedef struct {
	short iFstPage;		/* page range */
	short iLstPage;	
	short iCopies;		/* number of copies */
	char bJDocLoop;		/* doc style: draft, spool, ... */
	char fFromUsr;		/* printing from application */
	ProcPtr pIdleProc;	/* call while io waiting */
	char *pFileName;	/* spool file name */
	short iFileVol;		/* spool file volumn */
	char bFileVers;		/* spool file version */
	char bJobX;		/* and extra byte */
} TPrJob;

typedef TPrJob * TPPrJob;

/* document styles */
#define bDraftLoop 0
#define bSpoolLoop 1
#define bUser1Loop 2
#define bUser2Loop 3

/* paper feeder types */
#define feedCut 0
#define feedFanFold 1
#define feedMechCut 2
#define feedOther 3

typedef struct {
	short iPrVersion;	/* printing software version */
	TPrInfo PrInfo;		/* print info data */
	Rect rPaper;		/* the paper rectangle */
	TPrStl PrStl;		/* print request's style */
	TPrInfo PrInfoPT;	/* print time imaging metrics */
	TPrXInfo PrXInfo;	/* print time print info record */
	TPrJob PrJob;		/* print job request */
	char PrintX[iPrintX * 2];	/* filler to 120 bytes */
} TPrint;

typedef TPrint * TPPrint;
typedef TPPrint * THPrint;

typedef struct {
	GrafPort GPort;			/* printer graph port */
	QDProcs GProcs;			/* and procs */
	long lGParam1;			/* some parameters */
	long lGParam2;
	long lGParam3;
	long lGParam4;
	char fOurPtr;
	char fOurBits;
} TPrPort ;

typedef TPrPort * TPPrPort;

typedef struct {
	short iTotPages;		/* total pages */
	short iCurPage;			/* current page number */
	short iTotCopies;		/* total copies requested */
	short iCurCopy;			/* current copy number */
	short iTotBands;		/* total bands per page */
	short iCurBand;			/* current band number */
	char fPgDirty;			/* true if current page written */
	char fImaging;			/* set while in drawpic call */
	THPrint hPrint;			/* handle to active print record */
	TPPrPort pPrPort;		/* pointer to active PrPort */
	PicHandle hPic;			/* handle to active Picture */
} TPrStatus;

typedef TPrStatus * TPPrStatus;

typedef struct {
	DialogRecord Dlg;		/* dialog window */
	ProcPtr pFltrProc;		/* filter proc */
	ProcPtr pItemProc;		/* item evaluating proc */
	THPrint hPrintUsr;		/* user's print record */
	char fDoIt;			
	char fDone;
	long lUser1;			/* global data */
	long lUser2;
	long lUser3;
	long lUser4;
	/* and some more stuff */
} TPrDlg;

typedef TPrDlg * TPPrDlg;

TPPrDlg PrStlInit();
TPPrDlg PrJobInit();
TPPrPort PrOpenDoc();
