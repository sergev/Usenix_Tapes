
#file hack.engrave.c
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* hack.engrave.c version 1.0.1 -
	corrected bug in rest_engravings(),
	added make_engr_at() */
#include	"hack.h"
extern char *nomovemsg;
extern char nul[];
struct engr {
	struct engr *nxt_engr;
	char *engr_txt;
	xchar engr_x, engr_y;
	unsigned engr_lth;	/* for save & restore; not length of text */
	long engr_time;	/* moment engraving was (will be) finished */
	xchar engr_type;
#define	DUST	1
#define	ENGRAVE	2
#define	BURN	3
} *head_engr;

struct engr *
engr_at(x,y) register xchar x,y; {
register struct engr *ep = head_engr;
	while(ep) {
		if(x == ep->engr_x && y == ep->engr_y)
			return(ep);
		ep = ep->nxt_engr;
	}
	return((struct engr *) 0);
}

sengr_at(s,x,y) register char *s; register xchar x,y; {
register struct engr *ep = engr_at(x,y);
register char *t;
register int n;
	if(ep && ep->engr_time <= moves) {
		t = ep->engr_txt;
/*
		if(!strcmp(s,t)) return(1);
*/
		n = strlen(s);
		while(*t) {
			if(!strncmp(s,t,n)) return(1);
			t++;
		}
	}
	return(0);
}

wipe_engr_at(x,y,cnt) register xchar x,y,cnt; {
register struct engr *ep = engr_at(x,y);
register int lth,pos;
char ch;
	if(ep){
		if(ep->engr_type != DUST) {
			cnt = rn2(1 + 50/(cnt+1)) ? 0 : 1;
		}
		lth = strlen(ep->engr_txt);
		if(lth && cnt > 0 ) {
			while(cnt--) {
				pos = rn2(lth);
				if((ch = ep->engr_txt[pos]) == ' ')
					continue;
				ep->engr_txt[pos] = (ch != '?') ? '?' : ' ';
			}
		}
		while(lth && ep->engr_txt[lth-1] == ' ')
			ep->engr_txt[--lth] = 0;
		while(ep->engr_txt[0] == ' ')
			ep->engr_txt++;
		if(!ep->engr_txt[0]) del_engr(ep);
	}
}

read_engr_at(x,y) register int x,y; {
register struct engr *ep = engr_at(x,y);
	if(ep && ep->engr_txt[0]) {
	    switch(ep->engr_type) {
	    case DUST:
		pline("Something is written here in the dust.");
		break;
	    case ENGRAVE:
		pline("Something is engraved here on the floor.");
		break;
	    case BURN:
		pline("Some text has been burned here in the floor.");
		break;
	    default:
		pline("Something is written in a very strange way.");
		impossible();
	    }
	    pline("You read: \"%s\".", ep->engr_txt);
	}
}
make_engr_at(x,y,s)
register int x,y;
register char *s;
{
	register struct engr *ep;

	if(ep = engr_at(x,y))
	    del_engr(ep);
	ep = (struct engr *)
	    alloc((unsigned)(sizeof(struct engr) + strlen(s) + 1));
	ep->nxt_engr = head_engr;
	head_engr = ep;
	ep->engr_x = x;
	ep->engr_y = y;
	ep->engr_txt = (char *)(ep + 1);
	(void) strcpy(ep->engr_txt, s);
	ep->engr_time = 0;
	ep->engr_type = DUST;
	ep->engr_lth = strlen(s) + 1;
}

doengrave(){
register int len;
register char *sp;
register struct engr *ep, *oep = engr_at(u.ux,u.uy);
char buf[BUFSZ];
xchar type;
int spct;		/* number of leading spaces */
register struct obj *otmp;
	multi = 0;
	if(u.uswallow) {
		pline("You're joking. Hahaha!");	/* riv05!a3 */
		return(0);
	}

	/* one may write with finger, weapon or wand */
	otmp = getobj("#-)/", "write with");
	if(!otmp) return(0);
	if(otmp == (struct obj *)(1))
		type = DUST;
	else if(otmp->otyp == WAN_FIRE && otmp->spe) {
		type = BURN;
		otmp->spe--;
	} else if(otmp->otyp == DAGGER || otmp->otyp == TWO_HANDED_SWORD ||
		otmp->otyp == CRYSKNIFE ||
		otmp->otyp == LONG_SWORD || otmp->otyp == AXE){
		type = ENGRAVE;
		if((int)otmp->spe <= -3) {
			type = DUST;
			pline("Your %s too dull for engraving.",
				aobjnam(otmp, "are"));
			if(oep && oep->engr_type != DUST) return(1);
		}
	} else	type = DUST;
	if(Levitation && type != BURN){		/* riv05!a3 */
		pline("You can't reach the floor!");
		return(1);
	}
	if(oep && oep->engr_type == DUST){
		  pline("You wipe out the message that was written here.");
		  del_engr(oep);
		  oep = 0;
	}
	if(type == DUST && oep){
	pline("You cannot wipe out the message that is %s in the rock.",
		    (oep->engr_type == BURN) ? "burned" : "engraved");
		  return(1);
	}

	pline("What do you want to %s on the floor here? ",
	  (type == ENGRAVE) ? "engrave" : (type == BURN) ? "burn" : "write");
	getlin(buf);
	clrlin();
	spct = 0;
	sp = buf;
	while(*sp == ' ') spct++, sp++;
	len = strlen(sp);
	if(!len) {
		if(type == BURN) otmp->spe++;
		return(0);
	}
	
	switch(type) {
	case DUST:
	case BURN:
		if(len > 15) {
			multi = -(len/10);
			nomovemsg = "You finished writing.";
		}
		break;
	case ENGRAVE:
		{	int len2 = (otmp->spe + 3) * 2 + 1;
			char *bufp = doname(otmp);
			if(digit(*bufp))
				pline("Your %s get dull.", bufp);
			else {
				if(!strncmp(bufp,"a ",2))
					bufp += 2;
				else if(!strncmp(bufp,"an ",3))
					bufp += 3;
				pline("Your %s gets dull.", bufp);
			}
			if(len2 < len) {
				len = len2;
				sp[len] = 0;
				otmp->spe = -3;
				nomovemsg = "You cannot engrave more.";
			} else {
				otmp->spe -= len/2;
				nomovemsg = "You finished engraving.";
			}
			multi = -len;
		}
		break;
	}
	if(oep) len += strlen(oep->engr_txt) + spct;
	ep = (struct engr *) alloc((unsigned)(sizeof(struct engr) + len + 1));
	ep->nxt_engr = head_engr;
	head_engr = ep;
	ep->engr_x = u.ux;
	ep->engr_y = u.uy;
	sp = (char *)(ep + 1);	/* (char *)ep + sizeof(struct engr) */
	ep->engr_txt = sp;
	if(oep) {
		(void) strcpy(sp, oep->engr_txt);
		(void) strcat(sp, buf);
		del_engr(oep);
	} else
		(void) strcpy(sp, buf);
	ep->engr_lth = len+1;
	ep->engr_type = type;
	ep->engr_time = moves-multi;

	/* kludge to protect pline against excessively long texts */
	if(len > BUFSZ-20) sp[BUFSZ-20] = 0;

	return(1);
}

save_engravings(fd) int fd; {
register struct engr *ep = head_engr;
	while(ep) {
		if(!ep->engr_lth || !ep->engr_txt[0]){
			ep = ep->nxt_engr;
			continue;
		}
		bwrite(fd, (char *) & (ep->engr_lth), sizeof(ep->engr_lth));
		bwrite(fd, (char *) ep, sizeof(struct engr) + ep->engr_lth);
		ep = ep->nxt_engr;
	}
	bwrite(fd, (char *) nul, sizeof(unsigned));
}

rest_engravings(fd) int fd; {
register struct engr *ep;
unsigned lth;
	head_engr = 0;
	while(1) {
		mread(fd, (char *) &lth, sizeof(unsigned));
		if(lth == 0) return;
		ep = (struct engr *) alloc(sizeof(struct engr) + lth);
		mread(fd, (char *) ep, sizeof(struct engr) + lth);
		ep->engr_txt = (char *) (ep + 1);	/* Andreas Bormann */
		ep->nxt_engr = head_engr;
		head_engr = ep;
	}
}

del_engr(ep) register struct engr *ep; {
register struct engr *ept;
	if(ep == head_engr)
		head_engr = ep->nxt_engr;
	else {
		for(ept = head_engr; ept; ept = ept->nxt_engr)
			if(ept->nxt_engr == ep) {
				ept->nxt_engr = ep->nxt_engr;
				goto fnd;
			}
		pline("Error in del_engr?"); impossible();
	fnd:	;
	}
	free((char *) ep);
}
#file hack.fight.c
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* hack.fight.c version 1.0.1 - corrected symbol of lurker above */

