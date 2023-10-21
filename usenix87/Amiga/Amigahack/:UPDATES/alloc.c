
#file alloc.c
#ifdef LINT

/*
   a ridiculous definition, suppressing
   "possible pointer alignment problem" for (long *) malloc()
   "enlarg defined but never used"
   "ftell defined (in <stdio.h>) but never used"
   from lint
*/
#include <stdio.h>
long *
alloc(n) unsigned n; {
long dummy = ftell(stderr);
   if(n) dummy = 0;   /* make sure arg is used */
   return(&dummy);
}

#else

extern char *malloc();
/* extern char *realloc(); */

long *
alloc(lth)
register unsigned lth;
{
   register char *ptr;

   if(!(ptr = malloc(lth)))
      panic("Cannot get %d bytes", lth);
   return((long *) ptr);
}

long *
enlarge(ptr,lth)
register char *ptr;
register unsigned lth;
{
   register char *nptr;

   nptr = alloc(lth);
   movmem(ptr,nptr,lth);
   free(ptr);
   return((long *) nptr);
}

#endif LINT
#file hack.apply.c
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* hack.apply.c version 1.0.1 - "The flash awakens %s" (riv05!a3) */

#include	"hack.h"
extern struct monst *bchit();
extern struct obj *addinv();
extern char pl_character[];

doapply() {
register struct obj *obj;
	obj = getobj("(", "use or apply");
	if(!obj) return(0);
	switch(obj->otyp){
	case EXPENSIVE_CAMERA:
		use_camera(obj); break;
	case ICE_BOX:
		use_ice_box(obj); break;
	case MAGIC_WHISTLE:
		if(pl_character[0] == 'W' || u.ulevel > 9) {
			use_magic_whistle(obj);
			break;
		}
		/* fall into next case */
	case WHISTLE:
		use_whistle(obj); break;
	default:
		pline("Sorry, I don't know how to use that.");
		return(0);
	}
	return(1);
}

/* ARGSUSED */
use_camera(obj) /* register */ struct obj *obj; {
register struct monst *mtmp;
	if(!getdir()){
		flags.move = multi = 0;
		return;
	}
	if(mtmp = bchit(u.dx, u.dy, COLNO, '!')) {
		if(mtmp->msleep){
			mtmp->msleep = 0;
			pline("The flash awakens %s.", monnam(mtmp));
		} else
		if(mtmp->data->mlet != 'y')
		if(mtmp->mcansee || mtmp->mblinded){
			register int tmp = dist(mtmp->mx,mtmp->my);
			register int tmp2;
			/* if(cansee(mtmp->mx,mtmp->my)) */
			  pline("%s is blinded by the flash!",Monnam(mtmp));
			setmangry(mtmp);
			if(tmp < 9 && !mtmp->isshk && !rn2(4))
				mtmp->mflee = 1;
			if(tmp < 3) mtmp->mcansee  = mtmp->mblinded = 0;
			else {
				tmp2 = mtmp->mblinded;
				tmp2 += rnd(1 + 50/tmp);
				if(tmp2 > 127) tmp2 = 127;
				mtmp->mblinded = tmp2;
				mtmp->mcansee = 0;
			}
		}
	}
}

struct obj *current_ice_box;	/* a local variable of use_ice_box, to be
				used by its local procedures in/ck_ice_box */
in_ice_box(obj) register struct obj *obj; {
	if(obj == current_ice_box ||
		(Punished && (obj == uball || obj == uchain))){
		pline("You must be kidding.");
		return(0);
	}
	if(obj->owornmask & (W_ARMOR | W_RING)) {
		pline("You cannot refrigerate something you are wearing.");
		return(0);
	}
	if(obj->owt + current_ice_box->owt > 70) {
		pline("It won't fit.");
		return(1);	/* be careful! */
	}
	if(obj == uwep) {
		if(uwep->cursed) {
			pline("Your weapon is welded to your hand!");
			return(0);
		}
		setuwep((struct obj *) 0);
	}
	current_ice_box->owt += obj->owt;
	freeinv(obj);
	obj->o_cnt_id = current_ice_box->o_id;
	obj->nobj = fcobj;
	fcobj = obj;
	obj->age = moves - obj->age;	/* actual age */
	return(1);
}

ck_ice_box(obj) register struct obj *obj; {
	return(obj->o_cnt_id == current_ice_box->o_id);
}

out_ice_box(obj) register struct obj *obj; {
register struct obj *otmp;
	if(obj == fcobj) fcobj = fcobj->nobj;
	else {
		for(otmp = fcobj; otmp->nobj != obj; otmp = otmp->nobj)
			if(!otmp->nobj) panic("out_ice_box");
		otmp->nobj = obj->nobj;
	}
	current_ice_box->owt -= obj->owt;
	obj->age = moves - obj->age;	/* simulated point of time */
	(void) addinv(obj);
}

use_ice_box(obj) register struct obj *obj; {
register int cnt = 0;
register struct obj *otmp;
	current_ice_box = obj;	/* for use by in/out_ice_box */
	for(otmp = fcobj; otmp; otmp = otmp->nobj)
		if(otmp->o_cnt_id == obj->o_id)
			cnt++;
	if(!cnt) pline("Your ice-box is empty.");
	else {
	    pline("Do you want to take something out of the ice-box? [yn] ");
	    if(readchar() == 'y')
		if(askchain(fcobj, (char *) 0, 0, out_ice_box, ck_ice_box, 0))
		    return;
		pline("That was all. Do you wish to put something in? [yn] ");
		if(readchar() != 'y') return;
	}
	/* call getobj: 0: allow cnt; #: allow all types; %: expect food */
	otmp = getobj("0#%", "put in");
	if(!otmp || !in_ice_box(otmp))
		flags.move = multi = 0;
}

