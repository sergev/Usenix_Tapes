/*	osintf.h	1.0	05/06/84	*/

/*
 * OS interface.
 *
 * Copyright 1983 by Apple Computer, Inc.  All Rights Reserved.
 *
 * C language version Copyright 1984, Stanford Univ. SUMEX project.
 * May be used but not sold without permission.
 *
 * history
 * 11/23/83	Sebok	Merge of heapzone.text and osmisc
 * 05/06/84	Croft	C language version.
 * 09/29/84	Croft	merged July/Oct supplement changes.
 */


/*for Event Manager*/

#define	everyEvent  	-1
#define	nullEvent   	0
#define	mouseDown   	1
#define	mouseUp     	2
#define	keyDown     	3
#define	keyUp       	4
#define	autoKey     	5
#define	updateEvt   	6
#define	diskEvt     	7
#define	activateEvt 	8
#define	abortEvt    	9
#define	networkEvt  	10
#define	driverEvt   	11
#define	app1Evt     	12
#define	app2Evt     	13
#define	app3Evt     	14
#define	app4Evt     	15

#define	DoubleTime	(*(long *)0x2f0)
#define	CaretTime	(*(long *)0x2f4)

/* event mask equates */
#define	nullMask      	1
#define	mDownMask     	2
#define	mUpMask       	4
#define	keyDownMask   	8
#define	keyUpMask     	16
#define	autoKeyMask   	32
#define	updateMask    	64
#define	diskMask      	128
#define	activMask     	256
#define	abortMask     	512
#define	networkMask   	1024
#define	driverMask    	2048
#define	app1Mask      	4096
#define	app2Mask      	8192
#define	app3Mask      	16384
#define	app4Mask      	0x8000

/* to decipher event message for keyDown events */
#define	charCodeMask	0xFF
#define	keyCodeMask	0xFF00

/* modifiers */
#define	optionKey	2048	/* Bit 3 of high byte */
#define	alphaLock	1024	/* Bit 2 */
#define	shiftKey	512	/* Bit 1 */
#define	cmdKey		256	/* Bit 0 */
#define	btnState	128	/* Bit 7 of low byte is mouse button state */

#define	activeFlag	1	/* bit 0 of modifiers for activate event */
#define	changeFlag	2	/* bit 1 of modifiers for activate event */

/* for PostEvent */

#define	evtNotEnb	1

/* for Memory Manager */

#define	memFullErr   -108  /* Not enough room in heap zone */
#define	nilHandleErr -109  /* Master Pointer was NIL in HandleZone or other   */
#define	memWZErr     -111  /* WhichZone failed (applied to free block)        */
#define	memPurErr    -112  /* trying to purge a locked or non-purgeable block */
#define	noErr        0     /* All is well */

/* file system error codes */

#define	dirFulErr  -33     /* Directory full*/
#define	dskFulErr  -34     /* disk full*/
#define	nsvErr     -35     /* no such volume*/
#define	ioErr      -36     /* I/O error (bummers)*/
#define	bdNamErr   -37     /* there may be no bad names in the final system!*/
#define	fNOpnErr   -38     /* File not open*/
#define	eofErr     -39     /* End of file*/
#define	posErr     -40     /* tried to position to before start of file (r/w)*/
#define	mFulErr    -41     /* memory full(open) or file won't fit (load)*/
#define	tmfoErr    -42     /* too many files open*/
#define	fnfErr     -43     /* File not found*/

#define	wPrErr     -44     /* diskette is write protected*/
#define	fLckdErr   -45     /* file is locked*/
#define	vLckdErr   -46     /* volume is locked*/
#define	fBsyErr    -47     /* File is busy (delete)*/
#define	dupFNErr   -48     /* duplicate filename (rename)*/
#define	opWrErr    -49     /* file already open with with write permission*/
#define	paramErr   -50     /* error in user parameter list*/
#define	refNumErr  -51     /* refnum error*/
#define	gfpErr     -52     /* get file position error*/
#define	volOffLinErr -53   /* volume not on line error (was Ejected)*/
#define	permErr    -54     /* permissions error (on file open)*/
#define	volOnLinErr -55    /* drive volume already on-line at MountVol*/
#define	nsDrvErr   -56     /* no such drive (tried to mount a bad drive num)*/
#define	noMacDskErr -57    /* not a mac diskette (sig bytes are wrong)*/
#define	extFSErr   -58     /* volume in question belongs to an external fs*/
#define	fsRnErr    -59     /* file system rename error:*/
                           /*  during rename the old entry was deleted */
                           /*  but could not be restored . . .*/