#include	"hack.h"
extern char *exclam(), *xname();

static boolean far_noise;
static long noisetime;

/* hitmm returns 0 (miss), 1 (hit), or 2 (kill) */
hitmm(magr,mdef) register struct monst *magr,*mdef; {
register struct permonst *pa = magr->data, *pd = mdef->data;
int hit;
schar tmp;
boolean vis;
	if(index("Eauy", pa->mlet)) return(0);
	if(magr->mfroz) return(0);		/* riv05!a3 */
	tmp = pd->ac + pa->mlevel;
	if(mdef->mconf || mdef->mfroz || mdef->msleep){
		tmp += 4;
		if(mdef->msleep) mdef->msleep = 0;
	}
	hit = (tmp > rnd(20));
	if(hit) mdef->msleep = 0;
	vis = (cansee(magr->mx,magr->my) && cansee(mdef->mx,mdef->my));
	if(vis){
		char buf[BUFSZ];
		if(mdef->mimic) seemimic(mdef);
		if(magr->mimic) seemimic(magr);
		(void) sprintf(buf,"%s %s", Monnam(magr),
			hit ? "hits" : "misses");
		pline("%s %s.", buf, monnam(mdef));
	} else {
		boolean far = (dist(magr->mx, magr->my) > 15);
		if(far != far_noise || moves-noisetime > 10) {
			far_noise = far;
			noisetime = moves;
			pline("You hear some noises%s.",
				far ? " in the distance" : "");
		}
	}
	if(hit){
		if(magr->data->mlet == 'c' && !magr->cham) {
			magr->orig_hp += 3;
			if(vis) pline("%s is turned to stone!", Monnam(mdef));
			else if(mdef->mtame)
     pline("You have a peculiarly sad feeling for a moment, then it passes.");
			monstone(mdef);
			hit = 2;
		} else
		if((mdef->mhp -= d(pa->damn,pa->damd)) < 1) {
			magr->orig_hp += 1 + rn2(pd->mlevel+1);
			if(magr->mtame && magr->orig_hp > 8*pa->mlevel){
				if(pa == PM_LI_DOG)
					magr->data = pa = PM_DOG;
				else if(pa == PM_DOG)
					magr->data = pa = PM_LA_DOG;
			}
			if(vis) pline("%s is killed!", Monnam(mdef));
			else if(mdef->mtame)
		pline("You have a sad feeling for a moment, then it passes.");
			mondied(mdef);
			hit = 2;
		}
	}
	return(hit);
}

/* drop (perhaps) a cadaver and remove monster */
mondied(mdef) register struct monst *mdef; {
register struct permonst *pd = mdef->data;
		if(letter(pd->mlet) && rn2(3)){
			mksobj_at(pd->mlet,CORPSE,mdef->mx,mdef->my);
			if(cansee(mdef->mx,mdef->my)){
				unpmon(mdef);
				atl(mdef->mx,mdef->my,fobj->olet);
			}
			stackobj(fobj);
		}
		mondead(mdef);
}

/* drop a rock and remove monster */
monstone(mdef) register struct monst *mdef; {
	extern char mlarge[];
	if(index(mlarge, mdef->data->mlet))
		mksobj_at(ROCK_SYM, ENORMOUS_ROCK, mdef->mx, mdef->my);
	else
		mksobj_at(WEAPON_SYM, ROCK, mdef->mx, mdef->my);
	if(cansee(mdef->mx, mdef->my)){
		unpmon(mdef);
		atl(mdef->mx,mdef->my,fobj->olet);
	}
	mondead(mdef);
}
		

fightm(mtmp) register struct monst *mtmp; {
register struct monst *mon;
	for(mon = fmon; mon; mon = mon->nmon) if(mon != mtmp) {
		if(DIST(mon->mx,mon->my,mtmp->mx,mtmp->my) < 3)
		    if(rn2(4))
			return(hitmm(mtmp,mon));
	}
	return(-1);
}

hitu(mtmp,dam)
register struct monst *mtmp;
register int dam;
{
	register int tmp;

	if(u.uswallow) return(0);

	if(mtmp->mhide && mtmp->mundetected) {
		mtmp->mundetected = 0;
		if(!Blind) {
			register struct obj *obj;
			extern char * Xmonnam();
			if(obj = o_at(mtmp->mx,mtmp->my))
				pline("%s was hidden under %s!",
					Xmonnam(mtmp), doname(obj));
		}
	}

	tmp = u.uac;
	/* give people with Ac = -10 at least some vulnerability */
	if(tmp < 0) {
		dam += tmp;		/* decrease damage */
		if(dam <= 0) dam = 1;
		tmp = -rn2(-tmp);
	}
	tmp += mtmp->data->mlevel;
	if(multi < 0) tmp += 4;
	if(Invis || !mtmp->mcansee) tmp -= 2;
	if(mtmp->mtrapped) tmp -= 2;
	if(tmp <= rnd(20)) {
		if(Blind) pline("It misses.");
		else pline("%s misses.",Monnam(mtmp));
		return(0);
	}
	if(Blind) pline("It hits!");
	else pline("%s hits!",Monnam(mtmp));
	losehp_m(dam, mtmp);
	return(1);
}

/* u is hit by sth, but not a monster */
thitu(tlev,dam,name)
register int tlev,dam;
register char *name;
{
char buf[BUFSZ];
	setan(name,buf);
	if(u.uac + tlev <= rnd(20)) {
		if(Blind) pline("It misses.");
		else pline("You are almost hit by %s!", buf);
		return(0);
	} else {
		if(Blind) pline("You are hit!");
		else pline("You are hit by %s!", buf);
		losehp(dam,name);
		return(1);
	}
}

char mlarge[] = "bCDdegIlmnoPSsTUwY\',&";

boolean
hmon(mon,obj,thrown)	/* return TRUE if mon still alive */
register struct monst *mon;
register struct obj *obj;
register int thrown;
{
	register int tmp;

	if(!obj){
		tmp = rnd(2);	/* attack with bare hands */
		if(mon->data->mlet == 'c' && !uarmg){
			pline("You hit the cockatrice with your bare hands");
			pline("You turn to stone ...");
			done_in_by(mon);
		}
	} else if(obj->olet == WEAPON_SYM) {
	    if(obj == uwep && (obj->otyp > SPEAR || obj->otyp < BOOMERANG))
		tmp = rnd(2);
	    else {
		if(index(mlarge, mon->data->mlet)) {
			tmp = rnd(objects[obj->otyp].wldam);
			if(obj->otyp == TWO_HANDED_SWORD) tmp += d(2,6);
			else if(obj->otyp == FLAIL) tmp += rnd(4);
		} else {
			tmp = rnd(objects[obj->otyp].wsdam);
		}
		tmp += obj->spe;
		if(!thrown && obj == uwep && obj->otyp == BOOMERANG
		 && !rn2(3)){
		  pline("As you hit %s, the boomerang breaks into splinters.",
				monnam(mon));
			freeinv(obj);
			setworn((struct obj *) 0, obj->owornmask);
			obfree(obj, (struct obj *) 0);
			tmp++;
		}
	    }
	    if(mon->data->mlet == 'O' && !strcmp(ONAME(obj), "Orcrist"))
		tmp += rnd(10);
	} else	switch(obj->otyp) {
		case HEAVY_IRON_BALL:
			tmp = rnd(25); break;
		case EXPENSIVE_CAMERA:
	pline("You succeed in destroying your camera. Congratulations!");
			freeinv(obj);
			if(obj->owornmask)
				setworn((struct obj *) 0, obj->owornmask);
			obfree(obj, (struct obj *) 0);
			return(TRUE);
		case DEAD_COCKATRICE:
			pline("You hit %s with the cockatrice corpse",
				monnam(mon));
			pline("%s is turned to stone!", Monnam(mon));
			killed(mon);
			return(FALSE);
		case CLOVE_OF_GARLIC:
			if(index(" VWZ", mon->data->mlet))
				mon->mflee = 1;
			tmp = 1;
			break;
		default:
			/* non-weapons can damage because of their weight */
			/* (but not too much) */
			tmp = obj->owt/10;
			if(tmp < 1) tmp = 1;
			else tmp = rnd(tmp);
			if(tmp > 6) tmp = 6;
		}

	/****** NOTE: perhaps obj is undefined!! (if !thrown && BOOMERANG) */

	tmp += u.udaminc + dbon();
	if(u.uswallow)
		if(mon->data->mlet == 'P') {
			if((tmp -= u.uswldtim) <= 0) {
				pline("Your arms are no longer able to hit.");
				return(TRUE);
			}
		}
	if(tmp < 1) tmp = 1;
	mon->mhp -= tmp;
	if(mon->mhp < 1) {
		killed(mon);
		return(FALSE);
	}

	if(thrown) {	/* this assumes that we cannot throw plural things */
		hit( xname(obj)		/* or: objects[obj->otyp].oc_name */,
			mon, exclam(tmp) );
		return(TRUE);
	}
	if(Blind) pline("You hit it.");
	else pline("You hit %s%s", monnam(mon), exclam(tmp));

	if(u.umconf) {
		if(!Blind) {
			pline("Your hands stop glowing blue.");
			if(!mon->mfroz && !mon->msleep)
				pline("%s appears confused.",Monnam(mon));
		}
		mon->mconf = 1;
		u.umconf = 0;
	}
	return(TRUE);	/* mon still alive */
}
#file hack.invent.c
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1984. */

