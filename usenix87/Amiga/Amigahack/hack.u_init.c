#file hack.u_init.c
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1984. */

#include "hack.h"
#include <stdio.h>
#include <signal.h>
#define   Strcat   (void) strcat
#define   UNDEF_TYP   0
#define   UNDEF_SPE   (-1)
extern struct obj *addinv();
extern char plname[];

char pl_character[PL_CSIZ];

struct trobj {
   uchar trotyp;
   schar trspe;
   char trolet;
   Bitfield(trquan,6);
   Bitfield(trknown,1);
};

#ifdef WIZARD
struct trobj Extra_objs[] = {
   { 0, 0, 0, 0, 0 },
   { 0, 0, 0, 0, 0 }
};
#endif WIZARD

struct trobj Cave_man[] = {
   { MACE, 1, WEAPON_SYM, 1, 1 },
   { BOW, 1, WEAPON_SYM, 1, 1 },
   { ARROW, 0, WEAPON_SYM, 25, 1 },   /* quan is variable */
   { LEATHER_ARMOR, 2, ARMOR_SYM, 1, 1 },
   { 0, 0, 0, 0, 0}
};

struct trobj Fighter[] = {
   { TWO_HANDED_SWORD, 0, WEAPON_SYM, 1, 1 },
   { RING_MAIL, 3, ARMOR_SYM, 1, 1 },
   { 0, 0, 0, 0, 0 }
};

struct trobj Knight[] = {
   { LONG_SWORD, 0, WEAPON_SYM, 1, 1 },
   { SPEAR, 2, WEAPON_SYM, 1, 1 },
   { RING_MAIL, 4, ARMOR_SYM, 1, 1 },
   { HELMET, 1, ARMOR_SYM, 1, 1 },
   { SHIELD, 1, ARMOR_SYM, 1, 1 },
   { PAIR_OF_GLOVES, 1, ARMOR_SYM, 1, 1 },
   { 0, 0, 0, 0, 0 }
};

struct trobj Speleologist[] = {
   { STUDDED_LEATHER_ARMOR, 3, ARMOR_SYM, 1, 1 },
   { UNDEF_TYP, 0, POTION_SYM, 2, 0 },
   { FOOD_RATION, 0, FOOD_SYM, 3, 1 },
   { ICE_BOX, 0, TOOL_SYM, 1, 0 },
   { 0, 0, 0, 0, 0}
};

struct trobj Tourist[] = {
   { UNDEF_TYP, 0, FOOD_SYM, 10, 1 },
   { POT_EXTRA_HEALING, 0, POTION_SYM, 2, 0 },
   { EXPENSIVE_CAMERA, 0, TOOL_SYM, 1, 1 },
   { DART, 2, WEAPON_SYM, 25, 1 },   /* quan is variable */
   { 0, 0, 0, 0, 0 }
};

struct trobj Wizard[] = {
   { ELVEN_CLOAK, 1, ARMOR_SYM, 1, 1 },
   { UNDEF_TYP, UNDEF_SPE, WAND_SYM, 2, 0 },
   { UNDEF_TYP, UNDEF_SPE, RING_SYM, 2, 0 },
   { UNDEF_TYP, UNDEF_SPE, POTION_SYM, 2, 0 },
   { UNDEF_TYP, UNDEF_SPE, SCROLL_SYM, 3, 0 },
   { 0, 0, 0, 0, 0 }
};

#ifdef NEWS
int u_in_infl;

u_in_intrup()
{
   u_in_infl++;
   (void) signal(SIGINT, u_in_intrup);
}
#endif NEWS

