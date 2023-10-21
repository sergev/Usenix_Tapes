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


#include "param"             /* bring in parameter file */


       /*----- global variables -----*/

int skill;                  /* players skill level  0-10
			       0 turns off real time attack */
int sp[2],                  /* Enterprise coordinates   */
capt 0,                     /* # of captured klingon vessels */
tow 0,                      /* # of vessels in tow by Enterprise */
hurt,                       /* Boolean, indicates if E. damaged in last attack */
hits 0,                     /* Enterprise was hit if > 0 */
q MAXQ,                     /* # of quarks available  */
sd 0,                       /* current stardate  */
wc 0,                       /* status of warp drive  0 => O.K.  */
sc 0,                       /* scanner condition  */
df 0,                       /* deflector condition  */
dfl MAXDFL,                 /* deflector power  */
bc 0,                       /* battle computer condition  */
t 10,                       /* # of photon torpedoes  */
tu 0,                       /* total photon torpedoes used */
tc 0,                       /* condition of torpedo room */
pc 0,                       /* phaser bank condition */
nm 1,                       /* # of nova-max missiles */
nmu 0,                      /* # of nova-max missiles used */
dk 0,                       /* docking Boolean  */
sdlast -10,                 /* date of last docking  */
ppw,                        /* phaser power  */
bholes,                     /* number of black holes */
kdfl[GSIZE][GSIZE],         /* Klingon deflector power  */
kleft KI,                   /* # of Klingons  */
rleft RI,                   /* # of Romulan ships */
cmax,cmin;                  /* limits to enemy chase distances */

float kqu;                   /* kilo-quarks used */

int  gal[3] {0,0,0},          /* Galileo info    0,1 = coordinates        */
     pro[3] {0,0,0};          /* Procyon info     2 = status  {     0 => not activated    }
							      {  negative => activated    }
							      {     1 => destroyed        }
							      {     2 => used succesfully }  */



char kvastring[80];     /* string for battle computer output */
char prostring[45];     /* string for Procyon message */
char galstring[45];     /* string for Galileo message */
char *chrp,*strp;       /* tempory char & string pointers */

int newscan 1;          /* boolean to indicate new scan needed */

char mp[GSIZE][GSIZE];   /* galaxy map  -----------------------

				   lower case => invisible

			'.' = empty space       ' ' = time warp
			'*' = star              'h' = black hole
			'E' = the Enterprise    'w' = space warp
			'B' = starbase          'M' = meteor shower
			'K' = Klingon ship      'P' = Procyon system
			'R' = Romulan ship      'G' = the Galileo scoutship
			'?' = unknown (in hyperscan display)
									    */
int randseed;           /*random number generator starting point */

int rubbed 0;           /* boolean to indicate that getval has
			   encountered a DEL character */

int debug;      /* debug boolean */



