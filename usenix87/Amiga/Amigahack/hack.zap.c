#file hack.zap.c
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1984. */

#include "hack.h"

extern struct monst *makemon();
struct monst *bhit();
char *exclam();

char *fl[]= {
	"magic missile",
	"bolt of fire",
	"sleep ray",
	"bolt of cold",
	"death ray"
};

dozap()
{
	register struct obj *obj;
	register struct monst *mtmp;
	xchar zx,zy;
	register int num;

	obj = getobj("/", "zap");
	if(!obj) return(0);
	if(obj->spe < 0 || (obj->spe == 0 && rn2(121))) {
		pline("Nothing Happens");
		return(1);
	}
	if(obj->spe == 0)
		pline("You wrest one more spell from the worn-out wand.");
	if(!(objects[obj->otyp].bits & NODIR) && !getdir())
		return(1); /* make him pay for knowing !NODIR */
	obj->spe--;
	if(objects[obj->otyp].bits & IMMEDIATE) {
		if((u.uswallow && (mtmp = u.ustuck)) ||
		   (mtmp = bhit(u.dx,u.dy,rn1(8,6),0))) {
			wakeup(mtmp);
			switch(obj->otyp) {
			case WAN_STRIKING:
				if(rnd(20) < 10+mtmp->data->ac) {
					register int tmp = d(2,12);
					hit("wand", mtmp, exclam(tmp));
					mtmp->mhp -= tmp;
					if(mtmp->mhp < 1) killed(mtmp);
				} else miss("wand", mtmp);
				break;
			case WAN_SLOW_MONSTER:
				mtmp->mspeed = MSLOW;
				break;
			case WAN_SPEED_MONSTER:
				mtmp->mspeed = MFAST;
				break;
			case WAN_UNDEAD_TURNING:
				if(index("WVZ&",mtmp->data->mlet)) {
					mtmp->mhp -= rnd(8);
					if(mtmp->mhp<1) killed(mtmp);
					else mtmp->mflee = 1;
				}
				break;
			case WAN_POLYMORPH:
				if( newcham(mtmp,&mons[rn2(CMNUM)]) )
					objects[obj->otyp].oc_name_known = 1;
				break;
			case WAN_CANCELLATION:
				mtmp->mcan = 1;
				break;
			case WAN_TELEPORT_MONSTER:
				rloc(mtmp);
				break;
			case WAN_MAKE_INVISIBLE:
				mtmp->minvis = 1;
				break;
#ifdef WAN_PROBING
			case WAN_PROBING:
				mstatusline(mtmp);
				break;
#endif WAN_PROBING
			default:
				pline("What an interesting wand (%d)",
					obj->otyp);
				impossible();
			}
		}
	} else {
	switch(obj->otyp){
		case WAN_LIGHT:
			litroom(TRUE);
			break;
		case WAN_SECRET_DOOR_DETECTION:
			if(!findit()) return(1);
			break;
		case WAN_CREATE_MONSTER:
			{ register int cnt = 1;
			if(!rn2(23)) cnt += rn2(7) + 1;
			while(cnt--)
			    (void) makemon((struct permonst *) 0, u.ux, u.uy);
			}
			break;
		case WAN_WISHING:
			{ char buf[BUFSZ];
			  register struct obj *otmp;
			  extern struct obj *readobjnam(), *addinv();
		      if(u.uluck + rn2(5) < 0) {
			pline("Unfortunately, nothing happens.");
			break;
		      }
		      pline("You may wish for an object. What do you want? ");
		      getlin(buf);
		      otmp = readobjnam(buf);
		      otmp = addinv(otmp);
		      prinv(otmp);
		      break;
			}
		case WAN_DIGGING:
			{ register struct rm *room;
			  register int digdepth;
			if(u.uswallow) {
				pline("You pierce %s's stomach wall!",
					monnam(u.ustuck));
				u.uswallow = 0;
				mnexto(u.ustuck);
				u.ustuck->mhp = 1;	/* almost dead */
				u.ustuck = 0;
				setsee();
				docrt();
				break;
			}
			zx = u.ux+u.dx;
			zy = u.uy+u.dy;
			if(!isok(zx,zy)) break;
			digdepth = 4 + rn2(10);
			if(levl[zx][zy].typ == CORR) num = CORR;
			else num = ROOM;
			Tmp_at(-1, '*');	/* open call */
			while(digdepth--) {
				if(zx == 0 || zx == COLNO-1 ||
					 zy == 0 || zy == ROWNO-1)
					break;
				room = &levl[zx][zy];
				Tmp_at(zx,zy);
				if(!xdnstair){
					if(zx < 3 || zx > COLNO-3 ||
					    zy < 3 || zy > ROWNO-3)
						break;
					if(room->typ == HWALL ||
					    room->typ == VWALL){
						room->typ = ROOM;
						break;
					}
				} else if(num == ROOM || num == 10){
					if(room->typ != ROOM && room->typ) {
						if(room->typ != CORR)
							room->typ = DOOR;
						if(num == 10) break;
						num = 10;
					} else if(!room->typ)
						room->typ = CORR;
				} else {
					if(room->typ != CORR && room->typ) {
						room->typ = DOOR;
						break;
					} else room->typ = CORR;
				}
				mnewsym(zx,zy);
				zx += u.dx;
				zy += u.dy;
			}
			mnewsym(zx,zy);	/* not always necessary */
			Tmp_at(-1,-1);	/* closing call */
			break;
			}
		default:
			buzz((int) obj->otyp - WAN_MAGIC_MISSILE,
				u.ux, u.uy, u.dx, u.dy);
			break;
		}
		if(!objects[obj->otyp].oc_name_known) {
			u.urexp += 10;
			objects[obj->otyp].oc_name_known = 1;
		}
	}
	return(1);
}

char *
exclam(force)
register int force;
{
	/* force == 0 occurs e.g. with sleep ray */
	/* note that large force is usual with wands so that !! would
		require information about hand/weapon/wand */
	return( (force < 0) ? "?" : (force <= 4) ? "." : "!" );
}

hit(str,mtmp,force)
register char *str;
register struct monst *mtmp;
register char *force;		/* usually either "." or "!" */
{
	if(!cansee(mtmp->mx,mtmp->my)) pline("The %s hits it.", str);
	else pline("The %s hits %s%s", str, monnam(mtmp), force);
}

miss(str,mtmp)
register char *str;
register struct monst *mtmp;
{
	if(!cansee(mtmp->mx,mtmp->my)) pline("The %s misses it.",str);
	else pline("The %s misses %s.",str,monnam(mtmp));
}

/* sets bhitpos to the final position of the weapon thrown */
/* coord bhitpos; */

/* check !u.uswallow before calling bhit() */
struct monst *
bhit(ddx,ddy,range,sym)
register int ddx,ddy,range;
char sym;
{
	register struct monst *mtmp;

	bhitpos.x = u.ux;
	bhitpos.y = u.uy;

	if(sym) tmp_at(-1, sym);	/* open call */
	while(range--) {
		bhitpos.x += ddx;
		bhitpos.y += ddy;
		if(mtmp = m_at(bhitpos.x,bhitpos.y)){
			if(sym) tmp_at(-1, -1);	/* close call */
			return(mtmp);
		}
		if(levl[bhitpos.x][bhitpos.y].typ<CORR) {
			bhitpos.x -= ddx;
			bhitpos.y -= ddy;
			break;
		}
		if(sym) tmp_at(bhitpos.x, bhitpos.y);
	}
	if(sym) tmp_at(-1, 0);	/* leave last symbol */
	return(0);
}

