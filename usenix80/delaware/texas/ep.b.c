#ifdef ADM
#include "epa.defs"
#endif
#ifdef INFOTON
#include "epi.defs"
#endif
#define EMPTY    (fin.nleft == 0 && empty(0))
#define MAX(A,B) ((A) > (B) ? (A) : (B))
#define MIN(A,B) ((A) < (B) ? (A) : (B))
#define STTYRAW 040 | (rtcode[2] & ~ 072);
#define DISPLAY  "/bin/cat","cat"
#define PRINT    "/bin/opr","opr"

struct getchrbuf {
	int fildes;     /* File descriptor */
	int nleft;      /* Chars left in buffer */
	char *nextp;    /* Ptr to next character */
	char buff[512]; /* The buffer */
	};
extern struct getchrbuf fin;   /* getchar buffer, used in EMPTY test */
extern char ttynstr[9] "/dev/tty";
extern int *bptr { bspace };   /* pointer into bundle space */
extern int iputcbuf 0;
extern int *errlab;
char *insertmode " *** I N S E R T   M O D E *** ";
char *cntrlmode  " *** < C T R L >   M O D E *** ";
char savebuf[LBSIZE];
int siginsert 0; /* signal insert mode: on if siginsert == 1 */
int forcedcol 0; /* signal forced column diaplay */
int argon NO;   /* flag: are we coming out of argument mode? */
char epfrst 0;  /* first time flag */
int char_sec;   /* tty output speed */
int ttyspeed[16] {4,6,10,12,15,30,60,120,180,200,240,360,480,720,960,1920};
double atof();

/* ep -
Ed for people.  Incorporates ed, along with features
suggested by the RAND editor.
The ep commands are in the routine feedin.
File line manipulation is carried out through ed.
For input, ed is alerted only in the event that a change is made to a line.
When the user positions the cursor to a line the global <change> flag is
set to OFF.
Every time an operation occurs that causes attention to shift away from the
current line the <change> flag is checked by the routine changes().
If the flag is ON, a change command for that line is issued to ed.
To the user, ep appears much the same as the RAND editor.
*/
ep() {
	/* initializations */
	change = OFF;
	load_line = -1;
	siginsert = OFF;
	forcedcol = OFF;
	tellinsert = NO;
	tellcntrl = NO;
	tellcol = NO;
	tellrow = -1;
	tellext = NO;
	tdisable = NO;
	searching = NO;
	msg_on = NO;
	user = BOY;
	initport();
	clear(combuf,CBSIZE);
	clear(buf,BSIZE);

	settty();

	/* Provide for user regain of control on error from ed */
	if ( setexit() ) {
		if ( searching ) {
			tell("STRING NOT FOUND");
			searching = NO;
			argon = NO; 
			}
		else    {
			user = BOY;
			if( epfrst == 0 ) {
				tell("Bad file");
				write(1,"\r\n",2);
				goto stop1;
			}
			tell("ERROR FROM ed");
			load_line = -1;
			}
		clear(buf,BSIZE);
		poscursor(col,row);
		pulldot();
		}

	if( epfrst == 0 ) {
		if( *globp == 'r' ) commands();
		else {
			ZAPTUBE;
			initscreen(1);
		}
		epfrst++;
	}

cmd:    FOREVER {
		update();
		if ( feedin(getchar()&0177) == QUIT ) break;
		}

	if( secondchance() ) {
		clearmessage();
		poscursor(col, row);
		goto cmd;
	}

stop:   ZAPTUBE;
stop1:  cleanup();
	return;
	}

/* settty - 
Set up tty raw mode.
*/
settty()
	{
	register i;

	ttycode[0] = ttyn(0);
	ttycode[1] = '\0';
	ttyname = tag_on(ttynstr,ttycode);
	/* set tty raw mode */
	gtty(0,rtcode);
	i = rtcode[2];
	rtcode[2] = STTYRAW;
	stty(0,rtcode);
	fin.nleft = 0;
	rtcode[2] = i;   /* set for cleanup routine */
	char_sec = ttyspeed[rtcode[0]&017];
	return;
	}

/* maketerm -
Make the terminal do something, e.g., go home, ring bell, etc.
*/
maketerm(dothis) char dothis; {
	register struct portdes *ppt;

	putcha(dothis);
	dumpcbuf();
	if( dothis == CLEARSCREEN ) {
		msg_on = NO;
		tellinsert = NO;
		tellcntrl = NO;
		tellcol = NO;
		tellrow = -1;
		tellext = NO;
		for( ppt = &port[0]; ppt < &port[SSIZE-1]; ppt++) {
			ppt->indent = MAXCOL;
			ppt->len = 0;
			ppt->disp = NO;
			}
		}
	}

dumpcbuf() {
	extern int iputcbuf;
	write(1,&putcbuf,iputcbuf);
	iputcbuf = 0;
	}

/* poscursor -
Position cursor   
*/
poscursor(colp,rowp) int colp, rowp;
{
	if( colp >= MAXCOL )
		if( rowp == tellrow ) {
			colp = EXTCOL + (colp+1) % EXTMOD;
			rowp = LOCMSG-1;
			}
		else colp = MAXCOL;
#ifdef LEADIN
	putcha(LEADIN);
#endif
	putcha(CURSORCOME);
#ifdef ROW_COL
	putcha(rowp + ROWBASE);
	putcha(colp + COLBASE);
#else
	putcha(colp + COLBASE);
	putcha(rowp + ROWBASE);
#endif
	dumpcbuf();
	return;
}

putcha(c) char c; {
	extern int iputcbuf;
	if (iputcbuf == 4) dumpcbuf();
	putcbuf[iputcbuf++] = c;
	return;
	}

/* tell -
Screen message function.
Shows the string <s> in user defined message space on screen.
<msg_on> is a global signal indicating the occurence of a message in the
message space.
*/
tell(s) char *s;
	{
	register char *sp;
	sp = s;
	if ( msg_on ) clearmessage();
	poscursor(0,LOCMSG);
	msg_on = YES;
	maketerm(RINGBELL);
	while( *sp && sp-s < ECAREA ) write(1,(sp++),1);
	if( *sp ) write(1,"\b>",2);
	return;
	}

/* cleanup -
Restore tty cooked mode.
*/
cleanup()
	{
	/* restore tty mode */
	stty(0,rtcode);
	fin.nleft = 0;
	}

/* tag_on -
Return the concatenation of the strings <tag> and <ext>.
*/
tag_on(tag,ext)
char *tag,*ext;
	{
	int lname;
	register char *c,*d,*newtag;

	lname = 0;
	c = tag; while (*c++) ++lname;
	c = ext;  while (*c++) ++lname;
	c = namekeep;
	newtag = c;
	d = tag; while (*d) *c++ = *d++;
	d = ext; while (*c++ = *d++);
	return newtag;
	}