struct monst *
bchit(ddx,ddy,range,sym) register int ddx,ddy,range; char sym; {
	register struct monst *mtmp = (struct monst *) 0;
	register int bchx = u.ux, bchy = u.uy;

	if(sym) Tmp_at(-1, sym);	/* open call */
	while(range--) {
		bchx += ddx;
		bchy += ddy;
		if(mtmp = m_at(bchx,bchy))
			break;
		if(levl[bchx][bchy].typ < CORR) {
			bchx -= ddx;
			bchy -= ddy;
			break;
		}
		if(sym) Tmp_at(bchx, bchy);
	}
	if(sym) Tmp_at(-1, -1);
	return(mtmp);
}

#include	"def.edog.h"
/* ARGSUSED */
use_whistle(obj) struct obj *obj; {
register struct monst *mtmp = fmon;
	pline("You produce a high whistling sound.");
	while(mtmp) {
		if(dist(mtmp->mx,mtmp->my) < u.ulevel*10) {
			if(mtmp->msleep)
				mtmp->msleep = 0;
			if(mtmp->mtame)
				EDOG(mtmp)->whistletime = moves;
		}
		mtmp = mtmp->nmon;
	}
}

/* ARGSUSED */
use_magic_whistle(obj) struct obj *obj; {
register struct monst *mtmp = fmon;
	pline("You produce a strange whistling sound.");
	while(mtmp) {
		if(mtmp->mtame) mnexto(mtmp);
		mtmp = mtmp->nmon;
	}
}
#file hack.bones.c
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1984. */

#include "hack.h"
extern char plname[PL_NSIZ];
extern struct monst *makemon();

char bones[] = "bones_xx";

/* save bones and possessions of a deceased adventurer */
savebones(){
register int fd;
register struct obj *otmp;
register struct gen *gtmp;
register struct monst *mtmp;
	if(!rn2(1 + dlevel/2)) return;	/* not so many ghosts on low levels */
	bones[6] = '0' + (dlevel/10);
	bones[7] = '0' + (dlevel%10);
	if((fd = open(bones,0)) >= 0){
		(void) close(fd);
		return;
	}
	/* drop everything; the corpse's possessions are usually cursed */
	otmp = invent;
	while(otmp){
		otmp->ox = u.ux;
		otmp->oy = u.uy;
		otmp->known = 0;
		otmp->age = 0;		/* very long ago */
		otmp->owornmask = 0;
		if(rn2(5)) otmp->cursed = 1;
		if(!otmp->nobj){
			otmp->nobj = fobj;
			fobj = invent;
			invent = 0;	/* superfluous */
			break;
		}
		otmp = otmp->nobj;
	}
	if(!(mtmp = makemon(PM_GHOST, u.ux, u.uy))) return;
	mtmp->mx = u.ux;
	mtmp->my = u.uy;
	mtmp->msleep = 1;
	(void) strcpy((char *) mtmp->mextra, plname);
	mkgold(somegold() + d(dlevel,30), u.ux, u.uy);
	u.ux = FAR;		/* avoid animals standing next to us */
	keepdogs();		/* all tame animals become wild again */
	for(mtmp = fmon; mtmp; mtmp = mtmp->nmon){
		mtmp->mlstmv = 0;
		if(mtmp->mdispl) unpmon(mtmp);
	}
	for(gtmp = ftrap; gtmp; gtmp = gtmp->ngen)
		gtmp->gflag &= ~SEEN;
	for(otmp = fobj; otmp; otmp = otmp->nobj)
		otmp->onamelth = 0;
	if((fd = creat(bones, FMASK)) < 0) return;
	savelev(fd);
	(void) close(fd);
}

getbones(){
register int fd,x,y,ok;
	if(rn2(3)) return(0);	/* only once in three times do we find bones */
	bones[6] = '0' + dlevel/10;
	bones[7] = '0' + dlevel%10;
	if((fd = open(bones, 0)) < 0) return(0);
	if((ok = uptodate(fd)) != 0){
		(void) getlev(fd);
		(void) close(fd);
		for(x = 0; x < COLNO; x++) for(y = 0; y < ROWNO; y++)
			levl[x][y].seen = levl[x][y].new = 0;
	}
	if(unlink(bones) < 0){
		pline("Cannot unlink %s", bones);
		return(0);
	}
	return(ok);
}
#file hack.c
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* hack.c version 1.0.1 - many small and unimportant changes */
#include "hack.h"
#include <stdio.h>


extern char news0();
extern char *nomovemsg;
extern char *exclam();
extern struct obj *addinv();
extern boolean hmon();



/* called on movement:
	1. when throwing ball+chain far away
	2. when teleporting
	3. when walking out of a lit room
 */
unsee() {
	register int x,y;
	register struct rm *lev;

/*
	if(u.udispl){
		u.udispl = 0;
		newsym(u.udisx, u.udisy);
	}
*/
#ifndef QUEST
	if(seehx){
		seehx = 0;
	} else
#endif QUEST
	for(x = u.ux-1; x < u.ux+2; x++)
	  for(y = u.uy-1; y < u.uy+2; y++) {
		lev = &levl[x][y];
		if(!lev->lit && lev->scrsym == '.') {
			lev->scrsym =' ';
			lev->new = 1;
			on_scr(x,y);
		}
	}
}

