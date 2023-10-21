#
		/* Written by Russ Smith --- C.V.L. */

/******** THIS PROGRAM IS INCOMPLETE AND WILL NOT WORK *********/

/* To compile -- cc -O -s getw.c -lg; mv a.out /usr/bin/getw */

int overlay[16];

int cursors[4];

main(argc,argv)
{
	register *cur;

	cur = cursors;

	gopen(overlay,0,0,512,512);
	genter(overlay,4,0,0,0,0);
	grcur(overlay,cur,0);

	if((cur[0] > cur[2]) || (cur[1] > cur[3]))
		msg("Window undefined\nUse posw to define window");

	if(argc == 1) {
		gclear(overlay, 0, 0, 512, 512, 0);

		gwvec(overlay, cur[0], cur[1], cur[0], cur[3], 0);
		gwvec(overlay, cur[0], cur[1], cur[2], cur[1], 0);
		gwvec(overlay, cur[2], cur[3], cur[2], cur[1], 0);
		gwvec(overlay, cur[2], cur[3], cur[0], cur[3], 0);
	}
	else {
		gwcur(overlay,1,cur[0],cur[1],1,1);
		gwcur(overlay,2,cur[2],cur[3],1,1);
	}

	msg(stringf("fcol = %d  frow = %d  ncol = %d  nrow = %d",cur[0],cur[1],(cur[2]-cur[0])+1,(cur[3]-cur[1])+1));
}

extern fout;

msg(emsg)
char *emsg;
{
	fout = 2;
	printf("GETW --- %s\n", emsg);
	exit();
}