/* clrlines -
Blank out <extent> char positions of <num> lines
starting from row <start>.
*/
clrlines(start,num,extent) {
	register j, lct, cct;
	if ( start == LOCMSG ) msg_on = NO;     /* turn off message flag */
	lct = start;
	for (j=1; j <= num; j++) {
		poscursor(0,lct++);
		for( cct = 0; cct< extent; cct++) write(1," ",1);
		}
	poscursor(0,start);
	}

/* blank -
Fill a buffer with blanks.
*/
blank(buffer) char buffer[];
	{
	register j;
	for ( j=0; j < BSIZE; j++) buffer[j] = ' ';
	}

/* tabmov -
Modify col (which is global) til ya hit a tabstop.
Works only for tabs spaced equidistantly.
Tab interval is set globally, with TABDIS.
*/
tabmov(way) {
	if ( way == FORWARD ) {
		while( !tabpos(++col) );
		col--;
		moveright();
		}
	else {
		while( !tabpos(--col) );
		col++;
		moveleft();
		if( !tabpos(col) ) tabmov(BACK);
		}
	}

/* moveleft -
Move cursor one col left.
*/
moveleft() {
	if ( --col < 0 ) {
		loadline();
		poscursor(col=MAX(MAXCOL,load_len),row);  /* wrap around */
		return(0);      /* can't go left */
		}
	poscursor(col,row);
	return(1);      
	}

/* moveright -
Move cursor one col right.
*/
moveright() {
	if ( ++col > MAXCOL ) {
		loadline();
		if ( load_len <= MAXCOL ) {
			poscursor(col=0,row);   /* wrap around */
			return(0);      /* can't go right */
			}
		}
	poscursor(col,row);
	return(1);
	}

/* movedown -
Do a linefeed if typemove = 0, a <cr> otherwise.  
*/
movedown(typemove) int typemove;
	{
	row++;
	if ( row > MAXROW )     /* wrap around */
		row = MINROW;
	if ( typemove == 0 ) poscursor(col,row);
	else poscursor(col=0,row);
	return;
	}

/* moveup -
Move cursor up one row.
*/
moveup()
	{
	row--;
	if ( row < MINROW )     /* wrap around */
		row = MAXROW;
	poscursor(col,row);
	return;
	}

/* call_ed -
Assemble the command to be passed to ed; then reset bspace,
ready for new commands.
<Globp> is a global variable indicating to
ed the source of input to <egetchar()>.
*/
call_ed() {
	bundle("\n");   /* all ed commands are trailed by \n0 */
	globp = out = combuf;   /* out is incremented by collect's routines */
	collect(order); /* assemble command in command buffer */
			/* order points to head of bundle space */
	commands();     /* call editor */
	bclear();       /* clear bundle space */
	clear(combuf,CBSIZE);   /* clear command buffer */
	}

/* bundle -
Takes a variable number of arguments.
*/
bundle(a1,a2,a3,a4,a5,a6,a7) {
	int i,j,*p,*bptr1;

	i = nargs();
	if ( a1 == GO ) {
		i--;
		p = &a2;
		if ( bptr != bspace ) bptr++;
		}
	else  p = &a1;
	bptr1 = bptr;

	for ( j=0; j<i; j++ ) {
		if ( bptr > &bspace[BSIZE] ) {
			printf("blow up in bspace\n");
			exit();
			}
		*bptr++ = *p++;
		}
	*bptr = 0;
	return(bptr1);
	}

/* bout -
 * Output bundle - if the input argument is an address of an element in
 * bspace, then recursively feed bout the CONTENT of that address (which
 * is itself an address); otherwise take the argument as the address of
 * of a character string. */
bout(p) int *p; {
	if ( p >= bspace && p < &bspace[BSIZE] )
		while ( *p ) bout(*p++);
	else printf("%s",p);
	}

/* clear (initialize) bundle space */
bclear() {
	bptr = bspace;
	}

/* stuff -
Stuff bundle into combuf.   
*/
stuff(pr) char *pr; {
	while ( *pr ) {
		if ( out >= &combuf[CBSIZE] ) err("command line blowup");
		*out++ = *pr++;
		}
	*out = 0;
	}

/* collect -
Gather up pieces of bundle, stuff them into combuf.   
*/
collect(p) int *p; {
	if ( p >= bspace && p < &bspace[BSIZE] )
		while ( *p ) collect(*p++);
	else stuff(p);
	}

err(mes) char *mes; {
	printf("******* %s *******\n",mes);
	exit();
	}

clear(p,extent) char *p; {
	register j;
	for ( j=1; j <= extent; j++ ) *p++ = 0;
	}

/* tabpos -
Return yes if you're at a tabstop.
Works only with for tabs spaced equidistantly at TABDIS
*/
tabpos(ccol) {
	if ( ccol % TABDIS ) return(NO);
	else return(YES);
	}

/* detab -
Insert blanks until a tab position is reached.   
*/
char *detab(pos,start) char *pos, *start; {
	register column;
	column = pos - start;

	FOREVER {
		if ( tabpos(++column) == YES ) {
			*pos++ = ' ';
			return(pos);
			}
		else *pos++ = ' ';
		}
	}

/* entab -
Put tabs in a string.
Make logical adjustments for tabs already present.
*/
entab(p1,pos) char *p1; {
	register newpos;
	register char *p2;

	p2 = p1;
	newpos = pos;
	FOREVER {
		for ( ; *p2 == ' '; p2++ )
			if ( tabpos(++newpos) ) {
				*p1++ = TAB;
				pos = newpos;
				}
		if ( *p2 == TAB ) { /* logical skip to next tab position */
			while ( !tabpos(++newpos) );
			pos = newpos;
			*p1++ = *p2++;
			continue;
			}
		while ( pos++ < newpos ) *p1++ = ' ';
		while ( *p1++ = *p2++ );
		return;
		}
	}

/* mopup -
Erase remainder of screen line if necessary.
clm is the column position of cursor on start.
*/
mopup(pre,post,clm) {
	if ( pre <= post ) return;
	while ( post++ <= pre && clm++ <= MAXCOL) write(1," ",1);
	}

/* skipblk -
Scan string from left to right until first non-blank char is found.  
*/
skipblk(start) char *start; {
	register char *move;
	move = start;
	for ( ; *move == ' '; move++ )
		if ( *move ==  '\0' ) return(0);
	return(move-start);
	}

/* initport -
Initialize port descriptor.  
*/
initport() {
	register ct;
	register struct portdes *rpt;
	ct = 1;
	for ( rpt=port; rpt <= &port[SSIZE-1]; rpt++ ) {
		rpt->line = ct++;
		rpt->indent = rpt->len = rpt->disp = 0;
		}
	col = row = 0;
	}

/* cntrlchar -
Is char a control character?
*/
cntrlchar(c) char c; {
	c &= 0177;
	if( c <= 037 ) return(CNTRL);
	if( c == 0177 ) return(RUB);
	return(NO);
	}

