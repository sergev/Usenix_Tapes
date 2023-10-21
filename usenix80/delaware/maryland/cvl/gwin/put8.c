#
#define SHFT 4
		/* Written by Russ Smith --- C.V.L. */
int a[16];
int header[6];
int buf[512];
char cbuf[1024];
int cur[4];
int fcol;
int frow;
int ncol;
int nrow;
int rcnt;
int flag;
char *argv0;
rubout(){flag = 0;}
main(argc, argv)
char *argv[];
{
	register i, j, k;

	gopen(a,0,0,512,512);

	if(argc != 3) {
		grcur(a,cur,0);
		gwcur(a,1,cur[0],cur[1],1,1);

		signal(2, rubout);

		flag = 1;
		while(flag)
			grcur(a,cur,1);

		fcol = cur[0];
		frow = cur[1];
	}
	else {
		fcol = abs(atoi(argv[1]));
		frow = abs(atoi(argv[2]));
	}

	if (readrow(cbuf,12) != 0) msg("End of file while reading header");

	for(j=0;j<6;j++) {
		k = j * 2;
		header[j] = (cbuf[k] & 0377) | (cbuf[k+1] << 8);
	}

	ncol = min(header[1], 512 - fcol);
	nrow = min(header[3], 512 - frow);

	rcnt = header[1];
	if(header[5] == 1) {
		for(i=0;i<nrow;i++) {

			if (readrow (cbuf,rcnt) != 0) msg( stringf("End of file while reading row %d",i));

			for(j=0;j<ncol;j++)
				buf[j] = ((cbuf[j] & 0377) << SHFT);
			gwrow(a, buf, frow + i, fcol, ncol, 1);
		}
	}
	else {
		rcnt =* 2;
		for(i=0;i<nrow;i++) {

			if (readrow (cbuf,rcnt) != 0) msg( stringf("End of file while reading row %d",i));

			for(j=0;j<ncol;j++) {
				k = j * 2;
				buf[j] = (cbuf[k] & 0377) | (cbuf[k+1] << 8);
			}
			gwrow(a, buf, frow + i, fcol, ncol, 1);
		}
	}
}

int min(u,v){if(u<v)return(u);return(v);}

extern fout;

msg(emsg)
char *emsg;
{
	fout = 2;
	printf("PUT8 --- %s\n", emsg);
	exit();
}
