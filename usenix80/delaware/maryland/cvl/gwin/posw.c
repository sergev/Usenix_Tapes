#
		/* Written by Russ Smith --- C.V.L. */

/* To compile -- cc -O -s posw.c -lg; mv a.out /usr/bin/posw */

int overlay[16];

int cold[4] { 0, 0, 0, 0};
int cnew[4];

int ncol 100;
int nrow 100;

int predef 0;
int wflg 0;

main(argc,argv)
char *argv[];
{
	register *cur;

	gopen(overlay, 0, 0, 512, 512);
	genter(overlay, 4, 0, 0, 0, 0);

	cur = cnew;

	if(argv[0][0] == 'o') wflg++;   /* If 'oposw', overlay kept intact */

	switch (argc) {

	case 1: /* Dynamic window size and position */
		break;
	case 3: /* Dynamic window position only */
		getn(argv);     /* Get the ncol and nrow parameters */
		predef = 1;
		break;
	case 5: /* Static window size and position */
		cur[0] = atoi(argv[1]) & 511;
		cur[1] = atoi(argv[2]) & 511;
		getn(&argv[2]);
		if((cur[0] < 0) || (cur[2] > 511))
			msg("window size out of range");
		if((cur[1] < 0) || (cur[3] > 511))
			msg("window size out of range");
		gwcur(overlay,1,cur[0],cur[1],wflg,1);
		gwcur(overlay,2,cur[2],cur[3],wflg,1);
		if(!wflg) win();  /* Draw the window outline */
		exit();
	default:
		msg("Usage == posw [[fcol frow] ncol nrow]");
	}
	posw();
	msg(stringf("fcol = %d  frow = %d  ncol = %d  nrow = %d",cur[0],cur[1],(cur[2]-cur[0])+1,(cur[3]-cur[1])+1));
}

getn(argv)
char *argv[];
{
	register *cur;

	cur = cnew;

	if(((ncol = atoi(argv[1])) <= 0) || (ncol > 512)) {
		msg("ncol out of range");
		exit();
	}
	if(((nrow = atoi(argv[2])) <= 0) || (nrow > 512)) {
		msg("nrow out of range");
		exit();
	}
	cur[2] = cur[0] + ncol - 1;
	cur[3] = cur[1] + nrow - 1;
}

posw()
{
	grcur(overlay, cnew, 0);        /* Read current cursor positions */

	gwcur(overlay, 1, cnew[0], cnew[1], wflg, 1);
	gwcur(overlay, 2, cnew[2], cnew[3], wflg, 1);

	if(!wflg) win();  /* Draw the initial window */

	grcur(overlay, cnew, 0);

	while(compare()) {
		if(!wflg) win();
	}
	gwcur(overlay, 1, cnew[0], cnew[1], wflg, 1);
	gwcur(overlay, 2, cnew[2], cnew[3], wflg, 1);

	if(!wflg) win();  /* Draw the final window */
}

compare()
{
	register *o, *n, *in;
	int flg;
	int cur[4];

	n = cnew;
	o = cold;
	in = cur;

	flg = 1;

	grcur(overlay, in, 1);   /* Wait for asynchronous interrupt */

	if(predef) {
		in[2] = ((in[0] + ncol) - 1) & 511;
		in[3] = ((in[1] + nrow) - 1) & 511;
	}

	if((in[0] > in[2]) || (in[1] > in[3])) {
		gwcur(overlay, 1, n[0], n[1], wflg, 1);
		gwcur(overlay, 2, n[2], n[3], wflg, 1);
		return(1);
	}

	if((o[0] == n[0]) && (o[1] == n[1]) && (o[2] == n[2]) && (o[3] == n[3]))
		flg = 0;

	o[0] = n[0]; o[1] = n[1]; o[2] = n[2]; o[3] = n[3];
	n[0] = in[0]; n[1] = in[1]; n[2] = in[2]; n[3] = in[3];

	return(flg);
}

win()
{
	register *cur;

	cur = cnew;

	gclear(overlay, 0, 0, 512, 512, 0);

	gwvec(overlay, cur[0], cur[1], cur[0], cur[3], 0);
	gwvec(overlay, cur[0], cur[1], cur[2], cur[1], 0);
	gwvec(overlay, cur[2], cur[3], cur[2], cur[1], 0);
	gwvec(overlay, cur[2], cur[3], cur[0], cur[3], 0);
}

extern fout;

msg(emsg)
char *emsg;
{
	fout = 2;
	printf("POSW --- %s\n", emsg);
	exit();
}

