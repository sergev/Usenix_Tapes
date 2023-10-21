
#file hack.mkobj.c
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* hack.mkobc version 1.0.1 - mksobj() also in MKLEV */

#include "hack.h"

#include "hack.onames.h"

char mkobjstr[] = "))[[!!!!????%%%%/=**))[[!!!!????%%%%/=**(";
struct obj *mkobj(), *mksobj();

mkobj_at(let,x,y)
register int let,x,y;
{
	register struct obj *otmp = mkobj(let);
	otmp->ox = x;
	otmp->oy = y;
	otmp->nobj = fobj;
	fobj = otmp;
}

mksobj_at(let,otyp,x,y)
register int let,otyp,x,y;
{
	register struct obj *otmp = mksobj(let, otyp);
	otmp->ox = x;
	otmp->oy = y;
	otmp->nobj = fobj;
	fobj = otmp;
}

struct obj *
mkobj(let) {
	if(!let) let = mkobjstr[rn2(sizeof(mkobjstr) - 1)];
	return(mksobj(let, letter(let) ? CORPSE : probtype(let)));
}
	

struct obj zeroobj;

struct obj *
mksobj(let, otyp) {
	register struct obj *otmp;

	otmp = newobj(0);
	*otmp = zeroobj;
	otmp->age = (ismklev) ? 0 : moves;
	otmp->o_id = flags.ident++;
	otmp->quan = 1;
	if(letter(let)){
		otmp->olet = FOOD_SYM;
		otmp->otyp = CORPSE + ((let > 'Z') ? (let-'a'+'Z'-'@'+1) :
				(let-'@'));
		otmp->spe = let;
		otmp->known = 1;
		otmp->owt = weight(otmp);
		return(otmp);
	}
	otmp->olet = let;
	otmp->otyp = otyp;
	otmp->dknown = index("/=!?*", let) ? 0 : 1;
	switch(let) {
	case WEAPON_SYM:
		otmp->quan = (otmp->otyp <= ROCK) ? rn1(6,6) : 1;
		if(!rn2(11)) otmp->spe = rnd(3);
		else if(!rn2(10)) {
			otmp->cursed = 1;
			otmp->spe = -rnd(3);
		}
		break;
	case FOOD_SYM:
	case GEM_SYM:
		otmp->quan = rn2(6) ? 1 : 2;
	case TOOL_SYM:
	case CHAIN_SYM:
	case BALL_SYM:
	case ROCK_SYM:
	case POTION_SYM:
	case SCROLL_SYM:
	case AMULET_SYM:
		break;
	case ARMOR_SYM:
		if(!rn2(8)) otmp->cursed = 1;
		if(!rn2(10)) otmp->spe = rnd(3);
		else if(!rn2(9)) {
			otmp->spe = -rnd(3);
			otmp->cursed = 1;
		}
		otmp->spe += 10 - objects[otmp->otyp].a_ac;
		break;
	case WAND_SYM:
		if(otmp->otyp == WAN_WISHING) otmp->spe = 3; else
		otmp->spe = rn1(5,
			(objects[otmp->otyp].bits & NODIR) ? 11 : 4);
		break;
	case RING_SYM:
		if(objects[otmp->otyp].bits & SPEC) {
			if(!rn2(3)) {
				otmp->cursed = 1;
				otmp->spe = -rnd(2);
			} else otmp->spe = rnd(2);
		} else if(otmp->otyp == RIN_TELEPORTATION ||
			  otmp->otyp == RIN_AGGRAVATE_MONSTER ||
			  otmp->otyp == RIN_HUNGER || !rn2(9))
			otmp->cursed = 1;
		break;
	default:
		panic("impossible mkobj");
	}
	otmp->owt = weight(otmp);
	return(otmp);
}

letter(c) {
	return(('@' <= c && c <= 'Z') || ('a' <= c && c <= 'z'));
}

weight(obj)
register struct obj *obj;
{
register int wt = objects[obj->otyp].oc_weight;
	return(wt ? wt*obj->quan : (obj->quan + 1)/2);
}

mkgold(num,x,y)
register int num;
{
	register struct gen *gtmp;
	register int amount = num ? num : 1 + (rnd(dlevel+2) * rnd(30));

	if(gtmp = g_at(x,y,fgold))
		gtmp->gflag += amount;
	else {
		gtmp = newgen();
		gtmp->ngen = fgold;
		gtmp->gx = x;
		gtmp->gy = y;
		gtmp->gflag = amount;
		fgold = gtmp;
                if (ismklev)
		   levl[x][y].scrsym = '$';
	}
}
#file hack.mon.c
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* hack.mon.c version 1.0.1 - some unimportant changes */

#include "hack.h"
#include "hack.mfndpos.h"
#define   SIZE(x)   (int)(sizeof(x) / sizeof(x[0]))
#define   NULL   (char *) 0
extern struct monst *makemon();

int warnlevel;      /* used by movemon and dochugw */
long lastwarntime;
int lastwarnlev;
char *warnings[] = {
   "white", "pink", "red", "ruby", "purple", "black"
};

movemon()
{
   register struct monst *mtmp;
   register int fr;

   warnlevel = 0;

   while(1) {
      /* find a monster that we haven't treated yet */
      /* note that mtmp or mtmp->nmon might get killed
         while mtmp moves, so we cannot just walk down the
         chain (even new monsters might get created!) */
      for(mtmp = fmon; mtmp; mtmp = mtmp->nmon)
         if(mtmp->mlstmv < moves) goto next_mon;
      /* treated all monsters */
      break;

   next_mon:
      mtmp->mlstmv = moves;
      if(mtmp->mblinded && !--mtmp->mblinded)
         mtmp->mcansee = 1;
      if(mtmp->mimic) continue;
      if(mtmp->mspeed != MSLOW || !(moves%2)){
         /* continue if the monster died fighting */
         fr = -1;
         if(Conflict && cansee(mtmp->mx,mtmp->my)
            && (fr = fightm(mtmp)) == 2)
            continue;
         if(fr<0 && dochugw(mtmp))
            continue;
      }
      if(mtmp->mspeed == MFAST && dochugw(mtmp))
         continue;
   }

   warnlevel -= u.ulevel;
   if(warnlevel >= SIZE(warnings))
      warnlevel = SIZE(warnings)-1;
   if(warnlevel >= 0)
   if(warnlevel > lastwarnlev || moves > lastwarntime + 5){
       register char *rr;
       switch(Warning & (LEFT_RING | RIGHT_RING)){
       case LEFT_RING:
      rr = "Your left ring glows";
      break;
       case RIGHT_RING:
      rr = "Your right ring glows";
      break;
       case LEFT_RING | RIGHT_RING:
      rr = "Both your rings glow";
      break;
       default:
      rr = "Your fingertips glow";
      break;
       }
       pline("%s %s!", rr, warnings[warnlevel]);
       lastwarntime = moves;
       lastwarnlev = warnlevel;
   }

   dmonsfree();   /* remove all dead monsters */
}

justswld(mtmp,name)
register struct monst *mtmp;
char *name;
{

   mtmp->mx = u.ux;
   mtmp->my = u.uy;
   u.ustuck = mtmp;
   pmon(mtmp);
   kludge("%s swallows you!",name);
   more();
   seeoff(1);
	u.uswldtim = 0;
	u.uswallow = 1;
	swallowed();
}

youswld(mtmp,dam,die,name)
register struct monst *mtmp;
register int dam,die;
char *name;
{
   if(mtmp != u.ustuck) return;
   kludge("%s digests you!",name);
   u.uhp -= dam;
   if(u.uswldtim++ == die){
      pline("It totally digests you!");
      u.uhp = -1;
   }
   if(u.uhp < 1) done_in_by(mtmp);
}

