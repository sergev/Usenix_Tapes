#
		/* Written by Russ Smith --- C.V.L. */

/* To compile -- cc -O -s savw.c -lg; mv a.out /usr/bin/savw */

#define RED     00017
#define GREEN   00360
#define BLUE    07400

#define RSHFT   0
#define GSHFT   4
#define BSHFT   8

#define ROW     0
#define COL     1

int image[16];
int overlay[16];

int cursors[4];

int linc;
int sinc;
int mode;

int fcol;
int frow;
int ncol;
int nrow;

int header[6];
int nohead;

int buf[512];
char cbuf[512];

int bsize;

int wflg;
int mask;
int shift;

int start;
int fin;
int bstart;
int npnts;
int npnts2;

int file 1;

main(argc,argv)
char *argv[];
{
	register *cur;
	register char *c;

	if(argc < 2)
bad:            msg("Usage == savw <key> [-][mode] \n\t (key = a o r g b u6 u8 l6 l8)\n\t (mode = lb lt rb rt bl br tl tr)\n");

	header[5] = bsize = 1;
	nohead = 0;     /* Default is for header to be output */

	switch(argv[1][0]) {
	case 'o':
		wflg++;
		break;
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
	case 'a':
		header[5] = bsize = 2;
		break;
	default:
		goto bad;
	}

	gopen(overlay, 0, 0, 512, 512);

	cur = cursors;

	grcur(overlay,cur,0);   /* Get the window to save */

	if((cur[0] > cur[2]) || (cur[1] > cur[3]))
		msg("Window undefined\nUse posw to define window");

	if(!wflg) {
		genter(overlay,4,0,0,0,0);
		gclear(overlay, 0, 0, 512, 512, 0);
		gwvec(overlay, cur[0], cur[1], cur[0], cur[3], 0);
		gwvec(overlay, cur[0], cur[1], cur[2], cur[1], 0);
		gwvec(overlay, cur[2], cur[3], cur[2], cur[1], 0);
		gwvec(overlay, cur[2], cur[3], cur[0], cur[3], 0);
	}

	fcol = cur[0];
	frow = cur[1];
	header[1] = ncol = cur[2] - cur[0] + 1;
	header[3] = nrow = cur[3] - cur[1] + 1;

	mode = ROW;     /* Default is left to right, bottom to top scan */
	linc = 1;
	sinc = 1;
	start = 0;
	fin = nrow;
	bstart = 0;
	npnts2 = (npnts = ncol) * 2;

	if(argc == 3) {
		if(((c = argv[2])[0]) == '-') { /* No header output */
			nohead++;
			c = &argv[2][1];
		}
		switch(c[0]) {
		case 'l':
			if(c[1] != 'b') {
				linc = -1;
				start = nrow - 1;
				fin = -1;
			}
		default:        /* Default is 'lb' */
			break;
		case 'r':
			sinc = -1;
			bstart = ncol - 1;
			if(c[1] != 'b') {
				linc = -1;
				start = nrow - 1;
				fin = -1;
			}
			break;
		case 'b':
			mode = COL;
			sinc = 1;
			npnts2 = (npnts = nrow) * 2;
			fin = ncol;
			if(c[1] != 'l') {
				linc = -1;
				start = ncol - 1;
				fin = -1;
			}
			break;
		case 't':
			mode = COL;
			sinc = -1;
			bstart = nrow - 1;
			npnts2 = (npnts = nrow) * 2;
			fin = ncol;
			if(c[1] != 'l') {
				linc = -1;
				start = ncol - 1;
				fin = -1;
			}
			break;
		}
	}

	savew();
}

savew()
{
	register i, j;

	gopen(image,fcol,frow,ncol,nrow);
	if(wflg) genter(image,4,0,0,0,0);

	if(nohead == 0) {
		if(mode == COL) {
			header[1] =^ (header[3] =^ header[1]);
			header[3] =^ header[1];
		}
		if (write(file,header,12) != 12)    /* Write out the header */
			msg ("error while writing header");
	}

	for(i = start;i != fin;i =+ linc) {
		if(mode == ROW)
			grrow(image,buf,i,bstart,npnts,sinc);
		else
			grcol(image,buf,i,bstart,npnts,sinc);

		if(bsize == 1) {
			if(wflg) {
				for(j=0;j<npnts;j++)
					cbuf[j] = (buf[j] ? 0377 : 0);
			}
			else {
				for(j=0;j<npnts;j++)
					cbuf[j] = (buf[j] & mask) >> shift;
			}
			if (write(file,cbuf,npnts) != npnts)
				msg ("error while writing to output");
		}
		else
			if (write(file,buf,npnts2) != npnts2)
				msg ("error while writing to output");
	}
	gclose(image);
}

extern fout;

msg(emsg)
char *emsg;
{
	fout = 2;
	printf("SAVW --- %s\n", emsg);
	exit();
}
