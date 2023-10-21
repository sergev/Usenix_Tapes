/* CP/M call codes

	1983	Mark E. Mallett

 */

#define	_MRICC	1		/* Input console character */
#define	_MROCC	2		/* Output console character */
#define	_MRPTR	3		/* Read paper tape */
#define	_MRAUXI	3		/* Also known as auxiliary input */
#define	_MRPTP	4		/* Write paper tape */
#define	_MRAUXO	4		/* Also known as auxiliary output */
#define	_MRLPT	5		/* Write LPT */
#define	_MRDCIO	6		/* Direct console I/O */
#define	_MRRIO	7		/* Read IO status */
#define	_MRSIO	8		/* Write IO status */
#define	_MRWCS	9		/* Write string to console */
#define	_MRRBC	10		/* Read buffer from console */
#define	_MRRCS	11		/* Read console status */
#define	_MRCPV	12		/* CPM version number.. */
#define	_MRLFH	12		/* Lift head */
#define	_MRINI	13		/* Init BDOS */
#define	_MRSEL	14		/* Select and login a disk */
#define	_MROPN	15		/* Open a file */
#define	_MRCLS	16		/* Close a file */
#define	_MRSFL	17		/* Search for file */
#define	_MRSNF	18		/* Search for next file */
#define	_MRDEL	19		/* Delete a file */
#define	_MRREA	20		/* Read next record */
#define	_MRWRT	21		/* Write next record */
#define	_MRCRF	22		/* Create file */
#define	_MRREN	23		/* Rename file */
#define	_MRILV	24		/* Interrogate login vector */
#define	_MRIDN	25		/* Get drive number */
#define	_MRDMA	26		/* Set DMA address */
#define	_MRIAL	27		/* Get allocation vector */
#define	_MRWPD	28		/* Write-protect disc */
#define	_MRROV	29		/* Get R/O vector */
#define	_MRSFA	30		/* Set file attributes */
#define	_MRGDP	31		/* Get disc parms */
#define	_MRGUC	32		/* Get/set user code */
#define	_MRRRR	33		/* Read random record */
#define	_MRWRR	34		/* Write random record */
#define	_MRCFS	35		/* Compute file size */
#define	_MRSRR	36		/* Set random record */
#define	_MRWRZ	37		/* Write random record with zero fill */
#define	_MRCTP	47		/* Chain to program */
#define	_MRGDT	105		/* Get date and time. */



/* CP/M 3... */

#define	_MRFDS	46		/* Get free disk space */


/* Bios calls */

#define	_CBBOOT	0		/* Cold boot */
#define	_CBWBOOT 1		/* Warm boot */
#define	_CBCNST	2		/* Console status */
#define	_CBCNIN	3		/* Console input */
#define	_CBCNOUT 4		/* Console out */
#define	_CBLIST	5		/* Write to listing */
#define	_CBPUN	6		/* Write to punch */
#define	_CBRDR	7		/* Read from reader */
#define	_CBHOME	8		/* Home the disk */
#define	_CBSEL	9		/* Select disc */
#define	_CBSTRK	10		/* Set track */
#define	_CBSSEC	11		/* Set sector */
#define	_CBSDMA	12		/* Set DMA */
#define	_CBREAD	13		/* Read sector */
#define	_CBWRT	14		/* Write sector */
#define	_CBLSST	15		/* List status */
#define	_CBSTRN	16		/* Sector translate */
#define	_CBXIST	18		/* AUX in status */
