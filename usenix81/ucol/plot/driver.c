/*	driver.c modified by D. Harvey april 5, 1980 to get input from
	pread.c		*/
/*	modified may 10, 1980 to use character fonts from hershy fonts instead
	of internally generated characters  */

#include <math.h>
#include <stdio.h>

int fixflag;
int charwidth,bxoff,axoff;
int iflag = 0;
int xlenoff = 0;
int ylenoff = 0;
int isize = 1;
int textref = 0;
int textlen;
int ifirst = 1;
int xmin,ymin,xmax,ymax;
int lastx,lasty;
float deltx;
float delty;

double del;
double angle;
double slant = 0.0174532925;
double rad = .0174532925;
double height = 50.;
double ratio = 1.;
double tanslant = 0.0174550649;
double sinangle = 0.;
double cosangle = 1.;
double xstart,ystart;
double hscale,vscale;

int ffnt = 1;
int yht;

main(argc,argv)  char **argv; {
	int std=1;
	int fin;

	while(argc-- > 1) {
		if(*argv[1] == '-')
			switch(argv[1][1]) {
				case 'l':
					deltx = atoi(&argv[1][2]) - 1;
					break;
				case 'w':
					delty = atoi(&argv[1][2]) - 1;
					break;
				}

		else {
			std = 0;
			if ((fin = open(argv[1], 0)) == -1) {
				fprintf(stderr, "can't open %s\n", argv[1]);
				exit(1);
				}
			fplt(fin);
			}
		argv++;
		}
	if (std)
		fplt( 0 );
	exit(0);
	}


fplt(fin)  int fin; {
	int c;
	char s[256];
	int xi,yi,x0,y0,x1,y1,r,dx,n,i;
	int pat[256];
	double ddgg2;
	int pread(); float xfp[10]; long l; long sl;
	double sin(),cos();

	l = 256;
	hindex(ffnt, s[0]);
	vscale = height/yht;
	hscale = height*ratio;
	openpl();
	while((c=pread(pat, xfp, s, &sl, l, fin)) != -1){
		switch(c){
		/* case 'm' */
		case 1:
			lastx = pat[0];
			lasty = pat[1];
			move(lastx,lasty);
			iflag = 0;
			break;
		/* case 'l' */
		case 4:
			x0 = pat[0];
			y0 = pat[1];
			lastx = pat[2];
			lasty = pat[3];
			line(x0,y0,lastx,lasty);
			iflag = 0;
			break;
		/* case 't' */
		case 5:
			if (isize == 0)  {
				s[ (long) sl] = '\0';
				label(s);
				break;
			}
			if (ifirst == 1) {
				initfonts();
				ifirst = 0;
			}
			if (iflag == 0)  {
				xstart = lastx;
				ystart = lasty;
			}
			hindex(ffnt,s[0]);
			vscale = height/yht;
			hscale = vscale*ratio;
			textlen = 0;
			fixflag = 1;
			if (textref > 2) {
				for (y1=0;y1<(long)sl;y1++) {
					if (rchar(hindex(ffnt,s[y1])) > 0) {
						up_width(-bxoff+axoff);
						textlen += charwidth;
					}
				}
			}
			i = textref/3; n = textref - i*3;
			ylenoff = yht*n/2;
			xlenoff = textlen*i/2;
			for (y1 = 0; y1 < (long) sl; y1++) {
				if (rchar(hindex(ffnt,s[y1])) > 0) makec();
			}
			iflag = 1;
			break;
		/* case 'e' */
		case 8:
			erase();
			break;
		/* case 'p' */
		case 3:
			lastx = pat[0];
			lasty = pat[1];
			point(lastx,lasty);
			iflag = 0;
			break;
		/* case 'n' */
		case 2:
			lastx = pat[0];
			lasty = pat[1];
			cont(lastx,lasty);
			iflag = 0;
			break;
		/* case 's' */
		case 10:
			xmin = pat[0];
			ymin = pat[1];
			xmax = pat[2];
			ymax = pat[3];
			space(xmin,ymin,xmax,ymax);
			del = ymax - ymin;
			break;
		/* case 'a' */
		case 6:
			xi = pat[0];
			yi = pat[1];
			x0 = pat[2];
			y0 = pat[3];
			x1 = pat[4];
			y1 = pat[5];
			arc(xi,yi,x0,y0,x1,y1);
			break;
		/* case 'c' */
		case 7:
			xi = pat[0];
			yi = pat[1];
			r = pat[2];
			circle(xi,yi,r);
			break;
		/* case 'f' */
		case 9:
			switch(pat[0])	{
				case 1:
					linemod("dotted");
					break;
				case 2:
					linemod("solid");
					break;
				case 3:
					linemod("longdashed");
					break;
				case 4:
					linemod("shortdashed");
					break;
				case 5:
					linemod("dotdashed");
					break;
				}
			break;
		/* case 'b' */
		case 11:
			use(xfp);
			break;
		/* case 'o' */
		case 12:
			speed(pat[0]);
			break;
		/* case 'g' */
		case 13:
			mapuu(xfp);
			break;
		/* case 'h' */
		case 14:
			pen(pat[0]);
			break;
		/* case 'd' */
		case 19:
			xi = pat[0];
			yi = pat[1];
			dx = pat[2];
			n = pat[3];
			for(i=0; i<n; i++)pat[i] = pat[i+4];
			dot(xi,yi,dx,n,pat);
			break;
		/* case 'i' */
		case 15:
			ffnt = pat[0];
			if (ffnt == 0) break;
			isize = 1;
			hindex(ffnt,s[0]);
			vscale = height/yht;
			hscale = vscale*ratio;
			break;
		/* case 'j' */
		case 16:
			isize = 1;
			height = xfp[0];
			ratio = xfp[1];
			hindex(ffnt,s[0]);
			vscale = height/yht;
			hscale = vscale*ratio;
			slant = xfp[2];
			slant *= rad;
			tanslant = sin(slant)/cos(slant);
			break;
		/* case 'k' */
		case 17:
			isize = 1;
			angle = xfp[0];
			angle *= rad;
			sinangle = sin(angle);
			cosangle = cos(angle);
			break;
		/* case 'q' */
		case 18:
			textref = pat[0];
			break;
			}
		}
	closepl();
}