dochugw(mtmp) register struct monst *mtmp; {
register int x = mtmp->mx;
register int y = mtmp->my;
register int d = dochug(mtmp);
register int dd;
   if(!d)      /* monster still alive */
   if(Warning)
   if(!mtmp->mpeaceful)
   if((dd = dist(mtmp->mx,mtmp->my)) < dist(x,y))
   if(dd < 100)
   if(!cansee(mtmp->mx, mtmp->my) || (mtmp->minvis && !See_invisible))
   if(mtmp->data->mlevel > warnlevel)
      warnlevel = mtmp->data->mlevel;
   return(d);
}

/* returns 1 if monster died moving, 0 otherwise */
dochug(mtmp)
register struct monst *mtmp;
{
   register struct permonst *mdat;
   register int tmp;

   if(mtmp->cham && !rn2(6))
      (void) newcham(mtmp, &mons[dlevel+14+rn2(CMNUM-14-dlevel)]);
   mdat = mtmp->data;
   if(mdat->mlevel < 0)
      panic("bad monster %c (%d)",mdat->mlet,mdat->mlevel);
   if((!(moves%20) || index("ViT",mdat->mlet)) &&
       mtmp->mhp<mtmp->orig_hp)
      mtmp->mhp++; /* regenerate monsters. */
   if(mtmp->mfroz) return(0); /* frozen monsters don't do anything. */
   if(mtmp->msleep) {/* wake up a monster, or get out of here. */
      if(cansee(mtmp->mx,mtmp->my) && !Stealth &&
         (!index("NL",mdat->mlet) || !rn2(50)) &&
         (Aggravate_monster || (!rn2(7) && !mtmp->mimic)))
         mtmp->msleep = 0;
      else return(0);
   }

   /* not frozen or sleeping: wipe out texts written in the dust */
   wipe_engr_at(mtmp->mx, mtmp->my, 1);

   /* confused monsters get unconfused with small probability */
   if(mtmp->mconf && !rn2(50)) mtmp->mconf = 0;

   /* some monsters teleport */
   if(mtmp->mflee && index("tNL", mdat->mlet) && !rn2(40)){
      rloc(mtmp);
      return(0);
   }
   if(mdat->mmove < rnd(6)) return(0);
   if((mtmp->mflee ||
      mtmp->mconf ||
      (index("BIuy", mdat->mlet) && !rn2(4)) ||
      (mdat->mlet == 'L' && !u.ugold && (mtmp->mgold || rn2(2))) ||
      dist(mtmp->mx,mtmp->my) > 2 ||
      (!mtmp->mcansee && !rn2(4)) ||
      mtmp->mpeaceful
      ) && (tmp = m_move(mtmp,0)) && mdat->mmove <= 12)
      return(tmp == 2);
   if(tmp == 2) return(1);   /* monster died moving */

   if(!index("Ea", mdat->mlet) && dist(mtmp->mx, mtmp->my) < 3 &&
    !mtmp->mpeaceful && u.uhp > 0 &&
    !sengr_at("Elbereth", u.ux, u.uy) &&
    !sobj_at(SCR_SCARE_MONSTER, u.ux, u.uy)) {
      if(mhitu(mtmp))
         return(1);   /* monster died (e.g. 'y' or 'F') */
   }
   /* extra movement for fast monsters */
   if(mdat->mmove-12 > rnd(12)) tmp = m_move(mtmp,1);
   return(tmp == 2);
}

inrange(mtmp)
register struct monst *mtmp;
{
   register schar tx,ty;

   /* spit fire only when both in a room or both in a corridor */
   if(inroom(u.ux,u.uy) != inroom(mtmp->mx,mtmp->my)) return;
   tx = u.ux - mtmp->mx;
   ty = u.uy - mtmp->my;
   if((!tx && abs(ty) < 8) || (!ty && abs(tx) < 8)
       || (abs(tx) == abs(ty) && abs(tx) < 8)){
      /* spit fire in the direction of @ (not nec. hitting) */
      buzz(-1,mtmp->mx,mtmp->my,sgn(tx),sgn(ty));
      if(u.uhp < 1) done_in_by(mtmp);
   }
}

m_move(mtmp,after)
register struct monst *mtmp;
{
   register struct monst *mtmp2;
   register int nx,ny,omx,omy,appr,nearer,cnt,i,j;
   xchar gx,gy,nix,niy,chcnt;
   schar chi;
   boolean likegold, likegems, likeobjs;
   schar mmoved = 0;   /* not strictly nec.: chi >= 0 will do */
   coord poss[9];
   int info[9];

   if(mtmp->mtrapped) {
      i = mintrap(mtmp);
      if(i == 2) return(2);   /* he died */
      if(i == 1) return(0);   /* still in trap, so didnt move */
   }
   if(mtmp->mhide && o_at(mtmp->mx,mtmp->my) && rn2(10))
      return(0);      /* do not leave hiding place */

   /* my dog gets a special treatment */
   if(mtmp->mtame) {
      return( dog_move(mtmp, after) );
   }

   /* likewise for shopkeeper */
   if(mtmp->isshk) {
      mmoved = shk_move();
      goto postmov;
   }

   /* and for the guard */
   if(mtmp->isgd) {
      mmoved = gd_move();
      goto postmov;
   }

   if(mtmp->data->mlet == 't' && !rn2(5)) {
      if(rn2(2))
         mnexto(mtmp);
      else
         rloc(mtmp);
      mmoved = 1;
      goto postmov;
   }
   if(mtmp->data->mlet == 'D' && !mtmp->mcan)
      inrange(mtmp);
   if(!Blind && !Confusion && mtmp->data->mlet == 'U' && !mtmp->mcan
      && cansee(mtmp->mx,mtmp->my) && rn2(5)) {
      pline("%s's gaze has confused you!", Monnam(mtmp));
      if(rn2(5)) mtmp->mcan = 1;
      Confusion = d(3,4);      /* timeout */
   }
   if(!mtmp->mflee && u.uswallow && u.ustuck != mtmp) return(1);
   appr = 1;
   if(mtmp->mflee) appr = -1;
   if(mtmp->mconf || Invis ||  !mtmp->mcansee ||
      (index("BIy",mtmp->data->mlet) && !rn2(3)))
      appr = 0;
   omx = mtmp->mx;
   omy = mtmp->my;
   gx = u.ux;
   gy = u.uy;
   if(mtmp->data->mlet == 'L' && appr == 1 && mtmp->mgold > u.ugold)
      appr = -1;
#ifdef TRACK
   /* random criterion for 'smell'
      should use mtmp->msmell
    */
   if('a' <= mtmp->data->mlet && mtmp->data->mlet <= 'z') {
   extern coord *gettrack();
   register coord *cp;
   schar mroom;
      mroom = inroom(omx,omy);
      if(mroom < 0 || mroom != inroom(u.ux,u.uy)){
          cp = gettrack(omx,omy);
          if(cp){
         gx = cp->x;
         gy = cp->y;
          }
      }
   }
#endif TRACK
   /* look for gold or jewels nearby */
   likegold = (index("LOD", mtmp->data->mlet) != NULL);
   likegems = (index("ODu", mtmp->data->mlet) != NULL);
   likeobjs = mtmp->mhide;
#define   SRCHRADIUS   25
   { xchar mind = SRCHRADIUS;      /* not too far away */
     register int dd;
     if(likegold){
      register struct gen *gold;
      for(gold = fgold; gold; gold = gold->ngen)
        if((dd = DIST(omx,omy,gold->gx,gold->gy)) < mind){
          mind = dd;
          gx = gold->gx;
          gy = gold->gy;
      }
     }
     if(likegems || likeobjs){
      register struct obj *otmp;
      for(otmp = fobj; otmp; otmp = otmp->nobj)
      if(likeobjs || otmp->olet == GEM_SYM)
      if(mtmp->data->mlet != 'u' ||
         objects[otmp->otyp].g_val != 0)
      if((dd = DIST(omx,omy,otmp->ox,otmp->oy)) < mind){
          mind = dd;
          gx = otmp->ox;
          gy = otmp->oy;
      }
     }
     if(mind < SRCHRADIUS && appr == -1) {
      if(dist(omx,omy) < 10) {
          gx = u.ux;
          gy = u.uy;
      } else
          appr = 1;
     }
   }
   nix = omx;
   niy = omy;
   cnt = mfndpos(mtmp,poss,info,
      mtmp->data->mlet == 'u' ? NOTONL :
      index(" VWZ", mtmp->data->mlet) ? NOGARLIC : ALLOW_TRAPS);
      /* ALLOW_ROCK for some monsters ? */
   chcnt = 0;
   chi = -1;
   for(i=0; i<cnt; i++) {
      nx = poss[i].x;
      ny = poss[i].y;
      for(j=0; j<MTSZ && j<cnt-1; j++)
         if(nx == mtmp->mtrack[j].x && ny == mtmp->mtrack[j].y)
            if(rn2(4*(cnt-j))) goto nxti;
#ifdef STUPID
      /* some stupid compilers think that this is too complicated */
      { int d1 = DIST(nx,ny,gx,gy);
        int d2 = DIST(nix,niy,gx,gy);
        nearer = (d1 < d2);
      }
#else
      nearer = (DIST(nx,ny,gx,gy) < DIST(nix,niy,gx,gy));
#endif STUPID
      if((appr == 1 && nearer) || (appr == -1 && !nearer) ||
         !mmoved ||
         (!appr && !rn2(++chcnt))){
         nix = nx;
         niy = ny;
         chi = i;
         mmoved = 1;
      }
   nxti:   ;
   }
   if(mmoved){
      if(info[chi] & ALLOW_M){
         mtmp2 = m_at(nix,niy);
         if(hitmm(mtmp,mtmp2) == 1 && rn2(4) &&
           hitmm(mtmp2,mtmp) == 2) return(2);
         return(0);
      }
      if(info[chi] & ALLOW_U){
        (void) hitu(mtmp, d(mtmp->data->damn, mtmp->data->damd)+1);
        return(0);
      }
      mtmp->mx = nix;
      mtmp->my = niy;
      for(j=MTSZ-1; j>0; j--) mtmp->mtrack[j] = mtmp->mtrack[j-1];
      mtmp->mtrack[0].x = omx;
      mtmp->mtrack[0].y = omy;
#ifndef NOWORM
      if(mtmp->wormno) worm_move(mtmp);
#endif NOWORM
   } else {
      if(mtmp->data->mlet == 'u' && rn2(2)){
         rloc(mtmp);
         return(0);
      }
#ifndef NOWORM
      if(mtmp->wormno) worm_nomove(mtmp);
#endif NOWORM
   }
postmov:
   if(mmoved == 1) {
      if(mintrap(mtmp) == 2)   /* he died */
         return(2);
      if(likegold) mpickgold(mtmp);
      if(likegems) mpickgems(mtmp);
      if(mtmp->mhide) mtmp->mundetected = 1;
   }
   pmon(mtmp);
   return(mmoved);
}

