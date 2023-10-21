
#include "hack.h"
#ifdef QUEST
int shlevel = 0;
struct monst *shopkeeper = 0;
struct obj *billobjs = 0;
obfree(obj,merge) register struct obj *obj, *merge; {
	free((char *) obj);
}
inshop(){ return(0); }
addtobill(){}
subfrombill(){}
splitbill(){}
dopay(){}
paybill(){}
doinvbill(){}
shkdead(){}
shk_move(){ return(0); }
setshk(){}
char *shkname(){ return(""); }

#else
#include	"hack.mfndpos.h"
#include	"def.eshk.h"

#define	ESHK	((struct eshk *)(&(shopkeeper->mextra[0])))
#define	NOTANGRY	shopkeeper->mpeaceful
#define	ANGRY	!NOTANGRY

extern char plname[];
extern struct obj *o_on();
struct monst *shopkeeper = 0;
struct bill_x *bill;
int shlevel = 0;	/* level of this shopkeeper */
struct obj *billobjs;	/* objects on bill with bp->useup */
/* #define	billobjs	shopkeeper->minvent
   doesnt work so well, since we do not want these objects to be dropped
   when the shopkeeper is killed.
   (See also the save and restore routines.)
 */

/* invariants: obj->unpaid iff onbill(obj) [unless bp->useup]
		obj->quan <= bp->bquan
 */

long int total;

char shtypes[] = "=/)%?!["; /* 8 shoptypes: 7 specialized, 1 mixed */
char *shopnam[] = {
	"engagement ring", "walking cane", "antique weapon",
	"delicatessen", "second hand book", "liquor",
	"used armor", "assorted antiques"
};

char *
shkname() {
	return(ESHK->shknam);
}

shkdead(){
	rooms[ESHK->shoproom].rtype = 0;
	setpaid();
	shopkeeper = 0;
	bill = (struct bill_x *) -1000;	/* dump core when referenced */
}

setpaid(){	/* caller has checked that shopkeeper exists */
register struct obj *obj;
	for(obj = invent; obj; obj = obj->nobj)
		obj->unpaid = 0;
	for(obj = fobj; obj; obj = obj->nobj)
		obj->unpaid = 0;
	while(obj = billobjs){
		billobjs = obj->nobj;
		free((char *) obj);
	}
	ESHK->billct = 0;
}

addupbill(){	/* delivers result in total */
		/* caller has checked that shopkeeper exists */
register int ct = ESHK->billct;
register struct bill_x *bp = bill;
	total = 0;
	while(ct--){
		total += bp->price * bp->bquan;
		bp++;
	}
}

inshop(){
register int tmp = inroom(u.ux,u.uy);
	if(tmp < 0 || rooms[tmp].rtype < 8) {
		u.uinshop = 0;
		if(shopkeeper && ESHK->billct){
			pline("Somehow you escaped the shop without paying!");
			addupbill();
			pline("You stole for a total worth of %lu zorkmids.",
				total);
			ESHK->robbed += total;
			setpaid();
		}
		if(tmp >= 0 && rooms[tmp].rtype == 7){
			register struct monst *mtmp;
			pline("Welcome to David's treasure zoo!");
			rooms[tmp].rtype = 0;
			for(mtmp = fmon; mtmp; mtmp = mtmp->nmon)
				if(!rn2(4)) mtmp->msleep = 0;
		}
	} else {
		if(shlevel != dlevel) setshk();
		if(!shopkeeper) u.uinshop = 0;
		else if(!u.uinshop){
			if(!ESHK->visitct ||
				strncmp(ESHK->customer, plname, PL_NSIZ)){
				/* He seems to be new here */
				ESHK->visitct = 0;
				(void) strncpy(ESHK->customer,plname,PL_NSIZ);
				NOTANGRY = 1;
			}
			pline("Hello %s! Welcome%s to %s's %s shop!",
				plname,
				ESHK->visitct++ ? " again" : "",
				shkname(),
				shopnam[rooms[ESHK->shoproom].rtype - 8] );
			u.uinshop = 1;
		}
	}
	return(u.uinshop);
}

setshk(){
register struct monst *mtmp;
	for(mtmp = fmon; mtmp; mtmp = mtmp->nmon) if(mtmp->isshk){
		shopkeeper = mtmp;
		bill = &(ESHK->bill[0]);
		shlevel = dlevel;
		if(ANGRY && strncpy(ESHK->customer,plname,PL_NSIZ))
			NOTANGRY = 1;
		billobjs = 0;
		return;
	}
	shopkeeper = 0;
	bill = (struct bill_x *) -1000;	/* dump core when referenced */
}

struct bill_x *
onbill(obj) register struct obj *obj; {
register struct bill_x *bp;
	if(!shopkeeper) return(0);
	for(bp = bill; bp < &bill[ESHK->billct]; bp++)
		if(bp->bo_id == obj->o_id) {
			if(!obj->unpaid) pline("onbill: paid obj on bill?");
			return(bp);
		}
	if(obj->unpaid) pline("onbill: unpaid obj not on bill?");
	return(0);
}

/* called with two args on merge */
obfree(obj,merge) register struct obj *obj, *merge; {
register struct bill_x *bp = onbill(obj);
register struct bill_x *bpm;
	if(bp) {
		if(!merge){
			bp->useup = 1;
			obj->unpaid = 0;	/* only for doinvbill */
			obj->nobj = billobjs;
			billobjs = obj;
			return;
		}
		bpm = onbill(merge);
		if(!bpm){
			/* this used to be a rename */
			impossible();
			return;
		} else {
			/* this was a merger */
			bpm->bquan += bp->bquan;
			ESHK->billct--;
			*bp = bill[ESHK->billct];
		}
	}
	free((char *) obj);
}

pay(tmp) long tmp; {
	u.ugold -= tmp;
	shopkeeper->mgold += tmp;
	flags.botl = 1;
}

