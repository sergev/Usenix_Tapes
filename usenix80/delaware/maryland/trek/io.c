#
/*
 *      Star Trek
 *              by    Robert Haar, Computer Vision Lab., U. of Md.
 *
 *
 *                      I/O routines
 */


#include "param"             /* bring in parameter file */

#include "extern"          /* external definitions */

/*--------------------------------------------------------------*/
wrapup(code)                /*      wrap up the session  */
int code;
{
	float effic;
	float score();
	if(code == 1) destroy(sp[0],sp[1]);
	ptitle();

	switch (code)
	{
	default:
	case 1: if (sd >= DAYS) printf("\nTime has run out!");
		else  printf("\nThe Enterprise has been destroyed!");

	case -1: printf("\nThe Klingons will conquer the Federation!");
		break;

	case 0: printf("\nThe Federation has been saved!");
		break;
	}
	kqu = kqu - (dfl/1000.0);

	effic = score(code);
	printf("\n\nEfficiency rating... %3.0f\n",effic);

	if(effic < 200) printf("\nYou must be a novice. Try again.");
	else if(effic < 300) printf("\nYour poor performance has been noted.");

	if(effic >= 400&& code > 0) printf("\nA valiant effort, but not good enough.\n");

	if(effic >= 400 && code == 0) printf("\nYour skill and bravery are an inspiration to all Federation forces.");
	if(effic >= 450 && code == 0) printf("\nYou are being recommended for a promotion.");
	if(effic >= 500 && code == 0) printf("\nYou are being awarded the Good Conduct medal");
	if(effic >= 510 && code == 0) printf(" with starbursts.\n");
	else printf(".\n");

	if(effic >= 550 && code == 0) printf("\nWho do you think you are? Capt. Kirk?");
	if(effic >= 600 && code == 0)  printf("\n\nYOU MUST HAVE CHEATED!!!!");

	printf("\n\n----STATISTICS----");
	printf("\nenergy used:%7.2f (kilo quarks)",kqu);
	printf("\ntorpedoes used: %d",tu);
	printf("\nstardates used: %d",sd);
	if(kleft+rleft) printf("\nenemy left: %d",kleft+rleft);
	printf("\nenemy captured: %d\n",capt);
	printf("\n\nSeed value: %d\n",randseed);

	resterm();                      /* reset terminal */
	exit();
}

ptitle() {                      /*  clear screen and print title */
	homers();
	printf(TITLE);
}

computer()
{
	register int i,j;
	int spy,spx,zy,zx,course,count;
	double atan();
	char *chrp,*strp;       /* temporary char & string pointers */
	zy = sp[0] - 5;
	zx = sp[1] - 5;
	count = 0;
	chrp = "KVA=   ";
	strp = kvastring;
	while(*strp++ = *chrp++);
	strp = kvastring + 5;
	for (i=zy; i< zy + 11; i++)
		for (j=zx; j< zx + 11; j++)
		{
			if (offmap(i,j) || (mp[i][j] != 'K' && mp[i][j] != 'R') ) continue;
			course = traject(sp[0],sp[1],i,j);
			chrp = stringf(" %d ",course);
			while( *strp++ = *chrp++ );
			strp--;
			count++;
		}
	if (count == 0) {
		chrp = "no Klingons targeted";
		strp = kvastring;
		while( *strp++ = *chrp++ );
	}
}


					/* display all info on screen */
showit(){
#ifdef REALTIME
	if(empty(0)){
#endif
	    homers();
	    scan(0);
	    newmap();
	    coms();
	    printmsg(stringf("RANDSEED=%d",randseed));
#ifdef REALTIME
	}
#endif
	status();
}

newmap()                             /*   galaxy map ==  long range sensors */
{
	register int kk,l;
	int qy,qx,qk,qstar,qb,i,j,ppl;
	static int nk[QUADS][QUADS], nb[QUADS][QUADS], ns[QUADS][QUADS];
	poscur(GMCOL,GMROW);
	printf("+------------GALAXY MAP------------+");
	for (i=0; i< QUADS; i++)
	{
		qy =  i*SECTORS;
		ppl = GMROW + 2*i + 1;
		poscur(GMCOL,ppl);
		printf("| ");
		for (j=0; j< QUADS; j++)
		{
			qx = j*SECTORS;
			if(sc == 0 || debug)
			    {
				qk = qstar = qb = 0;
				for (kk=qy; kk<qy + SECTORS; kk++)
					for (l=qx; l<qx + SECTORS; l++)
						switch(mp[kk][l])
						{
							case 'p':
							case 'P':
							case '*':
							case 'h': qstar++;
								  break;
							case 'B': qb++;
								  break;
							case 'R':
							case 'r':
							case 'K':
							case 'k': qk++;
								  break;
							default: break;
						}

				nk[i][j] = qk;
				nb[i][j] = qb;
				ns[i][j] = qstar;
			}
		if(bc != 0 && !debug) printf("**-**-**   ");
		else printf("%2d-%2d-%2d   ",nk[i][j],nb[i][j],ns[i][j]);
		}
		printf("|");
		poscur(GMCOL,ppl+1);
		printf("|                                  |");
	}
	poscur(GMCOL,ppl+1);
	printf("+----------------------------------+");
}


