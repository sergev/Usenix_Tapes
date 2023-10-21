/* multivol(8)  V1.00  5-Jun-85  Tony O'Hagan
 * Handles multivolume files
 */
#include <stdio.h>
#include "multivol.h"

/*  Globals */
	extern	char	*label, *device;
	extern	long	blk_lim;
	extern	long	blk_siz;

	/* TRUE when block containing ENDBLK block type has been read */
	static	bool	END_read;  
	static	long	blk_num;

void
rd_vols()
{
	char	*dta_ptr;
	long	dta_got;

	void	show_blks();
	long	rd_get_dta(), rd_put_dta();

	trace("+ rd_vols()");

	END_read = FALSE;
	do {
		dta_got = rd_get_dta(&dta_ptr);
		tracef((tr, "dta_got = %ld", dta_got));
		if (dta_got > 0L)
			rd_put_dta(dta_ptr, dta_got);
	} while (!END_read);

	show_blks(-1L);	/* terminate display of blocks */
	trace("- rd_vols()");
	return;
}

long
rd_put_dta(dta_ptr, dta_len)
	char	*dta_ptr;
	long	dta_len;
{
	long	n_writ;
	trace("+ rd_put_dta()");

	if ((n_writ = write(fileno(stdout), dta_ptr, (int)dta_len)) == -1L) {
		n_writ = 0L;
		swarning("stdout");
	} else if (n_writ < dta_len)
		warning("** Only output %ld bytes of %ld bytes in block #%ld",
			n_writ, dta_len, blk_num);

	tracef((tr, "- rd_put_dta() n_writ = %ld", n_writ));
	return n_writ ;
}

/* Controls when volumes require mounting and
 * allocates buffer space for the 1st volume.
 */
long
rd_get_dta(dta_ptr)
	char	**dta_ptr;
{
	void	show_blks();
	long	rd_blk();
	char	*new_buf(), *vol_file();
	up_vhdr	*mount_vol();

	char	*vol_nam;
	long	n_read;
	up_vhdr	*upvh;

	static	char	*blk_ptr;
	static	int	dev, vol_num = 0;
	static	bool	new_vol = TRUE;

	tracef((tr, "+ rd_get_dta(%x)", *dta_ptr));

	if (new_vol) {
		blk_num = 0L;
		new_vol = FALSE;
		vol_nam = vol_file(device, ++vol_num);
		if (vol_num > 1) {
			show_blks(-1L);	
			trace("close");
			close(dev);	/* Last volume is closed by exit() */
		}
		if ((upvh = mount_vol(vol_nam, vol_num, 0L )) == NULVHD) {
			END_read = TRUE;
			trace("- rd_get_dta()");
			return 0L;
		} 

		trace("set blk_siz & blk_lim from vol header");
		blk_siz = upvh->up_siz_blk;
		blk_lim = upvh->up_lim_blk;

		if (vol_num == 1) {
			blk_ptr = new_buf(blk_siz + VHDSIZ);
		}

		if ((dev = open(vol_nam, READ)) == -1)
			sfatal(vol_nam);

		/* setup pointer to data in block */
		*dta_ptr = blk_ptr + BHDSIZ + VHDSIZ;
	} else
		*dta_ptr = blk_ptr + BHDSIZ;

	if ((n_read = rd_blk(dev, blk_ptr)) == 0L) {
		/* Couldn't read last block on current volume */
		new_vol = TRUE;
		if (blk_lim != NOLIMIT && !END_read)
			fprintf(stderr,
			"** Could only read %ld blocks on volume", blk_num);
		trace("- rd_get_dta()");
		return 0L ;
	} else {
		if (blk_lim != NOLIMIT && blk_num >= blk_lim)
			new_vol = TRUE;
		trace("- rd_get_dta()");
		/* return amount of data written (excl. headers - See diag.) */
		return(n_read - (*dta_ptr - blk_ptr));
	}
}

long
rd_blk(dev, blk_ptr)
	int	dev;
	char	*blk_ptr;
#define bh_ptr	((blk_hdr *) blk_ptr)
{
	int	chk_sum;
	char	xor_sum();
	long	blk_len, n_read;

	trace("+ rd_blk()");
	n_read = read(dev, blk_ptr, (int)blk_siz);
	if (n_read == blk_siz) {
		show_blks(++blk_num);
	} else {
		if (n_read == -1L)
			swarning(device);
		else
			warning("** Block #%ld is only %ld bytes",
				blk_num+1, n_read);
		/* END_read = TRUE;	- abort gracefully */
		trace("- rd_blk()");
		return 0L ;
	}

	END_read = bh_ptr->bh_blktyp == ENDBLK;

	sscanf(bh_ptr->bh_dtalen, "%6ld", &blk_len);
	tracef((tr, "blk_len = %ld", blk_len));

		/* Calc. checksum */
	if (chk_sum = xor_sum(blk_ptr, blk_siz) != '\0')
		warning("** Block #%ld check sum error (sum = %x hex)\n",
			blk_num, chk_sum);

	trace("- rd_blk()");
	return(blk_len);
}