/* called:
	in hack.eat.c: seeoff(0) - blind after eating rotten food
	in hack.mon.c: seeoff(0) - blinded by a yellow light
	in hack.mon.c: seeoff(1) - swallowed
	in hack.do.c:  seeoff(0) - blind after drinking potion
	in hack.do.c:  seeoff(1) - go up or down the stairs
	in hack.trap.c:seeoff(1) - fall through trapdoor
 */
seeoff(mode)	/* 1 to redo @, 0 to leave them */
{	/* 1 means misc movement, 0 means blindness */
	register int x,y;
	register struct rm *lev;

	if(u.udispl && mode){
		u.udispl = 0;
		levl[u.udisx][u.udisy].scrsym = news0(u.udisx,u.udisy);
	}
#ifndef QUEST
	if(seehx) {
		seehx = 0;
	} else
#endif QUEST
	if(!mode) {
		for(x = u.ux-1; x < u.ux+2; x++)
			for(y = u.uy-1; y < u.uy+2; y++) {
				lev = &levl[x][y];
				if(!lev->lit && lev->scrsym == '.')
					lev->seen = 0;
			}
	}
}

/* 'rogue'-like direction commands */
char sdir[] = "hykulnjb";
schar xdir[8] = { -1,-1,0,1,1,1,0,-1 };
schar ydir[8] = { 0,-1,-1,-1,0,1,1,1 };

movecm(cmd)
register char *cmd;
{
register char *dp;
		if(!(dp = index(sdir, *cmd))) return(0);
		u.dx = xdir[dp-sdir];
		u.dy = ydir[dp-sdir];
		return(1);
}
getdir()
{
	char buf[2];
	register int x;

	pline("What direction?");
	buf[0] = readchar();
	buf[1] = 0;
	x = movecm(buf);
	if(x && Confusion) confdir();
	return(x);
}

confdir()
{
	register int x = rn2(8);
	u.dx = xdir[x];
	u.dy = ydir[x];
}

#ifdef QUEST
finddir(){
register int i, ui = u.di;
	for(i = 0; i <= 8; i++){
		if(flags.run & 1) ui++; else ui += 7;
		ui %= 8;
		if(i == 8){
			pline("Not near a wall.");
			flags.move = multi = 0;
			return(0);
		}
		if(!isroom(u.ux+xdir[ui], u.uy+ydir[ui]))
			break;
	}
	for(i = 0; i <= 8; i++){
		if(flags.run & 1) ui += 7; else ui++;
		ui %= 8;
		if(i == 8){
			pline("Not near a room.");
			flags.move = multi = 0;
			return(0);
		}
		if(isroom(u.ux+xdir[ui], u.uy+ydir[ui]))
			break;
	}
	u.di = ui;
	u.dx = xdir[ui];
	u.dy = ydir[ui];
}

isroom(x,y)  register int x,y; {
	return(isok(x,y) && (levl[x][y].typ == ROOM ||
				(levl[x][y].typ >= LDOOR && flags.run >= 6)));
}
#endif QUEST

isok(x,y) register int x,y; {
	return(x >= 0 && x <= COLNO-1 && y >= 0 && y <= ROWNO-1);
}