#define	badMDBErr  -60     /* bad master directory block*/
#define	wrPermErr  -61     /* write permissions error*/
#define	firstDskErr -84    /*first of the range of low-level disk errors*/
#define	lastDskErr -64     /*last of the range of low-level disk errors*/


#define	maxSize  0x800000  /* Max data block size is 512K bytes  */

/* finder constants */

#define	fHasBundle 8192		/* was 32 */
#define	fInvisible 16384	/* was 64 */
#define	fTrash     -3
#define	fDesktop   -2
#define	fDisk      0

/* io constants */

/* ioPosMode values */
#define fsAtMark	0
#define fsFromStart	1
#define fsFromLEOF	2
#define fsFromMark	3

/* ioPermission values */
#define fsCurPerm	0
#define fsRdPerm	1
#define fsWrPerm	2
#define fsRdWrPerm	3

/* refNums for the serial ports */
#define aInRefNum	-6       /* serial port A input */
#define aOutRefNum	-7       /* serial port A output */
#define bInRefNum	-8       /* serial port B input */
#define bOutRefNum	-9       /* serial port B output */

/* baud rate constants */
#define baud300		380
#define baud600		189
#define baud1200	94
#define baud1800	62
#define baud2400	46
#define baud3600	30
#define baud4800	22
#define baud7200	14
#define baud9600	10
#define baud19200	4
#define baud57600	0

/* SCC channel configuration word */
/* driver reset information masks */
#define stop10		16384
#define stop15		-32768
#define stop20		-16384
#define noParity	8192
#define oddParity	4096
#define evenParity	12288
#define data5		0
#define data6		2048
#define data7		1024
#define data8		3072


/* serial driver error masks */
#define swOverrunErr	0
#define parityErr	16
#define hwOverrunErr	32
#define framingErr	64

/* serial driver message constant */
#define xOffWasSent	0x80

/* for application parameter */
/* constants for message returned by the finder on launch */
#define appOpen		0
#define appPrint	1

/* for sound driver */
#define swMode	-1
#define ftMode	1
#define ffMode	0

/* for disk driver */
#define	currPos	0
#define	absPos	1
#define	relPos	3
#define	rdVerify 64


/* Event Manager */

typedef	struct {		/* EventRecord */
	short	what;
	long	message;
	long	when;
	Point	where;
	short	modifiers;
} EventRecord;


typedef struct {		/* Zone */
	Ptr	bkLim;
	Ptr	purgePtr;
	Ptr	hFstFree;
	long	zCBFree;
	ProcPtr	gZProc;
	short	moreMast;
	short	flags;
	short	cntRel;
	short	maxRel;
	short	cntNRel;
	short	maxNRel;
	short	cntEmpty;
	short	cntHandles;
	long	minCBFree;
	ProcPtr	purgeProc;
	Ptr	sparePtr;
	Ptr	allocPtr;
	short	heapData;
} Zone;

typedef	Zone * THz;


/* general queue data structure */

struct QElem {			/* QElem */
	struct QElem *qLink;
	short	qType;
	char	qData[1];
};
/* qType values */
#define	vType	1		/* VBL queue */
#define	ioQType	2		/* IO queue */
#define	drvType	3		/* driver queue */
#define	evType	4		/* event queue */
#define	fsQType	5		/* file system queue */

typedef struct QElem QElem;
	
typedef struct {		/* QHdr */
	short	qFlags;
	QElem	*qHead;
	QElem	*qTail;
} QHdr;
/* qFlags values */
#define	qInUse	0x80


/* Vertical Blanking Control Block Queue Element */

struct VBLTask {		/* VBLTask */
	struct	VBLTask *qLink;
	short	qType;
	ProcPtr	vblAddr;	/* service routine */
	short	vblCount;	/* for timeout */
	short	vblPhase;	/* phase for synchronization */
};

typedef struct VBLTask VBLTask;
#define	VBLQHdr	((QHdr *) 0x160)


/* Event queue element */

struct EvQEl {
	struct	EvQEl *qLink;
	short	qType;
	short	evtQWhat;	/* remainder identical to EventRecord */
	long	evtQMessage;
	long	evtQWhen;
	Point	evtQWhere;
	short	evtQModifiers;
};

typedef	struct	EvQEl EvQEl;
#define	EvQHdr	((QHdr *) 0x14a)


/* Driver queue element */

struct DrvQEl {
	struct	DrvQEl *qLink;
	short	qType;
	short	dQDrive;
	short	dQRefNum;	/* ref num of drvr to handle this drive */
	short	dQFSID;		/* file system id */
	short	dQDrvSize;	/* size of drive in 512 blocks (but not 1&2) */
};

