/*  macros  */
#define store(WHO, CARD, NUM)  WHO[CARD / 100][CARD % 100] = NUM
#define GINTEXT "/usr2/suz/gin/gin.text"
#define YES 1
#define NO -1
#define NULL -1

/*	"global" variables	*/
int debug, deebug;	/* 2-level debugging */
int mine[4][13];	/*  program's hand  */
int yours[4][13];	/*  human's hand  */
int data[100],link[100],avail;	/*  stuff for linked list  */
int deck[52];	/*  deck of cards  */
int deckbot,decktop;	/*  bottom and top of unused deck  */
int pile;	/*  top of discard pile  */
char rank[] {"23456789tjqka"};	/*  the pips  */
char suite[] {"cdhs"};	/*  the suites  */
char ans[3];	/*  buffer used for human's answers  */
char buff[512];	/*  buffer used for reads  */
char huh[] {"Do what for?"};	/*  i.e. 'pardon?'  */
char name[] {"sweetie"};	/*  term of endearment  */

/*  The following 4 declarations are for things used in the evaluation
routines.
	cval[n]		value of n cards in one column
	rval[n]		value of n cards in a row (straight flush)
	decr[n]		decrements for cards unobtainable (column)
	cs		value of each card by virtue of column
	rs		value of each card by virtue of row
	sct		scratch array (used several places)
	quad		head of linked list for quads
	tri		head of linked list for triples
\*/
int cval[] {0,0,8,12,16};
int rval[] {2,1,8,12,16,17,18,18,18,18,18,18};
int decr[3] {0,1,2};
int cs[4][13],rs[4][13],sct[4][13],quad,tri;


main() {
	int card;	/*  used to store cards for deal and draw  */
	int discard;	/*  store discard  */
	int dead;	/*  head of linked-l of cards human picks  */
	int wins,losses;	/*  number of games won and lost  */
	int len,fd;	/*  used to read info file  */
	int gin;	/*  gin in hand, YES or NO  */
	int i,j,temp;	/*  i,j subscript var.  temp var.  */
	int seed;	/*  seed for random num. generator  */

	printf(" Please enter a 5 digit positive odd integer.\n");
	scanf("%5d",&seed);
	printf(" seed is: %d\n",seed);
	srand(seed);


	for(;;){			/*  the game starts here  */
	deebug = NO;
	debug = NO;
/*
	printf(" Do you want deebug on?\n");
	getans();
	if(ans[0] == 'y') deebug = YES;
	printf(" Do you want debug on?\n");
	getans();
	if(ans[0] == 'y') debug = YES;
*/
	printf(" Would you like to have some information about the game?\n");
	getans();
	if(ans[0] == 'y') {
		if((fd = open(GINTEXT,0)) < 0)
			printf("instructions not available\n");
		else { while((len = read(fd,buff,512)) > 0)
				write(1,buff,len);
			getans();
			}
		}
	newdeck();
	shuffle(52);
/* the deal */
	initall(0,100);
	i = 0;
	while(i < 20) {
		card = deck[i++];
		store(yours,card,1);
		card = deck[i++];
		store(mine,card,1);
		}
	decktop = 20;
	deckbot = 51;
	pile = -1;
	dead = -1;
	printf(" Your hand:\n");
	handout(yours);			/*  print humans hand  */
	if(debug == YES)  handout(mine);
	draw(&discard);		/*  top card face up  */

	for(;;) {		/*  beginning of infinite loop  */
	printf("%c%c is on top of the pile, do you want it?\n",rank[discard%100],suite[discard/100]);
	getans();
	if(ans[0] == 'l') {	/*  list hand  */
		handout(yours);
		if(deebug == YES)  handout(mine);
		printf(" Do you want the %c%c\n",rank[discard%100],suite[discard/100]);
		getans();
		}
	if (ans[0] != 'y') {			/*  you draw   */
		stack(&pile,discard);
		draw(&card);
		store(yours,card,1);
		store(mine,discard,-1);
		printf("You drew the %c %c\n",rank[card%100],suite[card/100]);
		}
	else {					/*   you took it  */
		store(yours,discard,1);
		stack(&dead,discard);
		store(mine,discard,-2);
		}
	if(rcard(&card) == YES) goto youwin;
	della(card); 		/*  delete card from your hand  */
	store(mine,card,1);		/*   put into mine  */
	gin = val(&discard);
	if(discard == card) {  			/*  I draw  */
		stack(&pile,discard);
		store(mine,discard,-1);
		draw(&card);
		store(mine,card,1);
		gin = val(&discard);
		printf(" I drew.\n");
		}
	else printf("I'll take that one.\n");
	if(gin == YES) goto iwin;
	} 				/*  end of infinite loop   */


youwin:
	for(i = 0;i < 4;i++)	/*  swap hands  */
		for(j = 0;j < 13;j++) {
			temp = mine[i][j];
			mine[i][j] = yours[i][j];
			yours[i][j] = temp;
			}
	gin = val(&discard);	/*  evaluate your hand  */
	if(gin == YES){		/*  you are right  */
		printf("You did it %s\n",name);
		printf("\n My hand:\n");
		handout(yours);
		losses++;
		}
	else {		/*  you are wrong, you forfeit the game  */
		printf("I think you are wrong, %s\n",name);
		wins++;
		}
	goto finish;
iwin:
	printf(" I discard %c %c and GIN!\n",rank[discard%100],suite[discard/100]);
	store(mine,discard,-1);
	handout(mine);
	wins++;
finish:
	printf("Would you like to play again?\n");
	getans();
	if(ans[0] != 'y') break;
	printf("Here we go.\n");
	}			/*   end of game  */

	printf("I won %d games and you won %d games.\nThank you for playing our game.\n",wins,losses);
	exit();
}			/*  end of main   */

