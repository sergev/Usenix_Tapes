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

warp(code)              /*  move Enterprise  */
int code;
{
	int course,warpf,x,y;
	switch (code)
	{
		/* activate impulse engines                              */
	case 0:
		printcom("impulse course");
		course = getang();
		if(rubbed) return;
		printcom("distance");
		do { warpf = getval();} while (warpf > SECTORS || warpf < 0);
		if(rubbed) return;
		if (warpf > 0)
			if (power(15*warpf*(tow/2+1)))
				psn(0,course,warpf,sp[0],sp[1]);
		break;

		/* activate warp engines                              */
	case 1:
		if( wc != 0) {
			printmsg("warp drive unavailable....") ;
			return;
		}
		printcom("warp course");
		course = getang();
		if(rubbed) return;
		printcom("warp factor");
		do { warpf = getval();} while (warpf > 7 || warpf < 0);
		if(rubbed) return;
		if (warpf > 0)
			if (power(15*warpf*(tow+1)+30))
			{
				course = course -10 + irand(20) ;
				psn(1,course,warpf,sp[0],sp[1]);
			}
		break;

	case 2:         /* panic - random jump  */
		if(power(100)) {
			printmsg("PANIC - random warp");
			sleep(1);
			while(mp[y=rndcor()][x=rndcor()] != '.');
			emove(y,x);
			newday(5);
		}
		break;

	}   /*end of switch(code)  */
	newscan = 1;
	return;
}