#include   "hack.h"
#include   <stdio.h>
#undef max

extern struct obj *splitobj();
extern char morc;
#ifndef NOWORM
#include   "def.wseg.h"

extern struct wseg *wsegs[32];
#endif NOWORM

struct obj *
addinv(obj) register struct obj *obj; {
   register struct obj *otmp;
   for(otmp = invent; otmp; otmp = otmp->nobj) {
      if(merged(otmp, obj, 0)) return(otmp);
      if(!otmp->nobj) {
         otmp->nobj = obj;
         obj->nobj = 0;
         return(obj);
      }
   }
   invent = obj;
   obj->nobj = 0;
   return(obj);
}

useup(obj)
register struct obj *obj;
{
   if(obj->quan > 1){
      obj->quan--;
      obj->owt = weight(obj);
   } else {
      setnotworn(obj);
      freeinv(obj);
      obfree(obj, (struct obj *) 0);
   }
}

freeinv(obj) register struct obj *obj; {
   register struct obj *otmp;
   if(obj == invent) invent = invent->nobj;
   else {
      for(otmp = invent; otmp->nobj != obj; otmp = otmp->nobj)
         if(!otmp->nobj) panic("freeinv");
      otmp->nobj = obj->nobj;
   }
}

/* destroy object in fobj chain (if unpaid, it remains on the bill) */
delobj(obj) register struct obj *obj; {
   freeobj(obj);
   unpobj(obj);
   obfree(obj, (struct obj *) 0);
}

/* unlink obj from chain starting with fobj */
freeobj(obj) register struct obj *obj; {
   register struct obj *otmp;

   if(obj == fobj) fobj = fobj->nobj;
   else {
      for(otmp = fobj; otmp->nobj != obj; otmp = otmp->nobj)
         if(!otmp) panic("error in freeobj");
      otmp->nobj = obj->nobj;
   }
}

/* Note: freegold throws away its argument! */
freegold(gold) register struct gen *gold; {
   register struct gen *gtmp;

   if(gold == fgold) fgold = gold->ngen;
   else {
      for(gtmp = fgold; gtmp->ngen != gold; gtmp = gtmp->ngen)
         if(!gtmp) panic("error in freegold");
      gtmp->ngen = gold->ngen;
   }
   free((char *) gold);
}

deltrap(trap)
register struct gen *trap;
{
   register struct gen *gtmp;

   if(trap==ftrap) ftrap=ftrap->ngen;
   else {
      for(gtmp=ftrap;gtmp->ngen!=trap;gtmp=gtmp->ngen) ;
      gtmp->ngen=trap->ngen;
   }
   free((char *) trap);
}

struct wseg *m_atseg;

struct monst *
m_at(x,y)
register int x,y;
{
   register struct monst *mtmp;
#ifndef NOWORM
   register struct wseg *wtmp;
#endif NOWORM

   m_atseg = 0;
   for(mtmp = fmon; mtmp; mtmp = mtmp->nmon){
      if(mtmp->mx == x && mtmp->my == y)
         return(mtmp);
#ifndef NOWORM
      if(mtmp->wormno){
          for(wtmp = wsegs[mtmp->wormno]; wtmp; wtmp = wtmp->nseg)
          if(wtmp->wx == x && wtmp->wy == y){
         m_atseg = wtmp;
         return(mtmp);
          }
      }
#endif NOWORM
   }
   return(0);
}

struct obj *
o_at(x,y)
register int x,y;
{
   register struct obj *otmp;

   for(otmp = fobj; otmp; otmp = otmp->nobj)
      if(otmp->ox == x && otmp->oy == y) return(otmp);
   return(0);
}

struct obj *
sobj_at(n,x,y)
register int n,x,y;
{
   register struct obj *otmp;

   for(otmp = fobj; otmp; otmp = otmp->nobj)
      if(otmp->ox == x && otmp->oy == y && otmp->otyp == n)
         return(otmp);
   return(0);
}

carried(obj) register struct obj *obj; {
register struct obj *otmp;
   for(otmp = invent; otmp; otmp = otmp->nobj)
      if(otmp == obj) return(1);
   return(0);
}

struct obj *
o_on(id, objchn) unsigned int id; register struct obj *objchn; {
   while(objchn) {
      if(objchn->o_id == id) return(objchn);
      objchn = objchn->nobj;
   }
   return((struct obj *) 0);
}

struct gen *
g_at(x,y,ptr)
register int x,y;
register struct gen *ptr;
{
   while(ptr) {
      if(ptr->gx == x && ptr->gy == y) return(ptr);
      ptr = ptr->ngen;
   }
   return(0);
}

/* getobj returns:
   struct obj *xxx:   object to do something with.
   0            error return: no object.
   1            explicitly no object (as in w-).
*/
struct obj *
getobj(let,word)
register char *let,*word;
{
   register struct obj *otmp;
   register char ilet,ilet1,ilet2;
   char buf[BUFSZ];
   char lets[BUFSZ];
   register int foo = 0, foo2, cnt;
   register char *bp = buf;
   xchar allowcnt = 0;   /* 0, 1 or 2 */
   boolean allowgold = FALSE;
   boolean allowall = FALSE;
   boolean allownone = FALSE;
   xchar foox = 0;

   if(*let == '0') let++, allowcnt = 1;
   if(*let == '$') let++, allowgold = TRUE;
   if(*let == '#') let++, allowall = TRUE;
   if(*let == '-') let++, allownone = TRUE;
   if(allownone) *bp++ = '-';
   if(allowgold) *bp++ = '$';
   if(bp[-1] == '-') *bp++ = ' ';

   ilet = 'a';
   for(otmp = invent; otmp; otmp = otmp->nobj){
      if(!*let || index(let, otmp->olet)) {
         bp[foo++] = ilet;
         /* ugly check: remove inappropriate things */
         if((!strcmp(word, "take off") &&
             !(otmp->owornmask & (W_ARMOR - W_ARM2)))
         || (!strcmp(word, "wear") &&
             (otmp->owornmask & (W_ARMOR | W_RING)))
         || (!strcmp(word, "wield") &&
             (otmp->owornmask & W_WEP))) {
            foo--;
            foox++;
         }
      }
      if(ilet == 'z') ilet = 'A'; else ilet++;
   }
   bp[foo] = 0;
   if(foo == 0 && bp > buf && bp[-1] == ' ') *--bp = 0;
   (void) strcpy(lets, bp);   /* necessary since we destroy buf */
   if(foo > 5) {         /* compactify string */
      foo = foo2 = 1;
      ilet2 = bp[0];
      ilet1 = bp[1];
      while(ilet = bp[++foo2] = bp[++foo]){
         if(ilet == ilet1+1){
            if(ilet1 == ilet2+1)
               bp[foo2 - 1] = ilet1 = '-';
            else if(ilet2 == '-') {
               bp[--foo2] = ++ilet1;
               continue;
            }
         }
         ilet2 = ilet1;
         ilet1 = ilet;
      }
   }
   if(!foo && !allowall && !allowgold && !allownone) {
      pline("You don't have anything %sto %s.",
         foox ? "else " : "", word);
      return(0);
   }
   for(;;) {
      if(!buf[0])
         pline("What do you want to %s [*]? ", word);
      else
         pline("What do you want to %s [%s or ?*]? ",
            word, buf);

      cnt = 0;
      ilet = readchar();
      while(digit(ilet) && allowcnt) {
         cnt = 10*cnt + (ilet - '0');
         allowcnt = 2;   /* signal presence of cnt */
         ilet = readchar();
      }
      if(digit(ilet)) {
         pline("No count allowed with this command.");
         continue;
      }
      if(ilet == '\033' || ilet == ' ' || ilet == '\n')
         return((struct obj *)0);
      if(ilet == '-') {
         return((struct obj *)(allownone ? 1 : 0));
      }
      if(ilet == '$') {
         if(!allowgold){
            pline("You cannot %s gold.", word);
            continue;
         }
         otmp = newobj(0);
         /* should set o_id etc. but otmp will be freed soon */
         otmp->olet = '$';
         if(allowcnt == 2 && cnt < u.ugold)
            u.ugold -= cnt;
         else {
            cnt = u.ugold;
            u.ugold = 0;
         }
         flags.botl = 1;
         otmp->quan = cnt;
         return(otmp);
      }
      if(ilet == '?') {
         doinv(lets);
         if(!(ilet = morc)) continue;
         /* he typed a letter (not a space) to more() */
      } else if(ilet == '*') {
         doinv("");
         if(!(ilet = morc)) continue;
         /* ... */
      }
      if(ilet >= 'A' && ilet <= 'Z') ilet += 'z'-'A'+1;
      ilet -= 'a';
      for(otmp = invent; otmp && ilet; ilet--, otmp = otmp->nobj) ;
      if(!otmp) {
         pline("You don't have that object.");
         continue;
      }
      if(cnt < 0 || otmp->quan < cnt) {
         pline("You don't have that many! [You have %d]"
         , otmp->quan);
         continue;
      }
      break;
   }
   if(!allowall && let && !index(let,otmp->olet)) {
      pline("That is a silly thing to %s.",word);
      return(0);
   }
   if(allowcnt == 2) {   /* cnt given */
      if(cnt == 0) return(0);
      if(cnt != otmp->quan) {
         register struct obj *obj;
         obj = splitobj(otmp, cnt);
         if(otmp == uwep) setuwep(obj);
      }
   }
   return(otmp);
}