domove()
{
	xchar oldx,oldy;
	register struct monst *mtmp;
	register struct rm *tmpr,*ust;
	struct gen *trap;
	register struct obj *otmp;

	if(!u.uswallow)
		wipe_engr_at(u.ux, u.uy, rnd(5));
	if(inv_weight() > 0){
		pline("You collapse under your load.");
		nomul(0);
		return;
	}
	if(u.uswallow) {
		u.dx = u.dy = 0;
		u.ux = u.ustuck->mx;
		u.uy = u.ustuck->my;
	} else {
		if(Confusion) {
			do {
				confdir();
			} while(!isok(u.ux+u.dx, u.uy+u.dy) ||
				levl[u.ux+u.dx][u.uy+u.dy].typ < DOOR);
		}
		if(!isok(u.ux+u.dx, u.uy+u.dy)){
			nomul(0);
			return;
		}
	}

	ust = &levl[u.ux][u.uy];
	oldx = u.ux;
	oldy = u.uy;
	if(!u.uswallow)
	    if(trap = g_at(u.ux+u.dx,u.uy+u.dy,ftrap)) {
		if(trap->gflag & SEEN) nomul(0);
	}
	if(u.ustuck && !u.uswallow && (u.ux+u.dx != u.ustuck->mx ||
		u.uy+u.dy != u.ustuck->my)) {
		if(dist(u.ustuck->mx, u.ustuck->my) > 2){
			/* perhaps it fled (or was teleported or ... ) */
			u.ustuck = 0;
		} else {
			if(Blind) pline("You cannot escape from it!");
			else pline("You cannot escape from %s!.",
				monnam(u.ustuck));
			nomul(0);
			return;
		}
	}
	if(u.uswallow || (mtmp = m_at(u.ux+u.dx,u.uy+u.dy))) {
	/* attack monster */
		schar tmp;
		boolean malive = TRUE;
		register struct permonst *mdat;

		nomul(0);
		gethungry();
		if(multi < 0) return;	/* we just fainted */
		if(u.uswallow) mtmp = u.ustuck;
		mdat = mtmp->data;
		if(mdat->mlet == 'L' && !mtmp->mfroz && !mtmp->msleep &&
		   !mtmp->mconf && mtmp->mcansee && !rn2(7) &&
		   (m_move(mtmp, 0) == 2 /* he died */ || /* he moved: */
			mtmp->mx != u.ux+u.dx || mtmp->my != u.uy+u.dy))
			goto nomon;
		if(mtmp->mimic){
			if(!u.ustuck && !mtmp->mflee) u.ustuck = mtmp;
			switch(levl[u.ux+u.dx][u.uy+u.dy].scrsym){
			case '+':
				pline("The door actually was a Mimic.");
				break;
			case '$':
				pline("The chest was a Mimic!");
				break;
			default:
				pline("Wait! That's a Mimic!");
			}
			wakeup(mtmp);	/* clears mtmp->mimic */
			return;
		}
		wakeup(mtmp);	/* clears mtmp->mimic */
		if(mtmp->mhide && mtmp->mundetected){
			register struct obj *obj;
			mtmp->mundetected = 0;
			if((obj = o_at(mtmp->mx,mtmp->my)) && !Blind)
				pline("Wait! There's a %s hiding under %s!",
					mdat->mname, doname(obj));
			return;
		}
		tmp = u.uluck + u.ulevel + mdat->ac + abon();
		if(uwep) {
			if(uwep->olet == WEAPON_SYM)
				tmp += uwep->spe;
			if(uwep->otyp == TWO_HANDED_SWORD) tmp -= 1;
			else if(uwep->otyp == DAGGER) tmp += 2;
			else if(uwep->otyp == CRYSKNIFE) tmp += 3;
			else if(uwep->otyp == SPEAR &&
				index("XDne", mdat->mlet)) tmp += 2;
		}
		if(mtmp->msleep) {
			mtmp->msleep = 0;
			tmp += 2;
		}
		if(mtmp->mfroz) {
			tmp += 4;
			if(!rn2(10)) mtmp->mfroz = 0;
		}
		if(mtmp->mflee) tmp += 2;
		if(u.utrap) tmp -= 3;
		if(tmp <= rnd(20) && !u.uswallow){
			if(Blind) pline("You miss it.");
			else pline("You miss %s.",monnam(mtmp));
		} else {
			/* we hit the monster; be careful: it might die! */

			if((malive = hmon(mtmp,uwep,0)) == TRUE) {
				/* monster still alive */
				if(!rn2(25) && mtmp->mhp < mtmp->orig_hp/2) {
					mtmp->mflee = 1;
					if(u.ustuck == mtmp && !u.uswallow)
						u.ustuck = 0;
				}
#ifndef NOWORM
				if(mtmp->wormno)
					cutworm(mtmp, u.ux+u.dx, u.uy+u.dy,
						uwep ? uwep->otyp : 0);
#endif NOWORM
			}
			if(mdat->mlet == 'a') {
				if(rn2(2)) {
				pline("You are splashed by the blob's acid!");
					losehp_m(rnd(6), mtmp);
				}
				if(!rn2(6)) corrode_weapon();
				else if(!rn2(60)) corrode_armor();
			}
		}
		if(malive && !Blind && mdat->mlet == 'E' && rn2(3)) {
		    if(mtmp->mcansee) {
		      pline("You are frozen by the floating eye's gaze!");
		      nomul((u.ulevel > 6 || rn2(4)) ? rn1(20,-21) : -200);
		    } else {
		      pline("The blinded floating eye cannot defend itself.");
		      if(!rn2(500)) u.uluck--;
		    }
		}
		return;
	}
nomon:
	/* not attacking an animal, so we try to move */
	if(u.utrap) {
		if(u.utraptype == TT_PIT) {
			pline("You are still in a pit.");
			u.utrap--;
		} else {
			pline("You are caught in a beartrap.");
			if((u.dx && u.dy) || !rn2(5)) u.utrap--;
		}
		return;
	}
	tmpr = &levl[u.ux+u.dx][u.uy+u.dy];
	if((tmpr->typ < DOOR) ||
	   (u.dx && u.dy && (tmpr->typ == DOOR || ust->typ == DOOR))){
		flags.move = 0;
		nomul(0);
		return;
	}
	while(otmp = sobj_at(ENORMOUS_ROCK, u.ux+u.dx, u.uy+u.dy)) {
		register xchar rx = u.ux+2*u.dx, ry = u.uy+2*u.dy;
		register struct gen *gtmp;
		nomul(0);
		if(isok(rx,ry) && (levl[rx][ry].typ > DOOR ||
		    (levl[rx][ry].typ == DOOR && (!u.dx || !u.dy))) &&
		    !sobj_at(ENORMOUS_ROCK, rx, ry)) {
			if(m_at(rx,ry)) {
			    pline("You hear a monster behind the rock.");
			    pline("Perhaps that's why you cannot move it.");
			    return;
			}
			if(gtmp = g_at(rx,ry,ftrap))
#include	"def.trap.h"
			    switch(gtmp->gflag & TRAPTYPE) {
			    case PIT:
				pline("You push the rock into a pit!");
				deltrap(gtmp);
				delobj(otmp);
				pline("It completely fills the pit!");
				continue;
			    case TELEP_TRAP:
				pline("You push the rock and suddenly it disappears!");
				delobj(otmp);
				continue;
			    }
			otmp->ox = rx;
			otmp->oy = ry;
			/* pobj(otmp); */
			if(cansee(rx,ry)) atl(rx,ry,otmp->olet);
			if(Invis) newsym(u.ux+u.dx, u.uy+u.dy);

			{ static long lastmovetime;
			/* note: this var contains garbage initially and
			   after a restore */
			if(moves > lastmovetime+2 || moves < lastmovetime)
			pline("With great effort you move the enormous rock.");
			lastmovetime = moves;
			}
		} else {
		    pline("You try to move the enormous rock, but in vain.");
		    return;
		}
	    }
	if(u.dx && u.dy && levl[u.ux][u.uy+u.dy].typ < DOOR &&
		levl[u.ux+u.dx][u.uy].typ < DOOR &&
		invent && inv_weight()+40 > 0) {
		pline("You are carrying too much to get through.");
		nomul(0);
		return;
	}
	if(Punished &&
	   DIST(u.ux+u.dx, u.uy+u.dy, uchain->ox, uchain->oy) > 2){
		if(carried(uball)) {
			movobj(uchain, u.ux, u.uy);
			goto nodrag;
		}

		if(DIST(u.ux+u.dx, u.uy+u.dy, uball->ox, uball->oy) < 3){
			/* leave ball, move chain under/over ball */
			movobj(uchain, uball->ox, uball->oy);
			goto nodrag;
		}

		if(inv_weight() + (int) uball->owt/2 > 0) {
			pline("You cannot %sdrag the heavy iron ball.",
			invent ? "carry all that and also " : "");
			nomul(0);
			return;
		}

		movobj(uball, uchain->ox, uchain->oy);
		unpobj(uball);		/* BAH %% */
		uchain->ox = u.ux;
		uchain->oy = u.uy;
		nomul(-2);
		nomovemsg = "";
	nodrag:	;
	}
	u.ux += u.dx;
	u.uy += u.dy;
	if(flags.run) {
		if(tmpr->typ == DOOR ||
		(xupstair == u.ux && yupstair == u.uy) ||
		(xdnstair == u.ux && ydnstair == u.uy))
			nomul(0);
	}
/*
	if(u.udispl) {
		u.udispl = 0;
		newsym(oldx,oldy);
	}
*/
	if(!Blind) {
#ifdef QUEST
		setsee();
#else
		if(ust->lit) {
			if(tmpr->lit) {
				if(tmpr->typ == DOOR) prl1(u.ux+u.dx,u.uy+u.dy);
				else if(ust->typ == DOOR) nose1(oldx-u.dx,oldy-u.dy);
			} else {
				unsee();
				prl1(u.ux+u.dx,u.uy+u.dy);
			}
		} else {
			if(tmpr->lit) setsee();
			else {
				prl1(u.ux+u.dx,u.uy+u.dy);
				if(tmpr->typ == DOOR) {
					if(u.dy) {
						prl(u.ux-1,u.uy);
						prl(u.ux+1,u.uy);
					} else {
						prl(u.ux,u.uy-1);
						prl(u.ux,u.uy+1);
					}
				}
			}
			nose1(oldx-u.dx,oldy-u.dy);
		}
#endif QUEST
	} else {
		pru();
	}
	if(!flags.nopick) pickup();
	if(trap) dotrap(trap);		/* fall into pit, arrow trap, etc. */
	(void) inshop();
	if(!Blind) read_engr_at(u.ux,u.uy);
}