dopay(){
long ltmp;
register struct bill_x *bp;
int shknear = (shlevel == dlevel && shopkeeper &&
	dist(shopkeeper->mx,shopkeeper->my) < 3);
int pass, tmp;

	multi = 0;
	if(!inshop() && !shknear) {
		pline("You are not in a shop.");
		return(0);
	}
	if(!shknear &&
	    inroom(shopkeeper->mx,shopkeeper->my) != ESHK->shoproom){
		pline("There is nobody here to receive your payment.");
		return(0);
	}
	if(!ESHK->billct){
		pline("You do not owe %s anything.", monnam(shopkeeper));
		if(!u.ugold){
			pline("Moreover, you have no money.");
			return(1);
		}
		if(ESHK->robbed){
			pline("But since the shop has been robbed recently,");
			pline("you %srepay %s's expenses.",
				(u.ugold < ESHK->robbed) ? "partially " : "",
				monnam(shopkeeper));
			pay((u.ugold<ESHK->robbed) ? u.ugold : ESHK->robbed);
			ESHK->robbed = 0;
			return(1);
		}
		if(ANGRY){
			pline("But in order to appease %s,",
				amonnam(shopkeeper, "angry"));
			if(u.ugold >= 1000){
				ltmp = 1000;
				pline(" you give him 1000 gold pieces.");
			} else {
				ltmp = u.ugold;
				pline(" you give him all your money.");
			}
			pay(ltmp);
			if(rn2(3)){
				pline("%s calms down.", Monnam(shopkeeper));
				NOTANGRY = 1;
			} else	pline("%s is as angry as ever.",
					Monnam(shopkeeper));
		}
		return(1);
	}
	for(pass = 0; pass <= 1; pass++) {
		tmp = 0;
		while(tmp < ESHK->billct) {
			bp = &bill[tmp];
			if(!pass && !bp->useup) {
				tmp++;
				continue;
			}
			if(!dopayobj(bp)) return(1);
			bill[tmp] = bill[--ESHK->billct];
		}
	}
	pline("Thank you for shopping in %s's %s store!",
		shkname(),
		shopnam[rooms[ESHK->shoproom].rtype - 8]);
	NOTANGRY = 1;
	return(1);
}

/* return 1 if paid successfully */
/*        0 if not enough money */
/*       -1 if object could not be found (but was paid) */
dopayobj(bp) register struct bill_x *bp; {
register struct obj *obj;
long ltmp;

	/* find the object on one of the lists */
	if(bp->useup)
		obj = o_on(bp->bo_id, billobjs);
	else if(!(obj = o_on(bp->bo_id, invent)) &&
		!(obj = o_on(bp->bo_id, fobj)) &&
		!(obj = o_on(bp->bo_id, fcobj))) {
		    register struct monst *mtmp;
		    for(mtmp = fmon; mtmp; mtmp = mtmp->nmon)
			if(obj = o_on(bp->bo_id, mtmp->minvent))
			    break;
		    for(mtmp = fallen_down; mtmp; mtmp = mtmp->nmon)
			if(obj = o_on(bp->bo_id, mtmp->minvent))
			    break;
		}
	if(!obj) {
		pline("Shopkeeper administration out of order.");
		impossible();
		setpaid();	/* be nice to the player */
		return(0);
	}

	if(!obj->unpaid && !bp->useup){
		pline("Paid object on bill??");
		impossible();
		return(1);
	}
	obj->unpaid = 0;
	ltmp = bp->price * bp->bquan;
	if(ANGRY) ltmp += ltmp/3;
	if(u.ugold < ltmp){
		pline("You don't have gold enough to pay %s.",
			doname(obj));
		obj->unpaid = 1;
		return(0);
	}
	pay(ltmp);
	pline("You bought %s for %ld gold piece%s.",
		doname(obj), ltmp, (ltmp == 1) ? "" : "s");
	if(bp->useup) {
		register struct obj *otmp = billobjs;
		if(obj == billobjs)
			billobjs = obj->nobj;
		else {
			while(otmp && otmp->nobj != obj) otmp = otmp->nobj;
			if(otmp) otmp->nobj = obj->nobj;
			else pline("Error in shopkeeper administration");
		}
		free((char *) obj);
	}
	return(1);
}

/* routine called after dying (or quitting) with nonempty bill */
paybill(){
	if(shopkeeper && ESHK->billct){
		addupbill();
		if(total > u.ugold){
			shopkeeper->mgold += u.ugold;
			u.ugold = 0;
		pline("%s comes and takes all your possessions.",
			Monnam(shopkeeper));
		} else {
			u.ugold -= total;
			shopkeeper->mgold += total;
	pline("%s comes and takes the %ld zorkmids you owed him.",
		Monnam(shopkeeper), total);
		}
		setpaid();	/* in case we create bones */
	}
}

/* called in hack.c when we pickup an object */
addtobill(obj) register struct obj *obj; {
register struct bill_x *bp;
	if(!inshop() || (u.ux == ESHK->shk.x && u.uy == ESHK->shk.y) ||
		(u.ux == ESHK->shd.x && u.uy == ESHK->shd.y) ||
		onbill(obj) /* perhaps we threw it away earlier */
	) return;
	if(ESHK->billct == BILLSZ){
		pline("You got that for free!");
		return;
	}
	bp = &bill[ESHK->billct];
	bp->bo_id = obj->o_id;
	bp->bquan = obj->quan;
	bp->useup = 0;
	bp->price = getprice(obj);
	ESHK->billct++;
	obj->unpaid = 1;
}

splitbill(obj,otmp) register struct obj *obj, *otmp; {
	/* otmp has been split off from obj */
register struct bill_x *bp;
register int tmp;
	bp = onbill(obj);
	if(!bp) { impossible(); return; }
	if(bp->bquan < otmp->quan) {
		pline("Negative quantity on bill??");
		impossible();
	}
	if(bp->bquan == otmp->quan) {
		pline("Zero quantity on bill??");
		impossible();
	}
	bp->bquan -= otmp->quan;

	/* addtobill(otmp); */
	if(ESHK->billct == BILLSZ) otmp->unpaid = 0;
	else {
		tmp = bp->price;
		bp = &bill[ESHK->billct];
		bp->bo_id = otmp->o_id;
		bp->bquan = otmp->quan;
		bp->useup = 0;
		bp->price = tmp;
		ESHK->billct++;
	}
}