/* putnow -
Put a character out pronto.
*/
putnow(c) char c; {
	write(1,&c,1);
	}

/* loadline -
Bring the line referenced on screen into linebuf.
Recall that zero is core address of start of line description table in ed.
*/
loadline() {
	register *a1;

	/* if screen line isn't loaded, load it */
	if ( load_line != port[row].line ) {
		changes();
		a1 = port[row].line + zero;
		if( a1 <= dol ) getline(*a1);
		else *linebuf = 0;
		/* signal that current line is loaded */
		load_line = port[row].line;
		load_len = length(linebuf);
		}
	}

/* getarg -
Gets argument from screen message space.
The control function which takes the argument is returned.
Ignores cursor positioning except RT, LT.
*/
char getarg() {
	register char *p;
	register char input;
	register ckeep;
	int rkeep,clm;

	p = buf;
	ckeep = col;
	rkeep = row;
	if ( msg_on ) clearmessage();
	poscursor(0,row=LOCMSG);
	write(1,"ARG: ",5);
	col = 5;
	msg_on = YES;

	FOREVER {
		switch(input = getchar()&0177)
			{
			case LT:
				if ( p > buf ) {
					--p;
					poscursor(--col,row);
					}
				else maketerm(RINGBELL);
				break;
			case RT:
				if ( col < ECAREA ) {
					if( *p == 0 ) *p = ' ';
					++p;
					poscursor(++col,row);
					}
				else maketerm(RINGBELL);
				break;
			case ARG:
				/* forget it */
				clearmessage();
				poscursor(col=ckeep,row=rkeep);
				return(DONE);
			case ED:
				/* user interface to ed */
				if ( *buf == 0 ) {
					tell("ED NEEDS AN ARGUMENT");
					tdisable = YES;
					poscursor(col=ckeep,row=rkeep);
					return(DONE);
					}
				poscursor(0,LOCMSG);
				write(1," ED: ",5);
				poscursor(0,LOCMSG);
				col = ckeep;
				row = rkeep;
				changes();
				user = BOSS;
				position = FULLSCREEN;
				trimblks(buf);
				order = bundle(GO,buf);
				call_ed();
				pulldot();
				user = BOY;
				if ( !tdisable ) clearmessage();
				poscursor(col,row);
				return(DONE);
			case HELP:
				clm = col;
				col = ckeep;
				row = rkeep;
				help();
				poscursor(0,LOCMSG);
				write(1,"ARG: ",5);
				puts1(buf);
				msg_on = YES;
				poscursor(col=clm,row=LOCMSG);
				break;
			case POSIT:
			case BAKPAGE:
			case PAGE: 
			case BSHIFT:
			case FSHIFT:
			case OPEN:
			case CLOSE: 
			case FSRCH:
			case BSRCH:
			case PICK:
			case MORE:
			case DROP:
				trimblks(buf);
				poscursor(col=ckeep,row=rkeep);
				return(input);
			default:
				if ( col >= ECAREA || cntrlchar(input) ) {
					maketerm(RINGBELL);
					break;
					}
				putnow(input);
				*p++ = input;
				col++;
			}
		}
	}

/* trimblks -
Trim blanks.
*/
trimblks(str) char *str; {
	register char *spt;
	register char *start;
	start = str;
	spt = scan(str,RIGHT,0);
	for ( --spt; *spt == ' ' && (spt >= start); spt-- ) *spt = 0;
	return;
	}

/* scan -
Moves left or right in a string looking for the first
instance of a specified character.
Returns address of target, if found; else returns null.
Caution: the input string must be trailed by a null to prevent blowup.
*/
scan(addr,way,target)
	char *addr;     /* current val of pt to char array */
	int way;        /* which way scan? */
	char target;    /* target character */
	{
	while ( *addr != target )
		{
		if ( *addr == 0 ) return(NULL);
		if ( way == LEFT ) addr--;
		else addr++;
		}
	return(addr);
	}