movobj(obj, ox, oy)
register struct obj *obj;
register int ox, oy;
{
	/* Some dirty programming to get display right */
	freeobj(obj);
	unpobj(obj);
	obj->nobj = fobj;
	fobj = obj;
	obj->ox = ox;
	obj->oy = oy;
}

dopickup(){
	if(!g_at(u.ux,u.uy,fgold) && !o_at(u.ux,u.uy)) {
		pline("There is nothing here to pick up.");
		return(0);
	}
	if(Levitation) {
		pline("You cannot reach the floor.");
		return(1);
	}
	pickup();
	return(1);
}

pickup(){
register struct gen *gold;
register struct obj *obj, *obj2;
register int wt;
	if(Levitation) return;
	while(gold = g_at(u.ux,u.uy,fgold)) {
		pline("%u gold piece%s.", gold->gflag, plur(gold->gflag));
		u.ugold += gold->gflag;
		flags.botl = 1;
		freegold(gold);
		if(flags.run) nomul(0);
		if(Invis) newsym(u.ux,u.uy);
	}
	for(obj = fobj; obj; obj = obj2) {
	    obj2 = obj->nobj;	/* perhaps obj will be picked up */
	    if(obj->ox == u.ux && obj->oy == u.uy) {
		if(flags.run) nomul(0);

#define	DEAD_c	CORPSE+('c'-'a'+'Z'-'@'+1)
		if(obj->otyp == DEAD_COCKATRICE && !uarmg){
		    pline("Touching the dead cockatrice is a fatal mistake.");
		    pline("You turn to stone.");
		    killer = "cockatrice cadaver";
		    done("died");
		}

		if(obj->otyp == SCR_SCARE_MONSTER){
		  if(!obj->spe) obj->spe = 1;
		  else {
		    /* Note: perhaps the 1st pickup failed: you cannot
			carry anymore, and so we never dropped it -
			let's assume that treading on it twice also
			destroys the scroll */
		    pline("The scroll turns to dust as you pick it up.");
		    delobj(obj);
		    continue;
		  }
		}

		/* do not pick up uchain */
		if(Punished && obj == uchain)
			continue;

		wt = inv_weight() + obj->owt;
		if(wt > 0) {
			if(obj->quan > 1) {
				/* see how many we can lift */
				extern struct obj *splitobj();
				int savequan = obj->quan;
				int iw = inv_weight();
				int qq;
				for(qq = 1; qq < savequan; qq++){
					obj->quan = qq;
					if(iw + weight(obj) > 0)
						break;
				}
				obj->quan = savequan;
				qq--;
				/* we can carry qq of them */
				if(!qq) goto too_heavy;
			pline("You can only carry %s of the %s lying here.",
					(qq == 1) ? "one" : "some",
					doname(obj));
				(void) splitobj(obj, qq);
				/* note: obj2 is set already, so we'll never
				 * encounter the other half; if it should be
				 * otherwise then write
				 *	obj2 = splitobj(obj,qq);
				 */
				goto lift_some;
			}
		too_heavy:
			pline("There %s %s here, but %s.",
				(obj->quan == 1) ? "is" : "are",
				doname(obj),
				!invent ? "it is too heavy for you to lift"
					: "you cannot carry anymore");
			break;
		}
	lift_some:
		if(inv_cnt() >= 52) {
		    pline("Your knapsack cannot accomodate anymore items.");
		    break;
		}
		if(wt > -5) pline("You have a little trouble lifting");
		freeobj(obj);
		if(Invis) newsym(u.ux,u.uy);
		addtobill(obj);       /* sets obj->unpaid if necessary */
		{ int pickquan = obj->quan;
		  int mergquan;
		if(!Blind) obj->dknown = 1;	/* this is done by prinv(),
				 but addinv() needs it already for merging */
		obj = addinv(obj);    /* might merge it with other objects */
		  mergquan = obj->quan;
		  obj->quan = pickquan;	/* to fool prinv() */
		prinv(obj);
		  obj->quan = mergquan;
		}
	    }
	}
}

