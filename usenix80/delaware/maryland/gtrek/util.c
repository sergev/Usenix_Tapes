#
/*
 *      Star Trek
 *              by    Robert Haar, Computer Vision Lab., U. of Md.
 *
 *
 *                      derived from a DOD version by :
 *                                              Paul J. Winslow
 *                                              David A. Neuman
 *                                              Dennis Mumaugh
 *
 */


#include "param"     /* bring in parameter file  */


	  /*----- external global variables -----*/

#include "extern"

float rand();

/*-----------------------------------------------------------*/

setupg()                                        /* set up the galaxy */
{
	int x,y,z;
	int ik ;                   /* number of invisible klingons */

	do {
	    printf("\nSkill level (0-10) ");
	    skill = getval();
	    } while (skill >10 || skill < 0) ;

	cmax = CMAX + skill ;
	cmin = CMIN;

	kqu =  (MAXQ+MAXDFL)/1000.0 ;

	/*
		 * initialize galaxy map to periods
		 */
	for (x=0; x< GSIZE2; x++) mp[0][x] = '.';

	/*
		 * position the Enterprise
		 */
	x = (y = (GSIZE-1)/2 );
	mp[sp[0] = y][sp[1] = x] = 'E';
	/*
		 *  position the stars
		 */
	for(z=0; z<STARS; z++)
	{
		while(mp[y=rndcor()][x=rndcor()] != '.') ;
		mp[y][x]= '*' ;
	}
			/* make Procyon the last star generated   */
		mp[y][x]= 'p' ;
		pro[0] = y;
		pro[1] = x;

	/*
		 * position the starbases
		 */
	for(z=0; z<BASES; z++)
	{
		while(mp[y = rndcor()][x=rndcor()] != '.') ;
		mp[y][x]= 'B' ;
		kdfl[y][x]= 100 ;
	}

		/* position the meteor shower */

	while(mp[y=rndcor()][x=rndcor()] != '.') ;
	mp[y][x]= 'm' ;


		/* position the space warp */

	while(mp[y=rndcor()][x=rndcor()] != '.') ;
	mp[y][x]= 'w' ;

	/*
		 * position the Romulan ships (all invisible)
		 */
	for(z=0; z<rleft; z++)
	{
		while(mp[y=rndcor()][x=rndcor()] != '.') ;
		mp[y][x]= 'r' ;
		kdfl[y][x]= 50 ;
	}
	/*
		 * position the Klingons
		 */
	ik = 2 + skill/2 ;
	while (irand(10) < 7) ik++;
	for(z=0; z<kleft; z++)
	{
		while(mp[y=rndcor()][x=rndcor()] != '.') ;
		if ( z >= ik )
		{                     /* visible */
		    mp[y][x]= 'K' ;
		    kdfl[y][x]= (z > 15 ? 125 : 75) ;
		}
		else {                /* invisible */
		    mp[y][x]= 'k' ;
		    kdfl[y][x]= 50 ;
		}
	}
	/*
		 * position the black holes
		 */
	bholes = 1 + skill/4 ;
	for(z=0; z<bholes; z++)
	{
		while(mp[y=rndcor()][x=rndcor()] != '.') ;
		mp[y][x]= 'h' ;
	}
}

float score(code) int code;
{   float effic;
	effic = 10*skill + 12*(KI-kleft) + 15*(RI-rleft) + 5*capt
		- sd - 2*(kqu-(q+dfl)/1000.) - 50*nmu - tu/2.0;
	if(code == 1 && sd < DAYS) effic =- 100;        /* E. was destroyed */
	if(gal[2]==1) effic =- 25;              /* Galileo lost */
	else if(gal[2]==2) effic =+ 25;
	if(pro[2]==1) effic =- 25;              /* Procyon lost */
	else if(pro[2]==2) effic =+ 25;
	return(effic);
}

