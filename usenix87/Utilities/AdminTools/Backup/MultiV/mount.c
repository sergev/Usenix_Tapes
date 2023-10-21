/* multivol  V1.00  5-Jun-85  Tony O'Hagan
 * multivol (8) - handle multivolume files
 * wrvols() - function of multivol main()
 */

#include <stdio.h>
#include <ctype.h>

#include "multivol.h"

/*  Globals */
	extern	bool	dsp_vols, vfy_vols, rd_flag, wr_flag;
	extern	char	*label;

up_vhdr *
mount_vol(vol_nam, vol_num, timestamp)
	/* returns header read on mounted volume */
	char	*vol_nam;
	int	vol_num;
	long	timestamp;
{
	up_vhdr	*upvh; /* returned value */
	bool	ok;

	bool	vol_ok();
	void	print_vhdr();
	bool	y_or_n();
	up_vhdr *rd_vhdr();
	char	*askf();
#ifdef	REWIND
	char	*ask();
#endif

	tracef((tr, "+ mount_vol(%s, %d, %ld)", vol_nam, vol_num, timestamp));

	do {	
		askf("Mount volume #%d and type <return> ", vol_num);

		if (rd_flag || dsp_vols || vfy_vols)
			upvh = rd_vhdr(vol_nam); /* Read volume header */
		else {
			upvh = NULVHD; 
			trace("goto no_vhdr");
			goto no_vhdr;  /* no volume header was read */
		}

		if (upvh == NULVHD) {
			ok = !rd_flag; /* ok unless attempting to read volume */
		} else {
			if (dsp_vols || vfy_vols) {
		/* Print header & check correct volume is read/overwritten */
				putc('\n', stderr);
				print_vhdr(upvh);
				putc('\n', stderr);
			}
			ok = vol_ok(upvh, vol_num, timestamp);
		}
		tracef((tr, "vfy_vols=%d, ok=%d upvh=x%x", vfy_vols, ok, upvh));
	} while ( (vfy_vols || !ok) && y_or_n("Remount required [y/n] ? ") );

#ifdef	REWIND
	ask("Ensure volume is rewound and type <return> ");
#endif

no_vhdr:
	trace("- mount_vol()");
	return(upvh);
}

bool
vol_ok(upvh, vol_num, timestamp)
up_vhdr	*upvh;
	int	vol_num;
	long	timestamp;
/* Check correct volume is mounted
 * vol_num	= volume no. expected on this volume
 * timestamp	= timestamp on volumes being written
 * upvh		= unpacked volume header on volume about to be read/overwritten.
 */
{
	bool	ok = TRUE;	/* returns ok */
	static	up_vhdr	up_1st;	/* first volume header read */

	trace("+ vol_ok()");
	if (wr_flag) {	/* if writing volumes */
		if (timestamp == upvh->up_timestamp) {
			fprintf(stderr, 
			"** This clobbers a volume just written !\n");
			ok = FALSE;
		}
	} else if (rd_flag) { /* if reading volumes */
		if (vol_num == 1) { /* record header of first volume */
			bcopy((char *)upvh, (char *)&up_1st, sizeof(up_vhdr));
			if (label != NULL &&strcmp(label, upvh->up_label) != 0){
				fprintf(stderr, "** Label does not match\n");
				ok = FALSE;
			}
		}
		if (vol_num >= 2 && up_1st.up_timestamp != upvh->up_timestamp) {
			fprintf(stderr, "** Volume not part of volume set\n");
			ok = FALSE;
		} else if (upvh->up_vol_num != vol_num) {
			fprintf(stderr, "** Volume no. out of sequence\n");
			ok = FALSE;
		}
	}
	trace("- vol_ok()");
	return(ok);
}