newdeck(){		/*  start with a "new" deck  */
	int k,i,j;
	k =0;
	for(i = 0;i < 4;i++)
	for(j = 0;j < 13;j++)
		deck[k++] = 100 * i + j;
	return;
	}

shuffle(no) int no; {	/*  algorithum from "Creative Computing"  */
	int i,k,l;
	float frand();
	for(i = 0;i < (no - 1);i++){
		k = deck[i];
		l = (no - 1 -i)*frand() + 1;
		deck[i] = deck[l + i - 1];
		deck[l +i - 1] = k;
		}
	return;
	}

float frand(){		/*  ramdom no. 0 <= no <= 1  */
	float big;
	big = 077777;
	return(rand()/big);
	}

handout(whos) int whos[4][13]; {	/*  list cards in hand  */
	int i,j;
	for(i = 0;i < 4;i++) {
		putchar('\n');
		for(j = 0;j < 13;j++) {
			if(whos[i][j] > 0)  printf("  %c%c",rank[j],suite[i]);
			else printf("    ");
			}
		}
	printf("\n\n\n");
	return;
	}

initall(i,cells) int i,cells; {		/*  init hands and free pool */
	int q,r;
	for(r = 0;r < 4;r++) for(q = 0;q < 13;q++) {
		mine[r][q] = i;
		yours[r][q] = i;
		}
	for(q = 0;q < cells;q = link[q] = q + 1);
	link[cells - 1] = NULL;
	avail = 0;
	return;
	}
 
celia(q) int *q; {	/*  get cell from free pool  */
	*q = avail;
	avail = link[avail];
	if(avail == NULL) erin(1);
	return(*q);
	}

rhett(q) int q; {	/*  return cell to free pool  */
	link[q] = avail;
	avail = q;
	return;
	}

stack(p,card) int *p,card; {	/*  push card onto stack p  */
	int temp,q;
	temp = *p;
	*p = celia(&q);
	data[*p] = card;
	link[*p] = temp;
	return;
	}

draw(c) int *c; {	/* draw card, reshuffle if necessary */
	int q,r,s,v,x,y;
	if((q = decktop++) > deckbot) {
		printf("We used the whole deck, I will reshuffle.\n");
		s = 0;
		for(r = pile;r >= 0;) {
			deck[s++] = data[r];
			x = data[r] / 100;
			y = data[r] % 100;
			mine[x][y] = 0;
			v = r;
			r = link[r];
			rhett(v);
			}
		shuffle(s);
		deckbot = s - 1;
		q = 0;
		decktop = 1;
		pile = -1;
		}
	*c = deck[q];
	return(*c);
	}

della(card) int card; {		/*  delete card from human's hand  */
	for(;;) {
		if(yours[card/100][card%100] == 1)  {
			yours[card/100][card%100] = 0;
			return;
			}
		printf(" You don't have that card.\n");
		rcard(&card);
		}
	}