struct monst *
boomhit(dx,dy) {
	register int i, ct;
	register struct monst *mtmp;
	char sym = ')';
	extern schar xdir[], ydir[];

	bhitpos.x = u.ux;
	bhitpos.y = u.uy;

	for(i=0; i<8; i++) if(xdir[i] == dx && ydir[i] == dy) break;
	tmp_at(-1, sym);	/* open call */
	for(ct=0; ct<10; ct++) {
		if(i == 8) i = 0;
		sym = ')' + '(' - sym;
		tmp_at(-2, sym);	/* change let call */
		dx = xdir[i];
		dy = ydir[i];
		bhitpos.x += dx;
		bhitpos.y += dy;
		if(mtmp = m_at(bhitpos.x, bhitpos.y)){
			tmp_at(-1,-1);
			return(mtmp);
		}
		if(levl[bhitpos.x][bhitpos.y].typ<CORR) {
			bhitpos.x -= dx;
			bhitpos.y -= dy;
			break;
		}
		if(bhitpos.x == u.ux && bhitpos.y == u.uy) { /* ct == 9 */
			if(rn2(20) >= 10+u.ulevel){	/* we hit ourselves */
				(void) thitu(10, rnd(10), "boomerang");
				break;
			} else {	/* we catch it */
				tmp_at(-1,-1);
				pline("Skillfully, you catch the boomerang.");
				return((struct monst *) -1);
			}
		}
		tmp_at(bhitpos.x, bhitpos.y);
		if(ct % 5 != 0) i++;
	}
	tmp_at(-1, -1);	/* do not leave last symbol */
	return(0);
}

char
dirlet(dx,dy) register int dx,dy; {
	return
		(dx == dy) ? '\\' : (dx && dy) ? '/' : dx ? '-' : '|';
}

/* type < 0: monster spitting fire at you */
buzz(type,sx,sy,dx,dy)
register int type;
register xchar sx,sy;
register int dx,dy;
{
	register char *fltxt = (type < 0) ? "blaze of fire" : fl[type];
	struct rm *lev;
	xchar range;
	struct monst *mon;

	if(u.uswallow) {
		register int tmp;

		if(type < 0) return;
		tmp = zhit(u.ustuck, type);
		pline("The %s rips into %s%s",
			fltxt, monnam(u.ustuck), exclam(tmp));
		return;
	}
	if(type < 0) pru();
	range = rn1(7,7);
	Tmp_at(-1, dirlet(dx,dy));	/* open call */
	while(range-- > 0) {
		sx += dx;
		sy += dy;
		if((lev = &levl[sx][sy])->typ) Tmp_at(sx,sy);
		else {
			int bounce = 0;
			if(cansee(sx-dx,sy-dy)) pline("The %s bounces!",fltxt);
			if(levl[sx][sy-dy].typ > DOOR) bounce = 1;
			if(levl[sx-dx][sy].typ > DOOR) {
				if(!bounce || rn2(2)) bounce = 2;
			}
			switch(bounce){
			case 0:
				dx = -dx;
				dy = -dy;
				continue;
			case 1:
				dy = -dy;
				sx -= dx;
				break;
			case 2:
				dx = -dx;
				sy -= dy;
				break;
			}
			Tmp_at(-2,dirlet(dx,dy));
			continue;
		}
		if((mon = m_at(sx,sy)) &&
		   (type >= 0 || mon->data->mlet != 'D')) {
			wakeup(mon);
			if(rnd(20) < 18 + mon->data->ac) {
				register int tmp = zhit(mon,type);
				if(mon->mhp < 1) {
					if(type < 0) {
					    if(cansee(mon->mx,mon->my))
					      pline("%s is killed by the %s!",
						Monnam(mon), fltxt);
					    mondied(mon);
					} else
					    killed(mon);
				} else
					hit(fltxt, mon, exclam(tmp));
				range -= 2;
			} else
				miss(fltxt,mon);
		} else if(sx == u.ux && sy == u.uy) {
			if(rnd(20) < 18+u.uac) {
				register int dam = 0;
				range -= 2;
				pline("The %s hits you!",fltxt);
				switch(type) {
				case 0:
					dam = d(2,6);
					break;
				case -1:	/* dragon fire */
				case 1:
					if(Fire_resistance)
						pline("You don't feel hot!");
					else dam = d(6,6);
					break;
				case 2:
					nomul(-rnd(25)); /* sleep ray */
					break;
				case 3:
					if(Cold_resistance)
						pline("You don't feel cold!");
					else dam = d(6,6);
					break;
				case 4:
					u.uhp = -1;
				}
				losehp(dam,fltxt);
			} else pline("The %s whizzes by you!",fltxt);
		}
		if(lev->typ <= DOOR) {
			int bounce = 0, rmn;
			if(cansee(sx,sy)) pline("The %s bounces!",fltxt);
			range--;
			if(!dx || !dy || !rn2(20)){
				dx = -dx;
				dy = -dy;
			} else {
			  if((rmn = levl[sx][sy-dy].typ) > DOOR &&
			    (
			     rmn >= ROOM ||
				levl[sx+dx][sy-dy].typ > DOOR)){
				bounce = 1;
			  }
			  if((rmn = levl[sx-dx][sy].typ) > DOOR &&
			    (
			     rmn >= ROOM ||
				levl[sx-dx][sy+dy].typ > DOOR)){
				if(!bounce || rn2(2)){
					bounce = 2;
				}
			  }
			  switch(bounce){
			  case 0:
				dy = -dy;
				dx = -dx;
				break;
			  case 1:
				dy = -dy;
				break;
			  case 2:
				dx = -dx;
				break;
			  }
			  Tmp_at(-2, dirlet(dx,dy));
			}
		}
	}
	Tmp_at(-1,-1);
}

zhit(mon,type)			/* returns damage to mon */
register struct monst *mon;
register int type;
{
	register int tmp = 0;

	switch(type) {
	case 0:			/* magic missile */
		tmp = d(2,6);
		break;
	case -1:		/* Dragon blazing fire */
	case 1:			/* fire */
		if(index("Dg", mon->data->mlet)) break;
		tmp = d(6,6);
		if(mon->data->mlet == 'Y') tmp += 7;
		break;
	case 2:			/* sleep*/
		mon->mfroz = 1;
		break;
	case 3:			/* cold */
		if(index("YFgf", mon->data->mlet)) break;
		tmp = d(6,6);
		if(mon->data->mlet == 'D') tmp += 7;
		break;
	case 4:			/* death*/
		if(index("WVZ",mon->data->mlet)) break;
		tmp = mon->mhp+1;
		break;
	}
	mon->mhp -= tmp;
	return(tmp);
}
#file makedefs.c
/* construct definitions of object constants */
#define	DEF_FILE	"def.objects.h"
#define	LINSZ	1000
#define	STRSZ	40

int fd;
char string[STRSZ];

main(){
register int index = 0;
register int propct = 0;
register char *sp;
	fd = open(DEF_FILE, 0);
	if(fd < 0) {
		perror(DEF_FILE);
		exit(1);
	}
	skipuntil("objects[] = {");
	while(getentry()) {
		if(!*string){
			index++;
			continue;
		}
		for(sp = string; *sp; sp++)
			if(*sp == ' ' || *sp == '\t')
				*sp = '_';
		if(!strncmp(string, "RIN_", 4)){
			capitalize(string+4);
			printf("#define	%s	u.uprops[%d].p_flgs\n",
				string+4, propct++);
		}
		for(sp = string; *sp; sp++) capitalize(sp);
		/* avoid trouble with stupid C preprocessors */
		if(!strncmp(string, "WORTHLESS_PIECE_OF_", 19))
			printf("/* #define %s	%d */\n", string, index);
		else
			printf("#define	%s	%d\n", string, index);
		index++;
	}
	printf("\n#define	CORPSE	DEAD_HUMAN\n");
	printf("#define	LAST_GEM	(JADE+1)\n");
	printf("#define	LAST_RING	%d\n", propct);
	printf("#define	NROFOBJECTS	%d\n", index-1);
}