ckunpaid(otmp) register struct obj *otmp; {
   return( otmp->unpaid );
}

/* interactive version of getobj */
/* used for Drop and Identify */
ggetobj(word, fn, max)
char *word;
int (*fn)(),  max;
{
char buf[BUFSZ];
register char *ip;
register char sym;
register int oletct = 0, iletct = 0;
register boolean allflag = FALSE;
char olets[20], ilets[20];
int (*ckfn)() = (int (*)()) 0;
   if(!invent){
      pline("You have nothing to %s.", word);
      return(0);
   } else {
      register struct obj *otmp = invent;
      register int uflg = 0;

      ilets[0] = 0;
      while(otmp) {
         if(!index(ilets, otmp->olet)){
            ilets[iletct++] = otmp->olet;
            ilets[iletct] = 0;
         }
         if(otmp->unpaid) uflg = 1;
         otmp = otmp->nobj;
      }
      ilets[iletct++] = ' ';
      if(uflg) ilets[iletct++] = 'u';
      ilets[iletct++] = 'a';
      ilets[iletct] = 0;
   }
   pline("What kinds of thing do you want to %s? [%s] ",
      word, ilets);
   getlin(buf);
   ip = buf;
   olets[0] = 0;
   while(sym = *ip++){
      if(sym == ' ') continue;
      if(sym == 'a') allflag = TRUE; else
      if(sym == 'u') ckfn = ckunpaid; else
      if(index("!%?[()=*/\"0", sym)){
         if(!index(olets, sym)){
            olets[oletct++] = sym;
            olets[oletct] = 0;
         }
      }
      else pline("You don't have any %c's.", sym);
   }
   return askchain(invent, olets, allflag, fn, ckfn, max);
}

/* Walk through the chain starting at objchn and ask for all objects
   with olet in olets (if nonNULL) and satisfying ckfn (if nonNULL)
   whether the action in question (i.e., fn) has to be performed.
   If allflag then no questions are asked. Max gives the max nr of
   objects treated.
 */
askchain(objchn, olets, allflag, fn, ckfn, max)
struct obj *objchn;
register char *olets;
int allflag;
int (*fn)(), (*ckfn)();
int max;
{
register struct obj *otmp, *otmp2;
register char sym, ilet;
register int cnt = 0;
   ilet = 'a'-1;
   for(otmp = objchn; otmp; otmp = otmp2){
      if(ilet == 'z') ilet = 'A'; else ilet++;
      otmp2 = otmp->nobj;
      if(olets && *olets && !index(olets, otmp->olet)) continue;
      if(ckfn && !(*ckfn)(otmp)) continue;
      if(!allflag) {
         prname(otmp, ilet, 1);
         addtopl(" (ynaq)? ");
         sym = readchar();
      }
      else   sym = 'y';

      switch(sym){
      case 'a':
         allflag = 1;
      case 'y':
         cnt += (*fn)(otmp);
         if(--max == 0) goto ret;
      case 'n':
      default:
         break;
      case 'q':
         goto ret;
      }
   }
   pline(cnt ? "That was all." : "No applicable objects.");
ret:
   if(!flags.echo) echo(OFF);
   return(cnt);
}

obj_to_let(obj)
register struct obj *obj;
{
   register struct obj *otmp;
   register char ilet = 'a';

   for(otmp = invent; otmp && otmp != obj; otmp = otmp->nobj)
      if(++ilet > 'z') ilet = 'A';
   return(otmp ? ilet : 0);
}

prinv(obj)
register struct obj *obj;
{
   prname(obj, obj_to_let(obj), 1);
}

prname(obj,let,onelin)
register struct obj *obj;
register char let;
{
   char li[BUFSZ];

   (void) sprintf(li, " %c - %s.", let, doname(obj));
   switch(onelin) {
   case 1:
      pline(li+1);
      break;
   case 0:
      myputs(li+1);
      break;
   case -1:
      cl_end();
      myputs(li);
      curx += strlen(li);
   }
}

ddoinv()
{
   doinv((char *) 0);
   return(0);
}

myddoinv()
{
   mydoinv((char *) 0);
   return(0);
}

myprname(obj,let,onelin)
register struct obj *obj;
register char let;
{
   char li[BUFSZ];

   (void) sprintf(li, " %c - %s.", let, mydoname(obj));
   switch(onelin) {
   case 1:
      pline(li+1);
      break;
   case 0:
      myputs(li+1);
      break;
   case -1:
      cl_end();
      myputs(li);
      curx += strlen(li);
   }
}

mydoinv(lets) register char *lets; {
   register struct obj *otmp;
   register char ilet = 'a';
   int ct = 0;
   int maxlth = 0;
   int lth;

   if(!invent){
      pline("Not carrying anything");
      if(lets) return;
   }
   if(!flags.oneline) {
       if(!lets || !*lets)
      for(otmp = invent; otmp; otmp = otmp->nobj) ct++;
       else
      ct = strlen(lets);
       if(ct > 1 && ct < ROWNO && (lets || !inshop())){
      for(otmp = invent; otmp; otmp = otmp->nobj) {
          if(!lets || !*lets || index(lets, ilet)) {
         lth = strlen(doname(otmp));
         if(lth > maxlth) maxlth = lth;
          }
          if(++ilet > 'z') ilet = 'A';
      }
      ilet = 'a';
      lth = COLNO - maxlth - 7;
      if(lth < 10) goto clrscr;
      home();
      cl_end();
      flags.topl = 0;
      ct = 0;
      for(otmp = invent; otmp; otmp = otmp->nobj) {
          if(!lets || !*lets || index(lets, ilet)) {
         curs(lth, ++ct);
         myprname(otmp, ilet, -1);
          }
          if(++ilet > 'z') ilet = 'A';
      }
      curs(lth, ct+1);
      cl_end();
      cmore();   /* sets morc */
      /* test whether morc is a reasonable answer */
      if(lets && *lets && !index(lets, morc)) morc = 0;

      home();
      cl_end();
      docorner(lth, ct);
      return;
       }
   }
    clrscr:
   if(ct > 1) cls();
   for(otmp = invent; otmp; otmp = otmp->nobj){
      if(!lets || !*lets || index(lets, ilet))
         myprname(otmp, ilet, (ct > 1) ? 0 : 1);
      if(++ilet > 'z') ilet = 'A';
   }
   /* tell doinvbill whether we cleared the screen */
   if(!lets) doinvbill((ct > 1));
   if(ct > 1){
      cgetret();
      docrt();
   } else
      morc = 0;   /* %% */
}

