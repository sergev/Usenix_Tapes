#
#define VSTRT   468
#define HSTRT   20
#define VINC    -9
#define HINC    7
#define LMARG   20
#define RMARG   490
#define TMARG   468
#define BMARG   20
#define CHGHT   9
#define CWDTH   7

int a[16];
int hpos;
int vpos;
int junk[2];
int sbuf[3];
char bch[2];
main(argc, argv)
char *argv[];
{
	register ch,t;

	gopen(a,0,0,512,512);
	genter(a,0,(argc > 1?atoi(argv[1]):0),0,1,t = (argc > 2? 1:0));
	bch[0] = bch[1] = 0;
	raw();
	t++;
	hpos = HSTRT;
	vpos = VSTRT;

	gwcur(a, 1, hpos, vpos, 1, 1);

	for(;;) {
		ch = getchar();
		if(ch < 040 || ch > 0137) {
			switch(ch) {
			case '\n':      /* DOWN */
			case '\r':     /* RETURN */
				if((vpos =+ VINC*t) < BMARG || vpos > TMARG)
					vpos = (vpos - 1) & 511;
				if(ch == '\r') hpos = HSTRT;
				gwcur(a, 1, hpos, vpos, 1, 1);
				continue;
			case 032:      /* UP */
				if((vpos =- VINC*t) < BMARG || vpos > TMARG)
					vpos = (vpos + 1) & 511;
				gwcur(a, 1, hpos, vpos, 1, 1);
				continue;
			case 010:       /* LEFT */
				if((hpos =- HINC*t) < LMARG || hpos > RMARG)
					hpos = HSTRT;
				gwcur(a, 1, hpos, vpos, 1, 1);
				continue;
			case 034:       /* RIGHT */
				if((hpos =+ HINC*t) < LMARG || hpos > RMARG)
					hpos = HSTRT;
				gwcur(a, 1, hpos, vpos, 1, 1);
				continue;
			case 033:	/* Allow track ball cursor positioning */
				grcur(a,&hpos,0);
				continue;
			case 0:
			case 0177:
				flush();
				cook();
				exit();
			default:
				continue;
			}
		}
		bch[0] = ch;
		gwstr(a, bch, hpos, vpos);
		if((hpos =+ HINC*t) < LMARG || hpos > RMARG) {
			hpos = HSTRT;
			if((vpos =+ VINC*t) < BMARG || vpos > TMARG)
				vpos = VSTRT;
		}
		gwcur(a, 1, hpos, vpos, 1, 1);
	}
}

raw()
{
	gtty(0, sbuf);
	sbuf[2] =| 040;
	sbuf[2] =& ~020;
	sbuf[2] =& ~010;
	stty(0, sbuf);
}

cook()
{
	gtty(0, sbuf);
	sbuf[2] =& ~040;
	sbuf[2] =| 020;
	sbuf[2] =| 010;
	stty(0, sbuf);
}