/* feedin -
Ep command switch.
Commands to ed are constructed here.
*/
feedin(input) char input; {
	register char *pc;
	register int *i, *j;
	int k;
	char chinput;

	switch(input)
		{
		case LT:
			moveleft();
			break;
		case RT:
			moveright();
			break;
		case UP:
			moveup();
			pulldot();
			infosho(LINE);
			break;
#ifdef DWN
		case DWN:
#endif
		case LF:
			movedown(KIND_LF);
			pulldot();
			infosho(LINE);
			break;
		case HOME:
			poscursor(col=0,row=0);
			pulldot();
			infosho(LINE);
			break;
		case CR:
			movedown(KIND_CR);
			pulldot();
			infosho(LINE);
			break;
		case TAB:
			tabmov(FORWARD); /* increments col to next tabstop */
			break;
		case BAKTAB:
			tabmov(BACK);   /* decrements col to next tabstop */
			break;
		case BOL:
			poscursor(col=0,row);
			break;
		case EOL:
			loadline();
			if( load_len <= 0 ) col = 0;
			else col = load_len - 1;
			poscursor(col,row);
			break;
		case POSIT:
			/*
			If user hit ARG key, check: was an argument
			actually typed?  If not, set up for go to end of
			text; else set up for user directed go to.  If
			user didn't hit ARG key, set up for go to top
			of text.
			*/
			switch ( argon ) {
				case YES:
					if ( *buf == '.' && *(buf+1) )
						if ( !numeric(buf+1) ) return(YES);
						else k = 0.5 + atof(buf) * (dol-zero);
					else if ( *buf )
						if ( !numeric(buf) ) return(YES);
						else k = atoi(buf);
					else k = (dol-zero);
					break;
				case NO:
					k = 1;         
			}
			if( k < 1 ) k = 1;
			/* if target line is not on screen, display new page */
			if( k<port[0].line || k>port[SSIZE-1].line) {
				position = FULLSCREEN;
				display(k,TOEND);
				}
			row = k - port[0].line;
			pulldot();
			infosho(LINE);
			if( argon == YES ) return(NO);
			else break;
		case BAKPAGE:
		case PAGE:
			/* display previous/next page */
			chinput = ( input == BAKPAGE ? BACK : FORWARD );
			switch( argon ) {
				case YES:
					if ( *buf )
						if ( !numeric(buf) ) return(YES);
						else page(atoi(buf),chinput);
					else page(1,chinput);
					return(NO);
				case NO:
					page(1,chinput);
			}
			break;
		case BSHIFT:
		case FSHIFT:
			/* shift text backward/forward */
			chinput = ( input == BSHIFT ? BACK : FORWARD );
			switch( argon ) {
				case YES:
					if ( *buf )
						if ( !numeric(buf) ) return(YES);
						else shift(atoi(buf),chinput);
					else shift(input==FSHIFT ? row : SSIZE-row-1,chinput);
					return(NO);
				case NO:
					shift(SSIZE/2,chinput);
			}                          
			break;       
		case ARG:
			/* prime buf with argument, then feed yourself */
			clear(buf,BSIZE);
			if ( (input=getarg()) != DONE ) {
				argon = YES;
				tdisable = feedin(input);
				argon = NO;
				}
			clear(buf,BSIZE);
			break;
		case ED:
			tell("ED NEEDS AN ARGUMENT");
			tdisable = YES;
			break;
		case INSERT:
			/* insert a character */
			if ( siginsert ) {
				drawline(0,length(insertmode),LOCMSG-1);
				poscursor(col,row);
				siginsert = OFF;
				tellinsert = NO;
				}
			else {
				tellinsert = YES;
				siginsert = ON;
				poscursor(0,LOCMSG-1);
				write(1,insertmode,length(insertmode));
				poscursor(col,row);
				}
			break;
		case RUB:
			/*
			Suck out a character, providing insert signal is ON
			and cursor is not past last char. in line.
			*/
			loadline();
			if( col < load_len ) {
				pc = linebuf + col; /* pick up core address */
				shosuck(pc);
				if( load_len > 0 ) --load_len;
				}
			break;
		case ESC:
			/* exit ep */
			changes();
			return(QUIT);
		case OPEN:
			/* open up spaces between lines */
			if ( *buf )
				if ( !numeric(buf) ) return(YES);
				else edopen(dot,atoi(buf));
			else edopen(dot,1);

			position = row; /* just show partial screen */
			display(port[row].line,TOEND);
			if( argon == YES ) return(NO);
			else break;
		case CLOSE:
			/* close spaces between lines */
			if ( dot > dol )   /* nothing to close */
				if( argon == YES ) return(NO);
				else break;

			changes();
			i = port[row].line + zero;      /* address 1 */

			if ( *buf ) {           /* derive address 2 */
				if ( numeric(buf) && (k=atoi(buf)) ) 
					j = i + (k-1);
				else return(YES);
				}
			else j = i;

			if( j > dol ) j = dol;

			/* copy lines to zap area */
			zappic(i,j,CLOSE);

			/* now remove lines from main text */
			for ( ; i <= j; i++ ) {
				order = bundle(GO,".d");
				call_ed();
				}

			/* incase last line is removed */
			pulldot();

			position = row; /* just show partial screen */
			display(port[row].line,TOEND);
			if( argon == YES ) return(NO);
			else break;
		case BSRCH:
		case FSRCH:
			chinput = ( input == BSRCH ? BACK : FORWARD );
			search(chinput);
			if( argon == YES ) return(NO);
			else break;
		case PICK:
		case MORE:
			/* buffer (copy) user specified lines */
			if ( dot > dol )   /* nothing to pick */
				if( argon == YES ) return(NO);
				else break;

			k = 1;
			i = port[row].line + zero;      /* address 1 */

			if ( *buf ) {           /* derive address 2 */
				if ( numeric(buf) && (k=atoi(buf)) ) 
					j = i + (k-1);
				else return(YES);
				}
			else j = i;

			if( j > dol) j = dol;

			markrows(row,row+(k-1));
			zappic(i,j,input);       /* copy */
			if( argon == YES ) return(NO);
			else break;
		case DROP:
			/* If argument flag is on, drop contents of zap
			buffer; else drop contents of pick buffer.
			*/
			if ( dot > dol ) extendtext();

			j = ( argon ? CLOSE : PICK );
			if( !drop(port[row].line+zero,j) ) 
				if( argon == YES ) return(YES);
				else break;

			position = row; /* just show partial screen */
			display(port[row].line,TOEND);
			if( argon == YES ) return(NO);
			else break;
		case HELP:
			help();
			break;
		case REFRESH:
			ZAPTUBE;
			initscreen(port[SSIZE/2].line);
			break;
		case CNTRL:
			tell(cntrlmode);
			poscursor(col,row);
			update();
			input = getchar()&0177;
			clearmessage();
			poscursor(col,row);
			if( input == CNTRL ) break;     /* forget it */
			if( !cntrlchar(input) ) input &= 037;
			switch(input)
				{
				case CR:
				case LF:
					/* split line */
					if( dot > dol )
						break; /* nothing to split */
					loadline();
					if( col >= load_len )
						break; /* past end of line */
					pc = linebuf + col;
					copy(linebuf,savebuf);
					if( input == CR )
						copy(pc,linebuf);
					else
						blankback(pc,0,col);
					change = ON;
					edopen(dot,1);
					copy(savebuf,linebuf);
					*pc = 0;
					load_line = port[row].line;
					load_len = col;
					change = ON;
					position = row;
					display(port[row].line,TOEND);
					break;
				case TAB:
					/* insert some blanks */
					if( dot > dol )
						break;  /* past last line */
					loadline();
					if( col >= load_len )
						break;  /* past end of line */
					k = TABDIS - col % TABDIS;
					if( load_len + k >= LBSIZE-2 ) {
						tell("LINE WOULD BE TOO LONG");
						tdisable = YES;
						break;
						}
					pc = linebuf + col;
					copy(pc,savebuf);
					blankback(pc+k,col,col+k);
					copy(savebuf,pc+k);
					load_len += k;
					change = ON;
					col += k;
					position = row;
					display(port[row].line,port[row].line);
					break;
				case NULL:
					/* encode null character */
					input = 0200;
					goto proc_chr;
				default:
					goto proc_chr;
			}
			break;
		case MERGE:
			if( dot >= dol ) break; /* nothing to merge */
			loadline();
			copy(linebuf,savebuf);
			k = load_len;
			i = port[row].line + 1 + zero;
			trimblks(getline(*i));
			load_line = -1;
			pc = linebuf + skipblk(linebuf);
			if( length(pc)+k >= LBSIZE-2 ) {
				tell("LINE WOULD BE TOO LONG");
				tdisable = YES;
				break;
				}
			copy(pc,savebuf+k);
			order = bundle(GO,".d");
			call_ed();
			copy(savebuf,linebuf);
			load_line = port[row].line;
			load_len = length(linebuf);
			change = ON;
			col = k;
			position = row;
			display(port[row].line,TOEND);
			break;
		case COLM:
			forcedcol = !forcedcol;
			break;
		default:
			if ( cntrlchar(input) ) {
				tell("UNDEFINED CONTROL CHARACTER");
				tdisable = YES; /* so clearmessage won't */
				break;
				}
		proc_chr:
			/* Cursor past end of text? */
			if ( port[row].line  > (dol-zero) ) extendtext();

			/* Bring line into linebuf if necessary */
			loadline();

			/* Test new length */
			k = load_len + (siginsert ? 1 : 0); 
			if( k < col+1 ) k = col+1;
			if( k >= LBSIZE-2 ) {
				tell("LINE BUFFER FULL, CHARACTER IGNORED");
				tdisable = YES;
				break;
				}

			pc = linebuf + col;     /* pick up core address */

			shochr(input);

			/* Cursor past end of line? */
			if ( col >= load_len ) extendline(pc);

			if ( col <= port[row].indent && input != ' ' )
				port[row].indent = col;
			if( port[row].len < col+1 ) port[row].len = col+1;

			/* col is changed appropriately by insertmode
			   and overwrite */
			if ( siginsert ) shoinsert(pc,input);
			else if ( *pc != input ) overwrite(pc,input);
			     else col++;

			/* change loaded line length */
			load_len = k;

			if ( col >= MAXCOL ) poscursor(col,row);

		} /* end of switch */

	if ( msg_on ) {
		if ( !tdisable ) clearmessage();
		else tdisable = NO;
		poscursor(col,row);
		}

	if ( siginsert && !tellinsert ) {
		poscursor(0,LOCMSG-1);
		write(1,insertmode,length(insertmode));
		tellinsert = YES;
		poscursor(col,row);
		}

	return(GO);

	} /* end of feedin */