doinv(lets) register char *lets; {
   register struct obj *otmp;
   register char ilet = 'a';
   int ct = 0;
   int maxlth = 0;
   int lth;

   if(!invent){
      pline("Not carrying anything");
      if(lets) return;
   }
   if(!flags.oneline) {
       if(!lets || !*lets)
      for(otmp = invent; otmp; otmp = otmp->nobj) ct++;
       else
      ct = strlen(lets);
       if(ct > 1 && ct < ROWNO && (lets || !inshop())){
      for(otmp = invent; otmp; otmp = otmp->nobj) {
          if(!lets || !*lets || index(lets, ilet)) {
         lth = strlen(doname(otmp));
         if(lth > maxlth) maxlth = lth;
          }
          if(++ilet > 'z') ilet = 'A';
      }
      ilet = 'a';
      lth = COLNO - maxlth - 7;
      if(lth < 10) goto clrscr;
      home();
      cl_end();
      flags.topl = 0;
      ct = 0;
      for(otmp = invent; otmp; otmp = otmp->nobj) {
          if(!lets || !*lets || index(lets, ilet)) {
         curs(lth, ++ct);
         prname(otmp, ilet, -1);
          }
          if(++ilet > 'z') ilet = 'A';
      }
      curs(lth, ct+1);
      cl_end();
      cmore();   /* sets morc */
      /* test whether morc is a reasonable answer */
      if(lets && *lets && !index(lets, morc)) morc = 0;

      home();
      cl_end();
      docorner(lth, ct);
      return;
       }
   }
    clrscr:
   if(ct > 1) cls();
   for(otmp = invent; otmp; otmp = otmp->nobj){
      if(!lets || !*lets || index(lets, ilet))
         prname(otmp, ilet, (ct > 1) ? 0 : 1);
      if(++ilet > 'z') ilet = 'A';
   }
   /* tell doinvbill whether we cleared the screen */
   if(!lets) doinvbill((ct > 1));
   if(ct > 1){
      cgetret();
      docrt();
   } else
      morc = 0;   /* %% */
}

stackobj(obj) register struct obj *obj; {
register struct obj *otmp = fobj;
   for(otmp = fobj; otmp; otmp = otmp->nobj) if(otmp != obj)
   if(otmp->ox == obj->ox && otmp->oy == obj->oy &&
      merged(obj,otmp,1))
         return;
}

/* merge obj with otmp and delete obj if types agree */
merged(otmp,obj,lose) register struct obj *otmp, *obj; {
   if(otmp->otyp == obj->otyp &&
     obj->unpaid == otmp->unpaid && obj->spe == otmp->spe &&
     obj->known == otmp->known && obj->dknown == otmp->dknown &&
     obj->cursed == otmp->cursed &&
     ((obj->olet == WEAPON_SYM && obj->otyp < BOOMERANG)
     || index("%?!*",otmp->olet))){
      otmp->quan += obj->quan;
      otmp->owt += obj->owt;
      if(lose) freeobj(obj);
      obfree(obj,otmp);   /* free(obj), bill->otmp */
      return(1);
   } else   return(0);
}

doprwep(){
   if(!uwep) pline("You are empty handed.");
   else prinv(uwep);
   return(0);
}

doprarm(){
   if(!uarm && !uarmg && !uarms && !uarmh)
      pline("You are not wearing any armor.");
   else {
      char lets[6];
      register int ct = 0;

      if(uarm) lets[ct++] = obj_to_let(uarm);
      if(uarm2) lets[ct++] = obj_to_let(uarm2);
      if(uarmh) lets[ct++] = obj_to_let(uarmh);
      if(uarms) lets[ct++] = obj_to_let(uarms);
      if(uarmg) lets[ct++] = obj_to_let(uarmg);
      lets[ct] = 0;
      doinv(lets);
   }
   return(0);
}

doprring(){
   if(!uleft && !uright)
      pline("You are not wearing any rings.");
   else {
      char lets[3];
      register int ct = 0;

      if(uleft) lets[ct++] = obj_to_let(uleft);
      if(uright) lets[ct++] = obj_to_let(uright);
      lets[ct] = 0;
      doinv(lets);
   }
   return(0);
}

digit(c) char c; {
   return(c >= '0' && c <= '9');
}

#file hack.lev.c
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* hack.lev.c version 1.0.1 - somewhat more careful monster regeneration */

#include "hack.h"
#include <signal.h>
#include <stdio.h>
extern struct monst *restmonchn();
extern struct obj *restobjchn();
extern struct obj *billobjs;
extern char *itoa();

extern char nul[];
#ifndef NOWORM
#include   "def.wseg.h"

extern struct wseg *wsegs[32], *wheads[32];
extern long wgrowtime[32];
#endif NOWORM

getlev(fd)
{
   register struct gen *gtmp;
#ifndef NOWORM
   register struct wseg *wtmp;
#endif NOWORM
   register int tmp;
   long omoves;

   if(fd<0 || read(fd, (char *) levl, sizeof(levl)) != sizeof(levl))
      return(1);
   fgold = 0;
   ftrap = 0;
   mread(fd, (char *)&omoves, sizeof(omoves));   /* 0 from MKLEV */
   mread(fd, (char *)&xupstair, sizeof(xupstair));
   mread(fd, (char *)&yupstair, sizeof(yupstair));
   mread(fd, (char *)&xdnstair, sizeof(xdnstair));
   mread(fd, (char *)&ydnstair, sizeof(ydnstair));

   fmon = restmonchn(fd);
   if(omoves) {
	/* regenerate animals while on another level */
	long tmoves = (moves > omoves) ? moves-omoves : 0;
	register struct monst *mtmp, *mtmp2;
	extern char genocided[];
	long newhp;

       for(mtmp = fmon; mtmp; mtmp = mtmp2) {
      mtmp2 = mtmp->nmon;
      if(index(genocided, mtmp->data->mlet)) {
         mondead(mtmp);
         continue;
		}
		newhp = mtmp->mhp +
			(index("ViT", mtmp->data->mlet) ? tmoves : tmoves/20);
		if(newhp > mtmp->orig_hp)
			mtmp->mhp = mtmp->orig_hp;
		else
			mtmp->mhp = newhp;
       }
   }

   setshk();
   setgd();
   gtmp = newgen();
   mread(fd, (char *)gtmp, sizeof(struct gen));
   while(gtmp->gx) {
      gtmp->ngen = fgold;
      fgold = gtmp;
      gtmp = newgen();
      mread(fd, (char *)gtmp, sizeof(struct gen));
   }
   mread(fd, (char *)gtmp, sizeof(struct gen));
   while(gtmp->gx) {
      gtmp->ngen = ftrap;
      ftrap = gtmp;
      gtmp = newgen();
      mread(fd, (char *)gtmp, sizeof(struct gen));
   }
   free((char *) gtmp);
   fobj = restobjchn(fd);
   billobjs = restobjchn(fd);
   rest_engravings(fd);
#ifndef QUEST
   mread(fd, (char *)rooms, sizeof(rooms));
   mread(fd, (char *)doors, sizeof(doors));
#endif QUEST
   if(!omoves) return(0);   /* from MKLEV */
#ifndef NOWORM
   mread(fd, (char *)wsegs, sizeof(wsegs));
   for(tmp = 1; tmp < 32; tmp++) if(wsegs[tmp]){
      wheads[tmp] = wsegs[tmp] = wtmp = newseg();
      while(1) {
         mread(fd, (char *)wtmp, sizeof(struct wseg));
         if(!wtmp->nseg) break;
         wheads[tmp]->nseg = wtmp = newseg();
         wheads[tmp] = wtmp;
      }
   }
   mread(fd, (char *)wgrowtime, sizeof(wgrowtime));
#endif NOWORM
   return(0);
}

mread(fd, buf, len)
register int fd;
register char *buf;
register unsigned len;
{
register int rlen;
   rlen = read(fd, buf, (int) len);
   if(rlen != len){
      pline("Read %d instead of %d bytes\n", rlen, len);
      panic("Cannot read %d bytes from file #%d\n", len, fd);
   }
}
#file hack.main.c
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* hack.main.c version 1.0.1 - some cosmetic changes */

#include <stdio.h>
#include <signal.h>
/* #include <errno.h> */
#include "hack.h"

extern char plname[PL_NSIZ], pl_character[PL_CSIZ];
#ifndef AMIGA
extern char *getlogin();
extern char *getenv();
#endif

int (*afternmv)();

int done1();
int hangup();

char safelock[] = "safelock";
xchar locknum;            /* max num of players */
char SAVEF[PL_NSIZ + 22] = "Saved Games/";
char perm[] = "perm";
char *hname;      /* name of the game (argv[0] of call) */
char obuf[BUFSIZ];   /* BUFSIZ is defined in stdio.h */

extern char *nomovemsg;
extern long wailmsg;

