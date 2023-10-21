int	whichc = 0;

circle(x,y,r){
	if(whichc == 2)
		circle2(x,y,r);
	else
		circle1(x,y,r);
}

circle1(x,y,r){
	arc(x,y,x+r,y,x+r,y);
}

/*
 * This is an implementation of Bresenham's circle generator
 * as described in CACM Feb 77 pp 100-106.  The whole 
 * thing is done with just integer addition and subtraction,
 * not a single multiplication. Amazing, isn't it?
 *
 * Written by Christopher A. Kent, October, 1978.
 */

int moves[4][3][2]{	/* 4 quadrants, 3 moves, 2 dimensions */
	0,1,  1,1,   1,0,
	1,0,  1,-1,  0,-1,
	0,-1, -1,-1, -1,0,
	-1,0, -1,1,  0,1
};

circle2(x0,y0,r){
	int delta, quad;
	register del, x, y;

	x0 =- r;	/* goto starting point */
	move(x0,y0);


	for(quad = 0; quad < 4; quad++){
		x = 1;
		y = r + r - 1;
		delta = y - 1;
		for(;;){
			if(y<0)break;

			del = delta + delta;
			if(del<0){
				del =+ x;
				if(del<0){
					y =- 2;
					delta =+ y;
					x0 =+ moves[quad][2][0];
					y0 =+ moves[quad][2][1];
					cont(x0,y0);
					continue;
				}
			}else{
				del =- y;
				if(del >= 0){
					x =+ 2;
					delta =- x;
					x0 =+ moves[quad][0][0];
					y0 =+ moves[quad][0][1];
					cont(x0,y0);
					continue;
				}
			}

	/* by the time you get here, move must be 2 */
			x =+ 2;
			y =- 2;
			delta =+ y - x;
			x0 =+ moves[quad][1][0];
			y0 =+ moves[quad][1][1];
			cont(x0,y0);
		}	/* end of while(1) */
	}
	return(0);
}