/* globals for scan routines */

char lastscan[11][11];      /* stored results of last scan update */
spos[2];                    /* coordinates of last scan */


scan(code)
int code;               /* code 2 => hyperscan - show invisibles as '?'
			   code 1 => normal scan
			   code 0 => refresh display with last scan results
			*/
{
	register int i,j;
	int ii,jj;
	char symb;

		/* test if new scan to be made */

	if((sc == 0 || debug)           /* scanners available ? */
	    &&(newscan || code == 2)){  /* new scan needed ? */

	    spos[0] = sp[0];
	    spos[1] = sp[1];

	    for (i=0; i<11; i++) {
		for (j=0; j<11; j++)  {
			ii = spos[0] + i -5;
			jj = spos[1] + j -5;
			lastscan[i][j] = ' ';
			if(offmap(ii,jj)) continue ;

			/* select symbol for empty space */
			if(bc == 0) symb = '.';
			else symb = ' ';

			switch (mp[ii][jj])
			{
				case 'w':
				case 'm':
				case 'h':
				case 'k':
				case 'r':
					if(code == 2 ) symb = '?';
					break;
				case 'p':
					symb = '*';
					break;
				case '.':   /* empty space, leave symb as is */
					break;
				default:
					symb = mp[ii][jj];
					break;
			}
			if(debug) symb = mp[ii][jj];
			lastscan[i][j] = symb;
		}
	    }
	}

	showscan();
	newscan = 0; /* clear scan-request boolean */
}


showscan()           /* display results of lastscan */
{  register int i,j;
   char space;

	poscur(SCANCOL,SCANROW);
	printf("#========SCANNER========#\n");

	for (i=0; i<11; i++){   /* print out row number i */
		if( (spos[0]-4+i)%10 || bc) space = ' ';
		    else space = '.';
		poscur(SCANCOL,SCANROW+i+1);
		printf("#");
		if( (spos[1]-5)%10 || bc) printf("%c",space);
		    else printf("|");
		for (j=0; j<11; j++) {
			printf("%c",lastscan[i][j]);
			if( (spos[1]-4+j)%10 || bc) printf("%c",space);
			    else printf("|");
		}
		printf("#");
	}
	poscur(SCANCOL,SCANROW+12);
	printf("#=======================#");
}

scanup(ii,jj) int ii,jj ;
		       /* show object at (ii,jj) in the scanner display */
		       /* and update the stored last scan results */
{
	char symb;
	if(sc < 0 && !debug)return;
	switch (mp[ii][jj]) {
		case 'w':
		case 'm':
		case 'h':
		case 'k':
		case 'r':
		case '.':
			if(bc == 0) symb = '.';
			else symb = ' ';
			break;
		case 'p':
			symb = '*';
			break;
		default:
			symb = mp[ii][jj];
			break;
	}
	if(debug) symb = mp[ii][jj];
	if(posscan(ii,jj)){
	    printf("%c",symb);
	    lastscan[ii-spos[0]+5][jj-spos[1]+5] = symb;
	}
}

posscan(y,x) int y,x;                   /* positions cursor to the scanner location
					   corresponding to map coordinates y,x   */
{       int sx,sy;
	sy = y-spos[0]+5;
	sx = x-spos[1]+5;
	if(sx<0 || sx > 10 || sy < 0 || sy > 10) return(0);
	poscur(2*sx+SCANCOL+2,sy+SCANROW+1);
	return(1);
}



/*    globals for rest of I/O  */

int lastcol;            /* last cursor position in comline */



/*------------------------------------------------------

				OUTPUT ROUTINES

   -----------------------------------------------------*/