/* stop running if we see something interesting */
/* turn around a corner if that is the only way we can proceed */
/* do not turn left or right twice */
lookaround(){
register int x,y,i,x0,y0,m0,i0 = 9;
register int corrct = 0, noturn = 0;
register struct monst *mtmp;
#ifdef lint
	/* suppress "used before set" message */
	x0 = y0 = 0;
#endif lint
	if(Blind || flags.run == 0) return;
	if(flags.run == 1 && levl[u.ux][u.uy].typ >= ROOM) return;
#ifdef QUEST
	if(u.ux0 == u.ux+u.dx && u.uy0 == u.uy+u.dy) goto stop;
#endif QUEST
	for(x = u.ux-1; x <= u.ux+1; x++) for(y = u.uy-1; y <= u.uy+1; y++){
		if(x == u.ux && y == u.uy) continue;
		if(!levl[x][y].typ) continue;
		if((mtmp = m_at(x,y)) && !mtmp->mimic &&
		    (!mtmp->minvis || See_invisible)){
			if(!mtmp->mtame || (x == u.ux+u.dx && y == u.uy+u.dy))
				goto stop;
		} else mtmp = 0; /* invisible M cannot influence us */
		if(x == u.ux-u.dx && y == u.uy-u.dy) continue;
		switch(levl[x][y].scrsym){
		case '|':
		case '-':
		case '.':
		case ' ':
			break;
		case '+':
			if(x != u.ux && y != u.uy) break;
			if(flags.run != 1) goto stop;
			/* fall into next case */
		case CORR_SYM:
		corr:
			if(flags.run == 1 || flags.run == 3) {
				i = DIST(x,y,u.ux+u.dx,u.uy+u.dy);
				if(i > 2) break;
				if(corrct == 1 && DIST(x,y,x0,y0) != 1)
					noturn = 1;
				if(i < i0) {
					i0 = i;
					x0 = x;
					y0 = y;
					m0 = mtmp ? 1 : 0;
				}
			}
			corrct++;
			break;
		case '^':
			if(flags.run == 1) goto corr;	/* if you must */
			if(x == u.ux+u.dx && y == u.uy+u.dx) goto stop;
			break;
		default:	/* e.g. objects or trap or stairs */
			if(flags.run == 1) goto corr;
			if(mtmp) break;		/* d */
		stop:
			nomul(0);
			return;
		}
	}
#ifdef QUEST
	if(corrct > 0 && (flags.run == 4 || flags.run == 5)) goto stop;
#endif QUEST
	if(corrct > 1 && flags.run == 2) goto stop;
	if((flags.run == 1 || flags.run == 3) && !noturn && !m0 && i0 &&
		(corrct == 1 || (corrct == 2 && i0 == 1))) {
		/* make sure that we do not turn too far */
		if(i0 == 2) {
		    if(u.dx == y0-u.uy && u.dy == u.ux-x0)
			i = 2;		/* straight turn right */
		    else
			i = -2;		/* straight turn left */
		} else if(u.dx && u.dy) {
		    if((u.dx == u.dy && y0 == u.uy) ||
			(u.dx != u.dy && y0 != u.uy))
			i = -1;		/* half turn left */
		    else
			i = 1;		/* half turn right */
		} else {
		    if((x0-u.ux == y0-u.uy && !u.dy) ||
			(x0-u.ux != y0-u.uy && u.dy))
			i = 1;		/* half turn right */
		    else
			i = -1;		/* half turn left */
		}
		i += u.last_str_turn;
		if(i <= 2 && i >= -2) {
			u.last_str_turn = i;
			u.dx = x0-u.ux, u.dy = y0-u.uy;
		}
	}
}