u_init(){
register int c,pc,i;
#ifdef NEWS
   /* It is not unlikely that we get an interrupt here
      intended to kill the news; unfortunately this would
      also kill (part of) the following question */
int (*prevsig)() = signal(SIGINT, u_in_intrup);
#endif NEWS
register char *cp;
char buf[256];
   if(pc = pl_character[0]) goto got_suffix;
   buf[0] = 0;
   Strcat(buf, "\nTell me what kind of character you are:\n");
   Strcat(buf, "Are you a Tourist, a Speleologist, a Fighter,\n");
   Strcat(buf, "        a Knight, a Cave-man or a Wizard? [TSFKCW] ");
intrup:
   for(cp = buf; *cp; cp++){
#ifdef NEWS
      if(u_in_infl){
         u_in_infl = 0;
         goto intrup;
      }
#endif NEWS
      (void) myputchar(*cp);
   }
loop:
   (void) myfflush(stdout);
   pc = 0;
   while((c = inchar()) != '\n') {
#ifndef AMIGA
      if(c == EOF) {
#ifdef NEWS
         if(u_in_infl) goto intrup;   /* %% */
#endif NEWS
         settty("\nEnd of input?\n");
         hackexit(0);
         }
      else
#endif !AMIGA
         if(pc && c==8)  /* backspace over it? */
         {
         myputchar(c);
         pc = 0;
         }
      else if (!pc)
         {
         pc = c;
         myputchar(c);
         }
   }
   if(!pc || !strchr("TSFKCWtsfkcw", pc)){
      myprintf("\nAnswer with T,S,F,K,C or W. What are you? ");
      goto loop;
   }
got_suffix:
   myputchar('\n');
   myfflush();
   if('a' <= pc && pc <= 'z') pc += 'A'-'a';

#ifdef NEWS
   (void) signal(SIGINT,prevsig);
#endif NEWS

   u.usym = '@';
   u.ulevel = 1;
   init_uhunger();
   u.uhpmax = u.uhp = 12;
   u.ustrmax = u.ustr = !rn2(20) ? 14 + rn2(7) : 16;
#ifdef QUEST
   u.uhorizon = 6;
#endif QUEST
   switch(pc) {
   case 'C':
      setpl_char("Cave-man");
      Cave_man[2].trquan = 12 + rnd(9)*rnd(9);
      u.uhp = u.uhpmax = 16;
      u.ustr = u.ustrmax = 18;
      ini_inv(Cave_man);
      break;
   case 'T':
      setpl_char("Tourist");
      Tourist[3].trquan = 20 + rnd(20);
      u.ugold = u.ugold0 = rnd(1000);
      u.uhp = u.uhpmax = 10;
      u.ustr = u.ustrmax = 8;
      ini_inv(Tourist);
      break;
   case 'W':
      setpl_char("Wizard");
      for(i=1; i<=4; i++) if(!rn2(5))
         Wizard[i].trquan += rn2(3) - 1;
      u.uhp = u.uhpmax = 15;
      u.ustr = u.ustrmax = 16;
      ini_inv(Wizard);
      break;
   case 'S':
      setpl_char("Speleologist");
      Fast = INTRINSIC;
      Stealth = INTRINSIC;
      u.uhp = u.uhpmax = 12;
      u.ustr = u.ustrmax = 10;
      ini_inv(Speleologist);
      break;
   case 'K':
      setpl_char("Knight");
      u.uhp = u.uhpmax = 12;
      u.ustr = u.ustrmax = 10;
      ini_inv(Knight);
      break;
   case 'F':
      setpl_char("Fighter");
      u.uhp = u.uhpmax = 14;
      u.ustr = u.ustrmax = 17;
      ini_inv(Fighter);
   }
   find_ac();
   /* make sure he can carry all he has - especially for T's */
   while(inv_weight() > 0 && u.ustr < 118)
      u.ustr++, u.ustrmax++;
#ifdef WIZARD
   if(wizard) wiz_inv();
#endif WIZARD
}

ini_inv(trop) register struct trobj *trop; {
register struct obj *obj;
extern struct obj *mkobj();
   while(trop->trolet) {
      obj = mkobj(trop->trolet);
      obj->known = trop->trknown;
      obj->cursed = 0;
      if(obj->olet == WEAPON_SYM){
         obj->quan = trop->trquan;
         trop->trquan = 1;
      }
      if(trop->trspe != UNDEF_SPE)
         obj->spe = trop->trspe;
      if(trop->trotyp != UNDEF_TYP)
         obj->otyp = trop->trotyp;
      obj->owt = weight(obj);   /* defined after setting otyp+quan */
      obj = addinv(obj);
      if(obj->olet == ARMOR_SYM){
         switch(obj->otyp){
         case SHIELD:
            if(!uarms) setworn(obj, W_ARMS);
            break;
         case HELMET:
            if(!uarmh) setworn(obj, W_ARMH);
            break;
         case PAIR_OF_GLOVES:
            if(!uarmg) setworn(obj, W_ARMG);
            break;
         case ELVEN_CLOAK:
            if(!uarm2)
               setworn(obj, W_ARM);
            break;
         default:
            if(!uarm) setworn(obj, W_ARM);
         }
      }
      if(obj->olet == WEAPON_SYM)
         if(!uwep) setuwep(obj);
      if(--trop->trquan) continue;   /* make a similar object */
      trop++;
   }
}