/* changes -
Called whenever a different line is loaded, or ed is to be called.
If no change has been made to the loaded line, ed is not told.
If there has been a change, the line will be entabbed by putline(), before
being appended to the end of the working file.
<load_line> is set to -1, below, to insure that subsequent changes to the
line will require use of getline(), which will detab the line again.
*/
changes() {
	register char c1, c2;
	if ( change ) {
		if( linebuf[0]=='.' && linebuf[1]=='\0' ) {
			linebuf[1] = ' ';
			linebuf[2] = '\0';
			}
		dot = load_line + zero;
		order = bundle(GO,".c\n",linebuf,"\n.");
		call_ed();
		pulldot();
		}
	load_line = -1;
	change = OFF;   /* new line can't yet have been changed */
	}

/* infosho -
Show file name and line number.
*/
infosho(all) {
	register setdol;
	register fiddle;
	register didge;

	if ( !all ) {   /* just show line number */
		fiddle = ( *savedfile ? length(savedfile) : 4 );
		if( fiddle > 17 ) fiddle = 17;
		/* The num 11 below fudges for "File " and " Line "
		   (how many hairs does a horse have?) */
		poscursor(SGENFO+fiddle+11,LOCMSG);
		printd(port[row].line);     
		}
	else {  /* show file name too */
		poscursor(SGENFO,LOCMSG);
		write(1,"File ",5);
		if ( *savedfile )                                     
			if( length(savedfile) < 18 )
				write(1,savedfile,length(savedfile));
			else
				write(1,savedfile+(length(savedfile)-17),17);
		else write(1,"????",4);
		write(1," Line ",6);
		printd(port[row].line);
		}
	/* mopup after line number (assume num lines < 10,000) */
	for ( didge=port[row].line; didge < 1000; didge =* 10 ) write(1," ",1);
	poscursor(col,row);
	return;
	}

/* printd -
	Print n in decimal, n non-negative
*/
printd(n)
{
	register nn;
	int ch;
	if( nn=n/10 )    
		printd(nn);
	ch = n%10 + '0';
	write(1,&ch,1);
}

/* length -
Return length of a string <str>.
*/
length(str) char *str; {
	register endofstr;
	endofstr = scan(str,RIGHT,0);
	return(endofstr-str);
	}

/* drawline -
Draw a line between two specified column positions at
a specified row.  User decides what line consists of: PTHL.
*/
drawline(lend,rend,drow) int lend,rend,drow;  /* left end, right end, row */
	{
	register i;

	if( lend < 0 || lend > MAXCOL ) return;
	if( rend > LINELEN ) rend = LINELEN;
	poscursor(lend,drow);
	for ( i=lend; i < rend; i++)
		write(1,PTHL,1);/* PTHL = point for a horizontal line */
	}

/* pulldot -
Pull ed's dot along with movement of screen cursor.
*/
pulldot() {
	dot = port[row].line + zero;
	}

/* blankback -
Fill a string with blanks, from just before pos2 all the way back to pos1.
*/
blankback(p,pos1,pos2) char *p; {
	for ( --pos2; pos2 >= pos1; pos2-- ) *(--p) = ' ';
	return;
	}

/* insert -
Insert a character into a string.
*/
insert(pc,c) char *pc; char c; {
	register char keep1, keep2;

	keep1 = *pc;
	keep2 = c;
	while ( (*pc++ = keep2) ) {
		keep2 = keep1;
		keep1 = *pc;
		}
	return;
	}

/* suck -
Suck the character pointed to by pc out of a string.
*/
suck(pc) char *pc; {
	for ( ; *pc; pc++ ) *pc = *(pc+1);
	}

/* shosuck -
Suck the character pointed to by pc out of a line,
and display result.
Assume col (corresponding to pc) is not beyond end of line.
Globals: col, row, change, port.
*/
shosuck(pc) char *pc; {
	register int clm,seconds;

	suck(pc);
	if ( port[row].disp && EMPTY ) {
		if ( (clm=shostr(pc,col)) <= MAXCOL ) {
			write(1," ",1); /* mop up */
			}
		poscursor(col,row);
		--port[row].len;
		if ( col < port[row].indent ) --port[row].indent;
		seconds = (clm-col+9)/char_sec;
		if( seconds > 0 ) sleep(seconds);
		}
	else {
		port[row].disp = NO;
		}
	change = ON;    /* alert ed */
	}

/* shoinsert -
Take care of inserting a character into text, and show result.
Assumes cursor (and dot) is not beyond end of line.
Globals: col, row, change, port.
*/
shoinsert(pc,input) char *pc; char input; {
	register int clm,seconds;

	insert(pc,input);
	if ( port[row].disp && EMPTY ) {
		clm = shostr(++pc,++col);
		poscursor(col,row);
		if( *pc != 0 ) ++port[row].len;
		seconds = (clm-col+9)/char_sec;
		if( seconds > 0 ) sleep(seconds);
		}
	else {
		++col;
		port[row].disp = NO;
		}
	change = ON;    /* alert ed */
	}

/* overwrite -
Overwrites character at pc with input.
Assume globals col and struct port.
*/
overwrite(pc,input) char *pc; char input; {
	*pc = input;
	change = ON;    /* alarm to ed */
	col++;
	}

/* extendline -
Extends the current line from its endpoint to position pc.
Assumes the global col.
*/
extendline(pc) char *pc; {
	blankback(pc,load_len,col);
	*pc = 0;
	*(pc+1) = 0;
	}

