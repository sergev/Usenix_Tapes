int a[16];
int cur[4];
int flag;
int abs();
double sqrt();
rubout(){flag = 0;}
main(argc,argv)
{
	register i, j, k;
	int cx, cy, color, rad, q;
	double rsqr, xsqr, ysqr, xval, yval;

	q = 0;
	if(argc > 1) q = 1;

	gopen(a,0,0,512,512);
	gwcur(a,1,50,240,1,1);
	gwcur(a,2,460,240,1,1);
loop:
	signal(2, rubout);
	flag = 1;
	while(flag)
		grcur(a,cur,1);

	cx = cur[0] + (cur[2] - cur[0]) / 2;
	cy = cur[1] + (cur[3] - cur[1]) / 2;
	xval = abs(cur[0] - cur[2]);
	yval = abs(cur[1] - cur[3]);
	rsqr = (xval*xval + yval*yval) / 4;
	rad = sqrt(rsqr);

	for(i=rad;i>0;i--) {
		xval = i;
		xsqr = xval * xval;
		ysqr = rsqr - xsqr;
		yval = sqrt(ysqr);
		j = yval;
		genter(a,0,rand() & 07777,0,0,0);
		gwvec(a,cx,cy,cx+i,cy+j,q);
		genter(a,0,rand() & 07777,0,0,0);
		gwvec(a,cx,cy,cx+i,cy-j,q);
		genter(a,0,rand() & 07777,0,0,0);
		gwvec(a,cx,cy,cx-i,cy+j,q);
		genter(a,0,rand() & 07777,0,0,0);
		gwvec(a,cx,cy,cx-i,cy-j,q);
		genter(a,0,rand() & 07777,0,0,0);
		gwvec(a,cx,cy,cx+j,cy+i,q);
		genter(a,0,rand() & 07777,0,0,0);
		gwvec(a,cx,cy,cx+j,cy-i,q);
		genter(a,0,rand() & 07777,0,0,0);
		gwvec(a,cx,cy,cx-j,cy+i,q);
		genter(a,0,rand() & 07777,0,0,0);
		gwvec(a,cx,cy,cx-j,cy-i,q);
	}
	goto loop;
}
