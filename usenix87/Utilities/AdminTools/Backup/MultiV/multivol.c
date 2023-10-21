/* multivol  V1.00  5-Jun-85  Tony O'Hagan
 * multivol (8) - handle multivolume files
 * multivol [-i] [-o] [-v] [-w] [-t] [-b blocksize]
 *      [-n blocks-per-vol] [-l label] [device]
 * Source files:
 *   ask.c bcopy.c error.c mount.c multivol.c rd.c testdata.c trace.c vhdr.c wr.c
 *   multivol.h options.h trace.h
 * Other files:
 *   Makefile READ_ME multivol.8 testdata.c
 */
#include <stdio.h>
#include <ctype.h>
#include "options.h"
#include "multivol.h"

#define	ACCREAD	4
#define ACCWRIT	2

#ifndef DEFBSIZ
#	include <sys/param.h>
#	undef	bcopy
#	define DEFBSIZ	BSIZE	/* default block size */
#endif

#ifndef DEFBLIM
#	define DEFBLIM	NOLIMIT	/* default block limit (no. per volume) */
#endif

#define DEFLABEL	"no_label"	/* Default label */

#ifndef DEFDEV  /* may be cc -DDEFDEV= */
#	define DEFDEV	"/dev/multivol"
#endif

/*  Globals */
	export	char	*prog_name, *label = DEFLABEL, *device = DEFDEV;
	export	bool	dsp_blks, dsp_vols, vfy_vols, rd_flag , wr_flag;
	export	long	blk_lim = DEFBLIM;
	export	long	blk_siz = DEFBSIZ;

void
main(argc, argv)	/* multivol main() */
	int	argc;
	char	*argv[];
{
	char	*bsizstr;
	up_vhdr	*upvh;

	void	rd_vols(), wrvols(), print_vhdr();
	up_vhdr *rd_vhdr(); 
	long	kval(), atol();
	char	*vol_file();
	int	t_int;
	bool	y_or_n();

	trace( "+ main()");

	rd_flag = wr_flag = dsp_vols = dsp_blks = vfy_vols = FALSE;
	OPTIONS ("[-i] [-o] [-v] [-w] [-t] [-b blocksize] [-n blocks-per-vol] [-l label] [device]")
		FLAG	('i', rd_flag)
#ifdef DEBUG
		FLAG	('D', tron)
#endif
		FLAG	('o', wr_flag)
		FLAG	('t', dsp_vols)
		FLAG	('v', dsp_blks)
		FLAG	('w', vfy_vols)
		STRING	('b', bsizstr)
			if ((blk_siz = kval(bsizstr)) == -1) 
				fatal("** Illegal block size %s", bsizstr);
			else if (blk_siz < MINBLK)
				fatal("** Minimum block size is %d", MINBLK);
			else { /* test if blk_siz value fits in int */
				t_int = blk_siz;
				if (blk_siz != t_int)
					fatal("** Block size too big");
			}
		NUMBER	('n', blk_lim)
		STRING	('l', label)
			if (index(label, ' ') != 0)
				fatal("** Label must not contain white space");
	ENDOPTS
	prog_name = argv[0];
	if (argc > 1)
		device = argv[1];

	tracef((tr, "BHDSIZ = %d   VHDSIZ = %d", BHDSIZ, VHDSIZ));
	tracef((tr, "prog_name:%s  label:%s  blk_siz:%ld",
		prog_name, label, blk_siz));
	tracef((tr, "blk_lim:%ld  dsp_blks:%d  dsp_vols:%d  vfy_vols:%d",
		blk_lim, dsp_blks, dsp_vols, vfy_vols));
	
	if (rd_flag && wr_flag)
		fatal("** Only -i or -o may be specified");
	else if (rd_flag) {
		if (access(device, ACCREAD) == -1)
			sfatal(device);
		rd_vols();
	} else if (wr_flag) {
		if (access(device, ACCREAD & ACCWRIT) == -1)
			sfatal(device);
		wrvols();
	} else {
		/* Print details of volume header */
		/* Note: No prompt to mount volume */
		if (access(device, ACCREAD) == -1)
			sfatal(device);
		if ((upvh = rd_vhdr(vol_file(device, 1))) != NULVHD)
			print_vhdr(upvh);
	}

	trace("- main()");
	exit(0);
}

long
kval(kstr) 
	char *kstr;
{
	long	n, atol();  /* return n */
	char	c;

	trace("+ kstr()");
	n = atol(kstr);
	c = kstr[strlen(kstr)-1];
	if (! isdigit(c)) {
		switch (c) {
		case 'b':case 'B': 
			n *= 512; break;
		case 'k':case 'K': 
			n *= 1024; break;
	/*	case 'm':case 'M': 
			n *= 1024*1024; break;	*/
		default:
			n = -1;
		}
	}
	trace("- kstr()");
	return(n);
}
