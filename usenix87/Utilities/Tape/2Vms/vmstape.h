#ifndef MAXNAMLEN /* maxnamlen is defined in the new directory package, but
			not the old.  This is our way of telling if we
			are using the new package or not */
#define MAXNAMLEN 14 /* This will set maxnamlen for old dir structs*/
#define OLD_DIR_STRUCT 1 /* for compilation of proper subrs */
#endif

#define TRUE    1
#define FALSE   0

#define	streq(a,b)	(!strcmp(a,b))

/* exit status codes for program
 */
#define	SUCCESS	0
#define	FAILURE (-1)

#define MAGTAPE "/dev/rmt8" 

int blocksz;
#define BLOCKSZ       2048		
#define MAXBLOCKSZ 	2048		/* random choice */
int fixreclen;
#define FIXRECLEN	128		/* should be multiple of BLOCKSZ */
#define MAXFIXRECLEN	MAXBLOCKSZ

#define	RECSIZE		80		/* size of Header and Trailer
					 * records
					 */

/*
 * The ANSI Standard Magtape produced by a VAX looks like
 *
 *      Volume Label            80 bytes
 *      --------------------------------
 *      Header 1                80 bytes
 *      Header 2                80 bytes
 *     (Header  3)	       (80 bytes)   ... optional
 *      TAPE MARK
 *      Data Block              2048 bytes
 *      ....
 *      TAPE MARK
 *      Trailer 1               80 bytes
 *      Trailer 2               80 bytes
 *     (Trailer 3)	       (80 bytes)   ... optional
 *      TAPE MARK
 *      ---------------------------------
 *      TAPE MARK
 *
 * The information between the dashed lines is repeated for
 * every file on the tape. See appendix B of VAX-11 RMS Reference
 * for additional detailed information.
 */

/* The definitions used here reflect the fact that in C, counting starts
 * at 0 and not 1
 */

/*
 * fields of Volume Label
 */
#define __XXX		0,2	/* alphabetic characters "VOL"	*/
#define __YYY		3,3	/* numberic character "1"	*/
#define	VLABEL		4,9
#define	VPROT		10,10	/* protection on volume		*/
#define	VRES1		11,36
#define	VOWNER		37,49
#define	VDSV		50,50	/* DIGITAL standard version	*/
#define	VRES2		51,78
#define	VLSV		79,79	/* Label standard version	*/

/*
 * fields of Header 1
 */
#define	__AAA		0,2	/* alphabetic characters "HDR"	*/
#define __BBB		3,3	/* numeric character "1"	*/
#define H1_FNAME        4, 20	/* filename			*/
#define __CCC		21,26	/* file set identifier		*/
#define __DDD		27,30	/* file section number		*/
#define	__EEE		31,34	/* file sequence number		*/
#define __FFF		35,38	/* generation number		*/
#define	__GGG		39,40	/* generation version		*/
#define	H1_CDATE	41,46	/* creation date		*/
#define	H1_XDATE	47,52	/* expiration date		*/
#define	H1_ACCESS	53,53	/* access security		*/
#define	H1_BCOUNT	54,59	/* block count (always 0)	*/
#define	H1_SYSTEM	60,72	/* system that produced it	*/
#define __HHH		73,79	/* reserved			*/

/*
 * fields of Header 2
 */
#define	__III		0,2	/* alphabetic characters "HDR"	*/
#define	__JJJ		3,3	/* numeric character "2"	*/
#define	H2_FMT		4,4	/* record format		*/
#define	H2_BSZ		5,9	/* # characters per block	*/
#define	H2_RECLEN	10,14	/* record length for fixed length records */
#define	__KKK		15,35	/* system dependent info	*/
#define	H2_FORMC	36,36	/* form control
				 *	'A' : first byte of record contains
				 *		fortran control characters
				 *	'M' : record contains all form control
				 *		info
				 *	' ' : lf/cr to be inserted between
				 *		records.
				 */
#define	__LLL		37,49	/* system dependent info	*/
#define	__MMM		50,51	/* buffer offset = "00"		*/
#define	__NNN		52,79	/* reserved			*/

/*
 * fields of Header 3
 */
#define	__OOO		0,2	/* alphabetic characters "HDR"	*/
#define	__PPP		3,3	/* numeric character "3"	*/
#define	__QQQ		4,67	/* FILES-11 attributes if made on VMS system */
#define	__RRR		68,79	/* system dependent info. spaces for VMS */

/*
 * DEFINITIONS FOR WRITING TAPES
 */

#define VOL_LABEL       "UNIX"		/* default volume label */
#define CREATION        " 70001"        /* Julian date: yyddd */
#define EXPIRATION      " 99365"        /* Julian date: yyddd */
#define FILLCHAR        '^'

int     magtape;

char    databuf[MAXBLOCKSZ+1];
char	vol_label[80];


 

/*
 * These flags represent which operation is to be performed
 */

int     createflag;             /* Create a new tape            */
int     listflag;               /* List the tape contents       */
int     extractflag;            /* Extract the files from tape  */

/*
 * These flags represent modifiers to the basic operations
 */

int     rawflag;
int     binflag;
int	verbose;
int 	fixed_length_flag;
int 	section_flag;

char    *field();

char pad_record[MAXFIXRECLEN];