mpickgold(mtmp) register struct monst *mtmp; {
register struct gen *gold;
   while(gold = g_at(mtmp->mx, mtmp->my, fgold)){
      mtmp->mgold += gold->gflag;
      freegold(gold);
      if(levl[mtmp->mx][mtmp->my].scrsym == '$')
         newsym(mtmp->mx, mtmp->my);
   }
}

mpickgems(mtmp) register struct monst *mtmp; {
register struct obj *otmp;
   for(otmp = fobj; otmp; otmp = otmp->nobj)
   if(otmp->olet == GEM_SYM)
   if(otmp->ox == mtmp->mx && otmp->oy == mtmp->my)
   if(mtmp->data->mlet != 'u' || objects[otmp->otyp].g_val != 0){
      freeobj(otmp);
      mpickobj(mtmp, otmp);
      if(levl[mtmp->mx][mtmp->my].scrsym == GEM_SYM)
         newsym(mtmp->mx, mtmp->my);   /* %% */
      return;   /* pick only one object */
   }
}

/* return number of acceptable neighbour positions */
mfndpos(mon,poss,info,flag)
register struct monst *mon; coord poss[9]; int info[9], flag; {
register int x,y,nx,ny,cnt = 0,tmp;
register struct monst *mtmp;
   x = mon->mx;
   y = mon->my;
   if(mon->mconf) {
      flag |= ALLOW_ALL;
      flag &= ~NOTONL;
   }
   for(nx = x-1; nx <= x+1; nx++) for(ny = y-1; ny <= y+1; ny++)
   if(nx != x || ny != y) if(isok(nx,ny))
   if((tmp = levl[nx][ny].typ) >= DOOR)
   if(!(nx != x && ny != y &&
      (levl[x][y].typ == DOOR || tmp == DOOR))){
      info[cnt] = 0;
      if(nx == u.ux && ny == u.uy){
         if(!(flag & ALLOW_U)) continue;
         info[cnt] = ALLOW_U;
      } else if(mtmp = m_at(nx,ny)){
         if(!(flag & ALLOW_M)) continue;
         info[cnt] = ALLOW_M;
         if(mtmp->mtame){
            if(!(flag & ALLOW_TM)) continue;
            info[cnt] |= ALLOW_TM;
         }
      }
      if(sobj_at(CLOVE_OF_GARLIC, nx, ny)) {
         if(flag & NOGARLIC) continue;
         info[cnt] |= NOGARLIC;
      }
      if(sobj_at(SCR_SCARE_MONSTER, nx, ny) ||
         (!mon->mpeaceful && sengr_at("Elbereth", nx, ny))) {
         if(!(flag & ALLOW_SSM)) continue;
         info[cnt] |= ALLOW_SSM;
      }
      if(sobj_at(ENORMOUS_ROCK, nx, ny)) {
         if(!(flag & ALLOW_ROCK)) continue;
         info[cnt] |= ALLOW_ROCK;
      }
      if(!Invis && online(nx,ny)){
         if(flag & NOTONL) continue;
         info[cnt] |= NOTONL;
      }
			/* we cannot avoid traps of an unknown kind */
			{ register struct gen *gtmp = g_at(nx, ny, ftrap);
			register int tt;
			if(gtmp) {
				tt = 1 << (gtmp->gflag & TRAPTYPE);
				if(mon->mtrapseen & tt){
					if(!(flag & tt)) continue;
					info[cnt] |= tt;
				}
         }
      }
      poss[cnt].x = nx;
      poss[cnt].y = ny;
      cnt++;
   }
   return(cnt);
}

dist(x,y) int x,y; {
   return((x-u.ux)*(x-u.ux) + (y-u.uy)*(y-u.uy));
}

poisoned(string, pname)
register char *string, *pname;
{
   if(Blind) pline("It was poisoned.");
   else pline("The %s was poisoned!",string);
   if(Poison_resistance) {
      pline("The poison doesn't seem to affect you.");
      return;
   }
   switch(rnd(6)) {
   case 1:
      u.uhp = -1;
      break;
   case 2:
   case 3:
   case 4:
      losestr(rn1(3,3));
      break;
   case 5:
   case 6:
      losehp(rn1(10,6), pname);
      return;
   }
   if(u.uhp < 1) killer = pname;
}

mondead(mtmp)
register struct monst *mtmp;
{
	relobj(mtmp,1);
	unpmon(mtmp);
	relmon(mtmp);
	if(u.ustuck == mtmp) {
		u.ustuck = 0;
		if(u.uswallow) {
			u.uswldtim = 0;
			u.uswallow = 0;
			setsee();
			docrt();
      }
   }
   if(mtmp->isshk) shkdead();
   if(mtmp->isgd) gddead();
#ifndef NOWORM
   if(mtmp->wormno) wormdead(mtmp);
#endif NOWORM
   monfree(mtmp);
}

