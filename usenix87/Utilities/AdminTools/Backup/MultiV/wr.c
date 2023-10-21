/* multivol(8)  V1.00  5-Jun-85  Tony O'Hagan */
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
	extern	int	errno;

#include "multivol.h"

/*  Globals */
	extern	bool	dsp_blks;
	extern	char	*label, *device;
	extern	long	blk_lim;
	extern	long	blk_siz;

	/* TRUE when ENDBLK block type has been written */
	static	bool	END_written;
	static	long	blk_num;

void
wrvols()
{
	char	*buf_ptr, *dta_ptr;
	long	usu_dta, dta_left, dta_put;

	void	bcopy(), show_blks();
	char	*new_buf();
	long	wr_get_dta(), wr_put_dta();

	trace("+ wrvols()");

	buf_ptr = new_buf(blk_siz+VHDSIZ);

	dta_left = 0L;
	END_written = FALSE;
	dta_ptr = buf_ptr + BHDSIZ + VHDSIZ; /* See diag. of buffer */
	usu_dta = blk_siz - BHDSIZ;	/* usual amount of data per block */

	while ( TRUE ) {
			/* Attempt to fill up data buffer to 'usu_dta' chars &
				append new data after data left over */
		dta_left += wr_get_dta(dta_ptr + dta_left, usu_dta - dta_left);
		tracef((tr, "after wr_get_dta dta_left = %ld", dta_left));

		if (dta_left <= 0L && END_written)
			break;

			/* Attempt to dispose of whatever is in data buffer */
		dta_left -= (dta_put = wr_put_dta(dta_ptr, dta_left));
		tracef((tr, "after wr_put_dta dta_left = %ld", dta_left));

			/* if any left, copy to top of data buffer */
		if (dta_left > 0L && dta_put > 0L)
			bcopy(dta_ptr + dta_put, dta_ptr, (int)dta_left);
	}
	show_blks(-1L);	/* terminate display of blocks */
	if (dsp_blks)
		fprintf(stderr, "%ld blocks on last volume", blk_num);
	trace("- wrvols()");
}

long
wr_get_dta(dta_ptr, dta_req)
	char	*dta_ptr;
	long	dta_req;
{
	long	n_read, tot_read;

	tracef((tr, "+ wr_get_dta(%x, %ld)", dta_ptr, dta_req));

	/* Get at least the required amount of data */
	for (tot_read = 0L; tot_read < dta_req; tot_read += n_read) {
		n_read = read(fileno(stdin),
			dta_ptr + tot_read, (int)(dta_req - tot_read));
		tracef((tr, "n_read = %ld", n_read));
		if (n_read == -1) {
			n_read = 0L;
			swarning("stdin");
		}
		if (n_read == 0L)
			break;
	}

	tracef((tr, "- wr_get_dta() tot_read = %ld", tot_read));
	return tot_read ;
}

/* Controls when volumes require mounting */
long
wr_put_dta(dta_ptr, dta_len)
	char	*dta_ptr;
	long	dta_len;
{
	void	wr_vol(), show_blks();
	char	*blk_ptr;
	long	wr_blk(), n_writ, blk_len;

	static	int	dev;
	static	bool	new_vol = TRUE;

	tracef((tr, "+ wr_put_dta(%x, %ld)", dta_ptr, dta_len));

	blk_ptr = dta_ptr;
	blk_len = dta_len;

	if (new_vol) {
		blk_ptr -= VHDSIZ;
		blk_len += VHDSIZ;
		/* mount next volume & pre-pend volume header to data buffer */
		wr_vol(&dev, blk_ptr);
		blk_num = 0L;
		new_vol = FALSE;
	}

	blk_ptr -= BHDSIZ;
	blk_len += BHDSIZ;
	if ((n_writ = wr_blk(dev, blk_ptr, blk_len)) == 0L) { 
		/* last block won't fit on current volume */
		new_vol = TRUE;
		if (blk_lim != NOLIMIT)
			fprintf(stderr,
			"\n** Could only fit %ld block(s) on volume\n",blk_num);
		trace("- wr_put_dta()");
		return(0L);
	} else { 
		show_blks(++blk_num);
		/* test volume block limit */
		if (blk_lim != NOLIMIT && blk_num >= blk_lim)
			new_vol = TRUE;
		/* return amount of data written (excl. headers - See diag.) */
		trace("- wr_put_dta()");
		return(n_writ - (dta_ptr - blk_ptr));
	}
}