main(argc,argv)
int argc;
char *argv[];
{
   int fd;
#ifdef NEWS
	int nonews = 0;
#endif NEWS
   char *dir;

   initterm();
#ifdef AMIGA
   if (argc == 0)
	{
	geticon();
	hname = HACKNAME;
	argc = 1;
	}
   else
#endif
   hname = argv[0];

   /*
    * See if we must change directory to the playground.
    * (Perhaps hack runs suid and playground is inaccessible
    *  for the player.)
    * The environment variable HACKDIR is overridden by a
    *  -d command line option.
    */
#ifndef AMIGA
   if ( (dir = getenv("HACKDIR")) == NULL)
#endif
	dir = HACKDIR;

   if(argc > 1 && !strncmp(argv[1], "-d", 2)) {
      argc--;
      argv++;
      dir = argv[0]+2;
      if(*dir == '=' || *dir == ':') dir++;
      if(!*dir && argc > 1) {
         argc--;
         argv++;
         dir = argv[0];
      }
      if(!*dir)
		    error("Flag -d must be followed by a directory name.");
   }
#ifndef AMIGA
	/*
	 * Now we know the directory containing 'record' and
	 * may do a prscore().
	 */
	if(argc > 1 && !strncmp(argv[1], "-s", 2)) {
		if(dir) chdirx(dir);
		prscore(argc, argv);
		hackexit(0);
	}
#endif
   /*
    * It seems he really wants to play. Find the creation date of
    * this game so as to avoid restoring outdated savefiles.
    */
   gethdate(hname);

   /*
    * We cannot do chdir earlier, otherwise gethdate will fail.
    */
   if(dir) chdirx(dir);
#ifdef GRAPHICS
   InitGraphics();
#endif
   /*
    * Who am i? Perhaps we should use $USER instead?
    */
#ifdef AMIGA
   if (!*plname)
#endif
	(void) strncpy(plname, getlogin(), sizeof(plname)-1);

	/*
	 * Process options.
	 */
	initoptions();
	while(argc > 1 && argv[1][0] == '-'){
		argv++;
		argc--;
		switch(argv[0][1]){
#ifdef WIZARD
		case 'w':
			if(!strcmp(getlogin(), WIZARD))
				wizard = TRUE;
			else myprintf("Sorry.\n");
			break;
#endif WIZARD
#ifdef NEWS
		case 'n':
			flags.nonews = TRUE;
			break;
#endif NEWS
		case 'u':
			if(argv[0][2])
			(void) strncpy(plname, argv[0]+2, sizeof(plname)-1);
			else if(argc > 1) {
				argc--;
				argv++;
			(void) strncpy(plname, argv[0], sizeof(plname)-1);
			} else
				myprintf("Player name expected after -u\n");
			break;
		default:
			myprintf("Unknown option: %s\n", *argv);
		}
	}

	if(argc > 1)
		locknum = atoi(argv[1]);
#ifdef WIZARD
	if(wizard) (void) strcpy(plname, "wizard"); else
#endif WIZARD
	if(!*plname || !strncmp(plname, "player", 4)) askname();
#ifdef AMIGA
	if (!pl_character[0])
#endif
	plnamesuffix();      /* strip suffix from name */

	setbuf(stdout,obuf);
	(void) srand(getpid());
	startup();
	cls();
	(void) signal(SIGHUP, hangup);
#ifdef WIZARD
	if(!wizard) {
#endif WIZARD
		(void) signal(SIGQUIT,SIG_IGN);
		(void) signal(SIGINT,SIG_IGN);
		if(locknum)
			lockcheck();
		else
			(void) strcpy(lock,plname);
#ifdef WIZARD
	} else {
		register char *sfoo;
		(void) strcpy(lock,plname);
#ifndef AMIGA
      if(sfoo = getenv("MAGIC"))
         while(*sfoo) {
            switch(*sfoo++) {
            case 'n': (void) srand(*sfoo++);
               break;
            }
         }
      if(sfoo = getenv("GENOCIDED")){
         if(*sfoo == '!'){
            extern struct permonst mons[PMONCOUNT];
            extern char genocided[], fut_geno[];
            register struct permonst *pm = mons;
            register char *gp = genocided;

            while(pm < mons+CMNUM+2){
               if(!index(sfoo, pm->mlet))
                  *gp++ = pm->mlet;
               pm++;
            }
            *gp = 0;
         } else
            (void) strcpy(genocided, sfoo);
         (void) strcpy(fut_geno, genocided);
      }
#endif
   }
#endif WIZARD
   u.uhp = 1;   /* prevent RIP on early quits */
   u.ux = FAR;   /* prevent nscr() */
   (void) strcat(SAVEF,plname);
   if((fd = open(SAVEF,0)) >= 0 &&
      (uptodate(fd) || unlink(SAVEF) == 666)) {
      (void) signal(SIGINT,done1);
      myputs("Restoring old save file...");
      (void) myfflush(stdout);
      dorecover(fd);
      flags.move = 0;
   } else {
#ifdef NEWS
	if(!flags.nonews)
		if((fd = open(NEWS,0)) >= 0)
			outnews(fd);
#endif NEWS
      flags.ident = 1;
      init_objects(0);
      u_init();
      (void) signal(SIGINT,done1);
      glo(1);
      mklev();
      u.ux = xupstair;
      u.uy = yupstair;
      (void) inshop();
      setsee();
      flags.botlx = 1;
      makedog();
      seemons();
      docrt();
      pickup();
      read_engr_at(u.ux,u.uy);   /* superfluous ? */
      flags.move = 1;
      flags.cbreak = ON;
      flags.echo = OFF;
   }
   setftty();
#ifdef TRACK
   initrack();
#endif TRACK
   for(;;) {
      if(flags.move) {
#ifdef TRACK
         settrack();
#endif TRACK
         if(moves%2 == 0 ||
           (!(Fast & ~INTRINSIC) && (!Fast || rn2(3)))) {
            extern struct monst *makemon();
            movemon();
            if(!rn2(70))
                (void) makemon((struct permonst *)0, 0, 0);
         }
         if(Glib) glibr();
         timeout();
			++moves;
			if(flags.time) flags.botl = 1;
			if(u.uhp < 1) {
				pline("You die...");
				done("died");
				}
         if(u.uhp*10 < u.uhpmax && moves-wailmsg > 50){
             wailmsg = moves;
             if(u.uhp == 1)
             pline("You hear the wailing of the Banshee...");
             else
             pline("You hear the howling of the CwnAnnwn...");
         }
         if(u.uhp < u.uhpmax) {
            if(u.ulevel > 9) {
               if(Regeneration || !(moves%3)) {
                   flags.botl = 1;
                   u.uhp += rnd((int) u.ulevel-9);
                   if(u.uhp > u.uhpmax)
                  u.uhp = u.uhpmax;
               }
            } else if(Regeneration ||
               (!(moves%(22-u.ulevel*2)))) {
               flags.botl = 1;
               u.uhp++;
            }
         }
         if(Teleportation && !rn2(85)) tele();
         if(Searching && multi >= 0) (void) dosearch();
         gethungry();
         invault();
      }
      if(multi < 0) {
         if(!++multi){
            pline(nomovemsg ? nomovemsg :
               "You can move again.");
            nomovemsg = 0;
            if(afternmv) (*afternmv)();
            afternmv = 0;
         }
      }
      flags.move = 1;
      find_ac();
#ifndef QUEST
      if(!flags.mv || Blind)
#endif QUEST
      {
         seeobjs();
         seemons();
         nscr();
      }
      if(flags.botl || flags.botlx) bot();
      if(multi > 0) {
#ifdef QUEST
         if(flags.run >= 4) finddir();
#endif QUEST
         lookaround();
         if(!multi) {   /* lookaround may clear multi */
            flags.move = 0;
            continue;
         }
         if(flags.mv) {
            if(multi<COLNO && !--multi)
               flags.mv = flags.run = 0;
            domove();
         } else {
            --multi;
            rhack(save_cm);
         }
		} else if(multi == 0)
			rhack((char *) 0);
		if(multi && multi%7 == 0)
			(void) fflush(stdout);
   }
}

lockcheck()
{
/*   extern int errno;                         */
/*   register int i, fd;                       */
/*                                             */
/* we ignore QUIT and INT at this point        */
/*    if (link(perm,safelock) == -1)           */
/*        error("Cannot link safelock. (Try again or rm safelock.)");*/
/*                                             */
/*                                             */
/*    for(i = 0; i < locknum; i++) {           */
/*       lock[0]= 'a' + i;                     */
/*       if((fd = open(lock,0)) == -1) {       */
/*          if(errno == ENOENT) goto gotlock;  */  /* no such file */
/*          (void) unlink(safelock);           */
/*          error("Cannot open %s", lock);	*/
/*       }					*/
/*       (void) close(fd);			*/
/*    }						*/
/*     (void) unlink(safelock);			*/
/*   error("Too many hacks running now.");	*/
/*	}					*/
/* gotlock:					*/
/*    fd = creat(lock,FMASK);                  */
/*	if(unlink(safelock) == -1) {		*/
/*		error("Cannot unlink safelock.");*/
/*    if(fd == -1) {                           */
/*       error("cannot creat lock file.");     */
/*    } else {                                 */
/*       int pid;                              */
/*                                             */
/*       pid = getpid();                       */ 
/*		if(write(fd, (char *) &pid, sizeof(pid)) != sizeof(pid)){ */
/*          error("cannot write lock");        */
/*       }                                     */
/*	if(close(fd) == -1) {			*/
/*          error("cannot close lock");        */
/*       }                                     */
/*    }                                        */
}

