#
		/* Written by Russ Smith --- C.V.L. */

/* To compile -- cc -O -s hstw.c -lg; mv a.out /usr/bin/hstw */

#define RED     00017
#define GREEN   00360
#define BLUE    07400

#define RSHFT   0
#define GSHFT   4
#define BSHFT   8

#define MAXLEN  350
#define ROWS    80
#define ROWSM2  78

#define HCOL    170
#define HROW    235
#define MCOL    150
#define MROW    40
#define SCOL    150
#define SROW    20
#define BCOL    70
#define BROW    60
#define TCOL    250
#define TROW    60

#define FCOL    1

#define BINNUM  256

double glbins[BINNUM];

double ftemp;
double mean;
double pmax;

int rr;
int top;
int bottom;
int scale;
int vcol;
int blen;

int image[16];
int overlay[16];
int cursors[4];
int row[512];

int mask;
int shift;

int fcol;
int frow;
int ncol;
int nrow;

main(argc, argv)
char *argv[];
{
	register *cur;

	if(argc < 2)
bad:            msg("Usage == hstw <key> \n\t (key = r g b u6 u8 l6 l8)");

	switch(argv[1][0]) {
	case 'r':
		mask = RED;
		shift = RSHFT;
		break;
	case 'g':
		mask = GREEN;
		shift = GSHFT;
		break;
	case 'b':
		mask = BLUE;
		shift = BSHFT;
		break;
	case 'l':
		shift = 0;
		if(argv[1][1] != '8')
			mask = 00077;
		else
			mask = 00377;
		break;
	case 'u':
		mask = 07760;
		if(argv[1][1] != '8')
			shift = 6;
		else
			shift = 4;
		break;
	default:
		goto bad;
	}

	gopen(overlay, 0, 0, 512, 512);
	genter(overlay,4,0,0,0,0);

	cur = cursors;

	grcur(overlay,cur,0);   /* Get the window to histogram */

	if((cur[0] > cur[2]) || (cur[1] > cur[3]))
		msg("Window undefined\nUse posw to define window");

	gclear(overlay, 0, 0, 512, 512, 0);

	gwvec(overlay, cur[0], cur[1], cur[0], cur[3], 0);
	gwvec(overlay, cur[0], cur[1], cur[2], cur[1], 0);
	gwvec(overlay, cur[2], cur[3], cur[2], cur[1], 0);
	gwvec(overlay, cur[2], cur[3], cur[0], cur[3], 0);
	fcol = cur[0];
	frow = cur[1];
	ncol = cur[2] - cur[0] + 1;
	nrow = cur[3] - cur[1] + 1;

	histw();
	gclose(overlay);
}



histw()
{
	register i, gl;
	register double *bins;

	gwstr(overlay, "COMPUTING HISTOGRAM", HCOL, HROW);

	top = pmax = mean = 0;
	bottom = BINNUM - 1;

	bins = glbins;
	for(i=0;i<BINNUM;i++) *bins++ = 0;

	gopen(image, fcol, frow, ncol, nrow);

	for(rr=0;rr<nrow;rr++) {
		if((rr / 25) * 25 == rr) printf("\07"); /* Computing... */
		gwcur(image, 1, 0, rr, 1, 1);
		grrow(image, row, rr, 0, ncol, 1);
		for(i = 0;i < ncol;i++){
			gl = (row[i] & mask) >> shift;

			mean = mean + gl;

			bins = glbins + gl;
			if((*bins = *bins + 1) > pmax) pmax = *bins;

			if(gl < bottom) bottom = gl;
			if(gl > top) top = gl;
		}
	}

	scale = ((pmax + MAXLEN) - 1) / MAXLEN;

	ftemp = ncol;
	ftemp = ftemp * nrow;
	mean = mean / ftemp;

	gclear(overlay,0,0,512,512,0);
	gwcur(overlay,1,cursors[0],cursors[1],0,0);

	gwstr(overlay,stringf("MEAN PIXEL VALUE --- %3.2f",mean),MCOL,MROW);
	gwstr(overlay,stringf("SCALE (PIXELS/DOT) --- %d",scale),SCOL,SROW);
	gwstr(overlay,stringf("LOWEST GREYLEVEL --- %d",bottom),BCOL,BROW);
	gwstr(overlay,stringf("HIGHEST GREYLEVEL --- %d",top),TCOL,TROW);

	gl = (top - bottom + 1) * 2;
	vcol = (top-bottom < 255 ? FCOL : 0);
	for(i=vcol;i<gl;i =+ 10) gwpnt(overlay,1,i,ROWSM2);

	for(i = bottom;i <= top;i++) {
		if((blen = (scale/2 + glbins[i]) / scale) > MAXLEN)
			blen = MAXLEN;
		gwvec(overlay,vcol++,ROWS,vcol++,ROWS + blen,1);
	}

	gclose(image);
}

extern fout;

msg(emsg)
char *emsg;
{
	fout = 2;
	printf("HSTW --- %s\n", emsg);
	exit();
}