long
wr_blk(dev, blk_ptr, blk_len)
	int	dev;
	char	*blk_ptr;
	long	blk_len;
{
#define BSZDIG	6	/* number of digits for block size string */

	char	 	len_str[BSZDIG+1];
	char	xor_sum(), *strncpy();
	blk_hdr	*bhd_ptr;
	long	n_writ, nulls;

	tracef((tr, "+ wr_blk(%d, %x, %ld)", dev, blk_ptr, blk_len));

	if (blk_len > blk_siz)
		blk_len = blk_siz;	/* write at most blk_siz chars */

	bhd_ptr = (blk_hdr *) blk_ptr;

	/* From: seismo!uwvax!prairie!dan			*/
	/* The following was changed by dmf.  The block length	*/
	/* was originally right-justified in the field, and if	*/
	/* it was follwed by a numeric character, the sscanf in	*/
	/* the read block code happily kept reading, leading to	*/
	/* odd block sizes.  This should fix that.		*/
  	sprintf(len_str, "%-6ld", blk_len);
	strncpy(bhd_ptr->bh_dtalen, len_str, BSZDIG);

	if (blk_len < blk_siz) {
		/* move nulls to end of last block of last volume */
		register char	*p;
		nulls = blk_siz - blk_len;
		p = blk_ptr + blk_len;
		while (blk_len++ < blk_siz)
			*p++ = '\0';
		bhd_ptr->bh_blktyp = ENDBLK;
		END_written = TRUE;
	} else {
		nulls = 0L;
		bhd_ptr->bh_blktyp = DTABLK;
	}
	tracef((tr, "blk_typ = %c", bhd_ptr->bh_blktyp));
		
		/* Calc. checksum */
	bhd_ptr->bh_chksum = 0;
	bhd_ptr->bh_chksum = xor_sum(blk_ptr, blk_siz);

	tracef((tr, "dev=%d,  blk_ptr=%x  blk_siz=%d",
		dev, blk_ptr, (int)blk_siz));

	n_writ = write(dev, blk_ptr, (int)blk_siz);
	tracef((tr, "n_writ = %ld", n_writ));
	if (n_writ < blk_siz ) {
		/* Assume can't fit last block & need new volume */
		if (n_writ == -1)
			swarning(device);
		n_writ = 0L;
		END_written = FALSE;
		errno = 0;
	} else 
		/* return size of data & headers written
		less nulls used to fill the last block */
		n_writ -= nulls;

	trace("- wr_blk()");
	return (n_writ);	
}

void
wr_vol(dev, vhd_ptr)
	int	*dev;
	char	*vhd_ptr;
{
#ifndef	time_t
#	include	<sys/types.h>
#endif
	/* mount and open next volume for writing */
	void	pack_vhdr();
	up_vhdr	*mount_vol();
	char	*vol_file(), *vol_nam;
	time_t	time();

	static	int	vol_num = 0;
	static	long	timestamp;

	tracef((tr, "+ wr_vol(%x, %x)", dev, vhd_ptr));
	if (++vol_num == 1)
		timestamp = time((time_t *) 0);
	else {	/* Note: the last volume is closed by exit() */
		show_blks(-1L);	
		trace("close");
		close(*dev);
	}
	
	pack_vhdr(vhd_ptr, vol_num, timestamp);
	
	vol_nam = vol_file(device, vol_num);

	mount_vol(vol_nam, vol_num, timestamp);

	if ((*dev = open(vol_nam, WRITE)) == -1)
		sfatal(vol_nam);

	trace("- wr_vol()");
}

/* ARGSUSED */
char *
vol_file(devname, vol_num)
	char	*devname;
	int	vol_num;
{
	static	char	vol_name[15];

#ifdef	VOLNUM
	/* used for testing when writting to files pretending to be volumes */
	sprintf(vol_name, "%s.%d", devname, vol_num);
#else
	sprintf(vol_name, "%s", devname);
#endif
	return((char *)vol_name);
}

char
xor_sum(ptr, siz)
	char	*ptr;
	long	siz;
{
	register char	sum;
	register int	i;

	sum = 0;
	for (i = siz; i-- ; )
		sum ^= *ptr++;
	return sum;
}

/* show_blks() is called from rd.c and wr.c */
void
show_blks(blk_num)
	long	blk_num;
{
	bool	y_or_n();
#define	DOTBLKS	10
#define	DOTCH	'.'
	/* Every DOTBLKS blocks print a dot */
	tracef((tr, "show_blks(#%ld)", blk_num));
	if (dsp_blks) {
		if (blk_num == -1L) 
			putc('\n', stderr);
		else if (blk_num % DOTBLKS == 0)	
#ifdef	DEBUG
			fprintf(stderr, "%8ld", blk_num);
#else
			putc(DOTCH, stderr);
#endif
	}
}

char *
new_buf(buf_siz)
	long	buf_siz;
{
	char	*buf_ptr;
	char	*malloc();

	tracef((tr, "+ new_buf(%ld)", buf_siz));
	if ((buf_ptr = malloc( (unsigned)buf_siz )) == (char *)NULL) {
		fatal("Insufficient memory for block size");
	}
	trace("- new_buf()");
	return buf_ptr;
}