checkbase()                  /* check if last move docked Enterprise at a base   */
{
	if (atbase())
	{
		if (!dk && !df && !sc && !wc && !t && !tow
		     && (abs(sdlast-sd) < 10) )
		{
			homers();
			printf("              message from Star Fleet Command ...\007\n");
			sleep(1);
			printf("Your hesitancy to leave the protection of Star Base\n");
			printf("has been noted.  The Klingon menace cannot be stopped\n");
			printf("if you persist in this behavior.  Cowardice will not\n");
			printf("be tolerated.  All privileges of the Star Base are\n");
			printf("hereby denied the Enterprise until you have\n");
			printf("demonstrated your willingness to carry out your\n");
			printf("duties as a starship captain.\n");
			sleep(5);
			homers();
			coms();
			dk = 2;
		}
		else if (!dk) {
			printmsg("Enterprise docking is complete");


			/* repair damage */
			df = 0;
			tc = 0;
			pc = 0;
			sc = 0;
			if(wc < 0) {
			    wc =+ 10;
			    if(wc > 0) wc = 0;
			}
			if(bc < 0 ) {
			    if(irand(10) > -(bc)) printmsg("Waiting for computer parts from Maynard,Mass.");
			    else bc = 0;
			}
			newscan = 1;
			sdlast = sd;
			dk = 1;
		}
		if(dk==1) {        /* replenish supplies if docked */
			tow = 0;
			t = 10;
			kqu =+ (MAXQ + MAXDFL - q - dfl)/1000.0 ;
			q = MAXQ;
			dfl = MAXDFL ;
		}
	}
	else dk = 0;
	return(dk);
}


checkhit(code)   int code;
{
	int i,j;
	int spy,spx,course;
	int y1,y2,x1,x2,testij;
	char spot;

	hits = 0;
	spy = sp[0];
	spx = sp[1];
	y1 = spy - HRANGE;
	if(y1 < 0) y1 = 0;
	y2 = spy + HRANGE;
	if(y2 >= GSIZE) y2 = GSIZE-1;
	x1 = spx - HRANGE;
	if(x1 < 0) x1 = 0;
	x2 = spx + HRANGE;
	if(x2 >= GSIZE) x2 = GSIZE-1;

	for (i= y1; i<= y2; i++)
	    for (j= x1; j<= x2; j++)  {
		testij = !offmap(i,j);
		spot = mp[i][j];
		switch (spot)
		{
		    case 'k':
		    case 'K':
		    case 'r':
		    case 'R':
			course = traject(i,j,spy,spx);
			psn(5,course,HRANGE,i,j);
			break;
		    default:
			break;
		}
	    }
	if(code == -1) return;
	damage(hits);
}


damage(hitp) int hitp;                  /* damage Enterprise when hit with power = hitp */
{       int x,y;
	int rndcor();

	hurt = 0;
	checkbase();

	if(hitp <= 0) return;

	if(irand(hitp) > (dfl+300)) allfail();
	if (df < 0) {
	    q =- (hitp/2);
	}
	else if ( (dfl=- irand(hitp)) < 0) {
		q =+ dfl ;
		dfl = 0;
		}
	if(q<=0) allfail();

	if (sc == 0)
		if (irand(hitp) > (dfl/2))
		{
			hurt++ ;
			sc = -5 - irand(5);
			newscan = 1;
		}
	if (bc == 0)
		if (irand(hitp) > (dfl/3))
		{
			hurt++ ;
			bc = -3 - irand(5);
			newscan = 1;
		}
	if (tc == 0)
		if (irand(hitp) > (dfl/2+200))
		{
			hurt++ ;
			tc = -4 - irand(10);
		}
	if (pc == 0)
		if (irand(hitp) > (dfl/3) )
		{
			hurt++ ;
			pc = -1;
		}

	if (wc == 0)
		if (irand(hitp) > (dfl/4 + 100) )
		{
			wc = -29 ;
			hurt++ ;
		}

	if ( (df == 0) && ( irand(hitp) > dfl) )
		{
		hurt++ ;
		dfl = 0;
		df = -(6 + hitp/100) ;
		while(tow > 0) {
		    tow--;
		    capt--;
		    kleft++;
		    while(mp[y=rndcor()][x=rndcor()] != '.');
		    mp[y][x] = 'K';
		    kdfl[y][x] = 50;
		    scanup(y,x);
#ifdef GRINNELL
		    gshow('K',y,x);
#endif
		    alarmmsg("CAPTURED VESSEL ESCAPED!!");
		    }
		}
	if (hurt) alarmmsg("damage reports coming in");
	return(hurt);
}

allfail()
{
	alarmmsg("ALERT: ALL SYSTEMS FAILING");
	sleep(1);
	q = 0;
	dfl = 0;
	wrapup(1);
}