main(argc,argv)  int argc; char *argv[];
{
	int x,y,z,course,spread;
	int comm;                  /* input command number */
	char n;
	float score();

			  /*  initialize random number generator   */
	if(argc > 1) x = atoi(argv[1]);
	else x = 0;
	srand(x);

	ptitle();
	printf("Space ... the final frontier. \n");
	printf("These are the voyages of the starship Enterprise.\n");
	printf("Its five year mission ... to explore the galaxy,\n");
	printf("to seek out new civilizations,\n");
	printf("to boldly go where no man has gone before!\n\n");
	sleep(1);
	printf("There are %d Klingon starships (some invisible)\n",kleft);
	printf("and %d Romulan vessel in Federation space\n\n",rleft);
	printf("You have %d stardates to destroy them.\n",DAYS);
	printf("There are %d starbases for supplies and repairs.\n\n",BASES);
	printf("BEWARE! Weather reports indicate the presence of meteor showers,\n");
	printf("        space warps, and black holes\n\n");
	printf("Goodby and good luck!\n\n");

	setupg();                  /* initialize the galaxy        */
	terminit();
	showit();

	checkbase();
	checkhit(-1);

	/* begin command processing loop
		continue as long as there are enemy left
		and the Enterprise has any energy             */

	while (q && (kleft+rleft))
	{
		comm = getcom();
		switch (comm)
		{
		default:
			printmsg(stringf("%d is an unknown command",comm));
			break;
		case 0:
			/*
						 * refresh screen with new image
						 */
			newday(1);
			showit();
			printmsg(stringf("Your current score is %3.0f",score(0)));
			break;
		case 1:
			/*
						 * activate impulse engines
						 */
			warp(0);
			break;
		case 2:
			/*
						 * activate warp engines
						 */
			warp(1);
			break;

		case 3:                                 /* fire phasers */
			if(pc != 0) {
				printmsg("phasers inoperable");
				break;
			}
			printcom("fire angle");
			course = getang();
			if(rubbed) break;
			printcom("power");
			do { ppw = getval(); } while (ppw < 0);
			if(rubbed) break;
			if (power(ppw))  psn(2,course,PRANGE,sp[0],sp[1]);
			newday(1);
			break;

		case 4:                                 /* fire torpedo */
			if (!t) {
				printmsg("torpedoes exhausted");
				break;
			}
			if (!power(110)) break;
			if (tc != 0) {
				printmsg("torpedo launchers disabled");
				break;
			}
			printcom("torpedo course");
			course = getang();
			if(rubbed) break;
			if (dk!=1) t--;
			tu++;
			psn(3,course,TRANGE,sp[0],sp[1]);
			newday(1);
			break;

		case 5:                                 /* torpedo spread */
			if(!t) {
				printmsg("torpedoes exhausted");
				break;
			}
			if (tc != 0) {
				printmsg("torpedo launchers disabled");
				break;
			}
			printcom("how many");
			do {z = getval();} while ((z<0) || (z>10)) ;
			if(rubbed) break;
			if (z > t)
			{
				z = t;
				printmsg(stringf("%d torpedo(es) remaining ... fire all %d?\t",t,t));
				n = mygetchar();
				if(rubbed) break;
				gobble();
				if (n != 'y')
				{
					break;
				}
			}

			if (!power(110*z)) break;
			printcom("initial course");
			course = getang();
			if(rubbed) break;
			printcom("spread");
			spread = getang();
			if(rubbed) break;
			for (x=0; x<z; x++)
			{
				psn(3,course,TRANGE,sp[0],sp[1]);
				chase();
				course = (course + spread) % 360;
			}
			if (dk!=1) t=- z;
			tu =+ z;
			newday(1);
			break;

		case 6:                         /* launch nova-max missile */
			if (nm == 0)
			{
				printmsg("nova-max missiles depleted");
				break;
			}
			if (!power(500)) break;
			printcom("Nova-max course");
			course = getang();
			if(rubbed) break;
			psn(4,course,SECTORS,sp[0],sp[1]);
			newday(5);
			break;


		case 7:                         /*raise deflector shields */
			if (df >= 0)
			{
				printcom("deflector power");
				do { z = getval();} while (z < 0);
				if(z > MAXDFL) z = MAXDFL ;
				if (power(z-dfl)) {
				    dfl = z;
				}
			}
			else
			{
				printmsg("deflectors out");
			}
			break;

		case 8:                         /* panic - random jump */
			warp(2);
			break;

		case 9:                                 /* hyper-scan
							   show invisible stuff */
			if(sc) printmsg("scanners out ... ");
			else if (power(50)) scan(2);
			break;


		case 10:                        /* battle computer
						   gets angles to klingons */
			if(sc==0)
			    {
				if(bc!=0) printmsg("computer down");
				else if (power(50)) computer();
				break;
			    }
			else printmsg("scanners out ... switching to trajectory program");

			/* fall thru to case 11   */

		case 11:                                /* calculate trajectory angle  */
			if (power(25))
			{
				printcom("vert. dist.");
				do { y = getval();} while (abs(y) > 10) ;
				printcom("horiz. dist.");
				do { x = getval();} while (abs(x) > 10) ;
				z = traject(0,0,-y,x);
				chrp = stringf("Trajectory to [%d,%d] = %d",y,x,z);
				strp = kvastring;
				while(*strp++ = *chrp++);
				poscomp(KVALINE);
				printf("%s",kvastring);
			}
			break;
			/*
							 * open communications
							 */
		case 12:
			if (power(10))
			{
			if (pro[2] < 0 ) {
				if (pro[1]/SECTORS == sp[1]/SECTORS && pro[0]/SECTORS == sp[0]/SECTORS)
				{
					poscomp(PCLINE);
					printf("time warp discovered in sector %1d - %1d\n",
					pro[0] % SECTORS + 1,pro[1] % SECTORS + 1);
					mp[pro[0]][pro[1]]= ' ' ;
					newscan = 1;
				}
				else  printmsg("communications declined");
			}
			else  printmsg("communications declined");
			}
			break;


		case 13:                        /* galaxy scan and hyperscan */
			if(sc==0 && power(50)) scan(2);
			if(sc == 0 && bc == 0 && power(50)) {
			    newmap();
			    newday(1);
			}
			break;


		case 14:                                /* grapple with tractor beams to
							   pick up Galileo or capture adjacent Klingons  */
			if(power(50))
			    {
				if((df==0)&&(dfl>1000)){
				    dfl =- 100;
				    tractor();
				}
				else printmsg("Cannot use tractor beams");
			    }
			newday(1);
			break;


		case 98:                /*      shell     */
			if(fork() == 0 ) {
				ptitle();
				printf("You have forked to shell. Hit <CTRL-D> to return to Startrek\n");
				resterm();
				if(execl("/bin/sh",0)) exit();
			}
			else {
				wait(&x);
					/* reconstruct display */
				terminit();
				showit();
			}
			break;

			/* self-destruct                  */
		case 99:
			printcom("Please confirm (y/n)");
			if(( n = mygetchar()) == 'y')
			{
				printmsg("DILITHIUM CRYSTALS IMPLODED ... GOODBYE.");
				sleep(1);
				nova(sp[0],sp[1]);
				wrapup(-1);
			}
			else gobble();
			break;

		case -1:       /* debug option - shows invisible things  */
			if(debug) debug = 0;
			else  {
			    printcom("enter password");
			    x = getval1();                 /* no echo */
			    if(x == PASSWD) {
				debug = 1;
				newscan = 1;
				showit();
			    }
			    else printmsg("sorry");
			}
			break;


		}
					/* end of command processor     */

					/* only refresh the display if there is  */
					/* no command pending i.e. input is empty */
		if(empty(0)) scan(1);
		if(debug) newmap();
		status();
	}
	wrapup(kleft+rleft>0);
}