subfrombill(obj) register struct obj *obj; {
long ltmp;
register int tmp;
register struct obj *otmp;
register struct bill_x *bp;
	if(!inshop() || (u.ux == ESHK->shk.x && u.uy == ESHK->shk.y) ||
		(u.ux == ESHK->shd.x && u.uy == ESHK->shd.y))
		return;
	if((bp = onbill(obj)) != 0){
		obj->unpaid = 0;
		if(bp->bquan > obj->quan){
			otmp = newobj(0);
			*otmp = *obj;
			bp->bo_id = otmp->o_id = flags.ident++;
			otmp->quan = (bp->bquan -= obj->quan);
			otmp->owt = 0;	/* superfluous */
			otmp->onamelth = 0;
			bp->useup = 1;
			otmp->nobj = billobjs;
			billobjs = otmp;
			return;
		}
		ESHK->billct--;
		*bp = bill[ESHK->billct];
		return;
	}
	if(obj->unpaid){
		pline("%s didn't notice.", Monnam(shopkeeper));
		obj->unpaid = 0;
		return;		/* %% */
	}
	/* he dropped something of his own - probably wants to sell it */
	if(shopkeeper->msleep || shopkeeper->mfroz ||
		inroom(shopkeeper->mx,shopkeeper->my) != ESHK->shoproom)
		return;
	if(ESHK->billct == BILLSZ ||
	  ((tmp = shtypes[rooms[ESHK->shoproom].rtype-8]) && tmp != obj->olet)
	  || index("_0", obj->olet)) {
		pline("%s seems not interested.", Monnam(shopkeeper));
		return;
	}
	ltmp = getprice(obj) * obj->quan;
	if(ANGRY) {
		ltmp /= 3;
		NOTANGRY = 1;
	} else	ltmp /= 2;
	if(ESHK->robbed){
		if((ESHK->robbed -= ltmp) < 0) ESHK->robbed = 0;
pline("Thank you for your contribution to restock this recently plundered shop.");
		return;
	}
	if(ltmp > shopkeeper->mgold) ltmp = shopkeeper->mgold;
	pay(-ltmp);
	if(!ltmp)
	pline("%s gladly accepts %s but cannot pay you at present.",
		Monnam(shopkeeper), doname(obj));
	else
	pline("You sold %s and got %ld gold piece%s.", doname(obj), ltmp,
		(ltmp == 1) ? "" : "s");
}

doinvbill(cl) int cl; {
register unsigned tmp,cnt = 0;
register struct obj *obj;
char buf[BUFSZ];
	if(!shopkeeper) return;
	for(tmp = 0; tmp < ESHK->billct; tmp++) if(bill[tmp].useup) cnt++;
	if(!cnt) return;
	if(!cl && !flags.oneline) cls();
	if(!flags.oneline) myputs("\n\nUnpaid articles already used up:\n");
	for(tmp = 0; tmp < ESHK->billct; tmp++) if(bill[tmp].useup){
		for(obj = billobjs; obj; obj = obj->nobj)
			if(obj->o_id == bill[tmp].bo_id) break;
		if(!obj) {
			pline("Bad shopkeeper administration.");
			impossible();
			return;
		}
		(void) sprintf(buf, "* -  %s", doname(obj));
		for(cnt=0; buf[cnt]; cnt++);
		while(cnt < 50) buf[cnt++] = ' ';
		(void) sprintf(&buf[cnt], " %5d zorkmids",
				bill[tmp].price * bill[tmp].bquan);
		if(flags.oneline)
			pline(buf);
		else
			myputs(buf);
	}
	if(!cl && !flags.oneline) {
		getret();
		docrt();
	}
}

getprice(obj) register struct obj *obj; {
register int tmp,ac;
	switch(obj->olet){
		case AMULET_SYM:
			tmp = 10*rnd(500);
			break;
		case TOOL_SYM:
			tmp = 10*rnd(150);
			break;
		case RING_SYM:
			tmp = 10*rnd(100);
			break;
		case WAND_SYM:
			tmp = 10*rnd(100);
			break;
		case SCROLL_SYM:
			tmp = 10*rnd(50);
			break;
		case POTION_SYM:
			tmp = 10*rnd(50);
			break;
		case FOOD_SYM:
			tmp = 10*rnd(5 + (2000/realhunger()));
			break;
		case GEM_SYM:
			tmp = 10*rnd(20);
			break;
		case ARMOR_SYM:
			ac = 10 - obj->spe;
			tmp = 100 + (10-ac)*(10-ac)*rnd(20-ac);
			break;
		case WEAPON_SYM:
			if(obj->otyp < BOOMERANG)
				tmp = 5*rnd(10);
			else if(obj->otyp == LONG_SWORD ||
				obj->otyp == TWO_HANDED_SWORD)
				tmp = 10*rnd(150);
			else	tmp = 10*rnd(75);
			break;
		case CHAIN_SYM:
			pline("Strange ..., carrying a chain?");
		case BALL_SYM:
			tmp = 10;
			break;
		default:
			tmp = 10000;
	}
	return(tmp);
}

realhunger(){	/* not completely foolproof */
register int tmp = u.uhunger;
register struct obj *otmp = invent;
	while(otmp){
		if(otmp->olet == FOOD_SYM && !otmp->unpaid)
			tmp += objects[otmp->otyp].nutrition;
		otmp = otmp->nobj;
	}
	return((tmp <= 0) ? 1 : tmp);
}

