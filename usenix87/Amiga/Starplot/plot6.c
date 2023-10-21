#include <stdio.h>
#include <math.h>

struct ent{
    float ra,dc;
    short mg,cl;
};

extern int MAX_X,MAX_Y;

/* 18.5cm/screen Y 26.2cm/screen X -> aspect ratio 1.4162 */

#define ASPECT 1.4162

/* MATH!

   spherical to rectangular coordinates.
   earth's axis is z axis
   dc measured from positive (north) z axis
   ra measured from x axis (we'll fix that later)
   All stars are on a unit sphere.
   
   x = sin(pi/2-dc)cos(ra)
   y = sin(pi/2-dc)sin(ra)
   z = cos(pi/2-dc)
   
   rotation rule (about origin [ie about z axis])
   
   a' = a cos(th) - b sin(th)
   b' = a sin(th) + b cos(th)
   
   rotate to see desired line of longitude(ra0).
   ie line up y axis with chosen longitude (remember we were measuring from x)
   rotate counter clockwise pi/2, rotate clockwise ra0 about z (ie pi/2-ra0)

   x' = sin(pi/2-dc)cos(ra)cos(pi/2-ra0) - sin(pi/2-dc)sin(ra)sin(pi/2-ra0)
      = sin(pi/2-dc)cos(ra+pi/2-ra0) [believe me!]
   y' = sin(pi/2-dc)cos(ra)sin(pi/2-ra0) + sin(pi/2-dc)sin(ra)cos(pi/2-ra0)
      = sin(pi/2-dc)sin(ra+pi/2-ra0)
   z' = cos(pi/2-dc)
   
   now rotate down from the north pole along your longitude to desired latitude
   ie rotate down pi/2 then up by dc0
   we are rotating around the x axis
   
   x" = sin(pi/2-dc)cos(ra+pi/2-ra0)
   y" = sin(pi/2-dc)sin(ra+pi/2-ra0)cos(pi/2-dc0)-cos(pi/2-dc)sin(pi/2-dc0)
      = cos(dc)sin(ra+pi/2-ra0)cos(pi/2-dc0)-sin(dc)sin(pi/2-dc0)
   z" = sin(pi/2-dc)sin(ra+pi/2-ra0)sin(pi/2-dc0)+cos(pi/2-dc)cos(pi/2-dc0)
      = cos(dc)sin(ra+pi/2-ra0)sin(pi/2-dc0) + sin(dc)cos(pi/2-dc0)
   
   I set dc0 to pi/2-dc0 and ra0 ot pi/2-ra0.
   a = sin(dc)		b = cos(dc)
   c = sin(dc0)		d = cos(dc0)
   e = sin(ra + ra0)	f = cos(ra + ra0)
   
   x" = df;
   y" = deb-ca;
   z" = dea+cb;
   
   Now! viewing transforms.  I assume we are at (0,0,0) looking at ra0 x dc0.
   We are looking down the z axis with y up.
   
   our famous Ys = d/s Ye/Ze view box transform gives us:
   (ie Y in screen coordinates from eye coordinates)
   
   Xs = df/(dea+cb);
   Ys = (deb-ca)/(dea+cb);
   
   first we discard any stars behind us (ie z < 0)
   if the star is more than half a screen to the side, discard it.
   NB: we need not calculate f until the last moment.
   
   divide x by the aspect ratio, then map to physical coordinates.
   
   the rest is a subjective star plotting mechanism
*/

#define mabs(x) (((x)<0.0)?-(x):(x))
#define mfloor(x) (((x)<0.0)?-(int)-(x):(int)(x))