newday(days)                            /*   new startdate     */
int days;
{       register int y,x;

	sd =+ days;
	if(sd >= DAYS) wrapup(1);  /* out of time */

	kvastring[0] = '\0';

	power(25);

	if (sc < 0) {
		sc =+ days;
		if(sc > 0) {
		    sc = 0;
		    newscan = 1;
		    }
	}

	if (wc < 0) {
		wc =+ days;
		if(wc > 0) wc = 0;
	}
	if (df < 0) {
		df =+ days;
		if (df >= 0) {
			df = 0;
			if(power(500)) dfl = 500;
		}
	}
	if (bc < 0) {
		bc =+ days;
		if(bc > 0) {
		    bc = 0;
		    newscan = 1;
		    }
	}
	if (tc < 0){
		tc =+ days;
		if (tc > 0) tc = 0;
	}

	attack();

	/* check Procyon and the Galileo */

	if (pro[2] == 0 && sd > 100 && irand(10)<1){
		pro[2] = -20;
		alarmmsg("subspace communication coming in");
		}
	else if (pro[2] < 0 && ++pro[2] == 0) {
		wipeout(pro[0],pro[1]);
		alarmmsg("Procyon going nova!!!");
		nova(pro[0],pro[1]);
	}

	if (gal[2] == 0 && sd > 75 && irand(8)<1)  {
		while (mp[gal[0] = rndcor()][gal[1] = rndcor()] != '.');
		alarmmsg("subspace communication coming in");
		mp[gal[0]][gal[1]]= 'G' ;
		gal[2] = -20;
	}
	else if (gal[2] < 0 && ++gal[2] == 0) {
		wipeout(gal[0],gal[1]);
		alarmmsg("Lost contact with Galileo ... presumed destroyed");
		gal[2] = 1;
	}
}