char line[LINSZ], *lp = line, *lp0 = line, *lpe = line;
int eof;

readline(){
register int n = read(fd, lp0, (line+LINSZ)-lp0);
	if(n < 0){
		printf("Input error.\n");
		exit(1);
	}
	if(n == 0) eof++;
	lpe = lp0+n;
}

char
nextchar(){
	if(lp == lpe){
		readline();
		lp = lp0;
	}
	return((lp == lpe) ? 0 : *lp++);
}

skipuntil(s) char *s; {
register char *sp0, *sp1;
loop:
	while(*s != nextchar())
		if(eof) {
			printf("Cannot skipuntil %s\n", s);
			exit(1);
		}
	if(strlen(s) > lpe-lp+1){
		register char *lp1, *lp2;
		lp2 = lp;
		lp1 = lp = lp0;
		while(lp2 != lpe) *lp1++ = *lp2++;
		lp2 = lp0;	/* save value */
		lp0 = lp1;
		readline();
		lp0 = lp2;
		if(strlen(s) > lpe-lp+1) {
			printf("error in skipuntil");
			exit(1);
		}
	}
	sp0 = s+1;
	sp1 = lp;
	while(*sp0 && *sp0 == *sp1) sp0++, sp1++;
	if(!*sp0){
		lp = sp1;
		return(1);
	}
	goto loop;
}

getentry(){
int inbraces = 0, inparens = 0, stringseen = 0, commaseen = 0;
int prefix = 0;
char ch;
#define	NSZ	10
char identif[NSZ], *ip;
	string[0] = string[4] = 0;
	/* read until {...} or XXX(...) followed by ,
	   skip comment and #define lines
	   deliver 0 on failure
	 */
	while(1) {
		ch = nextchar();
	swi:
		if(letter(ch)){
			ip = identif;
			do {
				if(ip < identif+NSZ-1) *ip++ = ch;
				ch = nextchar();
			} while(letter(ch) || digit(ch));
			*ip = 0;
			while(ch == ' ' || ch == '\t') ch = nextchar();
			if(ch == '(' && !inparens && !stringseen)
				if(!strcmp(identif, "WAND") ||
				   !strcmp(identif, "RING") ||
				   !strcmp(identif, "POTION") ||
				   !strcmp(identif, "SCROLL"))
				(void) strncpy(string, identif, 3),
				string[3] = '_',
				prefix = 4;
		}
		switch(ch) {
		case '/':
			/* watch for comment */
			if((ch = nextchar()) == '*')
				skipuntil("*/");
			goto swi;
		case '{':
			inbraces++;
			continue;
		case '(':
			inparens++;
			continue;
		case '}':
			inbraces--;
			if(inbraces < 0) return(0);
			continue;
		case ')':
			inparens--;
			if(inparens < 0) {
				printf("too many ) ?");
				exit(1);
			}
			continue;
		case '\n':
			/* watch for #define at begin of line */
			if((ch = nextchar()) == '#'){
				register char pch;
				/* skip until '\n' not preceded by '\\' */
				do {
					pch = ch;
					ch = nextchar();
				} while(ch != '\n' || pch == '\\');
				continue;
			}
			goto swi;
		case ',':
			if(!inparens && !inbraces){
				if(prefix && !string[prefix])
					string[0] = 0;
				if(stringseen) return(1);
				printf("unexpected ,\n");
				exit(1);
			}
			commaseen++;
			continue;
		case '\'':
			if((ch = nextchar()) == '\\') ch = nextchar();
			if(nextchar() != '\''){
				printf("strange character denotation?\n");
				exit(1);
			}
			continue;
		case '"':
			{
				register char *sp = string + prefix;
				register char pch;
				register int store = (inbraces || inparens)
					&& !stringseen++ && !commaseen;
				do {
					pch = ch;
					ch = nextchar();
					if(store && sp < string+STRSZ)
						*sp++ = ch;
				} while(ch != '"' || pch == '\\');
				if(store) *--sp = 0;
				continue;
			}
		}
	}
}

capitalize(sp) register char *sp; {
	if('a' <= *sp && *sp <= 'z') *sp += 'A'-'a';
}

letter(ch) register char ch; {
	return( ('a' <= ch && ch <= 'z') ||
		('A' <= ch && ch <= 'Z') );
}

digit(ch) register char ch; {
	return( '0' <= ch && ch <= '9' );
}

#file mklev.c
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* mklev.c version 1.0.1 - new makecorridor() */
#ifndef MKLEV
#define	MKLEV
#endif
#include <stdio.h>
#include "hack.h"
#include "def.trap.h"

extern struct monst *makemon();

static char tspe[2],**args;
static int doorindex;
static schar nxcor;

char ismklev;
struct mkroom *croom, *troom;
boolean goldseen;
int nroom;
int smeq[MAXNROFROOMS+1];
char *tfile;

#ifdef WIZARD
extern boolean wizard;
#endif

#define somex() ((rand()%(croom->hx-croom->lx+1))+croom->lx)
#define somey() ((rand()%(croom->hy-croom->ly+1))+croom->ly)

extern char nul[40];
extern struct rm levl[COLNO][ROWNO];
extern struct monst *fmon;
extern struct obj *fobj;
extern struct gen *fgold, *ftrap;
extern char *fut_geno;      /* monsters that should not be created anymore */
extern struct mkroom rooms[MAXNROFROOMS+1];
extern coord doors[DOORMAX];
extern int comp();
extern xchar dlevel;
extern xchar xdnstair,xupstair,ydnstair,yupstair;
   
zeroout(addr, len)
char *addr;
int len;
{
	while(--len>0) *addr++=0;
}