/*VARARGS1*/
error(s,a1,a2,a3,a4) char *s,*a1,*a2,*a3,*a4;
   {
   myprintf("Error: ");
   myprintf(s,a1,a2,a3,a4);
   (void) myputchar('\n');
   hackexit(1);
   }

glo(foo)
register int foo;
{
   /* construct the string  xlock.n  */
   register char *tf;

   tf = lock;
   while(*tf && *tf!='.') tf++;
   (void) sprintf(tf, ".%d", foo);
}

/*
 * plname is filled either by an option (-u Player  or  -uPlayer) or
 * explicitly (-w implies wizard) or by askname.
 * It may still contain a suffix denoting pl_character.
 */
askname(){
register int c,ct;
	myprintf("\nWho are you? ");
	ct = 0;
	(void) myfflush();
	while((c = inchar()) != '\n')
		{
		if (c != '-')
			if (c == 8) { /* backspace */
				if (ct) {
					ct--;
					backsp();
					myputchar(' ');
					backsp();
					myfflush();
					}
			continue;
			}
		else
            if (c < 'A' || (c > 'Z' && c < 'a') || c > 'z') c = '_';
               if (ct < sizeof(plname)-1)
                  {
                  plname[ct++] = c;
                  myprintf("%c", c);
                  }
	(void) myfflush();
      }
   plname[ct] = 0;
   if(ct == 0) askname();
#ifdef QUEST
   else myprintf("Hello %s, welcome to quest!\n", plname);
#else
   else myprintf("Hello %s, welcome to hack!\n", plname);
#endif QUEST
}

impossible(){
   pline("Program in disorder - perhaps you'd better Quit");
}

#ifdef NEWS
int stopnews;

stopnws(){
   (void) signal(SIGINT, SIG_IGN);
   stopnews++;
}

outnews(fd) int fd; {
int (*prevsig)();
char ch;
   prevsig = signal(SIGINT, stopnws);
   while(!stopnews && read(fd,&ch,1) == 1)
      (void) myputchar(ch);
   (void) myputchar('\n');
   (void) myfflush(stdout);
   (void) close(fd);
   (void) signal(SIGINT, prevsig);
   /* See whether we will ask TSKCFW: he might have told us already */
   if(!stopnews && pl_character[0])
      getret();
}
#endif NEWS

chdirx(dir) char *dir; {
   if(chdir(dir) < 0) {
      perror(dir);
      error("Cannot chdir to %s.", dir);
   }
}
#file hack.makemon.c
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* hack.makemon.c version 1.0.1 - newly created demons do not sleep */

#include	"hack.h"
extern char fut_geno[];

extern char *index();

struct monst zeromonst;

/*
 * called with [x,y] = coordinates;
 *	[0,0] means anyplace
 *	[u.ux,u.uy] means: call mnexto (not in MKLEV)
 *
 *	In case we make an Orc or killer bee, we make an entire horde (swarm);
 *	note that in this case we return only one of them (the one at [x,y]).
 */
struct monst *
makemon(ptr,x,y)
register struct permonst *ptr;
{
	register struct monst *mtmp;
	register int tmp, ct;
	boolean anything = (!ptr);

	if(x != 0 || y != 0) if(m_at(x,y)) return((struct monst *) 0);
	if(ptr){
		if(index(fut_geno, ptr->mlet)) return((struct monst *) 0);
	} else {
		ct = CMNUM - strlen(fut_geno);
		if(index(fut_geno, 'm')) ct++;  /* make only 1 minotaur */
		if(index(fut_geno, '@')) ct++;
		if(ct <= 0) return(0); 		  /* no more monsters! */
		tmp = rn2(ct*dlevel/24 + 7);
		if(tmp < dlevel - 4) tmp = rn2(ct*dlevel/24 + 12);
		if(tmp >= ct) tmp = rn1(ct - ct/2, ct/2);
		for(ct = 0; ct < CMNUM; ct++){
			ptr = &mons[ct];
			if(index(fut_geno, ptr->mlet))
				continue;
			if(!tmp--) goto gotmon;
		}
		panic("makemon?");
	}
gotmon:
	mtmp = newmonst(ptr->pxlth);
	*mtmp = zeromonst;	/* clear all entries in structure */
	for(ct = 0; ct < ptr->pxlth; ct++)
		((char *) &(mtmp->mextra[0]))[ct] = 0;
	mtmp->nmon = fmon;
	fmon = mtmp;
	mtmp->m_id = flags.ident++;
	mtmp->data = ptr;
	mtmp->mxlth = ptr->pxlth;
	if(ptr->mlet == 'D') mtmp->orig_hp = mtmp->mhp = 80;
	else if(!ptr->mlevel) mtmp->orig_hp = mtmp->mhp = rnd(4);
	else mtmp->orig_hp = mtmp->mhp = d(ptr->mlevel, 8);
	mtmp->mx = x;
	mtmp->my = y;
	mtmp->mcansee = 1;
	if(ptr->mlet == 'M')
		mtmp->mimic = ']';
/*        if (!ismklev)
           { */
	   if(x == u.ux && y == u.uy)
		mnexto(mtmp);
	   if(x == 0 && y == 0)
		rloc(mtmp);
/*           }*/
	if(ptr->mlet == 's' || ptr->mlet == 'S') {
		mtmp->mhide = mtmp->mundetected = 1;
		if(ismklev && mtmp->mx && mtmp->my)
			mkobj_at(0, mtmp->mx, mtmp->my);
	}
	if(ptr->mlet == ':') {
		mtmp->cham = 1;
                if (ismklev)
		   (void) newcham(mtmp, &mons[dlevel+14+rn2(CMNUM-14-dlevel)]);
	}
	if(ptr->mlet == 'I') mtmp->minvis = 1;
	if(ptr->mlet == 'L' || ptr->mlet == 'N'
		||(ismklev && ptr->mlet != '&' && ptr->mlet != 'w' && rn2(5))
	) mtmp->msleep = 1;

#ifndef NOWORM
/*        if (!ismklev) */
	   if(ptr->mlet == 'w' && getwn(mtmp))
		initworm(mtmp);
#endif NOWORM

	if(anything) if(ptr->mlet == 'O' || ptr->mlet == 'k') {
		coord enexto();
		coord mm;
		register int cnt = rnd(10);
		mm.x = x;
		mm.y = y;
		while(cnt--) {
			mm = enexto(mm.x, mm.y);
			(void) makemon(ptr, mm.x, mm.y);
		}
	}

	return(mtmp);
}

coord
enexto(xx,yy)
register xchar xx,yy;
{
	register xchar x,y;
	coord foo[15], *tfoo;
	int range;

	tfoo = foo;
	range = 1;
	do {	/* full kludge action. */
		for(x = xx-range; x <= xx+range; x++)
			if(goodpos(x, yy-range)) {
				tfoo->x = x;
				(tfoo++)->y = yy-range;
				if(tfoo == &foo[15]) goto foofull;
			}
		for(x = xx-range; x <= xx+range; x++)
			if(goodpos(x,yy+range)) {
				tfoo->x = x;
				(tfoo++)->y = yy+range;
				if(tfoo == &foo[15]) goto foofull;
			}
		for(y = yy+1-range; y < yy+range; y++)
			if(goodpos(xx-range,y)) {
				tfoo->x = xx-range;
				(tfoo++)->y = y;
				if(tfoo == &foo[15]) goto foofull;
			}
		for(y = yy+1-range; y < yy+range; y++)
			if(goodpos(xx+range,y)) {
				tfoo->x = xx+range;
				(tfoo++)->y = y;
				if(tfoo == &foo[15]) goto foofull;
			}
		range++;
	} while(tfoo == foo);
foofull:
	return( foo[rn2(tfoo-foo)] );
}

goodpos(x,y)	/* used only in mnexto and rloc */
{
	return(
	! (x < 1 || x > COLNO-2 || y < 1 || y > ROWNO-2 ||
	   m_at(x,y) || levl[x][y].typ < DOOR
	   || (ismklev && x == u.ux && y == u.uy)
	   || (ismklev && sobj_at(ENORMOUS_ROCK, x, y))
	));
}
#file hack.mhitu.c
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* hack.mhitu.c version 1.0.1 - corrected bug for 'R' (Mike Newton)
			      - also some separate code for swallowed (a3) */
#include	"hack.h"
extern struct monst *makemon();

/*
 * mhitu: monster hits you
 *	  returns 1 if monster dies (e.g. 'y', 'F'), 0 otherwise
 */
