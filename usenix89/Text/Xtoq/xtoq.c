/*
 *	xtoq - TeX font to QMS 1200 font.
 *	Copyright (c) 1986 by Col. G. L. Sicherman.
 *	You may use and alter this program freely for non-commercial ends
 *	so long as you leave this message here.
 *
 *	xtoq infile > outfile
 *	infile (pxl file) must have same endian as host.
 *	(That means, swab it if it's big-endian and host is little-endian.)
 */

#include <ctype.h>
#include <stdio.h>
#include <local/qfont.h>
#define MIN(x,y) ((x)<(y)? (x): (y))
#define MAX(x,y) ((x)>(y)? (x): (y))
#define RND(x)	((int)((x)+0.5))
#define MAGIC	1001
#define	HIGH	0x80000000

char *progname;
char *filename;
short x_height, x_width;
short x_voffset, x_hoffset;
long x_pointer;
long x_spacing;
long dirbuf[4];
int File;
int maxheight;
unsigned char thischar;
struct q_header qh;
struct q_glyph qg;

char *
unctrl(c)
unsigned char c;
{
	static char hold[3];
	if (c == '\177') return "^?";
	if (c<' ') sprintf(hold,"^%c", c^0100);
	else sprintf(hold,"%c",c);
	return hold;
}

main(argc,argv)
int argc;
char **argv;
{
	int k, vert;
	int i, j, raslen, outraslen;
	long *buf, *bp;
	long trailer[5];
	progname = *argv;
	while (--argc) {
		if ('-'==**++argv) switch (*++*argv) {
		default:
			bomb();
		}
		else break;
	}
	if (!argc) bomb();
	filename = *argv;
	if (--argc>0) bomb();
	File = open(filename,0);
	if (0>File) {
		fprintf(stderr,"%s: cannot read %s\n", progname,filename);
		exit(1);
	}
	lseek(File,-5L*sizeof(long),2);
	read(File,trailer,5*sizeof(long));
	if (MAGIC != trailer[4]) {
		fprintf(stderr,"%s: %s not a pxl file\n", progname,filename);
		exit(1);
	}
/*
 *	comb the directory to determine the QMS font height.
 */
	maxheight = 0;
	lseek(File, trailer[3]*sizeof(long), 0);
	for (thischar=0; thischar<128; thischar++) {
		read(File, (char *)dirbuf, 4*sizeof(long));
		x_height = dirbuf[0] & 0xffff;
		x_pointer = dirbuf[2];
		if (x_pointer) maxheight=MAX(maxheight, x_height);
	}
/*
 *	write the QMS header.
 */
	qh.q_number = 0;
	qh.q_orientation = 'P';
	qh.q_version = '1';
	strncpy(qh.q_name,filename,4);
	qh.q_fheight = maxheight;
	qh.q_baseline = 0;
	qh.q_format = Q_FORMAT2;
	qwriteh(stdout, &qh);
/*
 *	now convert the glyphs.
 */
	for (thischar=0; thischar<128; thischar++) {
		lseek(File, (-5+(-128L + thischar)*4)*sizeof(long), 2);
		read(File, (char *)dirbuf, 4*sizeof(long));
		x_height = dirbuf[0] & 0xffff;
		x_width = (dirbuf[0]>>16) & 0xffff;
		x_voffset = dirbuf[1] & 0xffff;
		x_hoffset = (dirbuf[1]>>16) & 0xffff;
		x_pointer = dirbuf[2];
		x_spacing = dirbuf[3];
		if (!x_pointer && !x_spacing) continue;
		raslen = (x_width+31) / 32;
		if (!x_pointer) continue;	/* DON'T CONVERT SPACES */
/*
 *	allocate buffers.
 */
		qg.q_char = thischar;
		qg.q_spacing =RND(300.*x_spacing*trailer[1]/(1000.*trailer[2]));
		qg.q_width = x_width;
/*
 *	set the height to the glyph height.
 *	the output file will have to be filtered through 'newheight'.
 */
		qg.q_height = x_height;
		qg.q_hoffset = -x_hoffset;
		qg.q_voffset = 37 + x_voffset - x_height;
		outraslen = (qg.q_height + 7) / 8;
		qg.q_bitmap = (unsigned char *)malloc(outraslen*qg.q_width);
		for (k=0; k<outraslen*qg.q_width; k++) qg.q_bitmap[k]=0;
		buf = (long *)malloc((unsigned)x_height*raslen*sizeof(long));
		lseek(File, x_pointer*sizeof(long), 0);
		read(File, (char *)buf, x_height * raslen * sizeof(long));
		bp = buf;
		for (i=0; i<x_height; i++) {
			long dots;
			for (j=0; j<x_width; j+=32) {
				dots = *bp++;
				vert = x_height - 1;
				for (k=0; k<32; k++) {
					if (32*j+k>=qg.q_width) break;
					if (HIGH & dots)
					qg.q_bitmap[(vert-i)/8+
						outraslen*(32*j+k)] |=
						1<<(7-(vert-i)%8);
					dots<<=1;
				}
			}
		}
		qwrite(stdout,&qh,&qg);
		free(buf);
		free(qg.q_bitmap);
	}
	qend(stdout);
	exit(0);
}

bomb()
{
	fprintf(stderr,"usage: %s file\n",progname);
	exit(1);
}