shk_move(){
register struct monst *mtmp;
register struct permonst *mdat = shopkeeper->data;
register xchar gx,gy,omx,omy,nx,ny,nix,niy;
register schar appr,i;
schar shkr,tmp,chi,chcnt,cnt;
boolean uondoor, avoid;
coord poss[9];
int info[9];
	omx = shopkeeper->mx;
	omy = shopkeeper->my;
	shkr = inroom(omx,omy);
	if(ANGRY && dist(omx,omy) < 3){
		(void) hitu(shopkeeper, d(mdat->damn, mdat->damd)+1);
		return(0);
	}
	appr = 1;
	gx = ESHK->shk.x;
	gy = ESHK->shk.y;
	if(ANGRY){
		long saveBlind = Blind;
		Blind = 0;
		if(shopkeeper->mcansee && !Invis && cansee(omx,omy)) {
			gx = u.ux;
			gy = u.uy;
		}
		Blind = saveBlind;
		avoid = FALSE;
	} else {
#define	GDIST(x,y)	((x-gx)*(x-gx)+(y-gy)*(y-gy))
		if(Invis)
		  avoid = FALSE;
		else {
		  uondoor = (u.ux == ESHK->shd.x && u.uy == ESHK->shd.y);
		  avoid = ((u.uinshop && dist(gx,gy) > 8) || uondoor);
		  if(((!ESHK->robbed && !ESHK->billct) || avoid)
		  	&& GDIST(omx,omy) < 3){
		  	if(!online(omx,omy)) return(0);
		  	if(omx == gx && omy == gy)
		  		appr = gx = gy = 0;
		  }
		}
	}
	if(omx == gx && omy == gy) return(0);
	if(shopkeeper->mconf) appr = 0;
	nix = omx;
	niy = omy;
	cnt = mfndpos(shopkeeper,poss,info,
		(avoid ? NOTONL : 0) | ALLOW_SSM);
	if(cnt == 0 && avoid && uondoor)
		cnt = mfndpos(shopkeeper,poss,info,ALLOW_SSM);
	chi = -1;
	chcnt = 0;
	for(i=0; i<cnt; i++){
		nx = poss[i].x;
		ny = poss[i].y;
	   	if((tmp = levl[nx][ny].typ) = ROOM ||
		(shkr != ESHK->shoproom && (tmp==CORR || tmp==DOOR)))
#ifdef STUPID
		/* cater for stupid compilers */
		{ int zz;
		if((!appr && !rn2(++chcnt)) ||
		   (appr && (zz = GDIST(nix,niy)) && zz > GDIST(nx,ny))){
#else
		if((!appr && !rn2(++chcnt)) ||
		   (appr && GDIST(nx,ny) < GDIST(nix,niy))){
#endif STUPID
			nix = nx;
			niy = ny;
			chi = i;
#ifdef STUPID
		   }
#endif STUPID
		}
	}
	if(nix != omx || niy != omy){
		if(info[chi] & ALLOW_M){
			mtmp = m_at(nix,niy);
			if(hitmm(shopkeeper,mtmp) == 1 && rn2(3) &&
			   hitmm(mtmp,shopkeeper) == 2) return(2);
			return(0);
		} else if(info[chi] & ALLOW_U){
			(void) hitu(shopkeeper, d(mdat->damn, mdat->damd)+1);
			return(0);
		}
		shopkeeper->mx = nix;
		shopkeeper->my = niy;
		pmon(shopkeeper);
		return(1);
	}
	return(0);
}
#endif QUEST

char *
plur(n) unsigned n; {
	return((n==1) ? "" : "s");
}

online(x,y) {
	return(x==u.ux || y==u.uy ||
		(x-u.ux)*(x-u.ux) == (y-u.uy)*(y-u.uy));
}
#file hack.stat.c
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1984. */

gethdate(name) char *name; {
/* register char *np;                                  */
/*    if(stat(name, &hbuf))                            */
/*       error("Cannot get status of %s.",             */
/*          (np = index(name, '/')) ? np+1 : name);    */
}

uptodate(fd) {
/*    if(fstat(fd, &buf)) {                            */
/*       pline("Cannot get status?");                  */
/*       return(0);                                    */
/*    }                                                */
/*    if(buf.st_ctime < hbuf.st_ctime) {               */
/*       pline("Saved level is out of date.");         */
/*       return(0);                                    */
/*    }                                                */
   return(1);
}
#file hack.steal.c
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1984. */

#include "hack.h"

stealgold(mtmp)  register struct monst *mtmp; {
register struct gen *gold = g_at(u.ux, u.uy, fgold);
register int tmp;
	if(gold && ( !u.ugold || gold->gflag > u.ugold || !rn2(5))) {
		mtmp->mgold += gold->gflag;
		freegold(gold);
		if(Invis) newsym(u.ux, u.uy);
		pline("%s quickly snatches some gold from between your feet!",
			Monnam(mtmp));
		if(!u.ugold || !rn2(5)) {
			rloc(mtmp);
			mtmp->mflee = 1;
		}
	} else if(u.ugold) {
		u.ugold -= (tmp = somegold());
		pline("Your purse feels lighter.");
		mtmp->mgold += tmp;
		rloc(mtmp);
		mtmp->mflee = 1;
		flags.botl = 1;
	}
}

somegold(){
	return( (u.ugold < 100) ? u.ugold :
		(u.ugold > 10000) ? rnd(10000) : rnd((int) u.ugold) );
}

/* steal armor after he finishes taking it off */
unsigned stealoid;		/* object to be stolen */
unsigned stealmid;		/* monster doing the stealing */
stealarm(){
	register struct monst *mtmp;
	register struct obj *otmp;

	for(otmp = invent; otmp; otmp = otmp->nobj)
	  if(otmp->o_id == stealoid) {
	    for(mtmp = fmon; mtmp; mtmp = mtmp->nmon)
	      if(mtmp->m_id == stealmid) {
		if(dist(mtmp->mx,mtmp->my) < 3) {
		  freeinv(otmp);
		  pline("%s steals %s!", Monnam(mtmp), doname(otmp));
		  mpickobj(mtmp,otmp);
		  mtmp->mflee = 1;
		  rloc(mtmp);
		}
		break;
	      }
	    break;
	  }
	stealoid = 0;
}

/* returns 1 when something was stolen */
/* (or at least, when N should flee now) */
/* avoid stealing the object stealoid */
steal(mtmp)
struct monst *mtmp;
{
	register struct obj *otmp;
	register int tmp;
	register int named = 0;

	if(!invent){
	    if(Blind)
	      pline("Somebody tries to rob you, but finds nothing to steal.");
	    else
	      pline("%s tries to rob you, but she finds nothing to steal!",
		Monnam(mtmp));
	    return(1);	/* let her flee */
	}
	tmp = 0;
	for(otmp = invent; otmp; otmp = otmp->nobj)
		tmp += ((otmp->owornmask & (W_ARMOR | W_RING)) ? 5 : 1);
	tmp = rn2(tmp);
	for(otmp = invent; otmp; otmp = otmp->nobj)
		if((tmp -= ((otmp->owornmask & (W_ARMOR | W_RING)) ? 5 : 1))
			< 0) break;
	if(!otmp) panic("Steal fails!");
	if(otmp->o_id == stealoid) return(0);
	if((otmp->owornmask & (W_ARMOR | W_RING))){
		switch(otmp->olet) {
		case RING_SYM:
			ringoff(otmp);
			break;
		case ARMOR_SYM:
			if(multi < 0 || otmp == uarms){
			  setworn((struct obj *) 0, otmp->owornmask & W_ARMOR);
			  break;
			}
		{ int curssv = otmp->cursed;
			otmp->cursed = 0;
			pline("%s seduces you and %s off your %s.",
				Amonnam(mtmp, Blind ? "gentle" : "beautiful"),
				otmp->cursed ? "helps you to take"
					    : "you start taking",
				(otmp == uarmg) ? "gloves" :
				(otmp == uarmh) ? "helmet" : "armor");
			named++;
			(void) armoroff(otmp);
			otmp->cursed = curssv;
			if(multi < 0){
				extern char *nomovemsg;
				extern int (*afternmv)();
				/*
				multi = 0;
				nomovemsg = 0;
				afternmv = 0;
				*/
				stealoid = otmp->o_id;
				stealmid = mtmp->m_id;
				afternmv = stealarm;
				return(0);
			}
			break;
		}
		default:
			impossible();
		}
	}
	else if(otmp == uwep)
		setuwep((struct obj *) 0);
	if(otmp->olet == CHAIN_SYM) {
		pline("How come you are carrying that chain?");
		impossible();
	}
	if(Punished && otmp == uball){
		Punished = 0;
		freeobj(uchain);
		free((char *) uchain);
		uchain = (struct obj *) 0;
		uball->spe = 0;
		uball = (struct obj *) 0;	/* superfluous */
	}
	freeinv(otmp);
	pline("%s stole %s.", named ? "She" : Monnam(mtmp), doname(otmp));
	mpickobj(mtmp,otmp);
	return((multi < 0) ? 0 : 1);
}

mpickobj(mtmp,otmp)
register struct monst *mtmp;
register struct obj *otmp;
{
	otmp->nobj = mtmp->minvent;
	mtmp->minvent = otmp;
}

/* release the objects the killed animal has stolen */
relobj(mtmp,show)
register struct monst *mtmp;
register int show;
{
	register struct obj *otmp, *otmp2;

	for(otmp = mtmp->minvent; otmp; otmp = otmp2){
		otmp->ox = mtmp->mx;
		otmp->oy = mtmp->my;
		otmp2 = otmp->nobj;
		otmp->nobj = fobj;
		fobj = otmp;
		stackobj(fobj);
		if(show & cansee(mtmp->mx,mtmp->my))
			atl(otmp->ox,otmp->oy,otmp->olet);
	}
	mtmp->minvent = (struct obj *) 0;
	if(mtmp->mgold || mtmp->data->mlet == 'L') {
		register int tmp;

		tmp = (mtmp->mgold > 10000) ? 10000 : mtmp->mgold;
		mkgold( tmp + d(dlevel,30), mtmp->mx, mtmp->my);
		if(show & cansee(mtmp->mx,mtmp->my))
			atl(mtmp->mx,mtmp->my,'$');
	}
}
#file hack.timeout.c
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1984. */



#include	"hack.h"

#define	SIZE(x)	(sizeof(x) / sizeof(x[0]))



timeout(){

register struct prop *upp;

	for(upp = u.uprops; upp < u.uprops+SIZE(u.uprops); upp++)

	    if((upp->p_flgs & TIMEOUT) && !--upp->p_flgs) {

		if(upp->p_tofn) (*upp->p_tofn)();

		else switch(upp - u.uprops){

		case SICK:

			pline("You die because of food poisoning");

			killer = u.usick_cause;

			done("died");

			/* NOTREACHED */

		case FAST:

			pline("You feel yourself slowing down");

			break;

		case CONFUSION:

			pline("You feel less confused now");

			break;

		case BLIND:

			pline("You can see again");

			setsee();

			break;

		case INVIS:

			on_scr(u.ux,u.uy);

			pline("You are no longer invisible.");

		}

	}

}

#file hack.topl.c
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1984. */

#include "hack.h"
#include <stdio.h>
extern char *eos();
#define   TOPLSZ   (COLNO-8)   /* leave room for --More-- */
char toplines[BUFSZ];
xchar tlx, tly;         /* set by pline; used by addtopl */

struct topl {
   struct topl *next_topl;
   char *topl_text;
} *old_toplines, *last_redone_topl;
#define   OTLMAX   20      /* max nr of old toplines remembered */

doredotopl(){
   if(last_redone_topl)
      last_redone_topl = last_redone_topl->next_topl;
   if(!last_redone_topl)
      last_redone_topl = old_toplines;
   if(last_redone_topl){
      (void) strcpy(toplines, last_redone_topl->topl_text);
   }
   redotoplin();
   return(0);
}

redotoplin() {
   home();
   if(index(toplines, '\n')) cl_end();
   putstr(toplines);
   cl_end();
   tlx = curx;
   tly = cury;
   flags.topl = 1;
   if(tly > 1)
      more();
}

remember_topl() {
register struct topl *tl;
register int cnt = OTLMAX;
   if(last_redone_topl &&
      !strcmp(toplines, last_redone_topl->topl_text)) return;
   if(old_toplines &&
      !strcmp(toplines, old_toplines->topl_text)) return;
   last_redone_topl = 0;
   tl = (struct topl *)
      alloc((unsigned)(strlen(toplines) + sizeof(struct topl) + 1));
   tl->next_topl = old_toplines;
   tl->topl_text = (char *)(tl + 1);
   (void) strcpy(tl->topl_text, toplines);
   old_toplines = tl;
   while(cnt && tl){
      cnt--;
      tl = tl->next_topl;
   }
   if(tl && tl->next_topl){
      free((char *) tl->next_topl);
      tl->next_topl = 0;
   }
}

addtopl(s) char *s; {
   curs(tlx,tly);
   if(tlx + strlen(s) > COLNO) putsym('\n');
   putstr(s);
   tlx = curx;
   tly = cury;
   flags.topl = 1;
}

xmore(spaceflag)
boolean spaceflag;   /* TRUE if space required */
{
   if(flags.topl) {
      curs(tlx, tly);
      if(tlx + 8 > COLNO) putsym('\n'), tly++;
   }
   putstr("--More--");
   xwaitforspace(spaceflag);
   if(flags.topl && tly > 1) {
      home();
      cl_end();
      docorner(1, tly-1);
   }
   flags.topl = 0;
}

more(){
   xmore(TRUE);
}

cmore(){
   xmore(FALSE);
}

clrlin(){
   if(flags.topl) {
      home();
      cl_end();
      if(tly > 1) docorner(1, tly-1);
      remember_topl();
   }
   flags.topl = 0;
}

/*VARARGS1*/
pline(line,arg1,arg2,arg3,arg4,arg5,arg6)
/* register */  char *line,*arg1,*arg2,*arg3,*arg4,*arg5,*arg6;
{
   char pbuf[BUFSZ];
   register char *bp = pbuf, *tl;
   register int n,n0;

   if(!line || !*line) return;
   if(!index(line, '%')) (void) strcpy(pbuf,line); else
   (void) sprintf(pbuf,line,arg1,arg2,arg3,arg4,arg5,arg6);
   if(flags.topl == 1 && !strcmp(pbuf, toplines)) return;
   nscr();      /* %% */

   /* If there is room on the line, print message on same line */
   /* But messages like "You die..." deserve their own line */
   n0 = strlen(bp);
   if(flags.topl == 1 && tly == 1 &&
       n0 + strlen(toplines) + 3 < TOPLSZ &&
       strncmp(bp, "You ", 4)) {
      (void) strcat(toplines, "  ");
      (void) strcat(toplines, bp);
      tlx += 2;
      addtopl(bp);
      return;
   }
   if(flags.topl == 1) more();
   remember_topl();
   toplines[0] = 0;
   while(n0){
      if(n0 >= COLNO){
         /* look for appropriate cut point */
         n0 = 0;
         for(n = 0; n < COLNO; n++) if(bp[n] == ' ')
            n0 = n;
         if(!n0) for(n = 0; n < COLNO-1; n++)
            if(!letter(bp[n])) n0 = n;
         if(!n0) n0 = COLNO-2;
      }
      (void) strncpy((tl = eos(toplines)), bp, n0);
      tl[n0] = 0;
      bp += n0;

      /* remove trailing spaces, but leave one */
      while(n0 > 1 && tl[n0-1] == ' ' && tl[n0-2] == ' ')
         tl[--n0] = 0;

      n0 = strlen(bp);
      if(n0 && tl[0]) (void) strcat(tl, "\n");
   }
   redotoplin();
}

putsym(c) char c; {
   switch(c) {
   case '\b':
      backsp();
      return;
   case '\n':
      curx = 1;
      cury++;
      if(cury > tly) tly = cury;
      break;
   default:
      curx++;
      if(curx == COLNO) putsym('\n');
   }
   (void) myputchar(c);
}

putstr(s) register char *s; {
   while(*s) putsym(*s++);
}

#file hack.track.c
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1984. */

#include "hack.h"
#ifdef TRACK
#define	UTSZ	50

coord utrack[UTSZ];
int utcnt = 0;
int utpnt = 0;

initrack(){
	utcnt = utpnt = 0;
}

/* add to track */
settrack(){
	if(utcnt < UTSZ) utcnt++;
	if(utpnt == UTSZ) utpnt = 0;
	utrack[utpnt].x = u.ux;
	utrack[utpnt].y = u.uy;
	utpnt++;
}

coord *
gettrack(x,y) register int x,y; {
register int i,cnt;
coord tc;
	cnt = utcnt;
	for(i = utpnt-1; cnt--; i--){
		if(i == -1) i = UTSZ-1;
		tc = utrack[i];
		if((x-tc.x)*(x-tc.x) + (y-tc.y)*(y-tc.y) < 3)
			return(&(utrack[i]));
	}
	return(0);
}
#endif TRACK
#file hack.trap.c
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* hack.trap.c version 1.0.1 - added dotele(); */

#include   "hack.h"
#include   "def.trap.h"

extern struct monst *makemon();

char vowels[] = "aeiou";

char *traps[] = {
   " bear trap",
   "n arrow trap",
   " dart trap",
   " trapdoor",
   " teleportation trap",
   " pit",
   " sleeping gas trap",
   " piercer",
   " mimic"
};

dotrap(trap) register struct gen *trap; {
   nomul(0);
   if(trap->gflag&SEEN && !rn2(5))
		pline("You escape a%s.",traps[trap->gflag & TRAPTYPE]);
   else {
      trap->gflag |= SEEN;
		switch(trap->gflag & TRAPTYPE) {
      case SLP_GAS_TRAP:
         pline("A cloud of gas puts you to sleep!");
         nomul(-rnd(25));
         break;
      case BEAR_TRAP:
         if(Levitation) {
            pline("You float over a bear trap.");
            break;
         }
         u.utrap = 4 + rn2(4);
         u.utraptype = TT_BEARTRAP;
         pline("A bear trap closes on your foot!");
         break;
      case PIERC:
         deltrap(trap);
         if(makemon(PM_PIERC,u.ux,u.uy)) {
           pline("A piercer suddenly drops from the ceiling!");
           if(uarmh)
            pline("Its blow glances off your helmet.");
           else
            (void) thitu(3,d(4,6),"falling piercer");
         }
         break;
      case ARROW_TRAP:
         pline("An arrow shoots out at you!");
         if(!thitu(8,rnd(6),"arrow")){
            mksobj_at(WEAPON_SYM, ARROW, u.ux, u.uy);
            fobj->quan = 1;
         }
         break;
      case TRAPDOOR:
         if(!xdnstair) {
pline("A trap door in the ceiling opens and a rock falls on your head!");
if(uarmh) pline("Fortunately, you are wearing a helmet!");
         losehp(uarmh ? 2 : d(2,10),"falling rock");
         } else {
             register int newlevel = dlevel + 1;
            while(!rn2(4) && newlevel < 29)
               newlevel++;
            pline("A trap door opens up under you!");
            if(Levitation || u.ustuck) {
             pline("For some reason you don't fall in.");
               break;
            }

            goto_level(newlevel, FALSE);
         }
         break;
      case DART_TRAP:
         pline("A little dart shoots out at you!");
         if(thitu(7,rnd(3),"little dart")) {
             if(!rn2(6))
            poisoned("dart","poison dart");
         } else {
            mksobj_at(WEAPON_SYM, DART, u.ux, u.uy);
            fobj->quan = 1;
         }
         break;
		case TELEP_TRAP:
			if(trap->gflag & ONCE) {
				deltrap(trap);
				newsym(u.ux,u.uy);
				vtele();
			} else {
				newsym(u.ux,u.uy);
				tele();
			}
			break;
      case PIT:
         if(Levitation) {
            pline("A pit opens up under you!");
            pline("You don't fall in!");
            break;
         }
         pline("You fall into a pit!");
         u.utrap = rn1(6,2);
         u.utraptype = TT_PIT;
         losehp(rnd(6),"fall into a pit");
         selftouch("Falling, you");
         break;
      default:
         pline("You hit a trap of type %d",trap->gflag);
         impossible();
      }
   }
}

mintrap(mtmp) register struct monst *mtmp; {
   register struct gen *gen = g_at(mtmp->mx, mtmp->my, ftrap);
   register int wasintrap = mtmp->mtrapped;

   if(!gen) {
      mtmp->mtrapped = 0;   /* perhaps teleported? */
   } else if(wasintrap) {
      if(!rn2(40)) mtmp->mtrapped = 0;
   } else {
	    register int tt = (gen->gflag & TRAPTYPE);
       int in_sight = cansee(mtmp->mx,mtmp->my);
       extern char mlarge[];
       if(mtmp->mtrapseen & (1 << tt)) {
      /* he has been in such a trap - perhaps he escapes */
      if(rn2(4)) return(0);
       }
       mtmp->mtrapseen |= (1 << tt);
       switch (tt) {
      case BEAR_TRAP:
         if(strchr(mlarge, mtmp->data->mlet)) {
            if(in_sight)
              pline("%s is caught in a bear trap!",
               Monnam(mtmp));
            else
              if(mtmp->data->mlet == 'o')
             pline("You hear the roaring of an angry bear!");
            mtmp->mtrapped = 1;
         }
         break;
      case PIT:
         if(!strchr("Eyw", mtmp->data->mlet)) {
            mtmp->mtrapped = 1;
            if(in_sight)
              pline("%s falls in a pit!", Monnam(mtmp));
         }
         break;
      case SLP_GAS_TRAP:
         if(!mtmp->msleep && !mtmp->mfroz) {
            mtmp->msleep = 1;
            if(in_sight)
              pline("%s suddenly falls asleep!",
               Monnam(mtmp));
         }
         break;
      case TELEP_TRAP:
         rloc(mtmp);
         if(in_sight && !cansee(mtmp->mx,mtmp->my))
            pline("%s suddenly disappears!",
               Monnam(mtmp));
         break;
      case ARROW_TRAP:
         if(in_sight) {
            pline("%s is hit by an arrow!",
               Monnam(mtmp));
         }
         mtmp->mhp -= 3;
         break;
      case DART_TRAP:
         if(in_sight) {
            pline("%s is hit by a dart!",
               Monnam(mtmp));
         }
         mtmp->mhp -= 2;
         /* not mondied here !! */
         break;
      case TRAPDOOR:
         if(!xdnstair) {
            mtmp->mhp -= 10;
            if(in_sight)
pline("A trap door in the ceiling opens and a rock hits %s!", monnam(mtmp));
            break;
         }
         if(mtmp->data->mlet != 'w'){
            fall_down(mtmp);
            if(in_sight)
      pline("Suddenly, %s disappears out of sight.", monnam(mtmp));
            return(2);   /* no longer on this level */
         }
         break;
      case PIERC:
         break;
      default:
         pline("Some monster encountered an impossible trap.");
         impossible();
       }
   }
   return(mtmp->mtrapped);
}

selftouch(arg) char *arg; {
   if(uwep && uwep->otyp == DEAD_COCKATRICE){
      pline("%s touch the dead cockatrice.", arg);
      pline("You turn to stone.");
      killer = objects[uwep->otyp].oc_name;
      done("died");
   }
}

float_up(){
   if(u.utrap) {
      if(u.utraptype == TT_PIT) {
         u.utrap = 0;
         pline("You float up, out of the pit!");
      } else {
         pline("You float up, only your leg is still stuck.");
      }
   } else
      pline("You start to float in the air!");
}

float_down(){
   register struct gen *trap;
   pline("You float gently to the ground.");
   if(trap = g_at(u.ux,u.uy,ftrap))
		switch(trap->gflag & TRAPTYPE) {
		case PIERC:
			break;
		case TRAPDOOR:
			if(!xdnstair || u.ustuck) break;
			/* fall into next case */
		default:
			dotrap(trap);
		}
   pickup();
}

vtele() {
#define	VAULT	6
	register struct mkroom *croom;
	for(croom = &rooms[0]; croom->hx >= 0; croom++)
	    if(croom->rtype == VAULT) {
		register int x,y;

		x = rn2(2) ? croom->lx : croom->hx;
		y = rn2(2) ? croom->ly : croom->hy;
		if(teleok(x,y)) {
		    teleds(x,y);
		    return;
		}
 	    }
	tele();
}

tele() {
	extern coord getpos();
	coord cc;
	register int nux,nuy;

   if(Teleport_control) {
      pline("To what position do you want to be teleported?");
      cc = getpos(1, "the desired position"); /* 1: force valid */
      /* possible extensions: introduce a small error if
         magic power is low; allow transfer to solid rock */
      if(teleok(cc.x, cc.y)){
			teleds(cc.x, cc.y);
			return;
      }
      pline("Sorry ...");
   }
   do {
      nux = rnd(COLNO-1);
      nuy = rn2(ROWNO);
   } while(!teleok(nux, nuy));

	teleds(nux, nuy);
}

teleds(nux, nuy)
register int nux,nuy;
{
   if(Punished) unplacebc();
   unsee();
   u.utrap = 0;
   u.ustuck = 0;
   u.ux = nux;
   u.uy = nuy;
   setsee();
   if(Punished) placebc(1);
   if(u.uswallow){
      u.uswldtim = u.uswallow = 0;
      docrt();
   }
   nomul(0);
   (void) inshop();
   pickup();
   if(!Blind) read_engr_at(u.ux,u.uy);
}

teleok(x,y) register int x,y; {
   return( isok(x,y) && levl[x][y].typ > DOOR && !m_at(x,y) &&
      !sobj_at(ENORMOUS_ROCK,x,y) && !g_at(x,y,ftrap)
   );
   /* Note: gold is permitted (because of vaults) */
}

dotele() {
	extern char pl_character[];

	if(
#ifdef WIZARD
	   !wizard &&
#endif WIZARD
		      (!Teleportation || u.ulevel < 6 ||
			(pl_character[0] != 'W' && u.ulevel < 10))) {
		pline("You are not able to teleport at will.");
		return(0);
	}
	if(u.uhunger <= 100 || u.ustr < 6) {
		pline("You miss the strength for a teleport spell.");
		return(1);
	}
	tele();
	morehungry(100);
	return(1);
}

placebc(attach) int attach; {
   if(!uchain || !uball){
      pline("Where are your chain and ball??");
      impossible();
      return;
   }
   uball->ox = uchain->ox = u.ux;
   uball->oy = uchain->oy = u.uy;
   if(attach){
      uchain->nobj = fobj;
      fobj = uchain;
      if(!carried(uball)){
         uball->nobj = fobj;
         fobj = uball;
      }
   }
}

unplacebc(){
   if(!carried(uball)){
      freeobj(uball);
      unpobj(uball);
   }
   freeobj(uchain);
   unpobj(uchain);
}

level_tele() {
register int newlevel = 5 + rn2(20);   /* 5 - 24 */
   if(dlevel == newlevel)
      if(!xdnstair) newlevel--; else newlevel++;
   goto_level(newlevel, FALSE);
}

#file hack.tty.c
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1984. */

#include   "hack.h"
#include   <stdio.h>

char inchar();

gettty(){
}

/* reset terminal to original state */
settty(s) char *s; {
   clear_screen();
   if(s) myprintf(s);
   (void) myfflush(stdout);
   flags.echo = OFF;
   flags.cbreak = OFF;
}

setctty(){
}

setftty(){
}

echo(n)
register int n;
{
}

/* always want to expand tabs, or to send a clear line char before
   printing something on topline */
xtabs()
{
}

#ifdef LONG_CMD
cbreak(n)
register int n;
{
}
#endif LONG_CMD

getlin(bufp)
register char *bufp;
{
   register char *obufp = bufp;
   register int c;

   flags.topl = 2;      /* nonempty, no --More-- required */
   for(;;) {
      (void) myfflush(stdout);
      c = inchar();
      if(c == '\b') {
         if(bufp != obufp) {
            bufp--;
            putstr("\b \b"); /* putsym converts \b */
         } else   bell();
      } else if(c == '\n') {
         *bufp = 0;
         return;
      } else {
         *bufp = c;
         bufp[1] = 0;
         putstr(bufp);
         if(bufp-obufp < BUFSZ-1 && bufp-obufp < COLNO)
            bufp++;
      }
   }
}

getret() {
   xgetret(TRUE);
}

cgetret() {
   xgetret(FALSE);
}

xgetret(spaceflag)
boolean spaceflag;   /* TRUE if space (return) required */
{
   myprintf("\nHit %s to continue: ",
      flags.cbreak ? "space" : "return");
   xwaitforspace(spaceflag);
}

char morc;   /* tell the outside world what char he used */

xwaitforspace(spaceflag)
boolean spaceflag;
{
register int c;

   (void) myfflush(stdout);
   morc = 0;

   while((c = inchar()) != '\n')
      {
      if (flags.cbreak)
         {
         if (c == ' ')
            break;
         if (!spaceflag && letter(c))
            {
            morc = c;
            break;
            }
         }
      }
   }

char *
parse()
{
   static char inline[COLNO];
   register int foo;

   flags.move = 1;
   if(!Invis) curs(u.ux,u.uy+2); else home();
   (void) myfflush(stdout);
   while((foo = inchar()) >= '0' && foo <= '9')
      multi += 10*multi+foo-'0';
   if(multi) {
      multi--;
      save_cm = inline;
   }
   inline[0] = foo;
   inline[1] = 0;
   if(foo == 'f' || foo == 'F'){
      inline[1] = inchar();
#ifdef QUEST
      if(inline[1] == foo) inline[2] = inchar(); else
#endif QUEST
      inline[2] = 0;
   }
   if(foo == 'm' || foo == 'M'){
      inline[1] = inchar();
      inline[2] = 0;
   }
   clrlin();
   return(inline);
}

char
readchar() {
   register int sym;
   (void) myfflush(stdout);
   sym = inchar();
   if(flags.topl == 1) flags.topl = 2;
   return((char) sym);
}
