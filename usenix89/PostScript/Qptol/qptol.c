
/*
 *	qptol - portrait to landscape.
 *	Copyright 1985 by G. L. Sicherman.
 *	You may use and alter this program freely for noncommercial
 *	purposes so long as you leave this message alone.
 */

#include <stdio.h>
#include <local/qfont.h>

int	raslen;

bomb()
{
	fprintf(stderr,"usage: qptol [file]\n");
	exit(1);
}

main(argc,argv)
int argc;
char **argv;
{
	extern qfonterror;
	FILE *In;
	float atof();
	struct q_header qh, newqh;
	struct q_glyph qg;
	int save_spacing, save_width, save_hoffset;
	while (--argc) {
		if ('-'==**++argv) switch (*++*argv) {
/*
 *	space for future options.
 */
		default:
			bomb();
		}
		else break;
	}
	if (!argc) In=stdin;
	else {
		if (--argc) bomb();
		if (!(In=fopen(*argv,"r"))) {
			fprintf(stderr,"qptol: cannot read %s\n",*argv);
			exit(1);
		}
	}
	if (qreadh(In,&qh)) {
		fprintf(stderr,"qptol: format error %d on input\n",
			qfonterror);
		exit(1);
	}
	if (qh.q_orientation != 'P') {
		fprintf(stderr,"qptol: input not portrait\n");
		exit(1);
	}
	newqh = qh;		/* Copy the header */
	newqh.q_orientation = 'L';
	qwriteh(stdout,&newqh);
	raslen = (qh.q_fheight + 7)/8;
	while (!qread(In,&qh,&qg)) {
		save_spacing = qg.q_spacing;
/*
 *	The output width must be an even number of BYTES.
 */
		if (((qg.q_width+7)/8)%2) {
			unsigned char *pad;
			pad = (unsigned char *)malloc(raslen * (8+qg.q_width));
			bcopy(qg.q_bitmap, pad, raslen * qg.q_width);
			bzero(pad+raslen*qg.q_width, raslen*8);
			qg.q_width += 8;
			free(qg.q_bitmap);
			qg.q_bitmap = pad;
		}
		save_width = qg.q_width;
		qrotate(&qg, 0);
/*
 *	left qtrim adjusts the horizontal offset - but we don't want that!
 */
		save_hoffset = qg.q_hoffset;
		qtrim(&qg, 0);
		qtrim(&qg, 1);
		qg.q_hoffset = save_hoffset;
/*
 *	restore the dimensions.  qrotate thinks it's still portrait;
 *	it exchanges the width and height, and kludges the spacing.
 */
		qg.q_spacing = save_spacing;
		qg.q_height = qg.q_width;	/* the width has been trimmed */
		qg.q_width = save_width;
/*
 *	Vertical offset - must be adjusted for change in glyph height.
 */
		qg.q_voffset = qg.q_voffset + qg.q_height - qh.q_fheight;
/*
 *	Now write the glyph:
 */
		qwrite(stdout,&newqh,&qg);
	}
	qend(stdout);
	exit(0);
}
-- 
Col. G. L. Sicherman
UU: ...{rocksvax|decvax}!sunybcs!colonel
CS: colonel@buffalo-cs
BI: csdsicher@sunyabva