/* called when monster is moved to larger structure */
replmon(mtmp,mtmp2)
register struct monst *mtmp, *mtmp2;
{
   relmon(mtmp);
   monfree(mtmp);
   mtmp2->nmon = fmon;
   fmon = mtmp2;
}

relmon(mon)
register struct monst *mon;
{
   register struct monst *mtmp;

   if(mon == fmon) fmon = fmon->nmon;
   else {
      for(mtmp = fmon; mtmp->nmon != mon; mtmp = mtmp->nmon) ;
      mtmp->nmon = mon->nmon;
   }
}

/* we do not free monsters immediately, in order to have their name
   available shortly after their demise */
struct monst *fdmon;   /* chain of dead monsters, need not to be saved */

monfree(mtmp) register struct monst *mtmp; {
   mtmp->nmon = fdmon;
   fdmon = mtmp;
}

dmonsfree(){
register struct monst *mtmp;
   while(mtmp = fdmon){
      fdmon = mtmp->nmon;
      free((char *) mtmp);
   }
}

killed(mtmp) struct monst *mtmp; {
#ifdef lint
#define   NEW_SCORING
#endif lint
register int tmp,tmp2,nk,x,y;
register struct permonst *mdat = mtmp->data;
   if(mtmp->cham) mdat = PM_CHAM;
   if(Blind) pline("You destroy it!");
   else {
      pline("You destroy %s!",
         mtmp->mtame ? amonnam(mtmp, "poor") : monnam(mtmp));
   }
   if(u.umconf) {
      if(!Blind) pline("Your hands stop glowing blue.");
      u.umconf = 0;
   }

   /* count killed monsters */
#define   MAXMONNO   100
   nk = 1;            /* in case we cannot find it in mons */
   tmp = mdat - mons;    /* index in mons array (if not 'd', '@', ...) */
   if(tmp >= 0 && tmp < CMNUM+2) {
       extern char fut_geno[];
       u.nr_killed[tmp]++;
       if((nk = u.nr_killed[tmp]) > MAXMONNO &&
      !index(fut_geno, mdat->mlet))
          charcat(fut_geno,  mdat->mlet);
   }

   /* punish bad behaviour */
   if(mdat->mlet == '@') Telepat = 0, u.uluck -= 2;
   if(mtmp->mpeaceful || mtmp->mtame) u.uluck--;
   if(mdat->mlet == 'u') u.uluck -= 5;

   /* give experience points */
   tmp = 1 + mdat->mlevel * mdat->mlevel;
   if(mdat->ac < 3) tmp += 2*(7 - mdat->ac);
   if(index("AcsSDXaeRTVWU&In:P", mdat->mlet))
      tmp += 2*mdat->mlevel;
   if(index("DeV&P",mdat->mlet)) tmp += (7*mdat->mlevel);
   if(mdat->mlevel > 6) tmp += 50;

#ifdef NEW_SCORING
   /* ------- recent addition: make nr of points decrease
         when this is not the first of this kind */
   { int ul = u.ulevel;
     int ml = mdat->mlevel;

   if(ul < 14)    /* points are given based on present and future level */
       for(tmp2 = 0; !tmp2 || ul + tmp2 <= ml; tmp2++)
      if(u.uexp + 1 + (tmp + ((tmp2 <= 0) ? 0 : 4<<(tmp2-1)))/nk
          >= 10*pow((unsigned)(ul-1)))
         if(++ul == 14) break;

   tmp2 = ml - ul -1;
   tmp = (tmp + ((tmp2 < 0) ? 0 : 4<<tmp2))/nk;
   if(!tmp) tmp = 1;
   }
   /* note: ul is not necessarily the future value of u.ulevel */
   /* ------- end of recent valuation change ------- */
#endif NEW_SCORING

   u.uexp += tmp;
   u.urexp += 4*tmp;
   flags.botl = 1;
   while(u.ulevel < 14 && u.uexp >= 10*pow(u.ulevel-1)){
      pline("Welcome to level %d.", ++u.ulevel);
      tmp = rnd(10);
      if(tmp < 3) tmp = rnd(10);
      u.uhpmax += tmp;
      u.uhp += tmp;
      flags.botl = 1;
   }

   /* dispose of monster and make cadaver */
   x = mtmp->mx;   y = mtmp->my;
   mondead(mtmp);
   tmp = mdat->mlet;
   if(tmp == 'm') { /* he killed a minotaur, give him a wand of digging */
         /* note: the dead minotaur will be on top of it! */
      mksobj_at(WAND_SYM, WAN_DIGGING, x, y);
      /* if(cansee(x,y)) atl(x,y,fobj->olet); */
      stackobj(fobj);
   } else
#ifndef NOWORM
   if(tmp == 'w') {
      mksobj_at(WEAPON_SYM, WORM_TOOTH, x, y);
      stackobj(fobj);
   } else
#endif   NOWORM
   if(!letter(tmp) || !rn2(3)) tmp = 0;

   if(levl[x][y].typ >= DOOR)   /* might be mimic in wall */
       if(x != u.ux || y != u.uy) /* might be here after swallowed */
      if(index("NTVm&",mdat->mlet) || rn2(5)) {
      mkobj_at(tmp,x,y);
      if(cansee(x,y)) atl(x,y,fobj->olet);
      stackobj(fobj);
   }
}

kludge(str,arg)
register char *str,*arg;
{
   if(Blind) {
      if(*str == '%') pline(str,"It");
      else pline(str,"it");
   } else pline(str,arg);
}

rescham()   /* force all chameleons to become normal */
{
   register struct monst *mtmp;

   for(mtmp = fmon; mtmp; mtmp = mtmp->nmon)
      if(mtmp->cham) {
         mtmp->cham = 0;
         (void) newcham(mtmp,PM_CHAM);
      }
}

newcham(mtmp,mdat)   /* make a chameleon look like a new monster */
         /* returns 1 if the monster actually changed */
register struct monst *mtmp;
register struct permonst *mdat;
{
   register int mhp, hpn, hpd;

   if(mdat == mtmp->data) return(0);   /* still the same monster */
#ifndef NOWORM
   if(mtmp->wormno) wormdead(mtmp);   /* throw tail away */
#endif NOWORM
   hpn = mtmp->mhp;
   hpd = (mtmp->data->mlevel)*8;
   if(!hpd) hpd = 4;
   mtmp->data = mdat;
   mhp = (mdat->mlevel)*8;
   /* new hp: same fraction of max as before */
   mtmp->mhp = 2 + (hpn*mhp)/hpd;
   hpn = mtmp->orig_hp;
   mtmp->orig_hp = 2 + (hpn*mhp)/hpd;
   mtmp->minvis = (mdat->mlet == 'I') ? 1 : 0;
#ifndef NOWORM
   if(mdat->mlet == 'w' && getwn(mtmp)) initworm(mtmp);
#endif NOWORM
   unpmon(mtmp);   /* necessary for 'I' and to force pmon */
			/* perhaps we should clear mtmp->mtame here? */
   pmon(mtmp);
   return(1);
}

mnexto(mtmp)   /* Make monster mtmp next to you (if possible) */
struct monst *mtmp;
{
   extern coord enexto();
   coord mm;
   mm = enexto(u.ux, u.uy);
   mtmp->mx = mm.x;
   mtmp->my = mm.y;
   pmon(mtmp);
}

rloc(mtmp)
struct monst *mtmp;
{
   register int tx,ty;
   register char ch = mtmp->data->mlet;

#ifndef NOWORM
   if(ch == 'w' && mtmp->mx) return;   /* do not relocate worms */
#endif NOWORM
   do {
      tx = rn1(COLNO-3,2);
      ty = rn2(ROWNO);
   } while(!goodpos(tx,ty));
   mtmp->mx = tx;
   mtmp->my = ty;
   if(u.ustuck == mtmp){
      if(u.uswallow) {
         u.ux = tx;
         u.uy = ty;
         docrt();
      } else   u.ustuck = 0;
   }
   pmon(mtmp);
}

