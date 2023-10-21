#
/*
 *      Star Trek
 *              by    Robert Haar, Computer Vision Lab., U. of Md.
 *
 *
 */


/* -------------------------------------------------

	functions to output to Grinnell video display

	due to Russell Smith
  --------------------------------------------------       */


#include "param"       /* bring in standard parameter file */


	/*----- parameters for grinnell display -----*/


#define WHITE   07777
#define BLACK  010000
#define RED     00017
#define GREEN   00360
#define BLUE    07400
#define YELLOW  (RED | GREEN)
#define PURPLE  (RED | BLUE)
#define AQUA    (GREEN | BLUE)
#define PINK    (RED | ((GREEN >> 2) & GREEN) | ((BLUE >> 2) & BLUE))

#define SPACE   01000



	/*----- external globals -----------------------------*/

#include "extern"

	/*----- global variables ----- */

int     eobj[31] {      /* The Enterprise */
	6,
	AQUA,5,10,10,13,
	AQUA,6,9,9,14,
	AQUA,7,4,8,8,
	AQUA,6,5,9,5,
	AQUA,4,1,5,7,
	AQUA,10,1,11,7
};

int     gobj[16] {      /* the scoutship Galileo */
	3,
	AQUA,5,3,10,4,
	AQUA,3,5,12,9,
	AQUA,5,10,10,11
};

int     bobj[31] {      /* A starbase */
	6,
	GREEN,2,2,13,13,
	PINK,6,6,9,9,
	BLUE,0,4,3,11,
	BLUE,12,4,15,11,
	BLUE,4,0,11,3,
	BLUE,4,12,11,15
};

int     sobj[21] {      /* A star */
	4,
	YELLOW,7,3,8,12,
	YELLOW,3,7,12,8,
	YELLOW,6,5,9,10,
	YELLOW,5,6,10,9
};

int     pobj[26] {      /* Procyon , when visible as such */
	5,
	YELLOW,7,3,8,12,
	YELLOW,3,7,12,8,
	YELLOW,6,5,9,10,
	YELLOW,5,6,10,9,
	RED,7,7,8,8
};

int     hobj[6] {       /* black hole */
	1,
	BLACK,1,1,14,14
};

int     mobj[6] {       /* meteor storm */
	9,
	RED,7,7,8,8,
	RED,1,5,2,6,
	RED,1,9,2,10,
	RED,5,1,6,2,
	RED,5,13,6,14,
	RED,9,1,10,2,
	RED,9,13,10,14,
	RED,13,5,14,6,
	RED,13,9,14,10
};

int     robj[26] {      /* A Romulan attack vessel */
	5,
	YELLOW,6,2,9,13,
	RED,2,9,13,10,
	RED,2,5,13,6,
	RED,4,11,11,12,
	RED,4,3,11,4
};

int     kobj[21] {      /* A Klingon battle cruiser */
	4,
	PURPLE,5,4,10,9,
	PURPLE,6,10,9,13,
	PURPLE,3,2,4,6,
	PURPLE,11,2,12,6
};

int     blot[6] {       /* empty space */
	1,
	SPACE,0,0,15,15
};

int     halo[21] {      /* halo is visible energy field */
	8,
	YELLOW,0,4,0,11,
	YELLOW,1,2,1,13,
	YELLOW,4,0,11,0,
	YELLOW,2,1,13,1,
	YELLOW,2,14,13,14,
	YELLOW,4,15,11,15,
	YELLOW,14,2,14,13,
	YELLOW,15,4,15,11
};

int     tract[21] {     /* tractor beam field */
	4,
	BLUE,2,0,13,1,
	BLUE,0,2,1,13,
	BLUE,2,14,13,15,
	BLUE,14,2,15,13
};



gmap()
{       int a[16];
	register int x,y;
	gopen(a,0,0,512,512);
	genter(a,0,SPACE,0,0,0);
	gwvec(a,0,0,511,511,1);
	for(x=0; x<GSIZE; x++) for(y=0; y<GSIZE; y++) gshow(mp[y][x],y,x);
	gclose(a);
}

gblank()
{       int a[16];
	gopen(a,0,0,512,512);
	genter(a,0,SPACE,0,0,0);
	gwvec(a,0,0,511,511,1);
	gclose(a);
}

gmove(y1,x1,y2,x2) int y1,x1,y2,x2;     /* show move from [y1,x1] to [y2,x2]  */
{
	gblot(y1,x1);
	gblot(y2,x2);
	gshow(mp[y2][x2],y2,x2);
}

gshow(o,y,x)                   /* display on grinnel the object o at [y,x] */
int y,x;
char o;
{
	int gx,gy;
	gx = grinx(x);
	gy = griny(y);
	switch(o)
	{
	case 'E':
		putobj(eobj,gx,gy);
		break;

	case 'G':
		putobj(gobj,gx,gy);
		break;

	case 'k':
		if(debug) putobj(kobj,gx,gy);
		else putobj(blot,gx,gy);
		break;

	case 'K':
		putobj(kobj,gx,gy);
		break;

	case 'r':
		if(debug) putobj(robj,gx,gy);
		else putobj(blot,gx,gy);
		break;

	case 'R':
		putobj(robj,gx,gy);
		break;

	case 'b':
	case 'B':
		putobj(bobj,gx,gy);
		break;

	case 'p':
			       /* non-activated Procyon, unless debug is on
				show as normal star */
		if(!debug) {
		    putobj(sobj,gx,gy);
		    break;
		}       /* if debug is on, show as if activated */
	case 'P':
		putobj(pobj,gx,gy);
		break;

	case '*':
		putobj(sobj,gx,gy);
		break;

	case 'h':
		putobj(hobj,gx,gy);
		break;

	case 'M':       /* visible meteor shower */
		putobj(mobj,gx,gy);
		break;

	case 'm':
		if(debug) {
			putobj(mobj,gx,gy);
			break;
		} /* else fall into blot */
	default:
	case '.':
		putobj(blot,gx,gy);
		break;

	}

}