destroy(y,x) int y,x;
{
	switch(mp[y][x])
	{
		case 'b':
		case 'B':
			printmsg("starbase destroyed!!");
			break;

		case 'p':
		case 'P':
			printmsg("PROCYON DESTROYED!!!");
			break;

		case '*':
			printmsg("star destroyed");
			break;

		case 'G' :
			printmsg("GALILEO DESTROYED!!!");
			break;

		case 'K':
		case 'k':
			printmsg("Klingon destroyed");
			break;

		case 'R':
		case 'r':
			printmsg("Romulan destroyed");
			break;

		default: break;
	}

#ifdef GRINNELL
	gexplode(y,x);
#endif
	scanblink(y,x);
	wipeout(y,x);
}

wipeout(y,x) int y,x;
{
	switch(mp[y][x]){
	    case 'k':
	    case 'K':
		kleft--;
		break;

	    case 'r':
	    case 'R':
		rleft--;
		break;

	    case 'G':
		gal[2] = 1 ;
		break;

	    case 'P':
	    case 'p':
		pro[2] = 1;
		break;

	    case 'B':
	    case 'b':
		break;

	    case '*':
		break;
	}

#ifdef GRINNELL
	gblot(y,x);
#endif

	mp[y][x]= '.' ;
	kdfl[y][x]= 0 ;
	scanup(y,x);
	return;
}



traject(sy,sx,ty,tx) int sy,sx,ty,tx;   /* calculate trajectory from sy,sx to ty,tx */
{   int course;
    double atan();


    if (tx == sx || ty == sy)       /* target in same column or row as source */
	  course = ty == sy ? (tx < sx ? 180 : 0) :
				(sy < ty ? 270 : 90);
    else
	{
	    course = integ( atan( (ty - sy)/(1.0*(tx -sx)) ) * 180/3.14159 ) ;
	    course = ((tx < sx ? 180 : 360) - course) % 360;
	}


    return(course);
}


offmap(y,x)
int y,x;
{
return ( x < 0 || x >= GSIZE || y < 0 || y >= GSIZE );
}

atbase()
{
	register int i,j,b;
	int sx,sy;
	b = 0;
	for (i= -1;i<=1;i++) for (j= -1;j<=1;j++)
	{
	    sy = sp[0]+i;
	    sx = sp[1]+j;
	    if(!offmap(sy,sx))
		switch(mp[sy][sx])
		{
		    case 'B':
			b++;
			break;
		    case 'b':
			if(kdfl[sy][sx]++ >= 0)
			{
			    b++;
			    kdfl[sy][sx]= 50 ;
			    mp[sy][sx]= 'B' ;
			}
			break;
		}
	}
	return(b);
}

power(needed)
int needed;
{
    if(dk!=1) {
	if (q < needed)
	{
		q =- 25;
		printmsg("not enough energy available");
		return(0);
	}
	q =- needed;
    }
    return(1);
}

dist(x1,y1,x2,y2)
int x1,y1,x2,y2;
{
	int d1,d2;
	d1 = abs(x1-x2);
	d2 = abs(y1-y2);
	if(d1 > d2) return(d1);
	else return(d2);
}

int integ(f) float f;
{
	register i;
	i = f;
	return(i);
}

delay(dt) int dt;                 /* delay proportional to dt  */
{       int i,j;
	for(i=0;i<dt;i++) for(j=0;j<1000;j++);
}


int rndcor ()               /* randomly picks a coordinate value in range [0,GSIZE-1]        */
{       int irand();
	int rtemp;
	rtemp = irand(GSIZE);
	if(rtemp >= GSIZE || rtemp < 0) rtemp = 0;
	return(rtemp);
}

irand(limit) int limit;         /* produces a random integer in range 0 to limit-1 */
{       if(limit<=0) return(0);
	return(integ(limit*rand()));
}

/*------------------------
	the following code is the random number generator (RAND)
	and the initialization function (SRAND).

	the integer variable randseed MUST be defined as a global
								   */

int lastrnd;

srand(x) int x;                /* setup the random number generator */
{
   int tvec[2];

   if(x) randseed = x ;
   else {
	time(tvec);
	randseed = tvec[0] ^ tvec[1];
	}
   lastrnd = randseed;
}


float rand()                    /* random number generator   RAND
				   returns a floating pt. value in (0,1)  */
{
	lastrnd =* 253 ;
	if(lastrnd<0) lastrnd =+ 32767;
	return(lastrnd*3.05175e-5);
}






































