rcard(c) int *c;  {	/*  read card human discards  */
	int len;
	char *t;
	for(;;)  {
		printf(" What is your discard ?\n");
		if((len = getans()) == 0) continue;
		if(deebug == YES) printf(" ans %c%c%c\n",ans[0],ans[1],ans[2]);
		if(ans[0] == 'l')  handout(yours);
		else  {
			if(ans[0] == 'g' && ans[1] == 'i' && ans[2] == 'n')
				return(YES);
			for(t = rank;*t != '\0' && ans[0] != *t;t++)  ;
			if(*t == '\0') continue;
			*c = t - rank;
			for(t = suite;*t != '\0' && ans[1] != *t;t++)  ;
			if(*t == '\0') continue;
			*c =+ (t - suite) * 100;
			return(NO);
			}
		}
	}

getans() {	/*  read human's answer  */
	int lans,len,i;
	lans = 0;
	len = (read(0,buff,512)) - 1;
	if(len < 0) exit();
	for(i = 0;i < len && lans < 3;i++)
		if(buff[i] != ' ') ans[lans++] = buff[i];
	return(lans);
	}

erin(q) int q; {	/*  fatal error routine  */
	if(q == 1) { printf(" AVAIL LIST IS EMPTY\n");
		printf("AVAIL IS %d\n",avail);
		}
	else if(q == 2) printf("I tried 6 times to decode your answer. \n");
	exit();
	}
 
val(t) int *t; {	/*  evaluation of cards in hand  */
			/*  *t - discard  */
	int q,r;	/*  subscript variables  */
	int cnt,miss;	/* cards present and absent in col. */
	int zip;	/* nonmnemonic - debit for dead cards in col */
	int pt1,pt2;	/*  pointers into row for counting flush  */
	int w,x;	/* used to evaluate rows */
	double a,b,fct();   /* fct(a,b) finds absolute value of card */
	quad = tri = NULL;	/* quad and triple set stacks */

/*  evaluate each card by virtue of column (cs is col. sum)  */

	if(deebug == YES) printf("entering val  ");
	for(q = 0;q < 13;q++){
		cnt = 0;
		miss = 0;
		for(r = 0;r < 4;r++){
			if(mine[r][q] > 0) cnt++;
			else if(mine[r][q] < 0) miss++;
			rs[r][q] = 0;
			cs[r][q] = 0;
			}
		if(cnt > 1) {
			if(cnt == 4) {
				stack(&quad,(q - 100));
				stack(&tri,(q - 100));
				zip = 0;
				}
			else if(cnt == 3) {
				stack(&tri,(q - 100));
				zip = 0;
				}
			else zip = decr[miss];
			for(r = 0;r < 4;r++)
				if(mine[r][q] > 0) cs[r][q] = cval[cnt] - zip;
			}
		}

/*  evaluate cards by virtue of row  (rs is row sum) */

	if(deebug == YES) printf("  row check complete  ");
	for(r = 0;r < 4;++r) {
		pt1 = 0;
		pt2 = -5;
		q = 0;
		while(q < 13) {
			miss = 0;
			while(mine[r][q] <= 0) {
				if(mine[r][q] < 0) miss++;
				if(++q > 12) goto leave2;
				}
			pt1 = q;
			if(((w = pt1 - pt2) < 3) && (miss == 0)) {
				rs[r][pt2 - 1] =+ rval[w - 1];
				rs[r][pt1] =+ rval[w - 1];
				}
			for(;(q < 13) && (mine[r][q] > 0);q++) ;
			pt2 = q;
			if((w = pt2 - pt1) > 1) for(x = pt1;x < pt2;x++) {
				rs[r][x] =+ rval[w];
				if((pt2 - x) >= 4) stack(&quad,(r * 100 + x));
				if((pt2 - x) >= 3) stack(&tri,(r * 100 + x));
				}
leave2:  ;		}
		}
	if(deebug == YES) printf("  column check complete  ");
	if(debug == YES) { putchar('\n'); barf(mine); barf(rs); barf(cs);} 
/*  return YES if gin found  */
	if((quad >=0) && (bombay(t) == YES))  return(YES);
	if(deebug == YES) printf("  bombay passed ");
/*  else find discard  */
	a = 4872.;
	for(q = 0;q < 13;q++)
		for(r = 0;r < 4;r++)
			if((mine[r][q] > 0) && ((b = fct(r,q)) <= a)) {
				a = b;
				*t = r * 100 + q;
				}
	if(deebug == YES) printf("  discard found  ");
/*  clean up stacks  */
	for(w = quad;w >= 0;) {
		x = w;
		w = link[w];
		rhett(x);
		}
	for(w = tri;w >= 0;) {
		x = w;
		w = link[w];
		rhett(x);
		}	
	if(deebug == YES) printf("  all set to return \n");
	if(debug == YES) printf(" I discard %d \n",*t);   
	return(NO);	/*  gin not found  */
	}