gblot(y,x) int y,x;          /* blot out grinnel display at [y,x] */
{
	putobj(blot,grinx(x),griny(y));
}

gexplode(y,x) int y,x;
{ explode(grinx(x),griny(y),16);
}

gnova(y,x) int y,x;
{       explode(grinx(x),griny(y+SECTORS-1),16*SECTORS);
}

gtorp(y,x) int y,x;             /* show torpedo from Enterprise to [y,x] */
{
	torpedo(grinx(sp[1])+7,griny(sp[0])+7,grinx(x)+7,griny(y)+7);
}


gphas(y,x) int y,x;             /* show phaser from Enterprise to [y,x] */
{
	if((y == sp[0]) && (x == sp[1])) return;
	phasor(grinx(sp[1])+7,griny(sp[0])+7,grinx(x)+7,griny(y)+7,25);
	ghalo(y,x);
	sleep(1);
	gblot(y,x);
	gshow(mp[y][x],y,x);
}


ghit(y,x) int y,x;             /* show enemy phaser fire */
{
	phasor(grinx(x)+7,griny(y)+7,grinx(sp[1])+7,griny(sp[0])+7,5);
}

ghalo(y,x) int y,x;             /* show halo around object at [y,x]   */
{       putobj(halo,grinx(x),griny(y));
}

gtract(y,x) int y,x;            /* show tractor beam at [y,x]   */
{       putobj(tract,grinx(x),griny(y));
	sleep(1);
	gblot(y,x);
}

grinx(sx) int sx;                       /* convert space coordinates to grinnell */
{
	return(16*sx);
}

griny(sy) int sy;
{
	return(16*(GSIZE-1-sy));
}


/*--------------------grinnell interface routines----*/


putobj(obj, x, y)
int obj[];
{
	register i, num;
	register *tobj;
	int w[16],fc,fr,lc,lr;

	tobj = obj;
	num = *tobj++;

	gopen(w,x,y,16,16);

	for(i=0;i<num;i++) {
		genter(w,0,*tobj++,0,0,0);
		fc = *tobj++;
		fr = *tobj++;
		lc = *tobj++;
		lr = *tobj++;
		gwvec(w,fc,fr,lc,lr,1);
	}
	gclose(w);
}

phasor(x0,y0,x1,y1,tlen)           /* show phasor from [x0,y0] to [x1,y1] */
{                                  /* for tlen display times */
	register i;
	int w[16];
	gopen(w,0,0,512,512);
	genter(w,4,0,0,0,0);
	for(i=0;i<tlen;i++) {
		gwvec(w,x0,y0,x1,y1,0);
		gclear(w,0,0,511,511,0);
	}
	gclose(w);
}

torpedo(x0,y0,x1,y1)
{
	double sqrt();

	register i, j, id;
	int w[16],idist;
	double fx,fy,dx,dy,dist;

	gopen(w,0,0,512,512);
	genter(w,4,0,0,0,0);

	idist =((x1-x0)*(x1-x0)+(y1-y0)*(y1-y0));
	dist = idist;
	if(idist == 0) return;
	dist = sqrt(dist);
	idist = dist;
	dx = (3.0*(x1-x0))/dist;
	dy = (3.0*(y1-y0))/dist;
	fx = x0;
	fy = y0;
	for(id = 0; id < idist; id =+ 3) {
		gclear(w,0,0,512,512,0);
		fx =+ dx;
		fy =+ dy;
		i = fx;
		j = fy;
		gwvec(w,i,j,i+2,j+2,1);
	}

	gclear(w,0,0,512,512,0);
	gclose(w);
}

explode(x,y,dim) int x,y,dim;
{
	register i, j, k;
	int flg, cnt;
	int w[16];

	gopen(w,x,y,dim,dim);

	j = (dim/2)-1;
	k = j+1;
	flg = 0;

	for(i=0;i<(dim/2);i++) {      /* EXPLODE */
		genter(w,0,((flg = 1 - flg)? RED : YELLOW),0,0,0);
		gwvec(w,j,j,k,k,1);
		j--;
		k++;
		for(cnt=0;cnt<3500;cnt++);
	}

	for(i=0;i<(dim/2);i++) {
		genter(w,0,((flg = 1 - flg)? RED : YELLOW),0,0,0);
		j++;
		k--;
		gwvec(w,j,j,k,k,1);
		for(cnt=0;cnt<3500;cnt++);
		genter(w,0,SPACE,0,0,0);
		gwvec(w,0,0,dim-1,dim-1,1);
	}
	gclose(w);
}
