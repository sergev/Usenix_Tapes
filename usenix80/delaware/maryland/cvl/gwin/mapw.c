#
		/* Written by Russ Smith --- C.V.L. */

/* To compile -- cc -O -s mapw.c -lg; mv a.out /usr/bin/mapw */


#define TABSIZE 4096

int table[TABSIZE];

int image[16];
int overlay[16];

int cursors[4];

int col[512];

int ncol;
int nrow;

main(argc,argv)
char *argv[];
{
	register *cur;
	register i, j;

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

	ncol = cur[2] - cur[0] + 1;
	nrow = cur[3] - cur[1] + 1;

	gopen(image, cur[0], cur[1], ncol, nrow);

	grtab(image,table,0,TABSIZE);

	for(i=0;i<ncol;i++) {
		grcol(image,col,i,0,nrow,1);
		for(j=0;j<nrow;j++)
			col[j] = table[col[j]];
		gwcol(image,col,i,0,nrow,1);
	}

	msg("Window mapping complete\07");
}

extern fout;

msg(emsg)
char *emsg;
{
	fout = 2;
	printf("MAPW --- %s\n", emsg);
	exit();
}