typedef	struct	DrvQEl DrvQEl;
#define	DrvQHdr	((QHdr *) 0x308)
#define	FSQHdr	((QHdr *) 0x360)
#define	VCBQHdr	((QHdr *) 0x356)


/* IO queue elements */

typedef	struct {		/* ResType / OsType */
	char	s[4];
} ResType;

typedef ResType OsType;

typedef struct {		/* FInfo, finder info */
	OsType	fdType;		/* type of the file */
	OsType	fdCreator;
	short	fdFlags;	/* has bundle,invisible,locked, etc. */
	Point	fdLocation;
	short	fdFldr;		/* folder */
} FInfo;

struct	IOParam {		/* general IO parameter block */
	struct IOParam	*qLink;
	short	qType;		/* = IOQType */
	short	ioTrap;		/* trap code */
	Ptr	ioCmdAddr;	/* address to dispatch to */
	ProcPtr	ioCompletion;	/* completion routine */
	short	ioResult;
	char	*ioNamePtr;	/* vol:filename string */
	short	ioVRefNum;	/* volume refnum (or drvnum) */

	short	ioRefNum;	/* refnum for IO operation */
	char	ioVersNum;	/* version */
	char	ioPermssn;	/* permissions */
	Ptr	ioMisc;		/* misc parameter */
				/* Rename: new name */
                                /* GetEOF,SetEOF: logical end of file*/
                                /* Open: optional ptr to buffer*/
                                /* SetFileType: new type*/
	Ptr	ioBuffer;	/* data buffer */
	long	ioReqCount;	/* requested byte count */
	long	ioActCount;	/* actual byte count on completion */
	short	ioPosMode;	/* initial file positioning */
	long	ioPosOffset;	/* file position offset */
};

typedef struct IOParam IOParam;

struct	FileParam {		/* get/set fileinfo parameter block */
	struct FileParam *qLink;
	short	qType;		/* = IOQType */
	short	ioTrap;		/* trap code */
	Ptr	ioCmdAddr;	/* address to dispatch to */
	ProcPtr	ioCompletion;	/* completion routine */
	short	ioResult;
	char	*ioNamePtr;	/* vol:filename string */
	short	ioVRefNum;	/* volume refnum (or drvnum) */

	short	ioFRefNum;	/* refnum */
	char	ioFVersNum;
	char	filler1;
	short	ioFDirIndex;	/* GetFileInfo directory index */
	char	ioFlAttrib;	/* in-use bin=7 lock bit=0 */
	char	ioFlVersNum;	/* file version number */
	FInfo	ioFlFndrInfo;	/* finder info */
	long	ioFlNum;	/* file number */
	short	ioFlStBlk;	/* start block */
	long	ioFlLgLen;	/* logical length */
	long	ioFlPyLen;	/* physical length */
	short	ioFlRStBlk;	/* resource start block */
	long	ioFlRLgLen;	/* logical length */
	long	ioFlRPyLen;	/* physical length */
	long	ioFlCrDat;	/* creation date/time */
	long	ioFlMdDat;	/* modified date/time */
};

typedef struct FileParam FileParam;

struct	VolumeParam {		/* get/set volumeinfo parameter block */
	struct VolumeParam *qLink;
	short	qType;		/* = IOQType */
	short	ioTrap;		/* trap code */
	Ptr	ioCmdAddr;	/* address to dispatch to */
	ProcPtr	ioCompletion;	/* completion routine */
	short	ioResult;
	char	*ioNamePtr;	/* vol:filename string */
	short	ioVRefNum;	/* volume refnum (or drvnum) */

	long	filler2;
	short	ioVolIndex;	/* volume index number */
	long	ioVCrDate;	/* creation date/time */
	long	ioVLsBkUp;	/* last backup date/time */
	short	ioVAtrb;	/* volume attribute */
	short	ioVNmFls;	/* number of files */
	short	ioVDirSt;	/* start block of directory */
	short	ioVBlLn;	/* length of dir in blocks */
	short	ioVNmAlBlks;	/* number of blocks of alloc size */
	long	ioVAlBLkSiz;	/* alloc blk byte size */
	long	ioVClpSiz;	/* bytes to allocate at a time */
	short	ioAlBlSt;	/* start block (512-byte) in block map */
	long	ioVNxtFNum;	/* next free file number */
	short	ioVFrBlk;	/* # free alloc blocks on this volume */
};

typedef struct VolumeParam VolumeParam;

/* parameters for control (CC) and status (SC) calls*/
/* maximum variant size is 22 bytes*/

