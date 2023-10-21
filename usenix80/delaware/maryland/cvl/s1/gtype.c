#

		/* Written by Russ Smith --- C.V.L. */

#define VSTRT   460
#define HSTRT   0
#define MINV    0
#define MAXV    50
#define MINH    0
#define MAXH    72

int image[16];
int hpos 0;
int vpos 0;
int minv MINV;
int maxv MAXV;
int minh MINH;
int maxh MAXH;
int vstart VSTRT;
int hstart HSTRT;

int zflg 0;
int col HSTRT;
int row VSTRT;
int dsize;

int sbuf1[3];
int sbuf[3];
char lbuf[80];
main(argc, argv)
char *argv[];
{
	register char ch, *cp;
	register mask;

	gopen(image,0,0,512,512);
	genter(image,0,0,0,1,0);

	raw();

	poscur(5);      /* Single size characters */

	for(;;) {
		ch = getchar();
		switch(ch) {
		case 015:       /* RETURN */
			poscur(0);
			break;
		case 012:       /* DOWN */
			poscur(1);
			break;
		case 032:       /* UP */
			poscur(2);
			break;
		case 010:       /* LEFT */
			poscur(3);
			break;
		case 034:       /* RIGHT */
			poscur(4);
			break;
		case 033:       /* Change character size */
			dsize = 1 - dsize;
			poscur(5);
			break;
		case 03:        /* Change color */
			genter(image,3,0,0,0,0);
			while((ch = getchar()) == ' ');
			switch(ch) {
			case 'r':
				mask = 017;
				break;
			case 'g':
				mask = 0360;
				break;
			case 'b':
				mask = 07400;
				break;
			case 'a':
			default:
				mask = 07777;
				break;
			case 'o':
				mask = 07777;
				genter(image,4,0,0,0,0);
				break;
			}
			genter(image,0,mask,0,0,0,0);
			while(getchar() != '\r');
			break;
		case 021:       /* Cursor positioning */
			while((ch = getchar()) < '0' || ch > '9');
			cp = lbuf;
			*cp++ = ch;
			while((*cp = getchar()) > '0' || (*cp < '9')) cp++;
			ch = *cp;
			*cp = 0;
			if((col = atoi(lbuf)) > (maxh * (dsize+1) * 7)) goto yuk;
			cp = lbuf;
			while((ch = getchar()) < '0' || (ch > '9'));
			*cp++ = ch;
			while((*cp = getchar()) > '0' || (*cp < '9')) cp++;
			ch = *cp;
			*cp = 0;
			if((row = atoi(lbuf)) > (maxv * (dsize+1) * 9)) goto yuk;
			compute();
			poscur(6);
		yuk:
			if(ch != '\r')  while(getchar() != '\r');
			break;
		case 0177:      /* exit */
			flush();
			cook();
			exit();
		default:        /* All other characters */
			ch =& 0137;
			if(ch < ' ') break;
			lbuf[0] = ch;
			lbuf[1] = 0;
			gwstr(image, lbuf, col, row);
			poscur(7);
		}       /* End case statement */
	}       /* end for loop */
}

raw()
{
	gtty(0, sbuf1);

	sbuf[0] = sbuf1[0];
	sbuf[1] = sbuf1[1];
	sbuf[2] = sbuf1[2];

	sbuf[2] =| 040;
	sbuf[2] =& ~020;
	sbuf[2] =& ~010;

	stty(0, sbuf);
}

cook()
{
	stty(0, sbuf1);
}