status()
{       int i,j,ppl;

	poscomp(POSLINE);
	if(bc == 0) {
		printf("Enterprise position ");
		printpos(sp[0],sp[1]);
	}
	else {
	    printf("computer down ... repair time = %d",abs(bc));

		/* blank out the galaxy map area */

	    if(!debug) {
		poscur(GMCOL,GMROW);
		printf("+------------GALAXY MAP------------+");
		for (i=0; i< QUADS; i++) {
		    ppl = GMROW + 2*i + 1;
		    poscur(GMCOL,ppl);
		    printf("| ");
		    for (j=0; j< QUADS; j++) printf("**-**-**   ");
		    printf("|");
		    poscur(GMCOL,ppl+1);
		    printf("|                                  |");
		}
		poscur(GMCOL,ppl+1);
		printf("+----------------------------------+");
	    }
	}

	if(sc != 0) {
		poscur(SCANCOL,SCANROW);
		printf(" scanner out for %d days   ",abs(sc));
	}

	poscomp(KHLINE);
	if(hits == 0) printf("No enemy vessels attacking");
	else  printf("Enterprise under attack!  level = %d",hits);

	poscomp(KVALINE);
	printf("%s",kvastring);

	if(mp[pro[0]][pro[1]] != ' ') {
	    poscomp(PCLINE);
	    if(pro[2] < 0 )
	       printf("urgent call from Procyon  Q: %1d-%1d",pro[0]/SECTORS+1,pro[1]/SECTORS+1);
	}

	poscomp(GPLINE);
	if(gal[2] < 0) {
		printf("Galileo requests pickup ");
		printpos(gal[0],gal[1]);
	}

	posstat(STATLINE);
	printf("number of stardates left = %d",DAYS-sd);
	erasel();

	posstat(STATLINE+1);
	printf("quarks in di-lithium crystals = %d",q);
	erasel();

	posstat(STATLINE+2);
	printf("Klingons: %d, Romulans: %d",kleft,rleft);
	erasel();
	posstat(STATLINE+3);
	printf("captured vessels (in tow) = %d(%d)",capt,tow);

	posstat(STATLINE+4);
	printf("number of torpedoes left = %d",t);
	erasel();

	posstat(STATLINE+5);
	erasel();
	if (df == 0) printf("energy to deflectors = %d",dfl) ;
	else printf("deflectors out ... repair time = %d",abs(df));


	posstat(STATLINE+6);
	erasel();
	if(tc < 0) printf("torpedos out ... repair time = %d",abs(tc));

	posstat(STATLINE+7);
	erasel();
	if(pc < 0) printf("phasers unusable");

	posstat(STATLINE+8);
	erasel();
	if(wc<0) printf("warp engines burned out");
}

coms() {
	poscur(0,STATLINE);
	printf(".........COMMAND LIST..........\n");
	printf("1 - impulse    9 - hyperscan\n");
	printf("2 - warp      10 - computer\n");
	printf("3 - phasers   11 - trajectory\n");
	printf("4 - torpedo   12 - open comm\n");
	printf("5 - spread    13 - galaxy map\n");
	printf("6 - nova-max  14 - tractor beam\n");
	printf("7 - shields   98 - unix shell\n");
	printf("8 - panic     99 - self-destruct\n");
}

printpos(y,x)
int x,y;
{
	printf("Q: %d-%d  S: %d-%d", y/SECTORS+1, x/SECTORS+1,
		y%SECTORS+1, x%SECTORS+1);

}

int slength(s) char *s ;                 /* finds length of a character string
					   up to the null */
{       int l ;
	l = 0;
	while(*s++ != '\0') l++ ;
	return (l);
}



printcom(s) char *s;
{
	poscur(lastcol,COMLINE);
	erasel();
	printf("%s",s);
	lastcol =+ slength(s);
}

printmsg(s) char *s;            /* print the message <s>  */
{
	poscur(0,MSGLINE);
	erasel();
	printf("%s",s);
}

alarmmsg(s) char *s;           /* ring the bell and print message */
{
	printf("\007");
	printmsg(s);
	sleep(1);
}

posstat(l) int l;
{
	poscur(STATCOL,l);
	erasel();
}
poscomp(l) int l;
{
	poscur(COMPCOL,l);
	erasel();
}

scanblink(y,x) int y,x;                 /*  blink the scanner position
					    corresponding to coordinate (y,x)       */
{
	if(sc < 0) return;
	posscan(y,x);
	blinkc(mp[y][x]);
	home();
	sleep(1);
}



/*--------------------------------------------------------------------

					INPUT ROUTINE

  --------------------------------------------------------------------*/

/*----- global variables used in these routines -----*/

char lastchar '\n';
char killchar,delchar;                  /* kill and delete characters */
int oldmode;                            /* saved value of terminal mode */
int echoflag 1;                         /* 1 => echo, 0 => no echo */

char numbuf[20];
int pos;                /* position in the numbuf array */