ishuman(mtmp) register struct monst *mtmp; {
   return(mtmp->data->mlet == '@');
}

setmangry(mtmp) register struct monst *mtmp; {
   if(!mtmp->mpeaceful) return;
   if(mtmp->mtame) return;
   mtmp->mpeaceful = 0;
   if(ishuman(mtmp)) pline("%s gets angry!", Monnam(mtmp));
}
#file hack.monst.c
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* hack.monst version 1.0.1 - corrected symbol for lurker above */

#include "mklev.h"
#include "def.eshk.h"
#include "def.edog.h"
extern char plname[PL_NSIZ];

struct permonst mons[PMONCOUNT] = {
	{ "bat",		'B',1,22,8,1,4,0 },
	{ "gnome",		'G',1,6,5,1,6,0 },
	{ "hobgoblin",		'H',1,9,5,1,8,0 },
	{ "jackal",		'J',0,12,7,1,2,0 },
	{ "kobold",		'K',1,6,7,1,4,0 },
	{ "leprechaun",		'L',5,15,8,1,2,0 },
	{ "giant rat",		'r',0,12,7,1,3,0 },
	{ "acid blob",		'a',2,3,8,0,0,0 },
	{ "floating eye",	'E',2,1,9,0,0,0 },
	{ "homunculus",		'h',2,6,6,1,3,0 },
	{ "imp",		'i',2,6,2,1,4,0 },
	{ "orc",		'O',2,9,6,1,8,0 },
	{ "yellow light",	'y',3,15,0,0,0,0 },
	{ "zombie",		'Z',2,6,8,1,8,0 },
	{ "giant ant",		'A',3,18,3,1,6,0 },
	{ "fog cloud",		'f',3,1,0,1,6,0 },
	{ "nymph",		'N',6,12,9,1,2,0 },
	{ "piercer",		'p',3,1,3,2,6,0 },
	{ "quasit",		'Q',3,15,3,1,4,0 },
	{ "quivering blob",	'q',3,1,8,1,8,0 },
	{ "violet fungi",	'v',3,1,7,1,4,0 },
	{ "giant beetle",	'b',4,6,4,3,4,0 },
	{ "centaur",		'C',4,18,4,1,6,0 },
	{ "cockatrice",		'c',4,6,6,1,3,0 },
	{ "gelatinous cube",	'g',4,6,8,2,4,0 },
	{ "jaguar",		'j',4,15,6,1,8,0 },
	{ "killer bee",		'k',4,14,4,2,4,0 },
	{ "snake",		'S',4,15,3,1,6,0 },
	{ "freezing sphere",	'F',2,13,4,0,0,0 },
	{ "owlbear",		'o',5,12,5,2,6,0 },
	{ "rust monster",	'R',10,18,3,0,0,0 },
	{ "scorpion",		's',5,15,3,1,4,0 },
	{ "tengu",		't',5,13,5,1,7,0 },
	{ "wraith",		'W',5,12,5,1,6,0 },
#ifdef NOWORM
	{ "wumpus",		'w',8,3,2,3,6,0 },
#else
	{ "long worm",		'w',8,3,5,1,4,0 },
#endif NOWORM
	{ "large dog",		'd',6,15,4,2,4,0 },
	{ "leocrotta",		'l',6,18,4,3,6,0 },
	{ "mimic",		'M',7,3,7,3,4,0 },
	{ "troll",		'T',7,12,4,2,6,0 },
	{ "unicorn",		'u',8,24,5,1,10,0 },
	{ "yeti",		'Y',5,15,6,1,6,0 },
	{ "stalker",		'I',8,12,3,4,4,0 },
	{ "umber hulk",		'U',9,6,2,2,10,0 },
	{ "vampire",		'V',8,12,1,1,6,0 },
	{ "xorn",		'X',8,9,-2,4,6,0 },
	{ "xan",		'x',7,18,-2,2,4,0 },
	{ "zruty",		'z',9,8,3,3,6,0 },
	{ "chameleon",		':',6,5,6,4,2,0 },
	{ "dragon",		'D',10,9,-1,3,8,0 },
	{ "ettin",		'e',10,12,3,2,8,0 },
	{ "lurker above",	'\'',10,3,3,0,0,0 },
	{ "nurse",		'n',11,6,0,1,3,0 },
	{ "trapper",		',',12,3,3,0,0,0 },
	{ "purple worm",	'P',15,9,6,2,8,0 },
	{ "demon",		'&',10,12,-4,1,4,0 },
	{ "minotaur",		'm',15,15,6,4,8,0 },
	{ "shopkeeper", 	'@', 10, 18, 0, 4, 8, sizeof(struct eshk) },
	{ "ghost",              ' ', 10, 3, -5, 1, 1, sizeof(plname) },
	{ "little dog", 	'd', 2,  18, 6, 1, 6, sizeof(struct edog) },
	{ "dog",		'd', 4,  16, 5, 1, 6, sizeof(struct edog) },
	{ "large dog",		'd', 6,  15, 4, 2, 4, sizeof(struct edog) }
};

#file hack.objnam.c
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1984. */

#include "hack.h"
#define Sprintf (void) sprintf
#define Strcat  (void) strcat
#define Strcpy  (void) strcpy
#define PREFIX  15
extern char *eos();
extern int bases[];

char *
strprepend(s,pref) /*register*/ char *s, *pref; {
/*register*/ int i = strlen(pref);
   if(i > PREFIX) {
      pline("WARNING: prefix too short.");
      return(s);
   }
   s -= i;
   (void) movmem(pref, s, i);           /* do not copy trailing 0 */
/*   (void) strncpy(s, pref, i); */     /* do not copy trailing 0 */
   return(s);
}

char *
sitoa(a) int a; {
static char buf[13];
   Sprintf(buf, (a < 0) ? "%d" : "+%d", a);
   return(buf);
}