mklev()
{
	register unsigned tryct;

	tfile = lock;
	troom = NULL;
	doorindex = 0;
	nroom = 0;

	if(getbones()) return;
	ismklev = 1;
	if(dlevel < rn1(3, 26)) tspe[0] = 'a';   /* normal level */
	else tspe[0] = 'b';         /* maze */
	tspe[1] = 0;

	/* zap the object bases */
	{int i;extern int bases[];
	for(i=0;i<15;bases[i++]=0);}

	/* zap the room pictures */
	zeroout(levl,COLNO*ROWNO*sizeof(struct rm));

	fmon = NULL;
	fobj = NULL;
	fgold = ftrap = NULL;
   
	zeroout( doors, sizeof(coord)*DOORMAX);

	xdnstair = xupstair = ydnstair = yupstair = 0;
	zeroout(rooms, (MAXNROFROOMS+1)*sizeof(struct mkroom));

	init_objects();
	rooms[0].hx = -1;   /* in case we are in a maze */

	/* a: normal; b: maze */
	if(*tspe == 'b') {
		makemaz();
#ifdef DOSAVE
		{
		int fd;
		if((fd = creat(tfile,FMASK)) < 0) 
			panic("Cannot create %s\n", tfile);
		savelev(fd);
		close(fd);
		}
#endif
		ismklev = 0;
		return(0);
	}

	/* construct the rooms */
	while(nroom < (MAXNROFROOMS/3)) {
		croom = rooms;
		nroom = 0;
		(void) makerooms(0);      /* not secret */
	}

	/* for each room: put things inside */
	for(croom = rooms; croom->hx > 0; croom++) {

		/* put a sleeping monster inside */
		if(!rn2(3)) (void)
			makemon((struct permonst *) 0, somex(), somey());

		/* put traps and mimics inside */
		goldseen = FALSE;
		while(!rn2(8-(dlevel/6))) mktrap(0,0);
		if(!goldseen && !rn2(3)) mkgold(0,somex(),somey());
		if(!rn2(3)) {
			mkobj_at(0, somex(), somey());
			tryct = 0;
			while(!rn2(5)) {
				if(++tryct > 100){
					myprintf("tryct overflow4\n");
					break;
				}
				mkobj_at(0, somex(), somey());
			}
		}
	}
	tryct = 0;
	do {
		if(++tryct > 1000) panic("Cannot make dnstairs\n");
		croom = &rooms[rn2(nroom)];
		xdnstair = somex();
		ydnstair = somey();
	} while((*tspe =='n' && (!(xdnstair%2) || !(ydnstair%2))) ||
		g_at(xdnstair,ydnstair,ftrap));
	levl[xdnstair][ydnstair].scrsym ='>';
	levl[xdnstair][ydnstair].typ = STAIRS;
	troom = croom;
	do {
		if(++tryct > 2000) panic("Cannot make upstairs\n");
		croom = &rooms[rn2(nroom)];
		xupstair = somex();
		yupstair = somey();
	} while(croom == troom || m_at(xupstair,yupstair) ||
		g_at(xupstair,yupstair,ftrap));
	levl[xupstair][yupstair].scrsym ='<';
	levl[xupstair][yupstair].typ = STAIRS;

#ifdef DEBUG
	dumpit();
#endif
	qsort((char *) rooms, nroom, sizeof(struct mkroom), comp);
	makecorridors();
	make_niches();

	/* make a secret treasure vault, not connected to the rest */
	if(nroom < (2*MAXNROFROOMS/3)) if(!rn2(3)) {
		register int x,y;
		troom = croom = &rooms[nroom];
		if(makerooms(1)) {      /* make secret room */
			troom->rtype = 6;      /* treasure vault */
			for(x = troom->lx; x <= troom->hx; x++)
			for(y = troom->ly; y <= troom->hy; y++)
				mkgold(rnd(dlevel*100) + 50, x, y);
			if(!rn2(3))
				makevtele();
		}
	}

#ifdef WIZARD
	if(wizard){
		if(rn2(3)) mkshop(); else mkzoo();
	} else
#endif WIZARD
	if(dlevel > 1 && dlevel < 20 && rn2(dlevel) < 2)
		mkshop();
	else
		if(dlevel > 6 && !rn2(7) )
			mkzoo();
#ifdef DOSAVE
	{
	int fd;
	if((fd = creat(tfile,FMASK)) < 0) 
		panic("Cannot create %s\n", tfile);
	savelev(fd);
	close(fd);
	}
#endif
	ismklev = 0;
	return(0);
}

makerooms(secret)
int secret;
{
	register int lowx, lowy;
	register int tryct = 0;

   while(nroom < (MAXNROFROOMS/2) || secret)
      for(lowy = rn1(3,3); lowy < ROWNO-7; lowy += rn1(2,4))
         {
         for(lowx = rn1(3,4); lowx < COLNO-10; lowx += rn1(2,7))
            {
            if (tryct++ > 10000) return(0);

            if ((lowy += (rn2(5)-2)) < 3)
               lowy = 3;
            else
               if(lowy > ROWNO-6)
                  lowy = ROWNO-6;

            if(levl[lowx][lowy].typ) continue;

            if ((secret && maker(lowx, 1, lowy, 1)) ||
                (!secret && maker(lowx,rn1(9,2),lowy,rn1(4,2))
                && nroom+2 > MAXNROFROOMS)) return(1);
            }
         }
	return(1);
}

comp(x,y)
register struct mkroom *x,*y;
{
   if(x->lx < y->lx) return(-1);
   return(x->lx > y->lx);
}

coord
finddpos(xl,yl,xh,yh) {
coord ff;
register int x,y;
	ff.x = (xl == xh) ? xl : (xl + rn2(xh-xl+1));
	ff.y = (yl == yh) ? yl : (yl + rn2(yh-yl+1));
	if(okdoor(ff.x, ff.y)) return(ff);
	if(xl < xh) for(x = xl; x <= xh; x++)
		if(okdoor(x, ff.y)){
			ff.x = x;
			return(ff);
		}
	if(yl < yh) for(y = yl; y <= yh; y++)
		if(okdoor(ff.x, y)){
			ff.y = y;
			return(ff);
		}
	return(ff);
}

/* if allowable, create a door at [x,y] */
okdoor(x,y)
register int x,y;
{
	if(levl[x-1][y].typ == DOOR || levl[x+1][y].typ == DOOR ||
	   levl[x][y+1].typ == DOOR || levl[x][y-1].typ == DOOR ||
	   levl[x-1][y].typ == SDOOR || levl[x+1][y].typ == SDOOR ||
	   levl[x][y-1].typ == SDOOR || levl[x][y+1].typ == SDOOR ||
	   (levl[x][y].typ != HWALL && levl[x][y].typ != VWALL) ||
	   doorindex >= DOORMAX)
		return(0);
	return(1);
}

dodoor(x,y,aroom)
register int x,y;
register struct mkroom *aroom;
{
	if(doorindex >= DOORMAX) panic("DOORMAX exceeded?");
	if(!okdoor(x,y) && nxcor) return;
	dosdoor(x,y,aroom,rn2(8) ? DOOR : SDOOR);
}

dosdoor(x,y,aroom,type)
register int x,y;
register struct mkroom *aroom;
register int type;
{
	register struct mkroom *broom;
	register int tmp;

	levl[x][y].typ = type;
	if(type == DOOR)
		levl[x][y].scrsym ='+';
	aroom->doorct++;
	broom = aroom+1;
	if(broom->hx < 0) tmp = doorindex; else
	for(tmp = doorindex; tmp > broom->fdoor; tmp--)
		doors[tmp] = doors[tmp-1];
	doorindex++;
	doors[tmp].x = x;
	doors[tmp].y = y;
	for( ; broom->hx >= 0; broom++) broom->fdoor++;
}

/* Only called from makerooms() */
maker(lowx,ddx,lowy,ddy)
schar lowx,ddx,lowy,ddy;
{
	register int x, y, hix = lowx+ddx, hiy = lowy+ddy;

	if(nroom >= MAXNROFROOMS) return(0);
	if(hix > COLNO-5) hix = COLNO-5;
	if(hiy > ROWNO-4) hiy = ROWNO-4;
chk:
	if(hix <= lowx || hiy <= lowy) return(0);

	/* check area around room (and make room smaller if necessary) */
	for(x = lowx-4; x <= hix+4; x++)
		for(y = lowy-3; y <= hiy+3; y++)
			if(levl[x][y].typ) {
				if(rn2(3)) return(0);
				lowx = x+5;
				lowy = y+4;
				goto chk;
			}

	/* on low levels the room is lit (usually) */
	/* secret vaults are always lit */
	if((rnd(dlevel) < 10 && rn2(77)) || (ddx == 1 && ddy == 1))
		for(x = lowx-1; x <= hix+1; x++)
			for(y = lowy-1; y <= hiy+1; y++)
				levl[x][y].lit = 1;
	croom->lx = lowx;
	croom->hx = hix;
	croom->ly = lowy;
	croom->hy = hiy;
	croom->rtype = croom->doorct = croom->fdoor = 0;
	for(x = lowx-1; x <= hix+1; x++)
	    for(y = lowy-1; y <= hiy+1; y += (hiy-lowy+2)) {
		levl[x][y].scrsym = '-';
		levl[x][y].typ = HWALL;
	}
	for(x = lowx-1; x <= hix+1; x += (hix-lowx+2))
	    for(y = lowy; y <= hiy; y++) {
		levl[x][y].scrsym = '|';
		levl[x][y].typ = VWALL;
	}
	for(x = lowx; x <= hix; x++)
	    for(y = lowy; y <= hiy; y++) {
		levl[x][y].scrsym = '.';
		levl[x][y].typ = ROOM;
	}
	croom++;
	croom->hx = -1;
	smeq[nroom] = nroom;
	nroom++;
	return(1);
}