typedef union {			/* CSParam */
	/* maximum variant size */
	char	csChar[22];

	/* arbitrary parameter sizes */
	short	csShort;
	long	csLong;

	/*for sound generator*/
	short	sndVal;

	/*for serial driver*/
	short	serConfig;		/* CC=8, stopbits, etc.  */
	struct {			/* CC=9 */
		Ptr	serBPtr;	/* new input buffer */
		short	serBLen;	/* buffer length */
	} serInBuff;
	struct SerShk {			/* CC=10 */
		char	fXOn;		/* XON flow ctl enabled */
		char	fCTS;		/* CTS flow ctl enabled */
		char	xOn;		/* xon char */
		char	xOff;		/* xoff char */
		char	errs;		/* errors mask */
		char	evts;		/* event mask */
		char	fInX;		/* input xon enabled */
		char	null;	
	} serShk;
	long	serNBytes;		/* SC=2, # buffered bytes */
	struct SerStaRec {		/* SC=8, status info */
		char	cumErrs;	/* cum errors */
		char	xOffSent;	/* xoff has been sent */
		char	rdPend;		/* read pending */
		char	wrPend;		/* write pending */
		char	ctsHold;	/* CTS flow hold */
		char	xOffHold;	/* Xoff flow hold */
	} serStaRec;

	/*for printer driver */
	struct {			/* CC=4, printing bitmap */
		long	param1;		/* pnts to quickdraw bitmap */
		long	param2;		/* pnts to rect within bitmap */
		long	param3;		/* 0 for screen printing */
	} printer;
					/* CC=5 writing raw to printer*/
					/* pointer to a byte string*/
					/* count of bytes*/
					/* pointer to an idle proc (or NIL)*/
					/* CC=6 special BitMap cases*/
					/* screen or top folder code*/
	/* for font manager */
	struct {			/* CC=8, tail hook */
		Ptr	fontRecPtr;	/* font mgr input record */
		short	fontCurDev;	/* current device */
	} fontMgr;

	/*for Sony Disk Driver*/
	Ptr	dskBuff;		/* CC=8, set tag buffer */
	struct DrvSts {			/* SC=8, drive status */
		short	track;		/* current track */
		char	writeProt;	/* 0x80 if locked */
		char	diskInPlace;
		char	installed;
		char	sides;		/* 0x80 for 2 sides */
		DrvQEl	drvQEl;		/* drive queue elem */
		short	diskErrs;	/* soft err count */
	} drvSts;

	/* for io driver/desk accessory */
	Ptr	event;			/* pointer to EventRecord */
} CSParam;			/* end union of control status params */

typedef	struct SerShk SerShk;
typedef	struct SerStaRec SerStaRec;
typedef	struct DrvSts DrvSts;

struct	CntrlParam {		/* control and status parameter block */
	struct CntrlParam *qLink;
	short	qType;		/* = IOQType */
	short	ioTrap;		/* trap code */
	Ptr	ioCmdAddr;	/* address to dispatch to */
	ProcPtr	ioCompletion;	/* completion routine */
	short	ioResult;
	char	*ioNamePtr;	/* vol:filename string */
	short	ioVRefNum;	/* volume refnum (or drvnum) */

	short	ioRefNum;	/* refnum for IO operation */
	short	csCode;		/* control and status code */
	CSParam csParam;	/* control and status parameters */
};

typedef struct CntrlParam CntrlParam;


/* 20 bytes of system parameter area */

typedef struct {		/* SysParmType */
	long	valid;		/* hi byte, validation field (0xa7) */
				/* lo 3 bytes, odometer */
	short	portA;		/* port A config */
	short	portB;	
	long	alarm;		/* alarm time */
	short	font;		/* default font id */
	short	kbdPrint;	/* hi byte, kbd repeat */
                                /* high nibble = thresh in 4/60ths*/
                                /* low nibble = rates in 2/60ths*/
                                /* low byte = print stuff*/
	short	volClik;	/* low 3 bits of high byte = volume control*/
                                /* hi nib of low byte = double time in 4/60ths*/
                                /* lo nib of low byte = caret blink in 4/60ths*/
	short	misc;		/* EEEC EEEE PSKB FFHH*/
                                /* E = extra*/
                                /* P = paranoia level*/
                                /* S = mouse scaling*/
                                /* K = key click*/
                                /* B = boot disk*/
                                /* F = menu flash*/
                                /* H = help level*/
} SysParmType;

#define	SysPPtr	((SysParmType *) 0x1f8)


/* master directory block */

