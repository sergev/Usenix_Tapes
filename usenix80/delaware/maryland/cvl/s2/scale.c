#

		/* Written by Russ Smith --- C.V.L. */

#define DIFF    100

int a[16];              /* Grinnell functions buffer */
int xc;                 /* Current x position */
int yc;                 /* Current y position */
int junk[2];
int tab[4096];          /* Lookup table */
int xp 0;               /* Previous x position */
int yp 0;               /* Previous y position */

main()
{
	register i, j, k;
	int loc, lo, hi, b;
	double slope, f1, f2;

	gopen(a,0,0,512,512);
	genter(a,4,0,0,0,0);
	gwcur(a,1,0,0,1,1);
	grcur(a,&xc,0);    /* Read the cursors to reset the interrupt flag */

	for(i=0;i<4096;i++)
		tab[i] = i;
	for(;;) {
		grcur(a,&xc,1);         /* Get the current cursor location */

		if((abs(i = xc - xp) < DIFF) && (abs(j = yc - yp) < DIFF)) {
			if(i == 0) {   /* Vertical line */
				i = xc * 8;
				j = yc * 8;
				for(k=0;k<8;k++)
					tab[i+k] = j;
			}
			else {
				hi = xp < xc? xc : xp;
				lo = xp < xc? xp : xc;
				if(j == 0) {   /* Horizontal line */
					i = yc * 8;
					for(loc = lo;loc <= hi; loc++) {
						j = loc * 8;
						for(k=0;k<8;k++)
							tab[j+k] = i;
					}
				}
				else {  /* Diagonal line */
					slope = (f1 = j) / (f2 = i);  /* Slope = dy/dx */
					b = lo == xp? yp : yc;
					for(i=lo;i<=hi;i++) {
						loc = i * 8;
						j = (slope * (f1 = i - lo) + (f2 = b)) * 8.;/* y = ax + b... */
						for(k=0;k<8;k++)
							tab[loc+k] = j + k;
					}
				}
			}
		}
		else {  /* Distance too great to interpolate */
			i = (xp = xc) * 8;      /* Don't forget to set xp and yp */
			j = (yp = yc) * 8;
			for(k=0;k<8;k++)
				tab[i+k] = j + k;
			gclear(a,xc,0,1,512,0);
			gwpnt(a,07777,xc,yc);
			gwtab(a,tab,0,4096);
			continue;
		}
		i = xc < xp? xc : xp;
		gclear(a,i,0,abs(xc-xp),512, 0);
		gwvec(a,xp,yp,xc,yc,0);
		gwtab(a,tab,0,4096);
		xp = xc;
		yp = yc;
	}
}