char *
xname(obj)
/*register*/ struct obj *obj;
{
static char bufr[BUFSZ];
/*register*/ char *buf = &(bufr[PREFIX]);   /* leave room for "17 -3 " */
/*register*/ int nn = objects[obj->otyp].oc_name_known;
/*register*/ char *an = objects[obj->otyp].oc_name;
/*register*/ char *dn = objects[obj->otyp].oc_descr;
/*register*/ char *un = objects[obj->otyp].oc_uname;
/*register*/ int pl = (obj->quan != 1);
   if(!obj->dknown && !Blind) obj->dknown = 1; /* %% doesnt belong here */
   switch(obj->olet) {
   case AMULET_SYM:
      Strcpy(buf, (obj->spe < 0 && obj->known)
         ? "cheap plastic imitation of the " : "");
      Strcat(buf,"Amulet of Yendor");
      break;
   case TOOL_SYM:
      if(!nn) {
         Strcpy(buf, dn);
         break;
      }
      Strcpy(buf,an);
      break;
   case FOOD_SYM:
      if(obj->otyp == CLOVE_OF_GARLIC && pl) {
         pl = 0;
         Strcpy(buf, "cloves of garlic");
         break;
      }
      if(obj->otyp == DEAD_HOMUNCULUS && pl) {
         pl = 0;
         Strcpy(buf, "dead homunculi");
         break;
      }
      /* fungis ? */
      /* fall into next case */
   case WEAPON_SYM:
      if(obj->otyp == WORM_TOOTH && pl) {
         pl = 0;
         Strcpy(buf, "worm teeth");
         break;
      }
      if(obj->otyp == CRYSKNIFE && pl) {
         pl = 0;
         Strcpy(buf, "crysknives");
         break;
      }
      /* fall into next case */
   case ARMOR_SYM:
   case CHAIN_SYM:
   case ROCK_SYM:
      Strcpy(buf,an);
      break;
   case BALL_SYM:
      Sprintf(buf, "%sheavy iron ball",
        (obj->owt > objects[obj->otyp].oc_weight) ? "very " : "");
      break;
   case POTION_SYM:
      if(nn || un || !obj->dknown) {
         Strcpy(buf, "potion");
         if(pl) {
            pl = 0;
            Strcat(buf, "s");
         }
         if(!obj->dknown) break;
         if(un) {
            Strcat(buf, " called ");
            Strcat(buf, un);
         } else {
            Strcat(buf, " of ");
            Strcat(buf, an);
         }
      } else {
         Strcpy(buf, dn);
         Strcat(buf, " potion");
      }
      break;
   case SCROLL_SYM:
      Strcpy(buf, "scroll");
      if(pl) {
         pl = 0;
         Strcat(buf, "s");
      }
      if(!obj->dknown) break;
      if(nn) {
         Strcat(buf, " of ");
         Strcat(buf, an);
      } else if(un) {
         Strcat(buf, " called ");
         Strcat(buf, un);
      } else {
         Strcat(buf, " labeled ");
         Strcat(buf, dn);
      }
      break;
   case WAND_SYM:
      if(!obj->dknown)
         Sprintf(buf, "wand");
      else if(nn)
         Sprintf(buf, "wand of %s", an);
      else if(un)
         Sprintf(buf, "wand called %s", un);
      else
         Sprintf(buf, "%s wand", dn);
      break;
   case RING_SYM:
      if(!obj->dknown)
         Sprintf(buf, "ring");
      else if(nn)
         Sprintf(buf, "ring of %s", an);
      else if(un)
         Sprintf(buf, "ring called %s", un);
      else
         Sprintf(buf, "%s ring", dn);
      break;
   case GEM_SYM:
      if(!obj->dknown) {
         Strcpy(buf, "gem");
         break;
      }
      if(!nn) {
         Sprintf(buf, "%s gem", dn);
         break;
      }
      if(pl && !strncmp("worthless piece", an, 15)) {
         pl = 0;
         Sprintf(buf, "worthless pieces%s", an+15);
         break;
      }
      Strcpy(buf, an);
      if(obj->otyp >= TURQUOISE && obj->otyp <= JADE)
         Strcat(buf, " stone");
      break;
   default:
      Sprintf(buf,"glorkum %c (0%o) %d %d",
         obj->olet,obj->olet,obj->otyp,obj->spe);
   }
   if(pl) {
      /*register*/ char *p = eos(buf)-1;
      if(*p == 's' || *p == 'z' || *p == 'x' ||
          (*p == 'h' && p[-1] == 's'))
         Strcat(buf, "es");   /* boxes */
      else if(*p == 'y' && !index(vowels, p[-1]))
         Strcpy(p, "ies");   /* rubies, zruties */
      else
         Strcat(buf, "s");
   }
   if(obj->onamelth) {
      Strcat(buf, " named ");
      Strcat(buf, ONAME(obj));
   }
   return(buf);
}

char *
mydoname(obj)
/*register*/ struct obj *obj;
{
char prefix[PREFIX];
/*register*/ char *bp;
obj->known = obj->dknown = 1;
objects[obj->otyp].oc_name_known = 1;
bp = xname(obj);
   if(obj->quan != 1)
      Sprintf(prefix, "%d ", obj->quan);
   else
      Strcpy(prefix, "a ");
   switch(obj->olet) {
   case AMULET_SYM:
      if(strncmp(bp, "cheap ", 6))
         Strcpy(prefix, "the ");
      break;
   case ARMOR_SYM:
      if(obj->owornmask & W_ARMOR)
         Strcat(bp, " (being worn)");
      Strcat(prefix, sitoa(obj->spe - 10 + objects[obj->otyp].a_ac));
      Strcat(prefix, " ");
      break;
   case WEAPON_SYM:
      Strcat(prefix, sitoa(obj->spe));
      Strcat(prefix, " ");
      break;
   case WAND_SYM:
      Sprintf(eos(bp), " (%d)", obj->spe);
      break;
   case RING_SYM:
      if(obj->owornmask & W_RINGR) Strcat(bp, " (on right hand)");
      if(obj->owornmask & W_RINGL) Strcat(bp, " (on left hand)");
      if(objects[obj->otyp].bits & SPEC) {
         Strcat(prefix, sitoa(obj->spe));
         Strcat(prefix, " ");
      }
      break;
   }
   if(obj->owornmask & W_WEP)
      Strcat(bp, " (weapon in hand)");
   if(obj->unpaid)
      Strcat(bp, " (unpaid)");
   if(!strcmp(prefix, "a ") && index(vowels, *bp))
      Strcpy(prefix, "an ");
   bp = strprepend(bp, prefix);
   return(bp);
}

char *
doname(obj)
/*register*/ struct obj *obj;
{
char prefix[PREFIX];
/*register*/ char *bp = xname(obj);

   if(obj->quan != 1)
      Sprintf(prefix, "%d ", obj->quan);
   else
      Strcpy(prefix, "a ");
   switch(obj->olet) {
   case AMULET_SYM:
      if(strncmp(bp, "cheap ", 6))
         Strcpy(prefix, "the ");
      break;
   case ARMOR_SYM:
      if(obj->owornmask & W_ARMOR)
         Strcat(bp, " (being worn)");
      if(obj->known) {
         Strcat(prefix,
             sitoa(obj->spe - 10 + objects[obj->otyp].a_ac));
         Strcat(prefix, " ");
      }
      break;
   case WEAPON_SYM:
      if(obj->known) {
         Strcat(prefix, sitoa(obj->spe));
         Strcat(prefix, " ");
      }
      break;
   case WAND_SYM:
      if(obj->known)
         Sprintf(eos(bp), " (%d)", obj->spe);
      break;
   case RING_SYM:
      if(obj->owornmask & W_RINGR) Strcat(bp, " (on right hand)");
      if(obj->owornmask & W_RINGL) Strcat(bp, " (on left hand)");
      if(obj->known && (objects[obj->otyp].bits & SPEC)) {
         Strcat(prefix, sitoa(obj->spe));
         Strcat(prefix, " ");
      }
      break;
   }
   if(obj->owornmask & W_WEP)
      Strcat(bp, " (weapon in hand)");
   if(obj->unpaid)
      Strcat(bp, " (unpaid)");
   if(!strcmp(prefix, "a ") && index(vowels, *bp))
      Strcpy(prefix, "an ");

   bp = strprepend(bp, prefix);
   return(bp);
}

/* used only in hack.fight.c (thitu) */
setan(str,buf)
/*register*/ char *str,*buf;
{
   if(index(vowels,*str))
      Sprintf(buf, "an %s", str);
   else
      Sprintf(buf, "a %s", str);
}

char *
aobjnam(otmp,verb) /*register*/ struct obj *otmp; /*register*/ char *verb; {
/*register*/ char *bp = xname(otmp);
char prefix[PREFIX];
   if(otmp->quan != 1) {
      Sprintf(prefix, "%d ", otmp->quan);
      bp = strprepend(bp, prefix);
   }

   if(verb) {
      /* verb is given in plural (i.e., without trailing s) */
      Strcat(bp, " ");
      if(otmp->quan != 1)
         Strcat(bp, verb);
      else if(!strcmp(verb, "are"))
         Strcat(bp, "is");
      else {
         Strcat(bp, verb);
         Strcat(bp, "s");
      }
   }
   return(bp);
}

char *wrp[] = { "wand", "ring", "potion", "scroll", "gem" };
char wrpsym[] = { WAND_SYM, RING_SYM, POTION_SYM, SCROLL_SYM, GEM_SYM };