struct Direct {
	short	drSigWord;		/* always 0xd2d7 */
	long	drCrDate;		/* date/time of initialization */
	long	drLsBkUp;		/* date/time of last backup */
	short	drAtrb;			/* volume attributes */
#define	DR_INCONSISTENT	0x0007
#define	DR_BUSY		0x0040
#define	DR_HARDLOCK	0x0080
#define	DR_SOFTLOCK	0x8000
	short	drNmFls;		/* # of files in directory */
	short	drDirSt;		/* first logical blk of directory */
	short	drBlLn;			/* # of blocks in directory */
	short	drNmAlBlks;		/* # of allocation blks on volume */
	long	drAlBlkSiz;		/* size of allocation blocks */
	long	drClpSiz;		/* # of bytes to allocate */
	short	drAlBlSt;		/* first logical blk of alloc blk */
	long	drNxtFNum;		/* next unused file number */
	short	drFreeBlks;		/* # of unused alloc blocks */
	char	drVN;			/* length of volume name */
	char	drVName[27];		/* volume name */
};

typedef struct Direct Direct;


/* volume control block per-volume information (incore) */

struct VCB {
	struct	VCB *qLink;
	short	qType;
	short	vcbFlags;
#define	vcbDirty	0x8000
	Direct	vcbD;			/* volume info from master dir blk */
	short	vcbDrvNum;		/* drive num of mounted volume */
	short	vcbDRefNum;		/* driver ref num of mounted vol */
	short	vcbFSID;		/* != 0 => external file system */
	short	vcbVRefNum;		/* volume reference number */
	Ptr	vcbMAdr;		/* ptr to volume block map */
	Ptr	vcbBufAdr;		/* ptr to volume buffer */
	short	vcbMLen;		/* length of volume block map */
	short	vcbDirIndex;
	short	vcbDirBlk;
};

typedef struct VCB VCB;

#define	DefVCBPtr	(*(VCB**) 0x352)	/* default volume */


/* Device control entry */

typedef struct {		/* DCtlEntry */
	Ptr	dCtlDriver;	/* pointer/handle to driver */
	short	dCtlFlags;
	QHdr	dCtlQHdr;	/* driver's IO queue */
	long	dCtlPosition;	/* offset used by R/W calls */
	Handle	dCtlStorage;	/* private storage */
	short	dCtlRefNum;	/* driver's refnum */
	long	dCtlCurTicks;	/* current tick count (kept by Device Mgr) */
	Ptr	dCtlWindow;	/* window record (if any) */
	short	dCtlDelay;	/* ticks between actions */
	short	dCtlEMask;	/* event mask (desk acc) */
	short	dCtlMenu;	/* menu ID */
} DCtlEntry;


/* for Sound Driver */
/* for 4-tone sound generation */

typedef struct {
	char	s[256];
} Wave;

typedef struct {		/* FTSoundRec */
	short	duration;
	long	sound1Rate;
	long	sound1Phase;
	long	sound2Rate;
	long	sound2Phase;
	long	sound3Rate;
	long	sound3Phase;
	long	sound4Rate;
	long	sound4Phase;
	Wave	*sound1Wave;
	Wave	*sound2Wave;
	Wave	*sound3Wave;
	Wave	*sound4Wave;
} FTSoundRec;

typedef struct {		/* FTSynthRec */
	short	mode;
	FTSoundRec *sndRec;
} FTSynthRec;

typedef struct {		/* Tone */
	short	count;
	short	amplitude;
	short	duration;
} Tone;

typedef struct {		/* SWSynthRec */
	short	mode;
	Tone	triplets[5000];
} SWSynthRec;

typedef struct {		/* FFSynthRec */
	short	mode;
	long	count;
	char	waveBytes[30000];
} FFSynthRec;


/* for date and time */
/* date index */

typedef struct {		/* DateTimeRec */
	short	year;		/* (0 is) 1904,1905,... */
	short	month;		/* 1..12 */
	short	day;		/* 1..31 */
	short	hour;		/* 0..23 */
	short	minute;		/* 0..59 */
	short	second;		/* 0..59 */
	short	dayOfWeek;	/* 1..7 (sun..sat) */
} DateTimeRec;


/* application parameters */

typedef struct {		/* AppFile */
	short	vRefNum;
	OsType	fType;
	short	versNum;
	char	fName[1];
} AppFile;


/* Functions returning other than integer */

THz	GetZone(),ApplicZone(),SystemZone(),PtrZone(),HandleZone();
Ptr	TopMem(),NewPtr();
Handle	NewHandle(),RecoverHandle(),GZSaveHnd(),GetDCtlEntry();