#ifdef QUEST
cansee(x,y) xchar x,y; {
register int dx,dy,adx,ady,sdx,sdy,dmax,d;
	if(Blind) return(0);
	if(!isok(x,y)) return(0);
	d = dist(x,y);
	if(d < 3) return(1);
	if(d > u.uhorizon*u.uhorizon) return(0);
	if(!levl[x][y].lit)
		return(0);
	dx = x - u.ux;	adx = abs(dx);	sdx = sgn(dx);
	dy = y - u.uy;  ady = abs(dy);	sdy = sgn(dy);
	if(dx == 0 || dy == 0 || adx == ady){
		dmax = (dx == 0) ? ady : adx;
		for(d = 1; d <= dmax; d++)
			if(!rroom(sdx*d,sdy*d))
				return(0);
		return(1);
	} else if(ady > adx){
		for(d = 1; d <= ady; d++){
			if(!rroom(sdx*( (d*adx)/ady ), sdy*d) ||
			   !rroom(sdx*( (d*adx-1)/ady+1 ), sdy*d))
				return(0);
		}
		return(1);
	} else {
		for(d = 1; d <= adx; d++){
			if(!rroom(sdx*d, sdy*( (d*ady)/adx )) ||
			   !rroom(sdx*d, sdy*( (d*ady-1)/adx+1 )))
				return(0);
		}
		return(1);
	}
}

rroom(x,y) register int x,y; {
	return(levl[u.ux+x][u.uy+y].typ >= ROOM);
}

#else

cansee(x,y) xchar x,y; {
	if(Blind || u.uswallow) return(0);
	if(dist(x,y) < 3) return(1);
	if(levl[x][y].lit && seelx <= x && x <= seehx && seely <= y &&
		y <= seehy) return(1);
	return(0);
}
#endif QUEST

sgn(a) register int a; {
	return((a> 0) ? 1 : (a == 0) ? 0 : -1);
}

pow(num) /* returns 2 to the num */
register unsigned num;
{
	return(1 << num);
}

#ifdef QUEST
setsee()
{
	register int x,y;

	if(Blind) {
		pru();
		return;
	}
	for(y = u.uy-u.uhorizon; y <= u.uy+u.uhorizon; y++)
		for(x = u.ux-u.uhorizon; x <= u.ux+u.uhorizon; x++) {
			if(cansee(x,y))
				prl(x,y);
	}
}

#else

setsee()
{
	register int x,y;

	if(Blind) {
		pru();
		return;
	}
	if(!levl[u.ux][u.uy].lit) {
		seelx = u.ux-1;
		seehx = u.ux+1;
		seely = u.uy-1;
		seehy = u.uy+1;
	} else {
		for(seelx = u.ux; levl[seelx-1][u.uy].lit; seelx--);
		for(seehx = u.ux; levl[seehx+1][u.uy].lit; seehx++);
		for(seely = u.uy; levl[u.ux][seely-1].lit; seely--);
		for(seehy = u.uy; levl[u.ux][seehy+1].lit; seehy++);
	}
	for(y = seely; y <= seehy; y++)
		for(x = seelx; x <= seehx; x++) {
			prl(x,y);
	}
	if(!levl[u.ux][u.uy].lit) seehx = 0; /* seems necessary elsewhere */
	else {
	    if(seely == u.uy) for(x = u.ux-1; x <= u.ux+1; x++) prl(x,seely-1);
	    if(seehy == u.uy) for(x = u.ux-1; x <= u.ux+1; x++) prl(x,seehy+1);
	    if(seelx == u.ux) for(y = u.uy-1; y <= u.uy+1; y++) prl(seelx-1,y);
	    if(seehx == u.ux) for(y = u.uy-1; y <= u.uy+1; y++) prl(seehx+1,y);
	}
}
#endif QUEST

nomul(nval)
register int nval;
{
	if(multi < 0) return;
	multi = nval;
	flags.mv = flags.run = 0;
}

abon()
{
	if(u.ustr == 3) return(-3);
	else if(u.ustr < 6) return(-2);
	else if(u.ustr < 8) return(-1);
	else if(u.ustr < 17) return(0);
	else if(u.ustr < 69) return(1);	/* up to 18/50 */
	else if(u.ustr < 118) return(2);
	else return(3);
}

dbon()
{
	if(u.ustr < 6) return(-1);
	else if(u.ustr < 16) return(0);
	else if(u.ustr < 18) return(1);
	else if(u.ustr == 18) return(2);	/* up to 18 */
	else if(u.ustr < 94) return(3);	/* up to 18/75 */
	else if(u.ustr < 109) return(4);	/* up to 18/90 */
	else if(u.ustr < 118) return(5);	/* up to 18/99 */
	else return(6);
}

losestr(num)
register int num;
{
	u.ustr -= num;
	while(u.ustr < 3) {
		u.ustr++;
		u.uhp -= 6;
		u.uhpmax -= 6;
	}
	flags.botl = 1;
}

losehp(n,knam)
register int n;
register char *knam;
{
	u.uhp -= n;
	if(u.uhp > u.uhpmax)
		u.uhpmax = u.uhp;	/* perhaps n was negative */
	flags.botl = 1;
	if(u.uhp < 1)
		killer = knam;	/* the thing that killed you */
}