makecorridors() {
	register int a,b;

	nxcor = 0;
	for(a = 0; a < nroom-1; a++)
		join(a, a+1);
	for(a = 0; a < nroom-2; a++)
	    if(smeq[a] != smeq[a+2])
		join(a, a+2);
	for(a = 0; a < nroom; a++)
	    for(b = 0; b < nroom; b++)
		if(smeq[a] != smeq[b])
		    join(a, b);
	if(nroom > 2)
	    for(nxcor = rn2(nroom) + 4; nxcor; nxcor--) {
		a = rn2(nroom);
		b = rn2(nroom-2);
		if(b >= a) b += 2;
		join(a, b);
	    }
}

join(a,b)
register int a,b;
{
	coord cc,tt;
	register int tx, ty, xx, yy;
	register struct rm *crm;
	register int dx, dy, dix, diy, cct;

	croom = &rooms[a];
	troom = &rooms[b];

	/* find positions cc and tt for doors in croom and troom
	   and direction for a corridor between them */

   if(troom->hx < 0 || croom->hx < 0 || doorindex >= DOORMAX) return;
   if(troom->lx > croom->hx) {
      dx = 1;
      dy = 0;
      xx = croom->hx+1;
      tx = troom->lx-1;
      cc = finddpos(xx,croom->ly,xx,croom->hy);
      tt = finddpos(tx,troom->ly,tx,troom->hy);
   } else if(troom->hy < croom->ly) {
      dy = -1;
      dx = 0;
      yy = croom->ly-1;
      cc = finddpos(croom->lx,yy,croom->hx,yy);
      ty = troom->hy+1;
      tt = finddpos(troom->lx,ty,troom->hx,ty);
   } else if(troom->hx < croom->lx) {
      dx = -1;
      dy = 0;
      xx = croom->lx-1;
      tx = troom->hx+1;
      cc = finddpos(xx,croom->ly,xx,croom->hy);
      tt = finddpos(tx,troom->ly,tx,troom->hy);
   } else {
      dy = 1;
      dx = 0;
      yy = croom->hy+1;
      ty = troom->ly-1;
      cc = finddpos(croom->lx,yy,croom->hx,yy);
      tt = finddpos(troom->lx,ty,troom->hx,ty);
   }
	xx = cc.x;
	yy = cc.y;
	tx = tt.x - dx;
	ty = tt.y - dy;
	if(nxcor && levl[xx+dx][yy+dy].typ)
		return;
	dodoor(xx,yy,croom);

	cct = 0;
	while(xx != tx || yy != ty) {
	    xx += dx;
	    yy += dy;

	    /* loop: dig corridor at [xx,yy] and find new [xx,yy] */
	    if(cct++ > 500 || (nxcor && !rn2(35)))
		return;

	    if(xx == COLNO-1 || xx == 0 || yy == 0 || yy == ROWNO-1)
		return;		/* impossible */

	    crm = &levl[xx][yy];
	    if(!(crm->typ)) {
		if(rn2(100)) {
			crm->typ = CORR;
			crm->scrsym = CORR_SYM;
		} else {
			crm->typ = SCORR;
			crm->scrsym = ' ';
		}
		if(nxcor && !rn2(50)) {
			mkobj_at(ROCK_SYM, xx, yy);
		}
	    } else
	    if(crm->typ != CORR && crm->typ != SCORR) {
		/* strange ... */
      return;
	    }

	    /* find next corridor position */
	    dix = abs(xx-tx);
	    diy = abs(yy-ty);

	    /* do we have to change direction ? */
	    if(dy && dix > diy) {
		register int ddx = (xx > tx) ? -1 : 1;

		crm = &levl[xx+ddx][yy];
		if(!crm->typ || crm->typ == CORR || crm->typ == SCORR) {
		    dx = ddx;
		    dy = 0;
		    continue;
		}
	    } else if(dx && diy > dix) {
		register int ddy = (yy > ty) ? -1 : 1;

		crm = &levl[xx][yy+ddy];
		if(!crm->typ || crm->typ == CORR || crm->typ == SCORR) {
		    dy = ddy;
		    dx = 0;
		    continue;
		}
	    }

	    /* continue straight on? */
	    crm = &levl[xx+dx][yy+dy];
	    if(!crm->typ || crm->typ == CORR || crm->typ == SCORR)
		continue;

	    /* no, what must we do now?? */
	    if(dx) {
		dx = 0;
		dy = (ty < yy) ? -1 : 1;
		crm = &levl[xx+dx][yy+dy];
		if(!crm->typ || crm->typ == CORR || crm->typ == SCORR)
		    continue;
		dy = -dy;
		continue;
	    } else {
		dy = 0;
		dx = (tx < xx) ? -1 : 1;
		crm = &levl[xx+dx][yy+dy];
		if(!crm->typ || crm->typ == CORR || crm->typ == SCORR)
		    continue;
		dx = -dx;
		continue;
	    }
   }
	/* we succeeded in digging the corridor */
	dodoor(tt.x, tt.y, troom);

	if(smeq[a] < smeq[b])
		smeq[b] = smeq[a];
	else
		smeq[a] = smeq[b];
}

make_niches()
{
	register int ct = rn2(nroom/2 + 1)+1;
	while(ct--) makeniche(FALSE);
}

makevtele()
{
	makeniche(TRUE);
}

makeniche(with_trap)
boolean with_trap;
{
	register struct mkroom *aroom;
	register struct rm *rm;
	register int vct = 8;
	coord dd;
	register int dy,xx,yy;
	register struct gen *gtmp;

	if(doorindex < DOORMAX)
	  while(vct--) {
	    aroom = &rooms[rn2(nroom-1)];
	    if(aroom->rtype != 0) continue;	/* not an ordinary room */
	    if(rn2(2)) {
		dy = 1;
		dd = finddpos(aroom->lx,aroom->hy+1,aroom->hx,aroom->hy+1);
	    } else {
		dy = -1;
		dd = finddpos(aroom->lx,aroom->ly-1,aroom->hx,aroom->ly-1);
	    }
	    xx = dd.x;
	    yy = dd.y;
	    if((rm = &levl[xx][yy+dy])->typ) continue;
	    if(with_trap || !rn2(4)) {
		rm->typ = SCORR;
		rm->scrsym = ' ';
		if(with_trap) {
		    gtmp = newgen();
		    gtmp->gx = xx;
		    gtmp->gy = yy+dy;
		    gtmp->gflag = TELEP_TRAP | ONCE;
		    gtmp->ngen = ftrap;
		    ftrap = gtmp;
		    make_engr_at(xx,yy-dy,"ad ae?ar um");
		}
		dosdoor(xx,yy,aroom,SDOOR);
	    } else {
		rm->typ = CORR;
		rm->scrsym = CORR_SYM;
		if(rn2(7))
		    dosdoor(xx,yy,aroom,rn2(5) ? SDOOR : DOOR);
		else {
		    mksobj_at(SCROLL_SYM,SCR_TELEPORTATION,xx,yy+dy);
		    if(!rn2(3)) mkobj_at(0,xx,yy+dy);
		}
	    }
	    return;
      }
}

