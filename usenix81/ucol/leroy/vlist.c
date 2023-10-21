/*
 *	storage reference by variable name
 */


#define MAXEN	10
#define VINC	20

struct var {
	POINT v;
	char *l;
} zstoinit[] =
{
/* unit vectors */
	{ {0.,1.}, "up"},
	{ {1.,0.}, "over"},
	{ {0.,-1.}, "down"},
	{ {-1.,0.}, "back"},
/* rotate responses */
	{ { 1., 1. },  "yes"},
	{ { 0., 0. },  "no"},
/* page corners */
	{ {0.,0. },"LL"},
	{ {10.,7.5}, "UR"},
/* font names */
	{ { 0., 0. },  "default"},
	{ { 1., 1. },  "KR"},
	{ { 2., 2. },  "KG"},
	{ { 3., 3. },  "SR"},
	{ { 4., 4. },  "SG"},
	{ { 5., 5. },  "SS"},
	{ { 6., 6. },  "PIR"},
	{ { 7., 7. },  "PIG"},
	{ { 8., 8. },  "PII"},
	{ { 9., 9. },  "PNR"},
	{ { 10., 10. },  "PNG"},
	{ { 11., 11. },  "PNI"},
	{ { 12., 12. },  "DR"},
	{ { 13., 13. },  "CS"},
	{ { 14., 14. },  "CC"},
	{ { 15., 15. },  "TR"},
	{ { 16., 16. },  "TI"},
	{ { 17., 17. },  "GG"},
	{ { 18., 18. },  "GE"},
	{ { 19., 19. },  "GI"},
	{ { 20., 20. },  "MAP"},
	{ { 21., 21. },  "AY"},
/* line sizes in inches */
	{ { 0.10, 0.10 },  "huge"},
	{ { 0.05, 0.05 },  "heavy"},
	{ { 0.01, 0.01 },  "light"},
/* pen colors */
	{ { 1., 1. }, "black"},
	{ { 2., 2. }, "blue"},
	{ { 3., 3. }, "green"},
	{ { 4., 4. }, "red"},
/* line types */
	{ {0.,0.}, "solid"},
	{ {1.,1.}, "dot"},
	{ {2.,2.}, "dotdash"},
	{ {3.,3.}, "shortdash"},
	{ {4.,4.}, "longdash"},
/* variable list */
	{ { 3., 3. }, "font"},
	{ { 0.17, 0.17 }, "size"},
	{ { 0.30, 0.30 }, "vs"},
	{ { 0.30, 0.30 }, "hs"},
	{ { 1.00, 1.00 }, "aspect"},
	{ { 0.00, 0.00 }, "thick"},
	{ { 0., 0. }, "line"},
	{ { 1.00, 1.00 }, "slant"},
	{ { 0.00, 0.00 }, "angle"},
	{ { 0.25, 0.25 }, "space"},
	{ { 0.00, 0.00 }, "x"},
	{ { 0., 0. }, "rotate"},
	{ { 1., 1. }, "noclip"},
	{ { 0., 0. }, "cmode"},
	{ { 1., 1. }, "repeat"},
	{ { 30., 30. }, "speed"},
	{ { 1., 1. }, "color"},
	{ { 0., 0. }, "ticks"},
	{ { 1., 1. }, "totick1"},
	{ { 1., 1. }, "nexttick"},
	{ { 0.2, 0.2}, "ticksize"},
	{ { 0.05, 0.05}, "tickthick"},
	{ { 0., 0.}, "style" },
	{ { 0.5, 0.5}, "margin" },
	{ { 0.,0.}, "cfill" },
	{ { 20.,20.}, "narcs" },
	{ { 1.,1.}, "graph_scale" },
	{ { 1.,0.}, "graph_rotate" },
	{ { 0.,0.}, "graph_offset" },
};

struct var *z = &zstoinit[0];
int nz	=	sizeof(zstoinit)/sizeof(struct var);
int tz	=	sizeof(zstoinit)/sizeof(struct var)-1;
int env	=	0;
int zb[MAXEN]	=	{0};
int zs;
int moved	=	0;

POINT fromst[MAXEN];
POINT tost[MAXEN];
POINT lleftst[MAXEN];
POINT urightst[MAXEN];


float vget(s)
char *s;
{
	POINT x, vgetp();

	x = vgetp(s);
	return(x.xp);
}

POINT vgetp(s)
char *s;
{
	int i;

	for( i=tz; i>=zb[0]; i-- )
		if( strcmp(z[i].l,s)==0 ) {
			if(yydebug!=0) {
				fprintf(stderr,"vlist:get:%s=(%f,%f)\n",
					s,z[i].v.xp,z[i].v.yp);
			}
			return( z[i].v );
		}

	verror(s);
	return(z[0].v);
}
long lget(s)
char *s;
{
	float x;
	x = vget(s);
	if( x < 0. ) x -= 0.01;
	else x += 0.01;
	return( (long) x );
}

float vput(s,x)
char *s;
float x;
{
	POINT y, vputp();
	y.xp=y.yp=x;
	vputp(s,y);
	return( x );
}

POINT vputp(s,x)
char *s;
POINT x;
{
	struct var *q;
	int found;

	found=0;
	for( zs=tz; zs >= zb[env]; zs-- )
		if( strcmp(z[zs].l,s)==0 ) {
			z[zs].v=x;
			found++;
		}

	if(found==0) {
		if( tz >= nz-1 ) {
			int i;
			q = (struct var *)malloc((unsigned)(sizeof(struct var)*(nz+VINC)));
			for(i=0; i<nz; i++) q[i]=z[i];
			if( moved++>0 ) free((char *)z);
			z=q;
			nz += VINC;
		}

		z[++tz].l = (char *)malloc( (unsigned)(strlen(s)+1) );
		strcpy(z[tz].l,s);
		z[tz].v=x;

	}
	if(yydebug!=0) {
		fprintf(stderr,"vlist:put:%s=(%f,%f)\n",
			s,x.xp,x.yp);
		}
	return(x);
}
verror( s )
char *s;
{
	fprintf( stderr, "vlist error:%s\n", s);
	exit(0);
	return(0);
}

vlist()	/* display current list */
{
	int i, n, t;

	t=tz;
	for(n=env; n>=0; n--) {
		fprintf(stderr,"environment %d\n",n);
		for( i=t; i>=zb[n]; i--) {
			fprintf(stderr,"(%7.3f,%7.3f) %-11s",z[i].v.xp,z[i].v.yp,z[i].l);
			if( (i+1)%2 == 0 ) fprintf( stderr,"\n");
		}
		if( (i+1)%2==0 ) fprintf( stderr,"\n");
	}

	return( 0 );
}

POINT valter(s,dx,dy)
char *s;
float dx,dy;
{
	POINT x;
	x=vgetp(s);
	x.xp += dx;
	x.yp += dy;
	return(vputp(s,x));
}

vpush()
{
	if( env>=MAXEN-1 ) verror("no env left");
	else {
		fromst[env]=from;
		tost[env]=to;
		lleftst[env]=lleft;
		urightst[env]=uright;
		zb[++env]=tz+1;
	}
	return(0);
}

vpop()
{
	if( env <= 0 ) return(0);
	else {
		for( ; tz >= zb[env]; tz-- )  free(z[tz].l);
		env--;
		from=fromst[env];
		to=tost[env];
		lleft=lleftst[env];
		uright=urightst[env];
		page(from,to);
		map(lleft,uright);
	}
	return(0);
}

