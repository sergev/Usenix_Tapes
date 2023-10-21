/*
 *	RT11 EMULATOR
 *	resident monitor fixed offsets
 *
 *	Daniel R. Strick
 *	July 21, 1981
 */

/*
 *  These definitions give the (byte!) offsets of interesting
 *  locations within the first block of the RT11 monitor.
 *  Programs that look at the most interesting locations are
 *  certain not to work correctly when run under the emulator.
 */

#define	R_INTVEC	0000	/* link to interrupt entry code */
#define	R_IOCHAN	0002	/* i/o channel table */
#define	R_SYSCHAN	0244	/* system i/o channel */
#define	R_BLKEY		0256	/* resident directory segment number */
#define	R_CHKEY		0260	/* resident directory device number */
#define	R_DATE		0262	/* current date */
#define	R_DFLG		0264	/* directory operation in progress flag */
#define	R_USRLC		0266	/* normal location of USR */
#define	R_QCOMP		0270	/* address of I/O completion manager */
#define	R_SPUSR		0272	/* MT/CT USR function failed flag */
#define	R_SYUNIT	0274	/* [system device unit number] << 8 */
#define	R_SYSVER	0276	/* monitor version number */
#define	R_SYSUPD	0277	/* version update number */
#define	R_CONFIG	0300	/* system configuration word */
#define	R_SCROLL	0302	/* address of VT11 scroller */
#define	R_TTKS		0304	/* address of console keyboard status */
#define	R_TTKB		0306	/* address of console keyboard buffer */
#define	R_TTPS		0310	/* address of console printer status */
#define	R_TTPB		0312	/* address of console printer buffer */
#define	R_MAXBLK	0314	/* maximum output file size */
#define	R_E16LST	0316	/* offset of EMT dispatch table */
#define	R_DATIME	0320	/* time of day (S/J monitor) */
#define	R_SYNCH		0324	/* address of synch routine */
#define	R_LOWMAP	0326	/* low memory protection map */
#define	R_USRLOC	0352	/* pointer to current user entry */
#define	R_GTVECT	0354	/* pointer to vt11 interrupt vector */
#define	R_ERRCNT	0356	/* system program error count */
#define	R_FUTURE	0357	/* this space intentionally left blank */
#define	R_ENTAB		0400	/* end of resident monitor table */

/*
 *  These offsets are for things belonging to the RT11 emulator
 *  that must be in the PDP11 address space.  They are not found
 *  in a real RT11 system.
 */
#define	R_FAKEREG	0400	/* harmless location for device registers */
#define	R_FAKESUB	0410	/* a fatal subroutine */
#define	R_COMPRET	0412	/* a short completion routine */
#define	R_TOTSIZ	0420	/* includes the 8 extra words */

word	*rmon;		/* address of fixed offset area */