#ifdef WIZARD
wiz_inv(){
register struct trobj *trop = &Extra_objs[0];
extern char *getenv();
register char *ep = getenv("INVENT");
register int type;
   while(ep && *ep) {
      type = atoi(ep);
      ep = strchr(ep, ',');
      if(ep) while(*ep == ',' || *ep == ' ') ep++;
      if(type <= 0 || type > NROFOBJECTS) continue;
      trop->trotyp = type;
      trop->trolet = objects[type].oc_olet;
      trop->trspe = 4;
      trop->trknown = 1;
      trop->trquan = 1;
      ini_inv(trop);
   }
   /* give him a wand of wishing by default */
   trop->trotyp = WAN_WISHING;
   trop->trolet = WAND_SYM;
   trop->trspe = 20;
   trop->trknown = 1;
   trop->trquan = 1;
   ini_inv(trop);
}
#endif WIZARD

setpl_char(plc) char *plc; {
   (void) strncpy(pl_character, plc, PL_CSIZ-1);
   pl_character[PL_CSIZ-1] = 0;
}

plnamesuffix() {
register char *p;
   if(p = strrchr(plname, '-')) {
      *p = 0;
      if(!plname[0]) {
         askname();
         plnamesuffix();
      }
      if(strchr("TSFKCWtsfkcw", p[1])) {
         pl_character[0] = p[1];
         pl_character[1] = 0;
      }
   }
}

#file hack.vault.c
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1984. */

#include	"hack.h"
#ifdef QUEST
setgd(/* mtmp */) /* struct monst *mtmp; */ {}
gd_move() { return(2); }
gddead(mtmp) struct monst *mtmp; {}
invault(){}

#else


extern struct monst *makemon();
#define	VAULT	6
#define	FCSIZ	(ROWNO+COLNO)
struct fakecorr {
	xchar fx,fy,ftyp;
};

struct egd {
	int fcbeg, fcend;	/* fcend: first unused pos */
	xchar gdx, gdy;		/* goal of guard's walk */
	unsigned gddone:1;
	struct fakecorr fakecorr[FCSIZ];
};

struct permonst pm_guard =
	{ "guard", '@', 12, 12, -1, 4, 10, sizeof(struct egd) };

struct monst *guard;
int gdlevel;
#define	EGD	((struct egd *)(&(guard->mextra[0])))

restfakecorr(){
register int fcx,fcy,fcbeg;
register struct rm *crm;

	while((fcbeg = EGD->fcbeg) < EGD->fcend) {
		fcx = EGD->fakecorr[fcbeg].fx;
		fcy = EGD->fakecorr[fcbeg].fy;
		if((u.ux == fcx && u.uy == fcy) || cansee(fcx,fcy) ||
		   m_at(fcx,fcy))
			return;
		crm = &levl[fcx][fcy];
		crm->typ = EGD->fakecorr[fcbeg].ftyp;
		if(!crm->typ) crm->seen = 0;
		newsym(fcx,fcy);
		EGD->fcbeg++;
	}
	/* it seems he left the corridor - let the guard disappear */
	mondead(guard);
	guard = 0;
}

setgd(){
register struct monst *mtmp;
	for(mtmp = fmon; mtmp; mtmp = mtmp->nmon) if(mtmp->isgd){
		guard = mtmp;
		gdlevel = dlevel;
		return;
	}
	guard = 0;
}

