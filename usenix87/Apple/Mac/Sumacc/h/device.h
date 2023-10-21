/*	device.h	1.0	07/21/84	*/

/*
 * Device driver definitions.
 */

/* 
 * Copyright (C) 1984, Stanford Univ. SUMEX project.  
 * May be used but not sold without permission.
 */

/*
 * history
 * 07/21/84	Croft	Created.
 * 07/27/84	Schuster Added accessory status codes.
 */

/*
 * Driver structure.  
 */
struct drvr {
	short	drvrFlags;	/* flags */
	short	drvrDelay;	/* ticks between actions */
	short	drvrEMask;	/* desk accessory event mask */
	short	drvrMenu;	/* menu ID */
	short	drvrOpen;	/* offset to open routine */
	short	drvrPrime;	/* ..		prime */
	short	drvrCtl;	/* ..		control */
	short	drvrStatus;	/* ..		status */
	short	drvrClose;	/* ..		close */
	char	drvrName[32];	/* driver name (pascal string) */
};

struct drvr drvr;		/* global instance of my drvr struct */

/* flags in drvrFlags */
#define	dReadEnable	0x100
#define	dWritEnable	0x200
#define	dCtlEnable	0x400
#define	dStatEnable	0x800
#define	dNeedGoodBye	0x1000
#define	dNeedTime	0x2000
#define	dNeedLock	0x4000
#define	dOpened		0x20
#define	dRAMBased	0x40
#define	drvrActive	0x80

/*
 * Commands in (low byte of) ioTrap.
 */
#define	aRdCmd	2
#define	aWrCmd	3
#define	aCtlCmd	4
#define	aStsCmd	5

/*
 * Special return codes from driver entry points.
 *
 * A zero or negative return from the driver routines causes an ioDone,
 * with the return status in D0.  The return values below (large 
 * positive numbers) cause the indicated action.
 */
#define	ioRTS 0x40000000
#define	ioKill	ioRTS
#define	ioDone	0

/*
 * Control and status codes.
 */
#define	killCode	1
#define	ejectCode	7
#define	drvStsCode	8

/*
 * IO System Errors (should be in osintf.h)
 */
#define	controlErr	-17
#define	statusErr	-18
#define	readErr		-19
#define	writErr		-20
#define	badUnitErr	-21
#define	unitEmptyErr	-22
#define	openErr		-23
#define	closErr		-24
#define	dRemovErr	-25
#define	dInstErr	-26
#define	abortErr	-27
#define	notOpenErr	-28


/*
 * Control and Status codes for desk accessories.
 */
#define accEvent 64
#define accRun 65
#define accCursor 66
#define accMenu 67
#define accUndo 68
#define accCut 70
#define accCopy 71
#define accPaste 72
#define accClear 73