struct obj *
readobjnam(bp) /*register*/ char *bp; {
/*register*/ char *p;
/*register*/ int i;
int cnt, spe, spesgn, typ, heavy;
char let;
char *un, *dn, *an;
/* int the = 0; char *oname = 0; */
   cnt = spe = spesgn = typ = heavy = 0;
   let = 0;
   an = dn = un = 0;
   for(p = bp; *p; p++)
      if('A' <= *p && *p <= 'Z') *p += 'a'-'A';
   if(!strncmp(bp, "the ", 4)){
/*      the = 1; */
      bp += 4;
   } else if(!strncmp(bp, "an ", 3)){
      cnt = 1;
      bp += 3;
   } else if(!strncmp(bp, "a ", 2)){
      cnt = 1;
      bp += 2;
   }
   if(!cnt && digit(*bp)){
      cnt = atoi(bp);
      while(digit(*bp)) bp++;
      while(*bp == ' ') bp++;
   }
   if(!cnt) cnt = 1;      /* %% what with "gems" etc. ? */

   if(*bp == '+' || *bp == '-'){
      spesgn = (*bp++ == '+') ? 1 : -1;
      spe = atoi(bp);
      while(digit(*bp)) bp++;
      while(*bp == ' ') bp++;
   } else {
      p = rindex(bp, '(');
      if(p) {
         if(p > bp && p[-1] == ' ') p[-1] = 0;
         else *p = 0;
         p++;
         spe = atoi(p);
         while(digit(*p)) p++;
         if(strcmp(p, ")")) spe = 0;
         else spesgn = 1;
      }
   }
   /* now we have the actual name, as delivered by xname, say
      green potions called whisky
      scrolls labeled "QWERTY"
      egg
      dead zruties
      fortune cookies
      very heavy iron ball named hoei
      wand of wishing
      elven cloak
   */
   for(p = bp; *p; p++) if(!strncmp(p, " named ", 7)) {
      *p = 0;
/*      oname = p+7; */
   }
   for(p = bp; *p; p++) if(!strncmp(p, " called ", 8)) {
      *p = 0;
      un = p+8;
   }
   for(p = bp; *p; p++) if(!strncmp(p, " labeled ", 9)) {
      *p = 0;
      dn = p+9;
   }

   /* first change to singular if necessary */
   if(cnt != 1) {
      /* find "cloves of garlic", "worthless pieces of blue glass" */
      for(p = bp; *p; p++) if(!strncmp(p, "s of ", 5)){
         while(*p = p[1]) p++;
         goto sing;
      }
      /* remove -s or -es (boxes) or -ies (rubies, zruties) */
      p = eos(bp);
      if(p[-1] == 's') {
         if(p[-2] == 'e') {
            if(p[-3] == 'i') {
               if(!strcmp(p-7, "cookies"))
                  goto mins;
               Strcpy(p-3, "y");
               goto sing;
            }

            /* note: cloves / knives from clove / knife */
            if(!strcmp(p-6, "knives")) {
               Strcpy(p-3, "fe");
               goto sing;
            }

            /* note: nurses, axes but boxes */
            if(!strcmp(p-5, "boxes")) {
               p[-2] = 0;
               goto sing;
            }
         }
      mins:
         p[-1] = 0;
      } else {
         if(!strcmp(p-9, "homunculi")) {
            Strcpy(p-1, "us"); /* !! makes string longer */
            goto sing;
         }
         if(!strcmp(p-5, "teeth")) {
            Strcpy(p-5, "tooth");
            goto sing;
         }
         /* here we cannot find the plural suffix */
      }
   }
sing:
   if(!strcmp(bp, "amulet of yendor")) {
      typ = AMULET_OF_YENDOR;
      goto typfnd;
   }
   p = eos(bp);
   if(!strcmp(p-5, " mail")){   /* Note: ring mail is not a ring ! */
      let = ARMOR_SYM;
      an = bp;
      goto srch;
   }
   for(i = 0; i < sizeof(wrpsym); i++) {
      /*register*/ int j = strlen(wrp[i]);
      if(!strncmp(bp, wrp[i], j)){
         let = wrpsym[i];
         bp += j;
         if(!strncmp(bp, " of ", 4)) an = bp+4;
         /* else if(*bp) ?? */
         goto srch;
      }
      if(!strcmp(p-j, wrp[i])){
         let = wrpsym[i];
         p -= j;
         *p = 0;
         if(p[-1] == ' ') p[-1] = 0;
         dn = bp;
         goto srch;
      }
   }
   if(!strcmp(p-6, " stone")){
      p[-6] = 0;
      let = GEM_SYM;
      an = bp;
      goto srch;
   }
   if(!strcmp(bp, "very heavy iron ball")){
      heavy = 1;
      typ = HEAVY_IRON_BALL;
      goto typfnd;
   }
   an = bp;
srch:
   if(!an && !dn && !un)
      goto any;
   i = 1;
   if(let) i = bases[letindex(let)];
   while(i <= NROFOBJECTS && (!let || objects[i].oc_olet == let)){
      if(an && strcmp(an, objects[i].oc_name))
         goto nxti;
      if(dn && strcmp(dn, objects[i].oc_descr))
         goto nxti;
      if(un && strcmp(un, objects[i].oc_uname))
         goto nxti;
      typ = i;
      goto typfnd;
   nxti:
      i++;
   }
any:
   if(!let) let = wrpsym[rn2(sizeof(wrpsym))];
   typ = probtype(let);
typfnd:
   { /*register*/ struct obj *otmp;
     extern struct obj *mksobj();
   let = objects[typ].oc_olet;
   if(let == FOOD_SYM && typ >= CORPSE)
       let = typ-CORPSE+'@'+((typ > CORPSE + 'Z' - '@') ? 'a'-'Z'-1 : 0);
   otmp = mksobj(let, typ);
   if(heavy)
      otmp->owt += 15;
   if(cnt > 0 && index("%?!*)", let) &&
      (cnt < 4 || (let == WEAPON_SYM && typ <= ROCK && cnt < 20)))
      otmp->quan = cnt;
   if(spesgn == -1)
      otmp->cursed = 1;
   if(spe > 3 && spe > otmp->spe)
      spe = 0;
   else if(let == WAND_SYM)
      spe = otmp->spe;
   if(spe == 3 && u.uluck < 0)
      spesgn = -1;
   if(let != WAND_SYM && spesgn == -1)
      spe = -spe;
   if(let == BALL_SYM)
      spe = 0;
   else if(let == AMULET_SYM)
      spe = -1;
   else if(typ == WAN_WISHING && rn2(10))
      spe = 0;
   else if(let == ARMOR_SYM) {
      spe += 10 - objects[typ].a_ac;
      if(spe > 5 && rn2(spe - 5))
         otmp->cursed = 1;
   }
   otmp->spe = spe;
   return(otmp);
   }
}
#file hack.options.c
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* hack.options.c version 1.0.1 - added HACKOPTIONS */

#include "config.h"
#ifdef OPTIONS
#include "hack.h"
extern char *eos();

initoptions()
{
	register char *opts;
	extern char *getenv();

	flags.time = flags.nonews = flags.notombstone = flags.end_own =
	flags.no_rest_on_space = FALSE;
	flags.end_top = 5;
	flags.end_around = 4;
	/* flags.oneline is set in hack.tty.c depending on the baudrate */

	if(opts = getenv("HACKOPTIONS"))
		parseoptions(opts,TRUE);
}