invault(){
register int tmp = inroom(u.ux, u.uy);
	if(tmp < 0 || rooms[tmp].rtype != VAULT) {
		u.uinvault = 0;
		return;
	}
	if(++u.uinvault % 50 == 0 && (!guard || gdlevel != dlevel)) {
	char buf[BUFSZ];
	register int x,y,dx,dy,gx,gy;

		/* first find the goal for the guard */
		for(dy = 0; dy < ROWNO; dy++)
		  for(y = u.uy-dy; y <= u.uy+dy; y++) {
		    if(y > u.uy-dy) y = u.uy+dy;
		    if(y < 0 || y > ROWNO-1) continue;
		    for(x = u.ux; x < COLNO; x++)
			if(levl[x][y].typ == CORR) goto fnd;
		    for(x = u.ux-1; x > 0; x--)
			if(levl[x][y].typ == CORR) goto fnd;
		}
		impossible();
		tele();
		return;
	fnd:
		gx = x; gy = y;

		/* next find a good place for a door in the wall */
		x = u.ux; y = u.uy;
		while(levl[x][y].typ > DOOR) {
			dx = (gx > x) ? 1 : (gx < x) ? -1 : 0;
			dy = (gy > y) ? 1 : (gy < y) ? -1 : 0;
			if(abs(gx-x) >= abs(gy-y)) x += dx;
			else y += dy;
		}

		/* make something interesting happen */
		if(!(guard = makemon(&pm_guard,x,y))) return;
		guard->isgd = guard->mpeaceful = 1;
		EGD->gddone = 0;
		gdlevel = dlevel;
		if(!cansee(guard->mx, guard->my)) {
			mondead(guard);
			guard = 0;
			return;
		}
		EGD->gdx = gx;
		EGD->gdy = gy;
		EGD->fcbeg = 0;
		EGD->fakecorr[0].fx = x;
		EGD->fakecorr[0].fy = y;
		EGD->fakecorr[0].ftyp = levl[x][y].typ;
		levl[x][y].typ = DOOR;
		EGD->fcend = 1;

		pline("Suddenly one of the Vault's guards enters!");
		pmon(guard);
		pline("\"Hello stranger, who are you?\" - ");
		getlin(buf);
		clrlin();
		pline("\"I don't know you.\"");
		if(!u.ugold) pline("\"Please follow me.\"");
		else {
		    pline("\"Most likely all that gold was stolen from this vault.\"");
		    pline("\"Please drop your gold (say d$ ) and follow me.\"");
		}
	}
}

gd_move(){
register int x,y,dx,dy,gx,gy,nx,ny,tmp;
register struct fakecorr *fcp;
register struct rm *crm;
	if(!guard || gdlevel != dlevel){
		pline("Where is the guard?");
		impossible();
		return(2);	/* died */
	}
	if(u.ugold || dist(guard->mx,guard->my) > 2 || EGD->gddone){
		restfakecorr();
		return(0);	/* didnt move */
	}
	x = guard->mx;
	y = guard->my;
	/* look around (hor & vert only) for accessible places */
	for(nx = x-1; nx <= x+1; nx++) for(ny = y-1; ny <= y+1; ny++)
	    if(nx == x || ny == y) if(nx != x || ny != y)
	    if(isok(nx,ny))
	    if((tmp = (crm = &levl[nx][ny])->typ) >= SDOOR) {
		register int i;
		for(i = EGD->fcbeg; i < EGD->fcend; i++)
			if(EGD->fakecorr[i].fx == nx &&
			   EGD->fakecorr[i].fy == ny)
				goto nextnxy;
		if((i = inroom(nx,ny)) >= 0 && rooms[i].rtype == VAULT)
			goto nextnxy;
		/* seems we found a good place to leave him alone */
		EGD->gddone = 1;
		if(tmp >= DOOR) goto newpos;
		crm->typ = (tmp == SCORR) ? CORR : DOOR;
		goto proceed;
	nextnxy:	;
	}
	nx = x;
	ny = y;
	gx = EGD->gdx;
	gy = EGD->gdy;
	dx = (gx > x) ? 1 : (gx < x) ? -1 : 0;
	dy = (gy > y) ? 1 : (gy < y) ? -1 : 0;
	if(abs(gx-x) >= abs(gy-y)) nx += dx; else ny += dy;

	while((tmp = (crm = &levl[nx][ny])->typ) != 0) {
	/* in view of the above we must have  tmp < SDOOR */
	/* must be a wall here */
		if(isok(nx+nx-x,ny+ny-y) && levl[nx+nx-x][ny+ny-y].typ > DOOR){
			crm->typ = DOOR;
			goto proceed;
		}
		if(dy && nx != x) {
			nx = x; ny = y+dy; dx = 0;
			continue;
		}
		if(dx && ny != y) {
			ny = y; nx = x+dx; dy = 0;
			continue;
		}
		/* I don't like this, but ... */
		crm->typ = DOOR;
		goto proceed;
	}
	crm->typ = CORR;
proceed:
	fcp = &(EGD->fakecorr[EGD->fcend]);
	if(EGD->fcend++ == FCSIZ) panic("fakecorr overflow");
	fcp->fx = nx;
	fcp->fy = ny;
	fcp->ftyp = tmp;
newpos:
	if(EGD->gddone) nx = ny = 0;
	guard->mx = nx;
	guard->my = ny;
	pmon(guard);
	restfakecorr();
	return(1);
}