psn(code,course,distance,y1,x1)         /* position change routine  */
int code,course,distance,y1,x1;
{
					/* code 0 : impulse engines
						1 : warp drive
						2 : phasers
						3 : torpedos
						4 : mova-max missile
						5 : enemy fire on Enterprise */
	int i,j,x,y,tx,ty,txs,tys;
	int lastx,lasty;
	float delta,fx,fy,deltax,deltay,angle;
	float sina,cosa;
	double sin(),cos(),fabs();


	lastx = x = x1;
	lasty = y = y1 ;

	fy = y;
	fx = x;
	delta = 1.0 ;

	switch(code) {
	    case 0: printmsg("Impulse engines firing");
		    break;
	    case 1: printmsg("Warping");
		    delta = 4.0 ;
		    break;
	    case 2: printmsg("Phasers firing");
		    sleep(1);
		    break;
	    case 3: printmsg(stringf("Torpedo launched on course %d",course));
		    sleep(1);
		    break;
	    case 4: printmsg("NOVA-MAX missile launched");
		    sleep(1);
		    break;
	}

	angle = course/180.0 * 3.14159 ;                /* convert to radians */

	sina = fabs(sin(angle));
	cosa = fabs(cos(angle));
						       /* adjust distance unit (delta) to
							  account for non-euclidean measure */
	if(sina > cosa ) delta = delta/sina ;
	     else        delta = delta/cosa ;

					/* calculate x,y steps */

	deltay = -delta*sin(angle);     /* negative since y runs from top to bottom */
	deltax = delta*cos(angle);


	for (i=0; i<distance; i++)
	{
		lastx = x;      /* remember last position */
		lasty = y;
		fx =+ deltax;   /* calculate the next position */
		fy =+ deltay;
		x = fx + 0.5 ;
		y = fy + 0.5 ;

		switch (code)
		{
		case 0:                 /*     impulse engines   */
		case 1:                 /*       warp drive      */
			if (offmap(y,x) || mp[y][x] == 'w')
			{
				alarmmsg("Enterprise has left known space...\n");
				while (mp[y = rndcor()][x = rndcor()] != '.');
				fy = y;
				fx = x;
				printmsg("   and has reappeared.");
			}
			switch(mp[y][x])
			{
			case '.':
				break;

			case ' ':
				sd =- 25;
				sdlast =- 25;
				pro[2] = 2;
				printmsg("time warp entered ... 25 stardates regained");
				break;

			case 'h':
				alarmmsg("THE ENTERPRISE HIT A BLACK HOLE");
				damage(500);
				newday(1);
				psn(1,(course+irand(180)-90),distance-i,sp[0],sp[1]);
				return;
			case 'm':
			case 'M':
				printmsg("METEOR SHOWER");
				dfl = 0;
				damage(200);
				break;

			case 'B':
				if(code == 0) {
				    alarmmsg("COLLISION");
				    printmsg("The Enterprise rammed a starbase at ");
				    printpos(y,x);
				    mp[y][x]= 'b' ;
				    scanblink(y,x);
				    printmsg("docking area damaged");
				    kdfl[y][x]= -3 ;
				    damage(100);
				}
				else  printmsg("Enterprise course blocked");
				newday(1);
				return;

			case 'G':
				alarmmsg("COLLISION");
				destroy(y,x);
				damage(100);
				break;

			case 'K':               /*ran into enemy ship*/
			case 'k':
			case 'R':
			case 'r':
				if(code == 0) {   /* if on impulse engines, destroy it*/
				    alarmmsg("COLLISION");
				    damage(kdfl[y][x]);
				    if(dfl > 0) {
					destroy(y,x);
					break;
				    }
				}
				/* otherwise fall into course blocked code */
			default:
				printmsg("course blocked");
				newday(1);
				return;
			}
			emove(y,x);
			newday(1);
			continue;

		case 2:                 /* fire phasers */

							/* attenuate phaser power */
			ppw =-  10 + irand(10)  ;

			if ( offmap(y,x) || (ppw <= 0) ) {
				printmsg("MISS");
				return;
			}

			if(mp[y][x] == '.') continue;
			switch(mp[y][x])
			{
			case '*':
				scanblink(y,x);
				printmsg("phaser absorbed by star");
				return;

			case 'p':
			case 'P':
				printmsg("Procyon system hit");
				scanblink(y,x);
				pro[2] = 1;
				mp[y][x]= '*' ;
				printmsg("All life forms destroyed");
				return;

			case 'B':
				if(kdfl[y][x] >= ppw)  {
					printmsg("you just hit a starbase .. extensive damage");
					scanblink(y,x);
					mp[y][x]= 'b' ;
					kdfl[y][x]= -8 ;
					return;
				}
			case 'b':
				printmsg("You just vaporized a starbase");
				break;

			case 'G':
				printmsg("you have just vaporized the Galileo");
				break;

			case 'k':
				if (ppw >= 25) mp[y][x]= 'K' ;
			case 'K':
				if (kdfl[y][x] >= ppw)
				{
					kdfl[y][x] =- ppw;
					printmsg("enemy hit ... deflectors held");
					scanblink(y,x);
					return;
				}
				else break;

			case 'r':
				if (ppw >= 25) mp[y][x]= 'R' ;
			case 'R':
				if (kdfl[y][x] > ppw)
				{
					kdfl[y][x] =- ppw;
					printmsg("enemy hit ... deflectors held");
					scanblink(y,x);
					return;
				}
				else break;

			default:
				printmsg("MISS");
				return;

			}
			destroy(y,x);
			return;

			/* fire photon torpedo                                */
		case 3:
			if(mp[y][x] == '.') continue;


			if(offmap(y,x) )
			{
				printmsg("MISS");
				return;
			}
			switch(mp[y][x])
			{
				case 'm':
				case 'M':
					printmsg("torpedo exploded prematurely");
					break;

				case 'w':
				case ' ':
				case 'h':
					printmsg("MISS");
					return;

			}
			destroy(y,x);
			if(dist(y,x,sp[0],sp[1]) < TMIN)
			{
			    alarmmsg("DANGER radiation damaging shields");
			    dfl = 0;
			 }
			return;

			/* fire nova-max missile                                   */
		case 4:
			if (offmap(y,x))
			{
				printmsg("missile not reporting ... presumed lost");
				return;
			}
			else continue;

			/* Klingon hits                          */
		case 5:
			switch(mp[y][x])
			{       case '.':
					continue;
				case 'E': hits =+ kdfl[y1][x1] ;
					return;
				default:
					return;
			 }

		}
		/* end of switch(code */
		break;
	}
	/* end of postion change loop  */
	switch(code)
	{
	case 0:
	case 1:
		break;
	case 2:
		printmsg("MISS");
		break;
	case 3:
		printmsg("MISS");
		break;
	case 4:
		txs = (tx = x/SECTORS)*SECTORS + SECTORS/2;
		tys = (ty = y/SECTORS)*SECTORS + SECTORS/2;

		printmsg(stringf("warhead detonated in quadrant %d-%d",ty+1,tx+1));
		nova(tys,txs);
		nm--;
		nmu++;
		break;
	}
}


tractor()               /* grapple with tractor beams */
{       int i,j,z,zz;
	if(gal[2] < 0 &&  dist(sp[0],sp[1],gal[0],gal[1]) <= 1)
	{
		poscomp(GPLINE);
		printmsg("Galileo retrieved with nova-max missile on board");
		gal[2] = 2;
		mp[gal[0]][gal[1]] = '.';
		scanup(gal[0],gal[1]);
		nm++;
		return;
	}
	if(!power(100)) return;
	if ( tow < MAXTOW )
	{
	    for(i= -1; i<2; i++)
		for(j= -1; j<2; j++)
		{

		    z = sp[0]+i;
		    zz = sp[1]+j;
		    if(offmap(z,zz)) continue ;
		    switch (mp[z][zz])
			{
			    case 'k':
			    case 'K':
				if(dk!=1) tow++;
				capt++;
				printmsg("Klingon captured");
				wipeout(z,zz) ;
				return;

			    case 'R':
			    case 'r':
				printmsg("Romulan self-destructed to avoid capture.");
				scanblink(z,zz);
				dfl = 0;
				if (dk != 1) df = -5;
				wipeout(z,zz) ;
				return;
			}  /* end of switch */
		}
		printmsg("nothing in range of tractor beams");
	}
	else printmsg("maximum number of captured vessels already in tow");
}