/* make a trap somewhere (in croom if mazeflag = 0) */
mktrap(num,mazeflag)
register int num,mazeflag;
{
	register struct gen *gtmp;
	register int kind,nopierc,nomimic,fakedoor,fakegold,tryct = 0;
	register xchar mx,my;


   if(!num || num >= TRAPNUM) {
      nopierc = (dlevel < 4) ? 1 : 0;
      nomimic = (dlevel < 9 || goldseen ) ? 1 : 0;
      if(index(fut_geno, 'M')) nomimic = 1;
      kind = rn2(TRAPNUM - nopierc - nomimic);
      /* note: PIERC = 7, MIMIC = 8, TRAPNUM = 9 */
   } else kind = num;

   if(kind == MIMIC) {
		register struct monst *mtmp;

      fakedoor = (!rn2(3) && !mazeflag);
      fakegold = (!fakedoor && !rn2(2));
      if(fakegold) goldseen = TRUE;
      do {
         if(++tryct > 200) return;
         if(fakedoor) {
            /* note: fakedoor maybe on actual door */
            if(rn2(2)){
               if(rn2(2))
                  mx = croom->hx+1;
               else mx = croom->lx-1;
               my = somey();
            } else {
               if(rn2(2))
                  my = croom->hy+1;
               else my = croom->ly-1;
               mx = somex();
            }
         } else if(mazeflag) {
            extern coord mazexy();
            coord mm;
            mm = mazexy();
            mx = mm.x;
            my = mm.y;
         } else {
            mx = somex();
            my = somey();
         }
      } while(m_at(mx,my));
      if(mtmp = makemon(PM_MIMIC,mx,my))
          mtmp->mimic =
         fakegold ? '$' : fakedoor ? '+' :
         (mazeflag && rn2(2)) ? AMULET_SYM :
         "=/)%?![<>" [ rn2(9) ];
      return;
   }
   gtmp = newgen();
   gtmp->gflag = kind;
   do {
      if(++tryct > 200){
         free((char *) gtmp);
         return;
      }
      if(mazeflag){
         extern coord mazexy();
         coord mm;
         mm = mazexy();
         gtmp->gx = mm.x;
         gtmp->gy = mm.y;
      } else {
         gtmp->gx = somex();
         gtmp->gy = somey();
      }
   } while(g_at(gtmp->gx, gtmp->gy, ftrap));
   gtmp->ngen = ftrap;
   ftrap = gtmp;
   if(mazeflag && !rn2(10) && gtmp->gflag < PIERC) gtmp->gflag |= SEEN;
}

#ifdef DEBUG
dumpit()
   {
   int x, y;
   struct rm *room;

   cgetret(); 

   /* kludge in making everything visible */
   for(y=0; y < ROWNO; y++)
      for(x=0; x < COLNO-3; x++)
         if ( (room = &levl[x][y])->typ)
            at(x,y, (room->scrsym) ? room->scrsym : '?');
   }
#endif
#file mklv.makemaz.c
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1984. */

#include "mklev.h"
extern struct monst *makemon();
extern coord mazexy();

makemaz()
{
	int x,y;
	register int zx,zy;
	coord mm;

	for(x = 2; x < COLNO-1; x++)
		for(y = 2; y < ROWNO-1; y++)
			levl[x][y].typ = (x%2 && y%2) ? 0 : HWALL;
	mm = mazexy();
	zx = mm.x;
	zy = mm.y;
	walkfrom(zx,zy);
	mkobj_at(AMULET_SYM, zx, zy);
	mkobj_at(ROCK_SYM, zx, zy);	/* put a rock on top of the amulet */
	/* (probably this means that one needs a wand of digging to reach 
	    the amulet - we must make sure that the player has a chance of
	    getting one; let us say when he kills the minotaur; of course
	    the minotaur itself may be blocked behind rocks, but well...) */
	for(x = 2; x < COLNO-1; x++)
		for(y = 2; y < ROWNO-1; y++) {
			switch(levl[x][y].typ) {
			case HWALL:
				levl[x][y].scrsym = '-';
				break;
			case ROOM:
				levl[x][y].scrsym = '.';
				break;
			}
		}
	for(x = rn1(8,11); x; x--) {
		mm = mazexy();
		mkobj_at(0, mm.x, mm.y);
	}
	for(x = rn1(10,2); x; x--) {
		mm = mazexy();
		mkobj_at(ROCK_SYM, mm.x, mm.y);
	}
	mm = mazexy();
	(void) makemon(PM_MINOTAUR, mm.x, mm.y);
	for(x = rn1(5,7); x; x--) {
		mm = mazexy();
		(void) makemon((struct permonst *) 0, mm.x, mm.y);
	}
	for(x = rn1(6,7); x; x--) {
		mm = mazexy();
		mkgold(0,mm.x,mm.y);
	}
	for(x = rn1(6,7); x; x--)
		mktrap(0,1);
	mm = mazexy();
	levl[(xupstair = mm.x)][(yupstair = mm.y)].scrsym = '<';
	levl[xupstair][yupstair].typ = STAIRS;
	xdnstair = ydnstair = 0;
}

walkfrom(x,y) int x,y; {
register int q,a,dir;
int dirs[4];
	levl[x][y].typ = ROOM;
	while(1) {
		q = 0;
		for(a = 0; a < 4; a++)
			if(okay(x,y,a)) dirs[q++]= a;
		if(!q) return;
		dir = dirs[rn2(q)];
		move(&x,&y,dir);
		levl[x][y].typ = ROOM;
		move(&x,&y,dir);
		walkfrom(x,y);
	}
}

move(x,y,dir)
register int *x, *y;
register int dir;
{
	switch(dir){
		case 0: --(*y); break;
		case 1: (*x)++; break;
		case 2: (*y)++; break;
		case 3: --(*x); break;
	}
}

okay(x,y,dir)
int x,y;
register int dir;
{
	move(&x,&y,dir);
	move(&x,&y,dir);
	if(x<3 || y<3 || x>COLNO-3 || y>ROWNO-3 || levl[x][y].typ != 0)
		return(0);
	else
		return(1);
}

coord
mazexy(){
	coord mm;
	mm.x = 3 + 2*rn2(COLNO/2 - 2);
	mm.y = 3 + 2*rn2(ROWNO/2 - 2);
	return mm;
}
#file mklv.shk.c
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1984. */

#ifndef QUEST
#include "mklev.h"
#include "def.eshk.h"
#define   ESHK   ((struct eshk *)(&(shk->mextra[0])))
extern struct monst *makemon();

extern char shtypes[]; /* = "=/)%?!["; ** 8 shoptypes: 7 specialised, 1 mixed */
schar shprobs[] = { 3,3,5,5,10,10,14,50 };   /* their probabilities */