double fct(r,q) int r,q; {	/* evaluation of card mine[r][q] */
	double d;
/*  this is just the distance formula - sqrt(a*a + b*b)  */
	return(sqrt(d = (rs[r][q] * rs[r][q] + cs[r][q] * cs[r][q])));
	}

/*  This is the heart of the program. bombay is a backtrack	*/
/*  algorithum that trys all combinations to find gin.		*/
/*  First a quad is layed out, then one triple and last		*/
/*  triple.  Backtrack from there.				*/
bombay(t) int *t; {
	int i,j,qpt,qcd;
	int out1,lout1,tpt1,tcd1;
	int out2,lout2,tpt2,tcd2;
	for(i = 0;i < 4;i++)
		for(j = 0;j < 13;j++)
			sct[i][j] = 0;
	for(qpt = quad;qpt != NULL;qpt = link[qpt]){  /*  outer loop */
		if((qcd = data[qpt]) < 0) nofk(4,(qcd + 100),NULL);
		else nrun(4,qcd,4);
		out1 = 0;
		lout1 = NO;
		for(tpt1 = tri;tpt1 != NULL;) {     /*  inner loop */
			if((tcd1 = data[tpt1]) < 0) {
				for(i = 0;i < 4;i++)
					if(mine[i][(tcd1 + 100)] <= 0){
						lout1 = YES;
						out1 = i;
						}
				nofk(1,(tcd1 + 100),out1);
				}
			else nrun(1,tcd1,3);
			out2 = 0;
			lout2 = NO;
			for(tpt2 = link[tpt1];tpt2 != NULL;) {
				if((tcd2 = data[tpt2]) < 0) {
					for(i = 0;i < 4;i++)
						if(mine[i][(tcd2 + 100)] <=0) {
							lout2 = YES;
							out2 = i;
						}	
					nofk(2,(tcd2 + 100),out2);
					}
				else  nrun(2,tcd2,3); 
				if(deebug == YES)  { barf(sct); getans(); }
				if(win(t) == YES) return(YES);
				if(tcd2 < 0) {
					nofk(-2,(tcd2 + 100),out2);
					if((lout2 == YES) || (out2++ == 3)) {
						out2 = 0;
						tpt2 = link[tpt2];
						lout2 = NO;
						}
					}
				else {
					tpt2 = link[tpt2];
					nrun(-2,tcd2,3);
					}
			}  		 /*   end of innermost loop  */
			if(tcd1 < 0) {
				nofk(-1,(tcd1 + 100),out1);
				if((lout1 == YES) || (out1++ == 3)) {
					tpt1 = link[tpt1];
					out1 = 0;
					lout1 = NO;
					}
				}
			else {
				tpt1 = link[tpt1];
				nrun(-1,tcd1,3);
				}
			}
		if(qcd < 0)  nofk(-4,(qcd + 100),NULL);
		else nrun(-4,qcd,4);
		}			/*   end of outer loop  */
	return(NO);
	}

nofk(valu,k,out) int valu,k,out; {
	int i;
	for(i = 0;i < 4;i++)
		sct[i][k] =+ valu;
	if(out != NULL) sct[out][k] =- valu;
	return;
	}

nrun(valu,k,n) int valu,k,n; {
	int i,j,q;
	i = k / 100;
	for(j = (k % 100);j <  (k % 100 + n);j++)
		sct[i][j] =+ valu;
	return;
	}

win(t) int *t; {
	int i,j;
	for(i = 0;i < 4;i++) 
		for(j = 0;j < 13;j++) {
			if((sct[i][j] > 4) || (sct[i][j] == 3)) return(NO);
			if((mine[i][j] > 0) && (sct[i][j] == 0))
				*t = i * 100 + j;
			}
	return(YES);
	}

barf(x) int x[4][13]; {
	int r,q;
	for(r = 0;r < 4;r++) {
		for(q = 0;q < 13;q++)
			printf("%3d",x[r][q]);
		putchar('\n');
		}
	putchar('\n');
	}