gddead(){
	guard = 0;
}


#endif QUEST
#file hack.version.c
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */

#include	"date.h"

doversion(){
	pline("AMIGA %s 1.0.1 - last edit %s.",
#ifdef QUEST
		"Quest"
#else
		"Hack"
#endif QUEST
	, datestring);
	return(0);
}
#file hack.whatis.c
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* hack.whatis.c version 1.0.1 - whatis asks for one char only. */

#include	<stdio.h>
#include "hack.h"

dowhatis()
{
	FILE *fp;
	char bufr[BUFSZ];
	register char *ep, q;
	extern char readchar();

	if(!(fp = fopen("data","r")))
		pline("Cannot open data file!");
	else {
		pline("Specify what? ");
		q = readchar();
		while(fgets(bufr,BUFSZ,fp))
			if(*bufr == q) {
				ep = index(bufr, '\n');
				if(ep) *ep = 0;
				else impossible();
				pline(bufr);
				if(ep[-1] == ';') morewhat(fp);
				goto fnd;
			}
		pline("I've never heard of such things.");
	fnd:
		(void) fclose(fp);
	}
}

morewhat(fp) FILE *fp; {
char bufr[BUFSZ];
register char *ep;
	pline("More info? ");
	if(readchar() != 'y') return;
	cls();
	while(fgets(bufr,BUFSZ,fp) && *bufr == '\t'){
		ep = index(bufr, '\n');
		if(!ep) break;
		*ep = 0;
		myputs(bufr+1);
	}
	more();
	docrt();
}
#file hack.wield.c
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* hack.wield.c version 1.0.1 - evaporate%s */

#include	"hack.h"

setuwep(obj) register struct obj *obj; {
	setworn(obj, W_WEP);
}

dowield()
{
	register struct obj *wep;
	register int res = 0;

	multi = 0;
	if(!(wep = getobj("#-)", "wield"))) /* nothing */;
	else if(uwep == wep)
		pline("You are already wielding that!");
	else if(uwep && uwep->cursed)
		pline("The %s welded to your hand!",
			aobjnam(uwep, "are"));
	else if((int) wep == 1) {
		if(uwep == 0){
			pline("You are already empty handed.");
		} else {
			setuwep((struct obj *) 0);
			res++;
			pline("You are empty handed.");
		}
	} else if(uarms && wep->otyp == TWO_HANDED_SWORD)
	pline("You cannot wield a two-handed sword and wear a shield.");
	else if(wep->owornmask & (W_ARMOR | W_RING))
		pline("You cannot wield that!");
	else {
		setuwep(wep);
		res++;
		if(uwep->cursed) pline("The %s itself to your hand!",
			aobjnam(uwep, "weld"));
		else prinv(uwep);
	}
	return(res);
}

corrode_weapon(){
	if(!uwep || uwep->olet != WEAPON_SYM) return;	/* %% */
	if(uwep->rustfree)
		pline("Your %s not affected.", aobjnam(uwep, "are"));
	else {
		pline("Your %s!", aobjnam(uwep, "corrode"));
		uwep->spe--;
	}
}

chwepon(otmp,amount)
register struct obj *otmp;
register int amount;
{
register char *color = (amount < 0) ? "black" : "green";
register char *time;
	if(!uwep || uwep->olet != WEAPON_SYM) {
		strange_feeling(otmp);
		return(0);
	}

	if(uwep->otyp == WORM_TOOTH && amount > 0) {
		uwep->otyp = CRYSKNIFE;
		pline("Your weapon seems sharper now.");
		uwep->cursed = 0;
		return(1);
	}

	if(uwep->otyp == CRYSKNIFE && amount < 0) {
		uwep->otyp = WORM_TOOTH;
		pline("Your weapon looks duller now.");
		return(1);
	}

	/* there is a (soft) upper limit to uwep->spe */
	if(amount > 0 && uwep->spe > 5 && rn2(3)) {
	    pline("Your %s violently green for a while and then evaporate%s.",
		aobjnam(uwep, "glow"), (uwep->quan == 1) ? "s" : "");
	    useup(uwep);
	    return(1);
	}
	if(!rn2(6)) amount *= 2;
	time = (amount*amount == 1) ? "moment" : "while";
	pline("Your %s %s for a %s.",
		aobjnam(uwep, "glow"), color, time);
	uwep->spe += amount;
	if(amount > 0) uwep->cursed = 0;
	return(1);
}
#file hack.worm.c
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1984. */