parseoptions(opts, from_env)
register char *opts;
boolean from_env;
{
	register char *op,*op2;
	unsigned num;
	boolean negated;

	if(op = index(opts, ',')) {
		*op++ = 0;
		parseoptions(op, from_env);
	}
	if(op = index(opts, ' ')) {
		op2 = op;
		while(*op++)
			if(*op != ' ') *op2++ = *op;
	}
	if(!*opts) return;
	negated = FALSE;
	while((*opts == '!') || !strncmp(opts, "no", 2)) {
		if(*opts == '!') opts++; else opts += 2;
		negated = !negated;
	}
	if(!strncmp(opts,"tombstone",4)) {
		flags.notombstone = negated;
		return;
	}
	if(!strncmp(opts,"news",4)) {
		flags.nonews = negated;
		return;
	}
	if(!strncmp(opts,"time",4)) {
		flags.time = !negated;
		flags.botl = 1;
		return;
	}
	if(!strncmp(opts,"oneline",1)) {
		flags.oneline = !negated;
		return;
	}
	if(!strncmp(opts,"restonspace",4)) {
		flags.no_rest_on_space = negated;
		return;
	}
	/* endgame:5t[op] 5a[round] o[wn] */
	if(!strncmp(opts,"endgame",3)) {
		op = index(opts,':');
		if(!op) goto bad;
		op++;
		while(*op) {
			num = 1;
			if(digit(*op)) {
				num = atoi(op);
				while(digit(*op)) op++;
			} else
			if(*op == '!') {
				negated = !negated;
				op++;
			}
			switch(*op) {
			case 't':
				flags.end_top = num;
				break;
			case 'a':
				flags.end_around = num;
				break;
			case 'o':
				flags.end_own = !negated;
				break;
			default:
				goto bad;
			}
			while(letter(*++op)) ;
			if(*op == '/') op++;
		}
		return;
	}
bad:
	if(!from_env) {
		if(!strncmp(opts, "help", 4)) {
			pline("%s%s%s",
"To set options use `HACKOPTIONS=\"<options>\"' in your environment, or ",
"give the command 'o' followed by the line `<options>' while playing. ",
"Here <options> is a list of <option>s separated by commas." );
			pline("%s%s",
"Simple (boolean) options are oneline,rest_on_space,news,time,tombstone. ",
"These can be negated by prefixing them with '!' or \"no\"." );
			pline("%s%s%s",
"A compound option is endgame; it is followed by a description of what ",
"parts of the scorelist you want to see. You might for example say: ",
"`endgame:own scores/5 top scores/4 around my score'." );
			return;
		}
		pline("Bad option: %s.", opts);
		pline("Type `o help<cr>' for help.");
		return;
	}
	puts("Bad syntax in HACKOPTIONS.");
	puts("Use for example:");
	puts(
"HACKOPTIONS=\"!restonspace,notombstone,endgame:own/5 topscorers/4 around me\""
	);
	getret();
}

doset()
{
	char buf[BUFSZ];

	pline("What options do you want to set? ");
	getlin(buf);
	if(!buf[0]) {
	    (void) strcpy(buf,"HACKOPTIONS=");
	    if(flags.oneline) (void) strcat(buf,"oneline,");
	    if(flags.nonews) (void) strcat(buf,"nonews,");
	    if(flags.time) (void) strcat(buf,"time,");
	    if(flags.notombstone) (void) strcat(buf,"notombstone,");
	    if(flags.no_rest_on_space)
		(void) strcat(buf,"!rest_on_space,");
	    if(flags.end_top != 5 || flags.end_around != 4 || flags.end_own){
		(void) sprintf(eos(buf), "endgame: %d topscores/%d around me",
			flags.end_top, flags.end_around);
		if(flags.end_own) (void) strcat(buf, "/own scores");
	    } else {
		register char *eop = eos(buf);
		if(*--eop == ',') *eop = 0;
	    }
	    pline(buf);
	} else
	    parseoptions(buf, FALSE);

	return(0);
}
#endif OPTIONS
#file hack.o_init.c
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1984. */

#include   "config.h"      /* for typedefs */
#include   "def.objects.h"
extern char ismklev;

int
letindex(let) register char let; {
register int i = 0;
register char ch;
   while((ch = obj_symbols[i++]) != 0)
      if(ch == let) return(i);
   return(0);
}

init_objects(){
register int i, j, first, last, sum, end;
register char let, *tmp;
   /* init base; if probs given check that they add up to 100, 
      otherwise compute probs; shuffle descriptions */
   end = sizeof(objects)/sizeof(objects[0]);
   first = 0;
   while( first < end ) {
      let = objects[first].oc_olet;
      last = first+1;
      while(last < end && objects[last].oc_olet == let
            && objects[last].oc_name != NULL)
         last++;
      i = letindex(let);
      if((!i && let != ILLOBJ_SYM) || bases[i] != 0)
         panic("initialization error");
      bases[i] = first;
   check:
#include   "hack.onames.h"
      if(ismklev && let == GEM_SYM) {
         extern xchar dlevel;
         for(j=0; j < 9-dlevel/3; j++)
            objects[first+j].oc_prob = 0;
         first += j;
         if(first >= last || first >= LAST_GEM)
            printf("Not enough gems? - first=%d last=%d j=%d LAST_GEM=%d\n", first, last, j, LAST_GEM);
         for(j = first; j < LAST_GEM; j++)
             objects[j].oc_prob = (20+j-first)/(LAST_GEM-first);
      }
      sum = 0;
      for(j = first; j < last; j++) sum += objects[j].oc_prob;
      if(sum == 0) {
         for(j = first; j < last; j++)
             objects[j].oc_prob = (100+j-first)/(last-first);
         goto check;
      }
      if(sum != 100)
         panic ("init-prob error for %c", let);
      /* shuffling is rather meaningless in mklev, 
         but we must update  last  anyway */
      if(objects[first].oc_descr != NULL && let != TOOL_SYM){
         /* shuffle, also some additional descriptions */
         while(last < end && objects[last].oc_olet == let)
            last++;
         j = last;
         while(--j > first) {
            i = first + rn2(j+1-first);
            tmp = objects[j].oc_descr;
            objects[j].oc_descr = objects[i].oc_descr;
            objects[i].oc_descr = tmp;
         }
      }
      first = last;
   }
}

probtype(let) register char let; {
register int i = bases[letindex(let)];
register int prob = rn2(100);
   while((prob -= objects[i].oc_prob) >= 0) i++;
   if(objects[i].oc_olet != let || !objects[i].oc_name)
      panic("probtype(%c) error, i=%d", let, i);
   return(i);
}

#define SIZE(x) (sizeof x)/(sizeof x[0])
extern long *alloc();

savenames(fd) register int fd; {
register int i;
unsigned len;
	bwrite(fd, (char *) bases, sizeof bases);
	bwrite(fd, (char *) objects, sizeof objects);
	/* as long as we use only one version of Hack/Quest we
		need not save oc_name and oc_descr, but we must save
		oc_uname for all objects */
	/* this assumes we always load at the same place */
	for(i=0; i < SIZE(objects); i++) {
#ifdef AMIGA
		if(objects[i].oc_name) {
			len = strlen(objects[i].oc_name)+1;
			bwrite(fd, (char *) &len, sizeof len);
			bwrite(fd, objects[i].oc_name, len);
		}
		if(objects[i].oc_descr) {
			len = strlen(objects[i].oc_descr)+1;
			bwrite(fd, (char *) &len, sizeof len);
			bwrite(fd, objects[i].oc_descr, len);
		}
#endif
		if(objects[i].oc_uname) {
			len = strlen(objects[i].oc_uname)+1;
			bwrite(fd, (char *) &len, sizeof len);
			bwrite(fd, objects[i].oc_uname, len);
		}
	}
}

restnames(fd) register int fd; {
register int i;
unsigned len;
	mread(fd, (char *) bases, sizeof bases);
	mread(fd, (char *) objects, sizeof objects);
	for(i=0; i < SIZE(objects); i++) {
		if(objects[i].oc_name) {
			mread(fd, (char *) &len, sizeof len);
			objects[i].oc_name = (char *) alloc(len);
			mread(fd, objects[i].oc_name, len);
		}
		if(objects[i].oc_descr) {
			mread(fd, (char *) &len, sizeof len);
			objects[i].oc_descr = (char *) alloc(len);
			mread(fd, objects[i].oc_descr, len);
		}
		if(objects[i].oc_uname) {
			mread(fd, (char *) &len, sizeof len);
			objects[i].oc_uname = (char *) alloc(len);
			mread(fd, objects[i].oc_uname, len);
		}
	}
}