getang()                /* input angle between 0 and 360 */
{       int ang;
	ang = getval();
	while (ang < 0) ang =+ 360 ;
	return(ang%360);
}

getcom()                /* get a command line from the input device
				always starts on a new line after requesting
				a command */
{
	int com;
	int tvec[2],firstvec[2];   /* time vectors  */
	int delta;      /* time difference */
	gobble();       /* suck up any left over characters from last input */
	if (q <= 0 && q + dfl > 0)
	{
		q =+ dfl/2;
		dfl = 0;
		alarmmsg("quarks dangerously low ... deflectors have been dropped");
		status();
	}

#ifdef REALTIME
	if(!rubbed) time(firstvec); /* get initial time  */
		   /* if rubbed, last command entry was aborted so use
		      previous starting time */

	if (empty(0) && skill > 0 ) delay(integ(1.5+5.0/skill));
#endif

    comloop:
	lastcol = 0;
	printcom("COMMAND:");

#ifdef REALTIME
	while(empty(0) && skill ) {      /* check for input
					 - if none, then allow attack
					 if skill is 0, no real-time feature  */
		time(tvec);
		delta = tvec[1]-firstvec[1];
		if (abs(delta) > ((10.0/skill) + hurt) ) {
		    newday(1);
		    status();
		    if (dfl < 2*hits && df == 0)  alarmmsg("deflector power dangerously low");
		    time(firstvec); /* reset initial time  */
		    poscur(lastcol,COMLINE);
		}
	}
#endif

	com = getval();
	if(rubbed) goto comloop ;
	poscur(0,MSGLINE);
	erasel();
	return(com);
}

gobble()                     /* gobble rest of last line up to newline char */
{
	if(lastchar != '\n')
	    while (mygetchar() != '\n');
}

backup()                                 /* backs up the input */
{       if(pos <= 0) return(0);
	if(echoflag) printf("\b \b");       /* echo backspace */
	lastcol--;
	return(--pos);
}

mygetchar()
{       lastchar = getchar() & 0177 ;   /* mask off parity bit */

	rubbed = 0;
	if(lastchar == DEL) {        /* DEL , so set rubbed true */
		lastchar = '\n';
		rubbed = 1;
	}

	if(lastchar == EOT) wrapup(-1);
	if(lastchar == delchar) {
		backup();
		return(mygetchar());
	}
	if(lastchar == killchar) {
		while(backup());
		return(mygetchar());
	}
	if(echoflag) {
		printf("%c",lastchar);
		lastcol++;
	}
	return (lastchar);
}
getval1()                       /* get next input value but don't echo */
{       int val;
	echoflag = 0;
	val = getval();
	echoflag = 1;
	return(val);
}

int oldcol;                             /* column position of last call to getval */

getagain()               /* repeat getval from same position as last one */
{
	lastcol = oldcol;
	poscur(COMLINE,lastcol);
	erasel();
	return(getval());
}


getval()                        /*  get the next integer value from input
				    check for character and line kills */
{
	oldcol = lastcol;
	pos = 0;
	printf(" ?\b");
	lastcol++;

	while(mygetchar() == ' ');      /* skip spaces */


				/* scan over the number,
				   transferring digits to numbuf */

	while(lastchar != ' ' && lastchar != '\n' && lastchar != '\r') {
	    switch (lastchar) {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		    numbuf[pos++] = lastchar;
		    break;

		case '+': break;

		case '-': if (pos == 0) {
				numbuf[pos++] = '-';
				break;
			   }
			 /* if not in first position, it is illegal */

		default:  /* illegal character */
			printmsg("INVALID DIGIT - retype");
			sleep(1);
			printmsg(" ");
			lastcol-- ;
			poscur(lastcol,COMLINE);
			printf("?");
			poscur(lastcol,COMLINE);
	    }
	    mygetchar();
	}
	numbuf[pos] = '\0';
	if(rubbed) return(0);
	else return(atoi(numbuf));
}

struct {
	char ispeed,oseed;
	char erase,kill;
	int mode;
       } ttystat;

rawterm()
{       gtty(0,&ttystat);
	killchar = ttystat.kill & 0177;
	delchar = ttystat.erase & 0177;
	oldmode = ttystat.mode;
	ttystat.mode = (oldmode | 060) & ~010 ;    /* set bits for raw mode,
						      CR-LF, and no echo  */
	stty(0,&ttystat);
	signal(2,1);
}

cookterm()
{       ttystat.mode = oldmode;
	stty(0,&ttystat);
	signal(2,0);
}

terminit()
{       rawterm();
	rolloff();
	homers();
}

resterm()
{
	cookterm();
	roll();
}
