/*
 *	RT11 EMULATOR
 *	system call codes
 *
 *	Daniel R. Strick
 *	May 3, 1979
 */


/*
 * EMTs with arguments in r0 and stack
 */
#define	TTYIN	0340
#define	TTYOUT	0341
#define	DSTATUS	0342
#define	FETCH	0343
#define	CSIGEN	0344
#define	CSISPC	0345
#define	LOCK	0346
#define	UNLOCK	0347
#define	EXIT	0350
#define	PRINT	0351
#define	SRESET	0352
#define	QSET	0353
#define	SETTOP	0354
#define	RCTRLO	0355
#define	EMT356	0356
#define	HRESET	0357
#define	EMT374	0374
#define	EMT375	0375

/*
 * EMT374 function codes
 */
#define	S_WAIT	000
#define	S_SPND	001
#define	S_RSUM	002
#define	S_PURGE	003
#define	S_SERR	004
#define	S_HERR	005
#define	S_CLOSE	006
#define	S_TLOCK	007
#define	S_CHAIN	010
#define	S_MWAIT	011
#define	S_DATE	012

/*
 * EMT375 function codes
 */
#define	S_DELETE	000
#define	S_LOOKUP	001
#define	S_ENTER		002
#define	S_TRPSET	003
#define	S_RENAME	004
#define	S_SAVSTS	005
#define	S_REOPEN	006
#define	S_007		007
#define	S_READ		010
#define	S_WRITE		011
#define	S_012		012
#define	S_CHCOPY	013
#define	S_DEVICE	014
#define	S_CDFN		015
#define	S_016		016
#define	S_017		017
#define	S_GTJB		020
#define	S_GTIM		021
#define	S_MRKT		022
#define	S_CMKT		023
#define	S_TWAIT		024
#define	S_SDAT		025
#define	S_RCVD		026
#define	S_CSTAT		027
#define	S_SFPA		030
#define	S_PROTECT	031
#define	S_SPFUN		032
#define	S_CNTXSW	033
#define	S_GVAL		034

/*
 * RT11 version 1 EMTs
 */
#define	V1_DELT	0000
#define	V1_LOOK	0020
#define	V1_ENTR	0040
#define	V1_60	0060
#define	V1_RENM	0100
#define	V1_SAVS	0120
#define	V1_REOP	0140
#define	V1_CLOS	0160
#define	V1_READ	0200
#define	V1_WRIT	0220
#define	V1_WAIT	0240
#define	V1_END	0260
