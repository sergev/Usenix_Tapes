#
		/* Written by Russ Smith --- C.V.L. */

/* To compile -- cc -O -s ersw.c -lg; mv a.out /usr/bin/ersw */

int image[16];
int overlay[16];
int ncol;
int nrow;
int cursors[4];
main(argc, argv)
char *argv[];
{
	register *cur, bits, iflg;

	gopen(overlay, 0, 0, 512, 512);
	genter(overlay,4,0,0,0,0);

	cur = cursors;

	grcur(overlay,cur,0);   /* Get the window to clear */

	if((cur[0] > cur[2]) || (cur[1] > cur[3]))
		msg("Window undefined\nUse posw to define window");

	ncol = cur[2] - cur[0] + 1;
	nrow = cur[3] - cur[1] + 1;

	gopen(image, cur[0], cur[1], ncol, nrow);

	iflg = 1;
	switch(argv[1][0]) {
	case 0 :
	case 'a' :
		bits = 0;
		break;
	case 'o' :
		iflg = 0;
		break;
	case 'r' :
		bits = 00017;
		break;
	case 'g' :
		bits = 00360;
		break;
	case 'b' :
		bits = 07400;
		break;
	case 'l' :
		if(argv[1][1] != '8')
			bits = 00077;
		else
			bits = 00377;
		break;
	case 'u' :
		if(argv[1][1] != '8')
			bits = 07700;
		else
			bits = 07760;
		break;
	default :
		msg("Usage == ersw [<key>]\n\t (key = a o r g b u6 u8 l6 l8)");
	}


	if(iflg) {
		gclear(overlay, 0, 0, 512, 512, 0);

		gwvec(overlay, cur[0], cur[1], cur[0], cur[3], 0);
		gwvec(overlay, cur[0], cur[1], cur[2], cur[1], 0);
		gwvec(overlay, cur[2], cur[3], cur[2], cur[1], 0);
		gwvec(overlay, cur[2], cur[3], cur[0], cur[3], 0);
	}
	else {
		gwcur(overlay,1,cur[0],cur[1],1,1);
		gwcur(overlay,2,cur[2],cur[3],1,1);
		genter(image,4,0,0,0,0);
		bits = 0;
	}

	gclear(image, 0, 0, ncol, nrow, bits);
}

extern fout;

msg(emsg)
char *emsg;
{
	fout = 2;
	printf("ERSW --- %s\n", emsg);
	exit();
}