/* update -
Update display port.
*/
update() {
	int indent;             /* present indentation */
	int ckeep;              /* present col */
	int rkeep;              /* present row */
	register struct portdes *ppt;
	register char *pc, *ps;
	register char c;
	int i, shoext, extlen;
	int seconds;
	int cursormoved;

	cursormoved = NO;
	rkeep = row;

	for( row=0; row < SSIZE; row++ ) {
		if( !EMPTY ) break;
		if( port[row].disp ) continue;
		cursormoved = YES;

		/* place line at position */
		ppt = &port[row];
		loadline();
		if( load_len  == 0 ) indent = MAXCOL;
		else indent = skipblk(linebuf);
		if( indent > MAXCOL ) indent = MAXCOL;
#ifdef CLEARLINE 
		poscursor(0,row);
		maketerm(CLEARLINE);
		poscursor(indent,row);
		if( load_len != 0 ) shostr(linebuf+indent,indent);
		seconds = (MIN(load_len,MAXCOL+1)-indent+9)/char_sec;
#else
		if ( ppt->indent < indent ) {
			/* position to previous indentation */
			poscursor(ppt->indent,row);
			if( load_len == 0 )
				mopup(ppt->len,ppt->indent,ppt->indent);
			else 
				mopup(ppt->len,load_len,
				      shostr(linebuf+ppt->indent,ppt->indent));
			}
		else {
			/* position to present indentation */
			poscursor(indent,row);
			if ( load_len == 0 ) mopup(ppt->len,indent,indent);
			else mopup(ppt->len,load_len,
				   shostr(linebuf+indent,indent));
			}

		seconds = (MIN(MAX(ppt->len,load_len),MAXCOL+1)
			   - MIN(indent,ppt->indent) + 4)/char_sec;
#endif
		if( seconds > 0) sleep(seconds);
		ppt->indent = indent;
		ppt->len = load_len;
		ppt->disp = YES;
		}

	row = rkeep; 
	loadline();
	ckeep = (load_len <= MAXCOL ? MAXCOL : LBSIZE-3);
	if( col > ckeep ) {
		col = ckeep;
		cursormoved = YES;
		}

	if( EMPTY ) {
		if( col < load_len ) c = cntrlchar(linebuf[col]);
		else c = NO;
		if( c == RUB && tellcntrl != RUB ) {
			poscursor(SCNTRL,LOCMSG-1);
			cursormoved = YES;
			write(1," <RUB> ",7);
			write(1,PTHL,1);
			tellcntrl = RUB;
			}
		else if( c == CNTRL && tellcntrl != CNTRL ) {
			poscursor(SCNTRL,LOCMSG-1);
			cursormoved = YES;
			write(1," <CTRL> ",8);
			tellcntrl = CNTRL;
			}
		else if( c == NO && tellcntrl != NO ) {
			drawline(SCNTRL,SCNTRL+8,LOCMSG-1);
			cursormoved = YES;
			tellcntrl = NO;
			}
		shoext = col > MAXCOL || (col == MAXCOL && load_len >= LINELEN);
		if( tellrow >= 0 && (!shoext || tellrow != row) ) {
			c = (port[tellrow].len <= MAXCOL ? ' ' : '>');
			rkeep = tellrow;
			tellrow = -1;
			poscursor(MAXCOL,rkeep);
			cursormoved = YES;
			write(1,&c,1);
			}
		if( shoext && tellrow != row ) {
			poscursor(MAXCOL,row);
			cursormoved = YES;
			write(1,"#",1);
			tellrow = row;
			}
		if( shoext || forcedcol ) {
			if( tellcol == NO ) {
				poscursor(EXTCOL-10,LOCMSG-1);
				cursormoved = YES;
				write(1," Col ",5);
				}
			if( tellcol != col+1 ) {
				if( tellcol != NO ) {
					poscursor(EXTCOL-5,LOCMSG-1);
					cursormoved = YES;
					}
				if( col < 99 )
					write(1,"  ",(col<9 ? 2 : 1));
				printd(col+1);
				write(1," ",1);
				tellcol = col+1;
				}
			}
		else if( tellcol != NO ) {
			drawline(EXTCOL-10,EXTCOL,LOCMSG-1);
			cursormoved = YES;
			tellcol = NO;
			}
		if( shoext && tellext == NO ) {
			poscursor(EXTCOL-1,LOCMSG-1);
			cursormoved = YES;
			write(1,"<",1);
			ps = tellstr;
			for( i=EXTCOL ; i<LINELEN ; i++) {
				write(1," ",1);
				*ps++ = ' ';
				}
			}
		if( shoext ) {
			i = EXTMOD * ( (col+1) / EXTMOD ) - 1;
			extlen = i + (LINELEN-EXTCOL);
			if( i < load_len ) pc = linebuf + i;
			else pc = "\0";
			ps = tellstr;
			ckeep = col;
			rkeep = row;
			col = -1;
			row = LOCMSG-1;
			for( i=EXTCOL ; i<LINELEN ; i++, ps++) {
				c = (*pc ? *pc++ : ' ');
				if( c != *ps ) {
					cursormoved = YES;
					if( i != col ) poscursor(col=i,row);
					shochr(c);
					col++;
					*ps = c;
					}
				}
			c = (load_len < extlen ? ' ' : '>');
			if( tellext != c ) {
				poscursor(MAXCOL,row);
				cursormoved = YES;
				write(1,&c,1);
				}
			col = ckeep;
			row = rkeep;
			tellext = c;
			}
		else if( tellext != NO ) {
			drawline(EXTCOL-1,LINELEN,LOCMSG-1);
			cursormoved = YES;
			tellext = NO;
			}
		}

	/* replace cursor */
	if( cursormoved ) poscursor(col,row);
	return;
	}


/* display -
Display lin1 through lin2.
*/
display(lin1,lin2) {
	register indent;        /* present indentation */
	int curat;              /* line that cursor pts. to initially */
	register struct portdes *ppt;

	changes();    /* most calls to display assume changes is called here */

	curat = port[row].line;

	/* in case user wants to print til bottom of screen */
	if ( lin2 == TOEND ) lin2 = lin1 + (SSIZE+1);

	if ( position == FULLSCREEN ) {
		/* display full screen, with target line at midpoint */
		lin1 -= SSIZE/2;
		ppt = port;
		row = 0;
		}
	else {  /* place line at position (trailed by subsequent lines) */
		row = position;
		ppt = &port[row];
		}

	if ( lin1 < 1 ) lin1 = 1;

	for ( ; row < SSIZE; row++ ) {
		ppt->line = lin1++;     /* pick up rel. line no. */
		if ( ppt->line > lin2 ) break;
		ppt->disp = NO;
		++ppt;
		}
	if ( searching ) return;

	/* if line pointed at initially is still on screen, replace cursor */
	if ( curat >= port[0].line && curat <= port[SSIZE-1].line ) {
		poscursor(col,row=curat-port[0].line);
		return;
		}

	if ( position == FULLSCREEN )
		poscursor(col=0,row=0);
	else poscursor(col,row=position);
	return;
	}