losehp_m(n,mtmp)
register int n;
register struct monst *mtmp;
{
	u.uhp -= n;
	flags.botl = 1;
	if(u.uhp < 1) done_in_by(mtmp);
}

losexp()	/* hit by V or W */
{
	register int num;

	if(u.ulevel > 1) pline("Goodbye level %d.",u.ulevel--);
	else u.uhp = -1;
	num = rnd(10);
	u.uhp -= num;
	u.uhpmax -= num;
	u.uexp = 10*pow(u.ulevel-1);
	flags.botl = 1;
}

inv_weight(){
register struct obj *otmp = invent;
register int wt = 0;
register int carrcap = 5*(((u.ustr > 18) ? 20 : u.ustr) + u.ulevel);
	if(carrcap > MAX_CARR_CAP) carrcap = MAX_CARR_CAP;
	if(Wounded_legs & LEFT_SIDE) carrcap -= 10;
	if(Wounded_legs & RIGHT_SIDE) carrcap -= 10;
	while(otmp){
		wt += otmp->owt;
		otmp = otmp->nobj;
	}
	return(wt - carrcap);
}

inv_cnt(){
register struct obj *otmp = invent;
register int ct = 0;
	while(otmp){
		ct++;
		otmp = otmp->nobj;
	}
	return(ct);
}

#file hack.cmdlist.c
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* hack.cmdlist.c version 1.0.1 - added '^T': dotele() and ',': dopickup() */
#include	"config.h"
#include	"def.objclass.h"
#include	"def.func_tab.h"

int doredraw(),doredotopl(),dodrop(),dodrink(),doread(),dosearch(),dopickup(),
doversion(),doweararm(),dowearring(),doremarm(),doremring(),dopay(),doapply(),
dosave(),dowield(),ddoinv(),dozap(),ddocall(),dowhatis(),doengrave(),dotele(),
dohelp(),doeat(),doddrop(),do_mname(),doidtrap(),doprwep(),doprarm(),doprring();
#ifdef SHELL
int dosh();
#endif SHELL
#ifdef OPTIONS
int doset();
#endif OPTIONS
int doup(), dodown(), done1(), donull();
int dothrow();
struct func_tab list[]={
	'\022', doredraw,
	'\024', dotele,
	'\020', doredotopl,
	'a', doapply,
/*	'A' : UNUSED */
/*	'b', 'B' : go sw */
	'c', ddocall,
	'C', do_mname,
	'd', dodrop,
	'D', doddrop,
	'e', doeat,
	'E', doengrave,
/*	'f', 'F' : multiple go (might become 'fight') */
/*	'g', 'G' : UNUSED */
/*	'h', 'H' : go west */
	'i', ddoinv,
#ifdef CHEATINV
	'I', myddoinv,
#else
        'i', ddoinv,
#endif
/*	'j', 'J', 'k', 'K', 'l', 'L', 'm', 'M', 'n', 'N' : move commands */
#ifdef OPTIONS
	'o', doset,
#endif OPTIONS
/*	'O' : UNUSED */
	'p', dopay,
	'P', dowearring,
	'q', dodrink,
	'Q', done1,
	'r', doread,
	'R', doremring,
	's', dosearch,
	'S', dosave,
	't', dothrow,
	'T', doremarm,
/*	'u', 'U' : go ne */
	'v', doversion,
/*	'V' : UNUSED */
	'w', dowield,
	'W', doweararm,
/*	'x', 'X' : UNUSED */
/*	'y', 'Y' : go nw */
	'z', dozap,
/*	'Z' : UNUSED */
	'<', doup,
	'>', dodown,
	'/', dowhatis,
	'?', dohelp,
#ifdef SHELL
	'!', dosh,
#endif SHELL
	',', dopickup,
	'.', donull,
	' ', donull,
	'^', doidtrap,
	 WEAPON_SYM,  doprwep,
	 ARMOR_SYM,  doprarm,
	 RING_SYM,  doprring,
	0,0,0
};

#file hack.decl.c
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1984. */



#include	"hack.h"

char nul[40];			/* contains zeros */

char plname[PL_NSIZ] = "player";/* player name */

char lock[32] = "1lock";	/* long enough for login name */



#ifdef WIZARD

boolean wizard;			/* TRUE when called as  hack -w */

#endif WIZARD



struct rm levl[COLNO][ROWNO];	/* level map */

#ifndef QUEST

struct mkroom rooms[MAXNROFROOMS+1];

coord doors[DOORMAX];

#endif QUEST

struct monst *fmon = 0;

struct gen *fgold = 0, *ftrap = 0;

struct obj *fobj = 0, *fcobj = 0, *invent = 0, *uwep = 0, *uarm = 0,

	*uarm2 = 0, *uarmh = 0, *uarms = 0, *uarmg = 0, *uright = 0,

	*uleft = 0, *uchain = 0, *uball = 0;

struct flag flags;

struct you u;



xchar dlevel = 1;

xchar xupstair, yupstair, xdnstair, ydnstair;

char *save_cm = 0, *killer, *nomovemsg;



long moves = 1;

long wailmsg = 0;



int multi = 0;

char genocided[60];

char fut_geno[60];



xchar curx,cury;

xchar seelx, seehx, seely, seehy;	/* corners of lit room */



coord bhitpos;