mhitu(mtmp)
register struct monst *mtmp;
{
	register struct permonst *mdat = mtmp->data;
	register int tmp, ctmp;

	nomul(0);

	/* If swallowed, can only be affected by hissers and by u.ustuck */
	if(u.uswallow) {
		if(mtmp != u.ustuck && mdat->mlet != 'c')
			return(0);
		switch(mdat->mlet) {
		case 'c':
			if(!rn2(13)) {
				pline("Outside, you hear %s's hissing!",
					monnam(mtmp));
				pline("%s gets turned to stone!",
					Monnam(u.ustuck));
				pline("And the same fate befalls you.");
				done_in_by(mtmp);
			}
			break;
		case ',':
			youswld(mtmp,4+u.uac,5,"The trapper");
			break;
		case '\'':
			youswld(mtmp,rnd(6),7,"The lurker above");
			break;
		case 'P':
			youswld(mtmp,d(2,4),12,"The purple worm");
			break;
		default:
			pline("The mysterious monster digests you.");
			u.uhp = 0;
		}
		if(u.uhp < 1) done_in_by(mtmp);
		return(0);
	}
	if(!index("&DuxynNF",mdat->mlet))
		tmp = hitu(mtmp,d(mdat->damn,mdat->damd));
	else
		tmp = 0;

	ctmp = tmp && !mtmp->mcan &&
	  (!uarm || objects[uarm->otyp].a_can < rnd(3) || !rn2(50));
	switch(mdat->mlet) {
	case '&':
		if(!mtmp->cham && !mtmp->mcan && !rn2(13)) {
			(void) makemon(PM_DEMON,u.ux,u.uy);
		} else {
			(void) hitu(mtmp,d(2,6));
			(void) hitu(mtmp,d(2,6));
			(void) hitu(mtmp,rnd(3));
			(void) hitu(mtmp,rnd(3));
			(void) hitu(mtmp,rn1(4,2));
		}
		break;
	case ',':
		if(tmp) justswld(mtmp,"The trapper");
		break;
	case '\'':
		if(tmp) justswld(mtmp,"The lurker above");
		break;
	case 'A':
		if(ctmp && rn2(2)) {
			pline("You feel weaker!");
			losestr(1);
		}
		break;
	case 'C':
		(void) hitu(mtmp,rnd(6));
		break;
	case 'c':
		if(!rn2(5)) {
			pline("You hear %s's hissing!", monnam(mtmp));
			if(ctmp || !rn2(5)) {
				pline("You get turned to stone!");
				done_in_by(mtmp);
			}
		}
		break;
	case 'D':
		if(rn2(6) || mtmp->mcan) {
			(void) hitu(mtmp,d(3,10));
			(void) hitu(mtmp,rnd(8));
			(void) hitu(mtmp,rnd(8));
			break;
		}
		kludge("%s breathes fire!","The dragon");
		buzz(-1,mtmp->mx,mtmp->my,u.ux-mtmp->mx,u.uy-mtmp->my);
		break;
	case 'd':
		(void) hitu(mtmp,d(2,4));
		break;
	case 'e':
		(void) hitu(mtmp,d(3,6));
		break;
	case 'F':
		if(mtmp->mcan) break;
		kludge("%s explodes!","The freezing sphere");
		if(Cold_resistance) pline("You don't seem affected by it.");
		else {
			xchar dn;
			if(17-(u.ulevel/2) > rnd(20)) {
				pline("You get blasted!");
				dn = 6;
			} else {
				pline("You duck the blast...");
				dn = 3;
			}
			losehp_m(d(dn,6), mtmp);
		}
		mondead(mtmp);
		return(1);
	case 'g':
		if(ctmp && multi >= 0 && !rn2(6)) {
			kludge("You are frozen by %ss juices","the cube'");
			nomul(-rnd(10));
		}
		break;
	case 'h':
		if(ctmp && multi >= 0 && !rn2(5)) {
			nomul(-rnd(10));
			kludge("You are put to sleep by %ss bite!",
				"the homunculus'");
		}
		break;
	case 'j':
		tmp = hitu(mtmp,rnd(3));
		tmp &= hitu(mtmp,rnd(3));
		if(tmp){
			(void) hitu(mtmp,rnd(4));
			(void) hitu(mtmp,rnd(4));
		}
		break;
	case 'k':
		if((hitu(mtmp,rnd(4)) || !rn2(3)) && ctmp){
			poisoned("bee's sting",mdat->mname);
		}
		break;
	case 'L':
		if(tmp) stealgold(mtmp);
		break;
	case 'N':
		if(mtmp->mcan && !Blind) {
	pline("%s tries to seduce you, but you seem not interested.",
			Amonnam(mtmp, "plain"));
			if(rn2(3)) rloc(mtmp);
		} else if(steal(mtmp)) {
			rloc(mtmp);
			mtmp->mflee = 1;
		}
		break;
	case 'n':
		if(!uwep && !uarm && !uarmh && !uarms && !uarmg) {
		    pline("%s hits! (I hope you don't mind)",
			Monnam(mtmp));
			u.uhp += rnd(7);
			if(!rn2(7)) u.uhpmax++;
			if(u.uhp > u.uhpmax) u.uhp = u.uhpmax;
			flags.botl = 1;
			if(!rn2(50)) rloc(mtmp);
		} else {
			(void) hitu(mtmp,d(2,6));
			(void) hitu(mtmp,d(2,6));
		}
		break;
	case 'o':
		tmp = hitu(mtmp,rnd(6));
		if(hitu(mtmp,rnd(6)) && ctmp &&
		    !u.ustuck && rn2(2)) {
			u.ustuck = mtmp;
			kludge("%s has grabbed you!","The owlbear");
			u.uhp -= d(2,8);
		} else if(u.ustuck == mtmp) {
			u.uhp -= d(2,8);
			pline("You are being crushed.");
		}
		break;
	case 'P':
		if(ctmp && !rn2(4))
			justswld(mtmp,"The purple worm");
		else
			(void) hitu(mtmp,d(2,4));
		break;
	case 'Q':
		(void) hitu(mtmp,rnd(2));
		(void) hitu(mtmp,rnd(2));
		break;
	case 'R':
		if(ctmp && uarmh && !uarmh->rustfree &&
		    (int) uarmh->spe >= -1) {
			pline("Your helmet rusts!");
			uarmh->spe--;
		} else
			if(ctmp && uarm && !uarm->rustfree &&
			 uarm->otyp < STUDDED_LEATHER_ARMOR &&
			 (int)uarm->spe >= -1) {
				pline("Your armor rusts!");
				uarm->spe--;
			}
		break;
	case 'S':
		if(ctmp && !rn2(8)) {
			poisoned("snake's bite",mdat->mname);
		}
		break;
	case 's':
		if(tmp && !rn2(8)) {
			poisoned("scorpion's sting",mdat->mname);
		}
		(void) hitu(mtmp,rnd(8));
		(void) hitu(mtmp,rnd(8));
		break;
	case 'T':
		(void) hitu(mtmp,rnd(6));
		(void) hitu(mtmp,rnd(6));
		break;
	case 't':
		if(!rn2(5)) rloc(mtmp);
		break;
	case 'u':
		mtmp->mflee = 1;
		break;
	case 'U':
		(void) hitu(mtmp,d(3,4));
		(void) hitu(mtmp,d(3,4));
		break;
	case 'v':
		if(ctmp && !u.ustuck) u.ustuck = mtmp;
		break;
	case 'V':
		if(tmp) u.uhp -= 4;
		if(ctmp && !rn2(3)) losexp();
		break;
	case 'W':
		if(ctmp && !rn2(5)) losexp();
		break;
#ifndef NOWORM
	case 'w':
		if(tmp) wormhit(mtmp);
#endif NOWORM
		break;
	case 'X':
		(void) hitu(mtmp,rnd(5));
		(void) hitu(mtmp,rnd(5));
		(void) hitu(mtmp,rnd(5));
		break;
	case 'x':
		{ register long side = rn2(2) ? RIGHT_SIDE : LEFT_SIDE;
		  pline("%s pricks in your %s leg!",
			Monnam(mtmp), (side == RIGHT_SIDE) ? "right" : "left");
		  Wounded_legs |= side + rnd(5);
		  losehp_m(2, mtmp);
		  break;
		}
	case 'y':
		if(mtmp->mcan) break;
		mondead(mtmp);
		if(!Blind) {
			pline("You are blinded by a blast of light!");
			Blind = d(4,12);
			seeoff(0);
		}
		return(1);
	case 'Y':
		(void) hitu(mtmp,rnd(6));
		break;
	}
	if(u.uhp < 1) done_in_by(mtmp);
	return(0);
}