/* copy -
Copy the string pointed to by p1 to successive locations starting at p2.
Danger: There are no bound checks:  know what you're doing.
*/
copy(p1,p2) char *p1, *p2; {
	while ( *p1 ) *p2++ = *p1++;
	*p2 = 0;
	}

/* edopen -
Use ed to open up space between lines in core image.
*/
edopen(pos,knum) int *pos; {
	register j;

	changes();

	if ( pos > dol ) {
		extendtext();
		return;
		}

	for ( j=1; j <= knum; j++ ) {
		order = bundle(GO,".i\n\n.");
		call_ed();
		}
	}

/* extendtext -
Append to dol until corresponding screen position is reached.
Change portdes to account for new CR's.
*/
extendtext() {
	register number, j;
	dot = dol;
	number = port[row].line - (dol-zero);
	for ( j=1; j <= number; j++ ) {
		order = bundle(GO,".a\n\n.");
		call_ed();
		}
	}

/* clearmessage -
Blank out screen message space.
*/
clearmessage() {
	clrlines(LOCMSG,1,SGENFO);
	msg_on = OFF;
	}

/* search -
Search interface between ep and ed.
Handles screen display details.
After calling ed, control returns to search() only on condition that
the search string is found.
ed considers a non-match an error, and exits through the setexit-reset
mechanism.
ep catches the error at the top level (in routine "ep") and issues the
appropriate message (string not found) to the user.
*/
search(type) {
	register matchat;

	changes();

	clrlines(LOCMSG,1,ECAREA);
	poscursor(0,LOCMSG);
	msg_on = YES;
	position = FULLSCREEN;
	searching = YES;
	if ( dol == zero ) reset();     /* nothing to search for */

	switch ( type ) {
		case FORWARD:
			write(1,"+SCH:",5);
			if ( *buf ) {
				write(1,buf,length(buf));
				copy(buf,skey);
				order = bundle(GO,"/",buf,"/");
				}
			else    {
				write(1,skey,length(skey));
				order = bundle(GO,"//");
				}
			break;
		case BACK:
			write(1,"-SCH:",5);
			if ( *buf ) {
				write(1,buf,length(buf));
				copy(buf,skey);
				order = bundle(GO,"?",buf,"?");
				}
			else    {
				write(1,skey,length(skey));
				order = bundle(GO,"??");
				}
		}

	/* ed's search routine must know relative start position */
	/* The routine matchalgol() was adapted from execute() in
	order to allow search through rest of current line. */
	loc1 = linebuf + col;
	clear(linebuf,LBSIZE);
	getline(*(port[row].line + zero));

	/* call_ed will return if a match is found;
	otherwise exit is through reset in ed */
	call_ed();

	matchat = dot - zero;   /* line num. of match */

	/* if match occurs in current port, redisplay is unnecessary */
	if ( matchat < port[0].line || matchat > port[SSIZE-1].line )
		display(matchat,TOEND);

	row = dot - (zero + port[0].line);
	col = loc1 - linebuf;   /* loc1 set by ed */
	clearmessage();
	infosho(LINE);
	searching = NO;
	return;
	}

/* page -
Display next page.
Position dot so that target line comes up centered in full screen.
<num> is the number of pages to be turned.
<direc> is the direction; it must be -1 (BACK), or +1 (FORWARD).
*/
page(num,direc) {
	register lin1;

	position = FULLSCREEN;
	lin1 =  port[SSIZE/2].line + (direc*(num*SSIZE));
	if ( lin1 < 1 ) lin1 = 1;
	display(lin1,TOEND);
	pulldot();
	infosho(LINE);
	return;
	}

/* shift -
Move the port <extent> number of lines up or down.
<direc> is the direction; it must be -1 (BACK), or +1 (FORWARD).
*/
shift(extent,direc) {
	register lin1;

	position = FULLSCREEN;
	lin1 = port[SSIZE/2].line + (direc*extent);
	if ( lin1 < 1 ) lin1 = 1;
	display(lin1,TOEND);
	pulldot();
	infosho(LINE);
	return;
	}

/* numeric -
Does the (non-null) string starting at <p> contain only digits?
*/
numeric(p) char *p; {
	for ( ; *p; *p++ )
		if ( *p < '0' || *p > '9' ) {
			tell("NON-NUMERIC STRING");
			poscursor(col,row);
			return(NO);
			}
	return(YES);
	}

/* zappic -
Logically zap or pick lines out of main text.
Lines are not physically moved.
The lines' file addresses are shuffled to the
end of the line description table.
The table is expanded if necessary.
*/
zappic(a1,a2,wish) int *a1, *a2; {
	register *p;

	switch(wish)
		{
		case PICK:
			clearpick();            /* clear pick buffer */
			p = dol + 1;
			break;
		case MORE:
			wish = PICK;            /* add to pick buffer */
			p = dol + 1;
			break;
		case CLOSE:
		default:
			mockdol = zapstart - 1; /* clear zap buffer */
			p = zapstart;
		}

	while ( a1 <= a2 ) {
		corealloc(++mockdol);
		insert1(p,mockdol,*a1++);
		if ( wish == PICK ) ++zapstart;
		}
	}

/* drop -
Transforms the line description table by placing addresses
in the pick or zap areas at the specified position.
*/
drop(at,from) int *at; {
	register int *stop;
	register int *p;

	if ( from == PICK ) {
		if ( zapstart == (dol+1) ) {
			tell("** NOTHING IN PICK BUFFER **");
			tdisable = YES;
			poscursor(col,row);
			return(0);
			}
		p = dol + 1;
		stop = zapstart - 1;
		}
	else    {                
		if ( zapstart > mockdol ) {
			tell("** NOTHING IN ZAP BUFFER **");
			tdisable = YES;
			poscursor(col,row);
			return(0);
			}
		p = zapstart;
		stop = mockdol;
		}

	for ( ; p <= stop; p =+ 2 ) {
		corealloc(++mockdol);
		insert1(at,mockdol,*p);
		++stop;
		++dol;
		++zapstart;
		}

	return(OK);
	}

clearpick() {
	register int *p, *q;

	p = dol + 1;
	q = zapstart;

	if( p == q ) return(1);

	while( q <= mockdol ) {
		*p++ = *q++;
		}
	zapstart = dol+1;
	mockdol = --p;
	return(1);
	}

/* insert1 -
General purpose insert routine.
Insert an item in a location and shuffle down all
successive locations until <stop> is reached.
*/
insert1(pc,stop,item) int *pc, *stop; {
	register int keep1, keep2;

	keep1 = *pc;
	keep2 = item;
	while ( pc < stop ) {
		*pc++ = keep2;
		keep2 = keep1;
		keep1 = *pc;
		}
	*pc = keep2;
	return;
	}

/* corealloc -
Allocate more core.
*/
corealloc(address) int *address; {
	struct { int integer; };

	if ( address >= endcore ) {
		if ( sbrk(1024) == -1 ) error;
		endcore.integer =+ 1024;
		}
	}