mkshop(){
register struct mkroom *sroom;
register int sh,sx,sy,i;
register char let;
int roomno;
register struct monst *shk;
   for(sroom = &rooms[0], roomno = 0; ; sroom++, roomno++){
      if(sroom->hx < 0) return;
      if(sroom->lx <= xdnstair && xdnstair <= sroom->hx &&
         sroom->ly <= ydnstair && ydnstair <= sroom->hy) continue;
      if(sroom->lx <= xupstair && xupstair <= sroom->hx &&
         sroom->ly <= yupstair && yupstair <= sroom->hy) continue;
      if(
#ifdef WIZARD
         wizard ||
#endif WIZARD
         sroom->doorct == 1) break;
   }
#ifdef WIZARD
   if(wizard){
      extern char *getenv();
      register char *ep = getenv("SHOPTYPE");
      if(ep){
         if(*ep == 'z' || *ep == 'Z'){
            mkzoo();
            return;
         }
         for(i=0; shtypes[i]; i++)
            if(*ep == shtypes[i]) break;
         let = i;
         goto gotlet;
      }
   }
#endif WIZARD
   for(i = rn2(100),let = 0; (i -= shprobs[let])>= 0; let++)
      if(!shtypes[let]) break;   /* superfluous */
#ifdef WIZARD
gotlet:
#endif WIZARD
   sroom->rtype = 8+let;
   let = shtypes[let];
   sh = sroom->fdoor;
   sx = doors[sh].x;
   sy = doors[sh].y;
   if(sx == sroom->lx-1) sx++; else
   if(sx == sroom->hx+1) sx--; else
   if(sy == sroom->ly-1) sy++; else
   if(sy == sroom->hy+1) sy--; else {
      myprintf("Where is shopdoor?");
      return;
   }
   if(!(shk = makemon(PM_SHK,sx,sy))) return;
   shk->isshk = shk->mpeaceful = 1;
   shk->msleep = 0;
   shk->mtrapseen = ~0;   /* we know all the traps already */
   ESHK->shoproom = roomno;
   ESHK->shd = doors[sh];
   ESHK->shk.x = sx;
   ESHK->shk.y = sy;
   ESHK->robbed = 0;
   ESHK->visitct = 0;
   shk->mgold = 1000 + 30*rnd(100);   /* initial capital */
   ESHK->billct = 0;
   findname(ESHK->shknam, let);
   for(sx = sroom->lx; sx <= sroom->hx; sx++)
   for(sy = sroom->ly; sy <= sroom->hy; sy++){
      register struct monst *mtmp;
      if((sx == sroom->lx && doors[sh].x == sx-1) ||
         (sx == sroom->hx && doors[sh].x == sx+1) ||
         (sy == sroom->ly && doors[sh].y == sy-1) ||
         (sy == sroom->hy && doors[sh].y == sy+1)) continue;
      if(rn2(100) < dlevel && !m_at(sx,sy) &&
         (mtmp = makemon(PM_MIMIC, sx, sy))){
         mtmp->mimic =
             (let && rn2(10) < dlevel) ? let : ']';
         continue;
      }
      mkobj_at(let, sx, sy);
   }
#ifdef WIZARD
   if(wizard) myprintf("I made a %c-shop.", let ? let : 'g');
#endif WIZARD
}

mkzoo(){
register struct mkroom *sroom;
register int sh,sx,sy,i;
int goldlim = 500 * dlevel;
   for(sroom = &rooms[0]; ; sroom++){
      if(sroom->hx < 0) return;
      if(sroom->lx <= xdnstair && xdnstair <= sroom->hx &&
         sroom->ly <= ydnstair && ydnstair <= sroom->hy) continue;
      if(sroom->lx <= xupstair && xupstair <= sroom->hx &&
         sroom->ly <= yupstair && yupstair <= sroom->hy) continue;
      if(sroom->doorct == 1) break;
   }
   sroom->rtype = 7;
   sh = sroom->fdoor;
   for(sx = sroom->lx; sx <= sroom->hx; sx++)
   for(sy = sroom->ly; sy <= sroom->hy; sy++){
      if((sx == sroom->lx && doors[sh].x == sx-1) ||
         (sx == sroom->hx && doors[sh].x == sx+1) ||
         (sy == sroom->ly && doors[sh].y == sy-1) ||
         (sy == sroom->hy && doors[sh].y == sy+1)) continue;
      (void) makemon((struct permonst *) 0,sx,sy);
      i = sq(dist2(sx,sy,doors[sh].x,doors[sh].y));
      if(i >= goldlim) i = 5*dlevel;
      goldlim -= i;
      mkgold(10 + rn2(i), sx, sy);
   }
#ifdef WIZARD
   if(wizard) myprintf("I made a zoo.");
#endif WIZARD
}

dist2(x0,y0,x1,y1){
   return((x0-x1)*(x0-x1) + (y0-y1)*(y0-y1));
}

sq(a) int a; {
   return(a*a);
}
#endif QUEST
#file mklv.shknam.c
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1984. */



#include "mklev.h"



char *shkliquors[] = {

	/* Ukraine */

	"Njezjin", "Tsjernigof", "Gomel", "Ossipewsk", "Gorlowka",

	/* N. Russia */

	"Konosja", "Weliki Oestjoeg", "Syktywkar", "Sablja",

	"Narodnaja", "Kyzyl",

	/* Silezie */

	"Walbrzych", "Swidnica", "Klodzko", "Raciborz", "Gliwice",

	"Brzeg", "Krnov", "Hradec Kralove",

	/* Schweiz */

	"Leuk", "Brig", "Brienz", "Thun", "Sarnen", "Burglen", "Elm",

	"Flims", "Vals", "Schuls", "Zum Loch",

	0

};



char *shkbooks[] = {

	/* Eire */

	"Skibbereen", "Kanturk", "Rath Luirc", "Ennistymon", "Lahinch",

	"Loughrea", "Croagh", "Maumakeogh", "Ballyjamesduff",

	"Kinnegad", "Lugnaquillia", "Enniscorthy", "Gweebarra",

	"Kittamagh", "Nenagh", "Sneem", "Ballingeary", "Kilgarvan",

	"Cahersiveen", "Glenbeigh", "Kilmihil", "Kiltamagh",

	"Droichead Atha", "Inniscrone", "Clonegal", "Lisnaskea",

	"Culdaff", "Dunfanaghy", "Inishbofin", "Kesh",

	0

};



char *shkarmors[] = {

	/* Turquie */

	"Demirci", "Kalecik", "Boyabai", "Yildizeli", "Gaziantep",

	"Siirt", "Akhalataki", "Tirebolu", "Aksaray", "Ermenak",

	"Iskenderun", "Kadirli", "Siverek", "Pervari", "Malasgirt",

	"Bayburt", "Ayancik", "Zonguldak", "Balya", "Tefenni",

	"Artvin", "Kars", "Makharadze", "Malazgirt", "Midyat",

	"Birecik", "Kirikkale", "Alaca", "Polatli", "Nallihan",

	0

};



char *shkwands[] = {

	/* Wales */

	"Yr Wyddgrug", "Trallwng", "Mallwyd", "Pontarfynach",

	"Rhaeader", "Llandrindod", "Llanfair-ym-muallt",

	"Y-Fenni", "Measteg", "Rhydaman", "Beddgelert",

	"Curig", "Llanrwst", "Llanerchymedd", "Caergybi",

	/* Scotland */

	"Nairn", "Turriff", "Inverurie", "Braemar", "Lochnagar",

	"Kerloch", "Beinn a Ghlo", "Drumnadrochit", "Morven",

	"Uist", "Storr", "Sgurr na Ciche", "Cannich", "Gairloch",

	"Kyleakin", "Dunvegan",

	0

};



char *shkrings[] = {

	/* Hollandse familienamen */

	"Feyfer", "Flugi", "Gheel", "Havic", "Haynin", "Hoboken",

	"Imbyze", "Juyn", "Kinsky", "Massis", "Matray", "Moy",

	"Olycan", "Sadelin", "Svaving", "Tapper", "Terwen", "Wirix",

	"Ypey",

	/* Skandinaviske navne */

	"Rastegaisa", "Varjag Njarga", "Kautekeino", "Abisko",

	"Enontekis", "Rovaniemi", "Avasaksa", "Haparanda",

	"Lulea", "Gellivare", "Oeloe", "Kajaani", "Fauske",

	0

};



char *shkfoods[] = {

	/* Indonesia */

	"Djasinga", "Tjibarusa", "Tjiwidej", "Pengalengan",

	"Bandjar", "Parbalingga", "Bojolali", "Sarangan",

	"Ngebel", "Djombang", "Ardjawinangun", "Berbek",

	"Papar", "Baliga", "Tjisolok", "Siboga", "Banjoewangi",

	"Trenggalek", "Karangkobar", "Njalindoeng", "Pasawahan",

	"Pameunpeuk", "Patjitan", "Kediri", "Pemboeang", "Tringanoe",

	"Makin", "Tipor", "Semai", "Berhala", "Tegal", "Samoe",

	0

};



