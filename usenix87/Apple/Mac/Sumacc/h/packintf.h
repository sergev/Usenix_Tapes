/*	packintf.h	1.0	05/25/84	*/

/*
 * Package definitions.
 *
 * Copyright (C) 1984 Apple Computer Inc.,  Rony Sebok.
 *
 * C language version (C) 1984, Stanford Univ. SUMEX project.
 * May be used but not sold without permission.
 */

/*
 * history
 * 01/28/84	Sebok	Created.
 * 04/23/84	Croft	Created C version.
 * 10/10/84	Croft	Merged mods from JTC and Rose.
 */


/*constants for Standard File package*/

#define	putDlgID  -3999     /*SFPutFile dialog template ID*/

#define	putSave  1          /*save button*/
#define	putCancel  2        /*cancel button*/
#define	putEject  5         /*eject button*/
#define	putDrive  6         /*drive button*/
#define	putName  7          /*editTExt item for file name*/

#define	getDlgID  -4000     /*SFGetFile dialog template ID*/

#define	getOpen  1          /*open button*/
#define	getCancel  3        /*cancel button*/
#define	getEject  5         /*eject button*/
#define	getDrive  6         /*drive button*/
#define	getNmList  7        /*userItem for file name list*/
#define	getScroll  8        /*userItem for scroll bar*/


/* constants for manipulation of international resources */
/* masks used for setting and testing currency format flags */

#define	currLeadingZ  128   /*set if leading zero*/
#define	currTrailingZ  64   /*set if trailing zero*/
#define	currNegSym  32      /*set if minus sign, reset if brackets for neg num*/
#define	currSymTrail  16    /*set if currency symbol trails, reset if leads*/

/* constants specifying absolute value of short date form */

#define	DMY  1              /*day,month,year*/
#define	YMD  2              /*year,month,day*/
#define	MDY  0              /*month,day,year*/

/* masks used for date element format flags */

#define	mntLdingZ  64       /*set if leading 0 for month*/
#define	dayLdingZ  32       /*set if leading zero for day*/
#define	century    128      /*set if century, reset if no century*/

/* masks used for time element format flags */

#define	hrLeadingZ  128     /*set if leading zero for hours*/
#define	minLeadingZ  64     /*set if leading zero for minutes*/
#define	secLeadingZ  32     /*set if leading zero for seconds*/

/* country codes for version numbers */

#define	verUS		0
#define	verFrance	1
#define	verBritain	2
#define	verGermany	3
#define	verItaly	4
#define	verNetherlands	5
#define	verBelgiumLux	6
#define	verSweden	7
#define	verSpain	8
#define	verDenmark	9
#define	verPortugal	10
#define	verFinland	11
#define	verNorway	12
#define	verIsrael	13
#define	verJapan	14
#define	verAustralia	15
#define	verArabia	16
#define	verFrCanada	17
#define	verSwissFr	18
#define	verSwissGr	19
#define	verGreece	20
#define	verIceland	21
#define	verMalta	22
#define	verCyprus	23
#define	verTurkey	24
#define	verYugoslavia	25

/*Standard File*/

typedef struct {
	char	good;		/* ignore if false */
	char	copy;
	OsType	ftype;		/* file type */
	short	vRefNum;	/* volume refnum */
	short	version;	/* version number */
	char	fName[64];	/* filename, pascal string */
} SFReply;

typedef struct {
	OsType	ftype[4];
	short	junk;
} SFTypeList;


/*interface for international resources*/

typedef struct {
	char	decimalPt;
	char	thousSep;
	char	listSep;
	char	currSym1;
	char	currSym2;
	char	currSym3;
	char	currFmt;
	char	dateOrder;	/* DMY, YMD, or MDY */
	char	shrtDateFmt;	/* date format */
	char	dateSep;
	char	timeCycle;
	char	timeFmt;
	char	mornStr[4];
	char	eveStr[4];
	char	timeSep;
	char	timeSuff[8];
	char	metricSys;
	short	intl0Vers;
} intl0Rec;

/* date formats */
#define	shortDate	0
#define	longDate	1
#define	abbrevDate	2

typedef struct {
	char	days[7][16];		/* pascal strings for Sun - Sat */
	char	months[12][16];
	char	suppressDay;
	char	lngDateFmt;
	char	dayLeading0;
	char	abbrLen;
	char	st[5][4];
	short	intl1Vers;
	short	localRtn;	/* local magnitude compare routine
				   starts here (0x4e75 for rts) */
} intl1Rec;

Handle	IUGetIntl();
