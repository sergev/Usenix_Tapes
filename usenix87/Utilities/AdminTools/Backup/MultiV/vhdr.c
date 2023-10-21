/* multivol  V1.00  5-Jun-85  Tony O'Hagan
 * multivol (8) - handle multivolume files
 * vhdr.c - functions performed on volume header
 */
#include <stdio.h>
#include "multivol.h"

	extern	long	blk_siz, blk_lim;
	extern	char	*label;

up_vhdr *
rd_vhdr(devname)
	char	*devname;
/* Read volume header from the first block and rewind (close()) the volume */
{
	up_vhdr *ans;
	int	dev;
	int	n_read;
	char	*blk;

	char	*new_buf();
	up_vhdr	*unpack_vhdr();
	void	free();

	tracef((tr, "+ rd_vhdr(%s)", devname));
	if ((dev = open(devname, READ)) == -1) {
		swarning(devname);
		trace("- rd_vhdr()");
		return(NULVHD);
	}

	blk = new_buf(blk_siz);
	trace("reading volume header");

	tracef((tr, "read(%d, %x, %d)", dev, blk, (int)blk_siz));
	if ((n_read = read(dev, blk, (int)blk_siz) ) == -1) {
		swarning(devname);
		ans = NULVHD;
	} else {
		if (n_read == 0) {
			fprintf(stderr, "** No header found on volume\n");
			ans = NULVHD;
		} else if (n_read < MINBLK) {
			fprintf(stderr,
				"** Header block is only %d bytes\n", n_read);
			ans = NULVHD;
		} else {	/* vol_hdr after blk_hdr */
			tracef((tr, "blk+BHDSIZ = %x",
				(vol_hdr *)(blk + BHDSIZ)));
			if (NULVHD ==
				(ans = unpack_vhdr((vol_hdr *)(blk + BHDSIZ))) )
				fprintf(stderr, "** Invalid volume header\n");
		}
	}
	close(dev);
	trace("free(blk)");
	free(blk);
	trace("- rd_vhdr()");
	return ans ;
}

void
print_vhdr(up)
	up_vhdr	*up;
/* Print volume header details */
{
#ifndef	time_t
#	include <sys/types.h>
#endif
	char	*ctime();
	time_t	t_stamp;

	trace("+print_vhdr()");
	t_stamp = (time_t) up->up_timestamp;

	/* Note: ctime() returns date & time with a \n following */
	fprintf(stderr, 
		"Written using: %s V%s  Label: %s  %s", up->up_prog_name,
		up->up_version, up->up_label, ctime(&t_stamp) );
	fprintf(stderr, 
		"Vol. no.: %d   Block size: %ld   Blocks per volume: %ld\n",
		up->up_vol_num, up->up_siz_blk, up->up_lim_blk);

	trace("-print_vhdr()");
}

void
pack_vhdr(pk, vol_num, timestamp)
	char	*pk;
	int	vol_num;
	long	timestamp;
{
	sprintf(pk, VHDPACK,
		vol_num, PROGNAM, VERSION, label, timestamp, blk_siz, blk_lim);
}

up_vhdr *
unpack_vhdr(pk)
	vol_hdr		*pk;
{
	static	up_vhdr		up;
	
	tracef((tr, "+ unpack_vhdr(pk = %x)", pk));
	if(VHDFIELDS == sscanf((char *)pk, VHDUNPACK,
		&up.up_vol_num, up.up_prog_name, up.up_version,
		up.up_label, &up.up_timestamp, &up.up_siz_blk, &up.up_lim_blk)) {
		trace("- unpack_vhdr");
		return &up;
	} else {
		trace("- unpack_vhdr = NULVHD");
		return NULVHD;
	}
}