#include "hack.h"
#ifndef NOWORM
#include "def.wseg.h"

struct wseg *wsegs[32];	/* linked list, tail first */
struct wseg *wheads[32];
long wgrowtime[32];

getwn(mtmp) struct monst *mtmp; {
register int tmp;
	for(tmp=1; tmp<32; tmp++) if(!wsegs[tmp]) {
		mtmp->wormno = tmp;
		return(1);
	}
	return(0);	/* level infested with worms */
}

/* called to initialize a worm unless cut in half */
initworm(mtmp) struct monst *mtmp; {
register struct wseg *wtmp;
register int tmp = mtmp->wormno;
	if(!tmp) return;
	wheads[tmp] = wsegs[tmp] = wtmp = newseg();
	wgrowtime[tmp] = 0;
	wtmp->wx = mtmp->mx;
	wtmp->wy = mtmp->my;
/*	wtmp->wdispl = 0;*/
	wtmp->nseg = 0;
}

worm_move(mtmp) struct monst *mtmp; {
register struct wseg *wtmp, *whd;
register int tmp = mtmp->wormno;
	wtmp = newseg();
	wtmp->wx = mtmp->mx;
	wtmp->wy = mtmp->my;
	wtmp->nseg = 0;
/*	wtmp->wdispl = 0;*/
	(whd = wheads[tmp])->nseg = wtmp;
	wheads[tmp] = wtmp;
	if(cansee(whd->wx,whd->wy)){
		unpmon(mtmp);
		atl(whd->wx, whd->wy, '~');
		whd->wdispl = 1;
	} else	whd->wdispl = 0;
	if(wgrowtime[tmp] <= moves) {
		if(!wgrowtime[tmp]) wgrowtime[tmp] = moves + rnd(5);
		else wgrowtime[tmp] += 2+rnd(15);
		mtmp->orig_hp++;
		mtmp->mhp++;
		return;
	}
	whd = wsegs[tmp];
	wsegs[tmp] = whd->nseg;
	remseg(whd);
}

worm_nomove(mtmp) register struct monst *mtmp; {
register int tmp;
register struct wseg *wtmp;
	tmp = mtmp->wormno;
	wtmp = wsegs[tmp];
	if(wtmp == wheads[tmp]) return;
	if(wtmp == 0 || wtmp->nseg == 0) panic("worm_nomove?");
	wsegs[tmp] = wtmp->nseg;
	remseg(wtmp);
	mtmp->mhp--;	/* orig_hp not changed ! */
}

wormdead(mtmp) register struct monst *mtmp; {
register int tmp = mtmp->wormno;
register struct wseg *wtmp, *wtmp2;
	if(!tmp) return;
	mtmp->wormno = 0;
	for(wtmp = wsegs[tmp]; wtmp; wtmp = wtmp2){
		wtmp2 = wtmp->nseg;
		remseg(wtmp);
	}
	wsegs[tmp] = 0;
}

wormhit(mtmp) register struct monst *mtmp; {
register int tmp = mtmp->wormno;
register struct wseg *wtmp;
	if(!tmp) return;	/* worm without tail */
	for(wtmp = wsegs[tmp]; wtmp; wtmp = wtmp->nseg)
		(void) hitu(mtmp,1);
}

wormsee(tmp) register unsigned tmp; {
register struct wseg *wtmp = wsegs[tmp];
	if(!wtmp) panic("wormsee: wtmp==0");
	for(; wtmp->nseg; wtmp = wtmp->nseg)
		if(!cansee(wtmp->wx,wtmp->wy) && wtmp->wdispl){
			newsym(wtmp->wx, wtmp->wy);
			wtmp->wdispl = 0;
		}
}

pwseg(wtmp) register struct wseg *wtmp; {
	if(!wtmp->wdispl){
		atl(wtmp->wx, wtmp->wy, '~');
		wtmp->wdispl = 1;
	}
}