main(argc,argv)
int argc;
char **argv;
{
    FILE *fp;
    struct ent *ar,*star;
    int t,i,num;
    double a,b,c,d,e,f,de,ra0,de0,wi,xt,yt;
    double HLF;
    double z;
    double TO_RADS;
    
    if (argc <= 1) {
        printf("usage plot <file-list>\n");
	exit(0);
    }
    
    /* beware the ffp constant expression bug! */
    TO_RADS  = (TO_RADS=2.0) * 3.1415926535 / (TO_RADS=360.0);
    
    HLF = 90.0 * TO_RADS;
    
    printf("ra,de,wi [degrees]:");fflush(stdout);
    scanf("%lf %lf %lf",&ra0,&de0,&wi);
    ra0 = HLF-ra0*TO_RADS;
    de0 = HLF-de0*TO_RADS;
    wi = tan(mabs(wi)*TO_RADS);

    window_init();
    a = sin(de0); b = cos(de0);

    while (argc-- > 1){
        fp = fopen(argv[argc],"r");
	
	if (fp == NULL){
	    printf("cant find %s\n",argv[argc]);
	    exit(0);
	}
	
	t = fread((char*)&num,4,1,fp);
	if (t != 1) {
	    printf("didn't read file size!\n");
	    exit(0);
	}
	
	printf("reading %d records\n",num);
	fflush(stdout);
	
	ar = (struct ent*)malloc(num*sizeof(struct ent));
	if (ar == NULL){
	    printf("couldn't malloc %d bytes\n",num*sizeof(struct ent));
	    exit(0);
	}
	
	t = fread((char*)ar,sizeof(struct ent),num,fp);
	fclose(fp);
	
	printf("read %d\n",t);
	
	for (i = 0;i<t;i++) {
	    check_input();
	    star = ar+i;
	    
	    c = sin(star->dc);
	    d = cos(star->dc);
	    
	    e = sin(star->ra + ra0);
	    
	    de = d * e;
	    z = wi*(de * a + c * b);
	    if (z <= 1.0e-16) continue;	/* behind us, so skip it */
	    
	    yt = (de * b - c * a) / z;
	    if (yt < -0.5 || yt > 0.5) continue;
	    
	    f = cos(star->ra + ra0);
	    
	    xt = d * f / ASPECT / z;
	    if (xt < -0.5 || xt > 0.5) continue;
	    
	    plot_star((int)((xt + 0.5)*MAX_X),
	          (int)((yt + 0.5)*MAX_Y),star->mg,star->cl);
	}
	printf("done\n");
	fflush(stdout);
	free (ar);
	
    }
    while (1){
        wt();
	check_input();
    }
}

plot_star(x,y,mg,cl)
int x,y;
short mg,cl;
{
    int c,colour,m;
    
    m = (int)mfloor(((double) mg)/100.0+0.49);
    
    c = (int)mfloor(cl/60.0+0.49) + 1;

    if (c < 0) c = 0;
    else if (c > 3) c = 3;
    
    c = 1 + 3 * c + 2;/*dimmest of range*/
    
    colour = c-1;/* medium */
    
    switch(m){
        case  0: point(x-3,y,colour);
        	 point(x-4,y,colour);
		 point(x+3,y,colour);
		 point(x+4,y,colour);
		 point(x+5,y,colour);
		 point(x-5,y,colour);
		 colour = c-2;
		 point(x+2,y+1,colour);
		 point(x+2,y-1,colour);
		 point(x-2,y+1,colour);
		 point(x-2,y-1,colour);
	case 1:	 point(x,y+2,colour);
		 point(x,y-2,colour);
		 colour = c-2;
	case  2: /* in bright colour */
		 point(x+3,y,colour);
		 point(x-3,y,colour);
		 point(x+2,y,colour);
		 point(x-2,y,colour);
		 colour = c-2;
		 point(x+1,y+1,colour);
		 point(x+1,y-1,colour);
		 point(x-1,y+1,colour);
		 point(x-1,y-1,colour);
		 /* in brightest colour */
	case  3: /* in dimmest colour */
		 colour = c-2;
		 /* brighten colour */
	case  4: /* in dimmest colour*/
		 point(x,y+1,colour);
		 point(x,y-1,colour);
		 point(x+1,y,colour);
	         point(x-1,y,colour);
		 point(x+2,y,colour);
	         point(x-2,y,colour);
	case  5: colour = c-2;/* set to brightest */
		 break;
	case  6: colour = c-1;
		 break;
        default: colour = c;/* in dimmest colour */
    }
    point(x,y,colour);

}