char *shkweapons[] = {

	/* Perigord */

	"Voulgezac", "Rouffiac", "Lerignac", "Touverac", "Guizengeard",

	"Melac", "Neuvicq", "Vanzac", "Picq", "Urignac", "Corignac",

	"Fleac", "Lonzac", "Vergt", "Queyssac", "Liorac", "Echourgnac",

	"Cazelon", "Eypau", "Carignan", "Monbazillac", "Jonzac",

	"Pons", "Jumilhac", "Fenouilledes", "Laguiolet", "Saujon",

	"Eymoutiers", "Eygurande", "Eauze", "Labouheyre",

	0

};



char *shkgeneral[] = {

	/* Suriname */

	"Hebiwerie", "Possogroenoe", "Asidonhopo", "Manlobbi",

	"Adjama", "Pakka Pakka", "Kabalebo", "Wonotobo",

	"Akalapi", "Sipaliwini",

	/* Greenland */

	"Annootok", "Upernavik", "Angmagssalik",

	/* N. Canada */

	"Aklavik", "Inuvik", "Tuktoyaktuk",

	"Chicoutimi", "Ouiatchouane", "Chibougamau",

	"Matagami", "Kipawa", "Kinojevis",

	"Abitibi", "Maganasipi",

	/* Iceland */

	"Akureyri", "Kopasker", "Budereyri", "Akranes", "Bordeyri",

	"Holmavik",

	0

};



struct shk_nx {

	char x;

	char **xn;

} shk_nx[] = {

	{ POTION_SYM,	shkliquors },

	{ SCROLL_SYM,	shkbooks },

	{ ARMOR_SYM,	shkarmors },

	{ WAND_SYM,	shkwands },

	{ RING_SYM,	shkrings },

	{ FOOD_SYM,	shkfoods },

	{ WEAPON_SYM,	shkweapons },

	{ 0,		shkgeneral }

};



findname(nampt, let) char *nampt; char let; {

register struct shk_nx *p = shk_nx;

register char **q;

register int i;

	while(p->x && p->x != let) p++;

	q = p->xn;

	for(i=0; i<dlevel; i++) if(!q[i]){

		/* Not enough names, try general name */

		if(let) findname(nampt, 0);

		else (void) strcpy(nampt, "Dirk");

		return;

	}

	(void) strncpy(nampt, q[i], PL_NSIZ);

	nampt[PL_NSIZ-1] = 0;

}

#file rnd.c
#define RND(x)	((rand()>>3) % x)

rn1(x,y)
register int x,y;
{
	return(RND(x)+y);
}

rn2(x)
register int x;
{
	return(RND(x));
}

rnd(x)
register int x;
{
	return(RND(x)+1);
}

d(n,x)
register int n,x;
{
	register int tmp = n;

	while(n--) tmp += RND(x);
	return(tmp);
}
#file savelev.c
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* savelev.h version 1.0.1 - also save engravings from MKLEV */

#include "hack.h"
#include <stdio.h>
extern struct monst *restmonchn();
extern struct obj *restobjchn();
extern struct obj *billobjs;
extern char *itoa();

extern char nul[];
#ifndef NOWORM
#include	"def.wseg.h"

extern struct wseg *wsegs[32], *wheads[32];
extern long wgrowtime[32];
#endif NOWORM

savelev(fd){
#ifndef NOWORM
	register struct wseg *wtmp, *wtmp2;
	register int tmp;
#endif NOWORM

	if(fd < 0)
           panic("Save on bad file!");

	bwrite(fd,(char *) levl,sizeof(levl));
	bwrite(fd,(char *) &moves,sizeof(long));
	bwrite(fd,(char *) &xupstair,sizeof(xupstair));
	bwrite(fd,(char *) &yupstair,sizeof(yupstair));
	bwrite(fd,(char *) &xdnstair,sizeof(xdnstair));
	bwrite(fd,(char *) &ydnstair,sizeof(ydnstair));
	savemonchn(fd, fmon);
	savegenchn(fd, fgold);
	savegenchn(fd, ftrap);
	saveobjchn(fd, fobj);
	saveobjchn(fd, billobjs);
/*	if (!ismklev) */
	   billobjs = 0;
	save_engravings(fd);
#ifndef QUEST
	bwrite(fd,(char *) rooms,sizeof(rooms));
	bwrite(fd,(char *) doors,sizeof(doors));
#endif QUEST
/* 	if (!ismklev) */
	   {
	   fgold = ftrap = 0;
	   fmon = 0;
	   fobj = 0;
	   }
/*--------------------------------------------------------------------*/
#ifndef NOWORM
	bwrite(fd,(char *) wsegs,sizeof(wsegs));
	for(tmp=1; tmp<32; tmp++){
		for(wtmp = wsegs[tmp]; wtmp; wtmp = wtmp2){
			wtmp2 = wtmp->nseg;
			bwrite(fd,(char *) wtmp,sizeof(struct wseg));
		}
/*		if (!ismklev) */
		   wsegs[tmp] = 0;
	}
	bwrite(fd,(char *) wgrowtime,sizeof(wgrowtime));
#endif NOWORM
/*--------------------------------------------------------------------*/
}

bwrite(fd,loc,num)
register int fd;
register char *loc;
register unsigned num;
{
/* lint wants the 3rd arg of write to be an int; lint -p an unsigned */
	if(write(fd, loc, (int) num) != num)
		panic("cannot write %d bytes to file #%d",num,fd);
}

saveobjchn(fd,otmp)
register int fd;
register struct obj *otmp;
{
	register struct obj *otmp2;
	unsigned xl;
	int minusone = -1;

	while(otmp) {
		otmp2 = otmp->nobj;
		xl = otmp->onamelth;
		bwrite(fd, (char *) &xl, sizeof(int));
		bwrite(fd, (char *) otmp, xl + sizeof(struct obj));
/*		if (!ismklev) */
			free((char *) otmp);
		otmp = otmp2;
	}
	bwrite(fd, (char *) &minusone, sizeof(int));
}

savemonchn(fd,mtmp)
register int fd;
register struct monst *mtmp;
{
	register struct monst *mtmp2;
	unsigned xl;
	int minusone = -1;
	int monnum;
#ifdef FUNNYRELOC
	struct permonst *monbegin = &mons[0];

	bwrite(fd, (char *) &monbegin, sizeof(monbegin));
#endif

	while(mtmp) {
		mtmp2 = mtmp->nmon;
		xl = mtmp->mxlth + mtmp->mnamelth;
		bwrite(fd, (char *) &xl, sizeof(int));

		/* JAT - just save the offset into the monster table, */
		/* it will be relocated when read in */
		monnum = mtmp->data - &mons[0];
		mtmp->data = (struct permonst *)monnum;
#ifdef DEBUGMON
		myprintf("Wrote monster #%d", monnum);
#endif
		bwrite(fd, (char *) mtmp, xl + sizeof(struct monst));
		if(mtmp->minvent) saveobjchn(fd,mtmp->minvent);
/*		if (!ismklev) */
		   free((char *) mtmp);
		mtmp = mtmp2;
	}
	bwrite(fd, (char *) &minusone, sizeof(int));
}

savegenchn(fd,gtmp)
register int fd;
register struct gen *gtmp;
{
	register struct gen *gtmp2;
	while(gtmp) {
		gtmp2 = gtmp->ngen;
		bwrite(fd, (char *) gtmp, sizeof(struct gen));
/*		if (!ismklev) */
		   free((char *) gtmp);
		gtmp = gtmp2;
	}
	bwrite(fd, nul, sizeof(struct gen));
}