/* subcontrol -
If global flag rein is TIGHT, carry out a substitution only if user
says go ahead (types DOIT).
If rein is LOOSE, don't ask user, just do it.
Display actions under TIGHT rein.
Display actions under LOOSE rein if current port contains substitution.
*/
subcontrol(a1) int *a1; {
	register matchat, seeable;
	register char input;
	char c2;

	matchat = a1 - zero;
	load_line = -1;         /* don't know all about the loaded line */

	if ( matchat >= port[0].line && matchat <= port[SSIZE-1].line )
		seeable = YES;
	else seeable = NO;

	switch ( rein ) {
		case TIGHT:
			if ( !seeable ) {
				position = 0;
				display(a1-zero,TOEND);
				}
			row = a1 - (zero + port[0].line);
			col = loc1 - linebuf;
			infosho(LINE);
			copy(linebuf,savebuf);  /* save line with match */
		ask:    update();               /* update display */
			copy(savebuf,linebuf);  /* restore line with match */
			load_line = -1;
			maketerm(RINGBELL);
			if ( (input=getchar()&0177) == DOIT ) {
				dosub();
				if( port[row].disp ) {
					mopup(port[row].len,
						(loc1-linebuf)+length(loc1),
						   shostr(loc1,col));
					port[row].len = length(linebuf);
					port[row].indent =
						     MIN(port[row].indent,col);
					}
				poscursor(0,LOCMSG);
				return(GO);
				}

			if ( input == HELP) {
				help();
				poscursor(0,LOCMSG);
				write(1," ED: ",5);
				puts1(buf);
				msg_on = YES;
				goto ask; }

			if ( input == ED ) return(ED);

			else   {        /* trick dosub */
				input = rhsbuf[0];
				c2 = rhsbuf[1];
				rhsbuf[0] = '&';        
				rhsbuf[1] = '\0';
				dosub();
				rhsbuf[0] = input;      
				rhsbuf[1] = c2;  
				poscursor(0,LOCMSG);
				return(GO);
				}
		case LOOSE:
			dosub();
			if ( seeable ) {
				row = a1 - (zero + port[0].line);
				col = loc1 - linebuf;
				poscursor(col,row);
				infosho(LINE);
				if( port[row].disp ) {
					mopup(port[row].len,
						(loc1-linebuf)+length(loc1),
						   shostr(loc1,col));
					port[row].len = length(linebuf);
					port[row].indent =
						     MIN(port[row].indent,col);
					}
				poscursor(0,LOCMSG);
				}
		}
	return(GO);
	}

/* shostr -
Display a string, taking account of cursor position on screen.
If the string will not fit on remainder of screen in line, display
as much as possible, followed by a '>' (off to the right) indicator.
Return column coordinate where the cursor was left.
Control characters are output as upper case letters.     
*/
shostr(str,colm) char *str; {
	char line[LINELEN];
	register char *as, *c;
	register int clm;

	as = str;
	clm = colm;
	if( clm==MAXCOL && *as && row==tellrow ) return(MAXCOL+1);
	c = line;
	while( *as && clm < MAXCOL ) {
		*c = *as++ & 0177;
		if( *c <= 037 ) *c += 0100;
		else if( *c == RUB ) *c = '#';
		c++;
		clm++;
		}
	if( *as && clm == MAXCOL ) {
		*c++ = (row == tellrow ? '#' : '>');
		clm++;
		}
	if( c != line ) write(1,line,c-line);
	return(clm);
	}

/* shochr -
Display a character by making a one character string and calling shostr.
*/
shochr(c) char c; {
	char str[2] ;
	str[0] = c;
	str[1] = 0;
	shostr(str,col);
	}

/* markrows -
Mark rows <row1> thru <row2> to show lines being picked.
*/
markrows(row1,row2) {
	register indent,seconds;
	register struct portdes *ppt;
	int rkeep;

	changes();
	update();
#ifdef BACKGROUND
	maketerm(BACKGROUND);
#endif
	ppt = &port[row1];
	rkeep = row;

	for( row=row1 ; row < SSIZE ; row++, ppt++) {
		if( row > row2 || !EMPTY ) break;
		if( ppt->disp ) {

			loadline();
			if( load_len == 0 ) continue;
			indent = skipblk(linebuf);
			if( indent >= MAXCOL ) continue;
#ifdef BACKGROUND 
			poscursor(indent,row);
			shostr(linebuf+indent,indent);
#else
			drawline(indent,MIN(load_len,MAXCOL),row);
#endif
			seconds = (MIN(load_len,MAXCOL)-indent+4) / char_sec;
			if( seconds > 0 ) sleep(seconds);
			ppt->indent = indent;
			ppt->disp = NO;
			}
		}
	poscursor(col,row=rkeep);
	if( EMPTY ) sleep(1);
#ifdef BACKGROUND
	maketerm(FOREGROUND);
#endif
	update();
	}

/* initscreen -
Throw up lines beginning at <lin1>.
*/
initscreen(lin1) {
	position = FULLSCREEN;
	display(lin1,TOEND);
	pulldot();
	drawline(0,LINELEN,LOCMSG-1);
	infosho(ALL);
	}

/* secondchance -
Is user forgetting to save work from editing session?
*/
secondchance()
{

tellit:
	tell("Save work (y or n)? ");
	switch( getchar()&0177 ) {
		case 'y':       addr2 = 0;
				if( savedfile[0] )
					writeout(savedfile);
				else
					error;
				addr1 = addr2 = 0;
				return(0);

		case 'n':       addr1 = addr2 = 0;
				return(0);

		case ARG:       return(1);

		default:        goto tellit;
	}

}

/* writeout -
Save the file.
*/
writeout(fname) char *fname; {
	setall();
	nonzero();
	if ((io = creat(fname, 0666)) < 0) error;
	tell("writing ...");
	putfile();
	exfile();
	}
/* help -
Display or print help file
*/
help() {
	ZAPTUBE;
	cleanup();
	helpout(NO);
	puts1("Do you want a copy ...");
	settty();
	if( (getchar()&0177) == 'y') {
		cleanup();
		puts1(" OK ... I'm printing it now.\n");
		helpout(YES);
		puts1("! CR to reenter ep ...");
		getchar();
		settty();
		}
	ZAPTUBE;
	initscreen(port[SSIZE/2].line);
	}
/* helpout -
Output help file
*/
helpout(print) {
	int savint, pid, rpid, retcode;

	if( (pid = fork()) == 0 ) {
		signal(SIGHUP,onhup);
		signal(SIGQUIT,onquit);
		if( print ) execl(PRINT,HELPFILE,0);
		else execl(DISPLAY,HELPFILE,0);
		exit();
		}
	savint = signal(SIGINTR,1);
	while( (rpid = wait(&retcode)) != pid && rpid != -1);
	signal(SIGINTR,savint);
	}
