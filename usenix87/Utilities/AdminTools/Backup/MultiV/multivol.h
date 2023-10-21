/* multivol  V1.00  5-Jun-85  Tony O'Hagan
 * multivol (8) - handle multivolume files
 * multivol.h 
 */
#define PROGNAM	"multivol"
#define VERSION "1.00"
#include <assert.h>
#include "trace.h"

#define export		/* used to import/export globals */
#define import	extern

#define bool	short
#define FALSE	0
#define TRUE	1

#define READ	0
#define WRITE	1
/*
 * The size of block & volume headers must be a multiple of
 * of 4 to keep the DMA controllers happy when asked to 
 * read or write a buffer, and to keep this multiple 
 * consistant for volumes being transfered between systems
 * which round up the size of structures to a mulitple of 4.
 */
#define wd_size	4
#define	wd_round(x) (( (x) + wd_size-1 ) & ~(wd_size-1))
#define BHDSIZ	wd_round(sizeof(blk_hdr))	/* Block header size */
#define VHDSIZ	wd_round(sizeof(vol_hdr))	/* Volume header size */
#define MINBLK	wd_round(BHDSIZ + VHDSIZ + 1)	/* Minmum block size */

#define	NOLIMIT	-1		/* used when no limit on blocks per volume */

#define BEGBLK	'B'		/* blk_hdr->bh_blk_typ values */
#define DTABLK	'D'
#define ENDBLK	'E'

	/* Unpacked volume header */
#define NULVHD	((up_vhdr *)NULL)
typedef struct {
	int	up_vol_num;
	long	up_siz_blk, up_lim_blk, up_timestamp;
	char	up_prog_name[15], up_version[5], up_label[15];
} up_vhdr ;

	/* Volume header (all ascii) */
typedef struct {
	char	v_volnum[4],sp0;	/* "0001" */
  	char	v_prog[14],sp1;		/* program name */
	char	v_version[5],sp2;	/* Vn.nn */
	char	v_label[14],sp3;	/* volume label */
	char	v_timestamp[10],sp4;	/* binary time stamp */
	char	v_siz_blk[6], sp5;	/* digits of physical block size */
	char	v_lim_blk[6];		/* maximum no. of blocks per volume */
	char	nullch;			/* Null ch placed by sprintf() */
} vol_hdr;

	/* used to pack/unpack volume headers */
#define VHDFIELDS	7
#define VHDPACK		"%4d %-14s %-5s %-14s %10ld %6ld %6ld" 
#define VHDUNPACK	"%d %s %s %s %ld %ld %ld"

	/* Block header */
typedef struct {
	char	bh_blktyp;	/* block type - indicates end of volume set */
	char	bh_chksum;	/* block check sum */
	char	bh_dtalen[6];	/* amount of data in the block */
} blk_hdr;