attack()        /* have the enemy ships attack the Enterprise */
{
	chase();
	checkhit();
	if (q <= 0 && q + dfl > 0)  {
		q =+ dfl/2;
		dfl = 0;
		alarmmsg("quarks dangerously low ... deflectors have been dropped");
	}
	if(q<=0) allfail();
}

chase ()                /* move nearby visible Klingons towards the Enterprise
			   unless docked, then move away instead.
				cmax is outer limit of movable range
				cmin is closest move distance
				klingons move one step with probability .3
			*/
{       int i,j,sy,sx;
	sy = sp[0];
	sx = sp[1];
	for(i=cmin; i<=cmax; i++)
	    for(j= (-i); j<= i; j++) {
		chase1(sy+i,sx+j);
		chase1(sy-i,sx+j);
		chase1(sy+j,sx+j);
		chase1(sy+j,sx-j);
	    }
}

chase1(iy,ix) int iy,ix;                /* make enemy ship at iy,ix
					   chase the Enterprise */
{       int dy,dx;
	if(offmap(iy,ix)) return;
	if (mp[iy][ix] != 'K') return;
	if(irand(100) > CPROB) return;
	dy = sp[0]-iy;
	dx = sp[1]-ix;
	if(dy != 0) dy = dy/abs(dy);
	if(dx != 0) dx = dx/abs(dx);
	if(dk) {
		dy = -dy;
		dx = -dx;
	}
		/* try direct approach first, if fails, try horiz., then vert. */

	if(chase2(iy,ix,dy,dx)) return;
	if (dy != 0 && dx != 0)
		if(chase2(iy,ix,0,dx) || chase2(iy,ix,dy,0) ) return;

		/* if original choice was horizontal or vertical
		   then try on a diagonal */

	if (dy == 0 && !chase2(iy,ix,1,dx) ) chase2(iy,ix,-1,dx);
	if (dx == 0 && !chase2(iy,ix,dy,1) ) chase2(iy,ix,dy,-1);

}

chase2(iy,ix,dy,dx) int iy,ix,dy,dx;
			/* try to move enemy ship at iy,ix in direction dy,dx */
{   int nx,ny;
	switch(mp[iy][ix]) {
		case 'K':
		case 'k':
		case 'R':
			break;
		default:
			return(0);
	}
	ny = iy+dy;
	nx = ix+dx;
	if(offmap(ny,nx) || dist(ny,nx,sp[0],sp[1]) < cmin
			 || mp[ny][nx] != '.'             ) return(0);
	move(iy,ix,ny,nx);
	return(1);
}


nova(y,x) int y,x;      /* explode nova-max around point [y,x] */
{       int ly,uy,lx,ux,nr,nk,ns,nb;
	register int i,j;


	ly = y - (SECTORS/2);
	uy = ly + SECTORS;
	lx = x - (SECTORS/2);
	ux = lx + SECTORS;
	if(ly<0) ly = 0;
	if(lx<0) lx = 0;
	if(uy>GSIZE) uy = GSIZE;
	if(ux>GSIZE) ux = GSIZE;

	nr = nk = ns = nb = 0;
	for (i=ly; i<uy; i++) for (j=lx; j<ux; j++)  {

		switch  (mp[i][j])
		{
		    case 'k':
		    case 'K':
			    nk++;
			    break;

		    case 'r':
		    case 'R':
			    nr++;
			    break;

		    case 'B':
			    alarmmsg("Starbase caught in nova");
			    break;

		    case 'G':
			    alarmmsg("Scoutship Galileo destroyed");
			    break;

		    case 'p':
		    case 'P':
			    alarmmsg("Procyon system vaporized");
			    break;

		    case 'E':
			    allfail();
			    break;
		}
		wipeout(i,j);
	}
	printmsg(stringf("%d enemy starships destroyed",nk+nr));
}


emove(y,x) int y,x;     /* move Enterprise to position [y,x]  */
{
	move(sp[0],sp[1],y,x);
	sp[0] = y;
	sp[1] = x;
}

move(y0,x0,y1,x1) int y0,x0,y1,x1;      /* move object at [y0,x0] to [y1,x1] */
{
	mp[y1][x1]= mp[y0][x0] ;
	mp[y0][x0]= '.' ;
	kdfl[y1][x1]= kdfl[y0][x0] ;
	kdfl[y0][x0]= 0 ;
	scanup(y0,x0);
	scanup(y1,x1);
}