cutworm(mtmp,x,y,weptyp)
register struct monst *mtmp;
register xchar x,y;
register uchar weptyp;		/* uwep->otyp or 0 */
{
	register struct wseg *wtmp, *wtmp2;
	register struct monst *mtmp2;
	register int tmp,tmp2;
	if(mtmp->mx == x && mtmp->my == y) return;	/* hit headon */

	/* cutting goes best with axe or sword */
	tmp = rnd(20);
	if(weptyp == LONG_SWORD || weptyp == TWO_HANDED_SWORD ||
		weptyp == AXE) tmp += 5;
	if(tmp < 12) return;

	/* if tail then worm just loses a tail segment */
	tmp = mtmp->wormno;
	wtmp = wsegs[tmp];
	if(wtmp->wx == x && wtmp->wy == y){
		wsegs[tmp] = wtmp->nseg;
		remseg(wtmp);
		return;
	}

	/* cut the worm in two halves */
	mtmp2 = newmonst(0);
	*mtmp2 = *mtmp;
	mtmp2->mxlth = mtmp2->mnamelth = 0;

	/* sometimes the tail end dies */
	if(rn2(3) || !getwn(mtmp2)){
		monfree(mtmp2);
		tmp2 = 0;
	} else {
		tmp2 = mtmp2->wormno;
		wsegs[tmp2] = wsegs[tmp];
		wgrowtime[tmp2] = 0;
	}
	do {
		if(wtmp->nseg->wx == x && wtmp->nseg->wy == y){
			if(tmp2) wheads[tmp2] = wtmp;
			wsegs[tmp] = wtmp->nseg->nseg;
			remseg(wtmp->nseg);
			wtmp->nseg = 0;
			if(tmp2){
				pline("You cut the worm in half.");
				mtmp2->orig_hp = mtmp2->mhp =
					d(mtmp2->data->mlevel, 8);
				mtmp2->mx = wtmp->wx;
				mtmp2->my = wtmp->wy;
				mtmp2->nmon = fmon;
				fmon = mtmp2;
				pmon(mtmp2);
			} else {
				pline("You cut off part of the worm's tail.");
				remseg(wtmp);
			}
			mtmp->mhp /= 2;
			return;
		}
		wtmp2 = wtmp->nseg;
		if(!tmp2) remseg(wtmp);
		wtmp = wtmp2;
	} while(wtmp->nseg);
	panic("Cannot find worm segment");
}

remseg(wtmp) register struct wseg *wtmp; {
	if(wtmp->wdispl)
		newsym(wtmp->wx, wtmp->wy);
	free((char *) wtmp);
}
#endif NOWORM
#file hack.worn.c
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1984. */



#include "hack.h"



struct worn {

	long w_mask;

	struct obj **w_obj;

} worn[] = {

	{ W_ARM, &uarm },

	{ W_ARM2, &uarm2 },

	{ W_ARMH, &uarmh },

	{ W_ARMS, &uarms },

	{ W_ARMG, &uarmg },

	{ W_RINGL, &uleft },

	{ W_RINGR, &uright },

	{ W_WEP, &uwep },

	{ W_BALL, &uball },

	{ W_CHAIN, &uchain },

	{ 0, 0 }

};



setworn(obj, mask)

register struct obj *obj;

long mask;

{

	register struct worn *wp;

	register struct obj *oobj;



	for(wp = worn; wp->w_mask; wp++) if(wp->w_mask & mask) {

		oobj = *(wp->w_obj);

		if(oobj && !(oobj->owornmask & wp->w_mask)){

			pline("Setworn: mask = %d.", wp->w_mask);

			impossible();

		}

		if(oobj) oobj->owornmask &= ~wp->w_mask;

		if(obj && oobj && wp->w_mask == W_ARM){

			if(uarm2) {

				pline("Setworn: uarm2 set?");

				impossible();

			} else

				setworn(uarm, W_ARM2);

		}

		*(wp->w_obj) = obj;

		if(obj) obj->owornmask |= wp->w_mask;

	}

	if(uarm2 && !uarm) {

		uarm = uarm2;

		uarm2 = 0;

		uarm->owornmask ^= (W_ARM | W_ARM2);

	}

}



/* called e.g. when obj is destroyed */

setnotworn(obj) register struct obj *obj; {

	register struct worn *wp;



	for(wp = worn; wp->w_mask; wp++)

		if(obj == *(wp->w_obj)) {

			*(wp->w_obj) = 0;

			obj->owornmask &= ~wp->w_mask;

		}

	if(uarm2 && !uarm) {

		uarm = uarm2;

		uarm2 = 0;

		uarm->owornmask ^= (W_ARM | W_ARM2);

	}

}
