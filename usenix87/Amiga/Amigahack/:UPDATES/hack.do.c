
#file hack.do.c
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* hack.do.c version 1.0.1 - check Levitation with POT_PARALYSIS
			   - added flags.no_rest_on_space */

#include <stdio.h>
#include <signal.h>
#include <fcntl.h>
#include "hack.h"
#include "def.func_tab.h"

extern char *getenv(),*parse(),*getlogin(),*lowc(),*unctrl();
extern int float_down();
extern char *nomovemsg, *catmore;
extern struct obj *splitobj(), *addinv();
extern boolean hmon();
extern char morc;

/*   Routines to do various user commands */

int done1();

dodrink() {
   register struct obj *otmp,*objs;
   register struct monst *mtmp;
   register int unkn = 0, nothing = 0;

   otmp = getobj("!", "drink");
   if(!otmp) return(0);
   switch(otmp->otyp){
   case POT_RESTORE_STRENGTH:
      unkn++;
      pline("Wow!  This makes you feel great!");
      if(u.ustr < u.ustrmax) {
         u.ustr = u.ustrmax;
         flags.botl = 1;
      }
      break;
   case POT_BOOZE:
      unkn++;
      pline("Ooph!  This tastes like liquid fire!");
      Confusion += d(3,8);
      /* the whiskey makes us feel better */
      if(u.uhp < u.uhpmax) losehp(-1, "bottle of whiskey");
      if(!rn2(4)) {
         pline("You pass out.");
         multi = -rnd(15);
         nomovemsg = "You awake with a headache.";
      }
      break;
   case POT_INVISIBILITY:
      if(Invis)
        nothing++;
      else {
        if(!Blind)
          pline("Gee!  All of a sudden, you can't see yourself.");
        else
          pline("You feel rather airy."), unkn++;
        newsym(u.ux,u.uy);
      }
      Invis += rn1(15,31);
      break;
   case POT_FRUIT_JUICE:
      pline("This tastes like fruit juice.");
      lesshungry(20);
      break;
   case POT_HEALING:
      pline("You begin to feel better.");
      flags.botl = 1;
      u.uhp += rnd(10);
      if(u.uhp > u.uhpmax)
         u.uhp = ++u.uhpmax;
      if(Blind) Blind = 1;   /* see on next move */
      if(Sick) Sick = 0;
      break;
   case POT_PARALYSIS:
		if(Levitation)
			pline("Your head is frozen to the ceiling!");
		else
			pline("Your feet are frozen to the floor!");
		nomul(-(rn1(10,25)));
		break;
   case POT_MONSTER_DETECTION:
      if(!fmon) {
         strange_feeling(otmp);
         return(1);
      } else {
         cls();
         for(mtmp = fmon; mtmp; mtmp = mtmp->nmon)
            if(mtmp->mx > 0)
            at(mtmp->mx,mtmp->my,mtmp->data->mlet);
         prme();
         pline("You sense the presence of monsters.");
         more();
         docrt();
      }
      break;
   case POT_OBJECT_DETECTION:
      if(!fobj) {
         strange_feeling(otmp);
         return(1);
      } else {
          for(objs = fobj; objs; objs = objs->nobj)
         if(objs->ox != u.ux || objs->oy != u.uy)
            goto outobjmap;
          pline("You sense the presence of objects close nearby.");
          break;
      outobjmap:
         cls();
         for(objs = fobj; objs; objs = objs->nobj)
            at(objs->ox,objs->oy,objs->olet);
         prme();
         pline("You sense the presence of objects.");
         more();
         docrt();
      }
      break;
   case POT_SICKNESS:
      pline("Yech! This stuff tastes like poison.");
      if(Poison_resistance)
    pline("(But in fact it was biologically contaminated orange juice.)");
      losestr(rn1(4,3));
      losehp(rnd(10), "poison potion");
      break;
   case POT_CONFUSION:
      if(!Confusion)
         pline("Huh, What?  Where am I?");
      else
         nothing++;
      Confusion += rn1(7,16);
      break;
   case POT_GAIN_STRENGTH:
      pline("Wow do you feel strong!");
      if(u.ustr == 118) break;
      if(u.ustr > 17) u.ustr += rnd(118-u.ustr);
      else u.ustr++;
      if(u.ustr > u.ustrmax) u.ustrmax = u.ustr;
      flags.botl = 1;
      break;
   case POT_SPEED:
      if(Wounded_legs) {
         if((Wounded_legs & BOTH_SIDES) == BOTH_SIDES)
            pline("Your legs feel somewhat better.");
         else
            pline("Your leg feels somewhat better.");
         Wounded_legs = 0;
         unkn++;
         break;
      }
      if(!(Fast & ~INTRINSIC))
         pline("You are suddenly moving much faster.");
      else
         pline("Your legs get new energy."), unkn++;
      Fast += rn1(10,100);
      break;
   case POT_BLINDNESS:
      if(!Blind)
         pline("A cloud of darkness falls upon you.");
      else
         nothing++;
      Blind += rn1(100,250);
      seeoff(0);
      break;
   case POT_GAIN_LEVEL:
      pluslvl();
      break;
   case POT_EXTRA_HEALING:
      pline("You feel much better.");
      flags.botl = 1;
      u.uhp += d(2,20)+1;
      if(u.uhp > u.uhpmax)
         u.uhp = (u.uhpmax += 2);
      if(Blind) Blind = 1;
      if(Sick) Sick = 0;
      break;
   case POT_LEVITATION:
      if(!Levitation)
         float_up();
      else
         nothing++;
      Levitation += rnd(100);
      u.uprops[PROP(RIN_LEVITATION)].p_tofn = float_down;
      break;
   default:
      pline("What a funny potion! (%d)", otmp->otyp);
      impossible();
      return(0);
   }
   if(nothing) {
       unkn++;
       pline("You have a peculiar feeling for a moment, then it passes.");
   }
   if(otmp->dknown && !objects[otmp->otyp].oc_name_known) {
      if(!unkn) {
         objects[otmp->otyp].oc_name_known = 1;
         u.urexp += 10;
      } else if(!objects[otmp->otyp].oc_uname)
         docall(otmp);
   }
   useup(otmp);
   return(1);
}

pluslvl()
{
   register int num;

   pline("You feel more experienced.");
   num = rnd(10);
   u.uhpmax += num;
   u.uhp += num;
   u.uexp = (10*pow(u.ulevel-1))+1;
   pline("Welcome to level %d.", ++u.ulevel);
   flags.botl = 1;
}

strange_feeling(obj)
register struct obj *obj;
{
   pline("You have a strange feeling for a moment, then it passes.");
   if(!objects[obj->otyp].oc_name_known && !objects[obj->otyp].oc_uname)
      docall(obj);
   useup(obj);
}

dodrop() {
   register struct obj *obj;

   obj = getobj("0$#", "drop");
   if(!obj) return(0);
   if(obj->olet == '$') {
      if(obj->quan == 0)
         pline("You didn't drop any gold pieces.");
      else {
         mkgold((int) obj->quan, u.ux, u.uy);
         pline("You dropped %u gold piece%s.",
            obj->quan, plur(obj->quan));
         if(Invis) newsym(u.ux, u.uy);
      }
      free((char *) obj);
      return(1);
   }
   return(drop(obj));
}

drop(obj) register struct obj *obj; {
   if(obj->owornmask & (W_ARMOR | W_RING)){
      pline("You cannot drop something you are wearing.");
      return(0);
   }
   if(obj == uwep) {
      if(uwep->cursed) {
         pline("Your weapon is welded to your hand!");
         return(0);
      }
      setuwep((struct obj *) 0);
   }
   pline("You dropped %s.", doname(obj));
   dropx(obj);
   return(1);
}

dropx(obj) register struct obj *obj; {
   if(obj->otyp == CRYSKNIFE)
      obj->otyp = WORM_TOOTH;
   freeinv(obj);
   obj->ox = u.ux;
   obj->oy = u.uy;
   obj->nobj = fobj;
   fobj = obj;
   if(Invis) newsym(u.ux,u.uy);
   subfrombill(obj);
   stackobj(obj);
}

/* drop several things */
doddrop() {
   return(ggetobj("drop", drop, 0));
}

rhack(cmd)
register char *cmd;
{
   register struct func_tab *tlist = list;
   boolean firsttime = FALSE;
   register int res;

   if(!cmd) {
      firsttime = TRUE;
      flags.nopick = 0;
      cmd = parse();
   }
	if(!*cmd || *cmd == 0377 || (flags.no_rest_on_space && *cmd == ' ')){
		flags.move = 0;
		return;      /* probably we just had an interrupt */
	}
   if(movecm(cmd)) {
   walk:
      if(multi) flags.mv = 1;
      domove();
      return;
   }
   if(movecm(lowc(cmd))) {
      flags.run = 1;
   rush:
      if(firsttime){
         if(!multi) multi = COLNO;
         u.last_str_turn = 0;
      }
      flags.mv = 1;
#ifdef QUEST
      if(flags.run >= 4) finddir();
      if(firsttime){
         u.ux0 = u.ux + u.dx;
         u.uy0 = u.uy + u.dy;
      }
#endif QUEST
      domove();
      return;
   }
   if((*cmd == 'f' && movecm(cmd+1)) ||
      movecm(unctrl(cmd))) {
      flags.run = 2;
      goto rush;
   }
   if(*cmd == 'F' && movecm(lowc(cmd+1))) {
      flags.run = 3;
      goto rush;
   }
   if(*cmd == 'm' && movecm(cmd+1)) {
      flags.run = 0;
      flags.nopick = 1;
      goto walk;
   }
   if(*cmd == 'M' && movecm(lowc(cmd+1))) {
      flags.run = 1;
      flags.nopick = 1;
      goto rush;
   }
#ifdef QUEST
   if(*cmd == cmd[1] && (*cmd == 'f' || *cmd == 'F')) {
      flags.run = 4;
      if(*cmd == 'F') flags.run += 2;
      if(cmd[2] == '-') flags.run += 1;
      goto rush;
   }
#endif QUEST
   while(tlist->f_char) {
      if(*cmd == tlist->f_char){
         res = (*(tlist->f_funct))(0);
         if(!res) {
            flags.move = 0;
            multi = 0;
         }
         return;
      }
      tlist++;
   }
   pline("Unknown command '%s'",cmd);
   multi = flags.move = 0;
}

doredraw()
{
   docrt();
   return(0);
}

dohelp()
{
	FILE *fp;
	char bufr[BUFSZ];
	int line, i;
	
	if ( (fp = fopen(HELP,"r")) == NULL)
		pline("cannot access help");
	else
		{
		cls();
		line = 1;
		while(fgets(bufr,BUFSZ,fp))
			{
			myprintf("%s", bufr);
			if (line++ > ROWNO)
				{
				myprintf("---more---");
				xwaitforspace(FALSE);
				morc = 0;
				for (i=0;i<10;i++)
					backsp();
				cl_end();
				line = 1;
				}
			}
		more();
		docrt();
		}
}

#ifdef SHELL
dosh(){
   char *file, *Open();
   if ( (file = Open("CON:1/1/639/199/Hack SubProcess", 1006)) == NULL)
	pline("cannot create process window");
   if (Execute("", file, NULL))
      pline("cannot execute commands");
   Close(file);
   return(0);
}
#endif SHELL

child(wt) {
   pline("Cannot create children");
   docrt();
   return(0);
}

dodown()
{
   if(u.ux != xdnstair || u.uy != ydnstair) {
      pline("You can't go down here.");
      return(0);
   }
   if(u.ustuck) {
      pline("You are being held, and cannot go down.");
      return(1);
   }
   if(Levitation) {
      pline("You're floating high above the stairs.");
      return(0);
   }

   goto_level(dlevel+1, TRUE);
   return(1);
}

doup()
{
   if(u.ux != xupstair || u.uy != yupstair) {
      pline("You can't go up here.");
      return(0);
   }
   if(u.ustuck) {
      pline("You are being held, and cannot go up.");
      return(1);
   }
   if(inv_weight() + 5 > 0) {
      pline("Your load is too heavy to climb the stairs.");
      return(1);
   }

   goto_level(dlevel-1, TRUE);
   return(1);
}

goto_level(newlevel, at_stairs)
register int newlevel;
register boolean at_stairs;
{
   register int fd;
   register boolean up = (newlevel < dlevel);

	if(newlevel <= 0) done("escaped");    /* in fact < 0 is impossible */
	if(newlevel == dlevel) return;	      /* this cannot happen either */

   glo(dlevel);
   fd = creat(lock,FMASK);
   if(fd < 0) {
      /*
       * This is not quite impossible: e.g., we may have
       * exceeded our quota. If that is the case then we
       * cannot leave this level, and cannot save either.
       */
      pline("A mysterious force prevents you from going %s.",
         up ? "up" : "down");
      return;
   }

   if(Punished) unplacebc();
   keepdogs();
   seeoff(1);
   flags.nscrinh = 1;
   u.ux = FAR;            /* hack */
   (void) inshop();         /* probably was a trapdoor */

   savelev(fd);
   (void) close(fd);

   dlevel = newlevel;
   if(maxdlevel < dlevel)
      maxdlevel = dlevel;
   glo(dlevel);
   if((fd = open(lock,0)) < 0)
      mklev();
   else {
      (void) getlev(fd);
      (void) close(fd);
   }

   if(at_stairs) {
       if(up) {
      u.ux = xdnstair;
      u.uy = ydnstair;
      if(!u.ux) {      /* entering a maze from below? */
          u.ux = xupstair;   /* this will confuse the player! */
          u.uy = yupstair;
      }
      if(Punished){
         pline("With great effort you climb the stairs");
         placebc(1);
      }
       } else {
      u.ux = xupstair;
      u.uy = yupstair;
      if(inv_weight() + 5 > 0 || Punished){
         pline("You fall down the stairs.");
         losehp(rnd(3), "fall");
         if(Punished) {
             if(uwep != uball && rn2(3)){
            pline("... and are hit by the iron ball");
            losehp(rnd(20), "iron ball");
             }
             placebc(1);
         }
         selftouch("Falling, you");
      }
       }
   } else {   /* trapdoor or level_tele */
       do {
      u.ux = rnd(COLNO-1);
      u.uy = rn2(ROWNO);
       } while(levl[u.ux][u.uy].typ != ROOM ||
         m_at(u.ux,u.uy));
       if(Punished){
      if(uwep != uball && !up /* %% */ && rn2(5)){
         pline("The iron ball falls on your head.");
         losehp(rnd(25), "iron ball");
      }
      placebc(1);
       }
       selftouch("Falling, you");
   }
   (void) inshop();
#ifdef TRACK
   initrack();
#endif TRACK

   losedogs();
   flags.nscrinh = 0;
   setsee();
	{ register struct monst *mtmp;
	  if(mtmp = m_at(u.ux, u.uy)) mnexto(mtmp);	/* riv05!a3 */
	}
   docrt();
   pickup();
   read_engr_at(u.ux,u.uy);
}

donull() {
   return(1);   /* Do nothing, but let other things happen */
}

struct monst *bhit(), *boomhit();
dothrow()
{
   register struct obj *obj;
   register struct monst *mon;
   register int tmp;

	obj = getobj("#)", "throw");   /* it is also possible to throw food */
				       /* (or jewels, or iron balls ... ) */
   if(!obj || !getdir())
      return(0);
   if(obj->owornmask & (W_ARMOR | W_RING)){
      pline("You can't throw something you are wearing");
      return(0);
   }
   if(obj == uwep){
      if(obj->cursed){
         pline("Your weapon is welded to your hand");
         return(1);
      }
      if(obj->quan > 1)
         setuwep(splitobj(obj, 1));
      else
         setuwep((struct obj *) 0);
   }
   else if(obj->quan > 1)
      (void) splitobj(obj, 1);
   freeinv(obj);
   if(u.uswallow) {
      mon = u.ustuck;
      bhitpos.x = mon->mx;
      bhitpos.y = mon->my;
   } else if(obj->otyp == BOOMERANG) {
      mon = boomhit(u.dx,u.dy);
      /* boomhit delivers -1 if the thing was caught */
      if((int) mon == -1) {
         (void) addinv(obj);
         return(1);
      }
   } else
      mon = bhit(u.dx,u.dy,
         (!Punished || obj != uball) ? 8 :
            !u.ustuck ? 5 : 1,
         obj->olet);
   if(mon) {
      /* awake monster if sleeping */
      wakeup(mon);

      if(obj->olet == WEAPON_SYM) {
         tmp = -1+u.ulevel+mon->data->ac+abon();
         if(obj->otyp < ROCK) {
            if(!uwep ||
                uwep->otyp != obj->otyp+(BOW-ARROW))
               tmp -= 4;
            else {
               tmp += uwep->spe;
            }
         } else
         if(obj->otyp == BOOMERANG) tmp += 4;
         tmp += obj->spe;
         if(u.uswallow || tmp >= rnd(20)) {
            if(hmon(mon,obj,1) == TRUE){
              /* mon still alive */
#ifndef NOWORM
              cutworm(mon,bhitpos.x,bhitpos.y,obj->otyp);
#endif NOWORM
            } else mon = 0;
            /* weapons thrown disappear sometimes */
            if(obj->otyp < BOOMERANG && rn2(3)) {
               /* check bill; free */
               obfree(obj, (struct obj *) 0);
               return(1);
            }
         } else miss(objects[obj->otyp].oc_name, mon);
      } else if(obj->otyp == HEAVY_IRON_BALL) {
         tmp = -1+u.ulevel+mon->data->ac+abon();
         if(!Punished || obj != uball) tmp += 2;
         if(u.utrap) tmp -= 2;
         if(u.uswallow || tmp >= rnd(20)) {
            if(hmon(mon,obj,1) == FALSE)
               mon = 0;   /* he died */
         } else miss("iron ball", mon);
      } else {
         if(cansee(bhitpos.x,bhitpos.y))
            pline("You miss %s.",monnam(mon));
         else pline("You miss it.");
         if(obj->olet == FOOD_SYM && mon->data->mlet == 'd')
            if(tamedog(mon,obj)) return(1);
         if(obj->olet == GEM_SYM && mon->data->mlet == 'u'){
          if(obj->dknown && objects[obj->otyp].oc_name_known){
           if(objects[obj->otyp].g_val > 0){
             u.uluck += 5;
             goto valuable;
           } else {
             pline("%s is not interested in your junk.",
            Monnam(mon));
           }
          } else { /* value unknown to @ */
             u.uluck++;
         valuable:
             pline("%s graciously accepts your gift.",
            Monnam(mon));
             mpickobj(mon, obj);
             rloc(mon);
             return(1);
          }
         }
      }
   }
   obj->ox = bhitpos.x;
   obj->oy = bhitpos.y;
   obj->nobj = fobj;
   fobj = obj;
   /* prevent him from throwing articles to the exit and escaping */
   /* subfrombill(obj); */
   stackobj(obj);
   if(Punished && obj == uball &&
      (bhitpos.x != u.ux || bhitpos.y != u.uy)){
      freeobj(uchain);
      unpobj(uchain);
      if(u.utrap){
         if(u.utraptype == TT_PIT)
            pline("The ball pulls you out of the pit!");
         else {
             register long side =
            rn2(3) ? LEFT_SIDE : RIGHT_SIDE;
             pline("The ball pulls you out of the bear trap.");
             pline("Your %s leg is severely damaged.",
            (side == LEFT_SIDE) ? "left" : "right");
             Wounded_legs |= side + rnd(1000);
             losehp(2, "thrown ball");
         }
         u.utrap = 0;
      }
      unsee();
      uchain->nobj = fobj;
      fobj = uchain;
      u.ux = uchain->ox = bhitpos.x - u.dx;
      u.uy = uchain->oy = bhitpos.y - u.dy;
      setsee();
      (void) inshop();
   }
   if(cansee(bhitpos.x, bhitpos.y)) prl(bhitpos.x,bhitpos.y);
   return(1);
}

/* split obj so that it gets size num */
/* remainder is put in the object structure delivered by this call */
struct obj *
splitobj(obj, num) register struct obj *obj; register int num; {
register struct obj *otmp;
   otmp = newobj(0);
   *otmp = *obj;      /* copies whole structure */
   otmp->o_id = flags.ident++;
   otmp->onamelth = 0;
   obj->quan = num;
   obj->owt = weight(obj);
   otmp->quan -= num;
   otmp->owt = weight(otmp);   /* -= obj->owt ? */
   obj->nobj = otmp;
   if(obj->unpaid) splitbill(obj,otmp);
   return(otmp);
}

char *
lowc(str)
register char *str;
{
   static char buf[2];

   if(*str >= 'A' && *str <= 'Z') *buf = *str+'a'-'A';
   else *buf = *str;
   buf[1] = 0;
   return(buf);
}

char *
unctrl(str)
register char *str;
{
   static char buf[2];
   if(*str >= ('A' & 037) && *str <= ('Z' & 037))
      *buf = *str + 0140;
   else *buf = *str;
   buf[1] = 0;
   return(buf);
}

#file hack.dog.c
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* hack.dog.c version 1.0.1 - "You feel worried about %s." (Adri Verhoef) */

#include	"hack.h"
#include	"hack.mfndpos.h"
extern char POISONOUS[];
extern struct monst *makemon();
#include "def.edog.h"

makedog(){
register struct monst *mtmp = makemon(PM_LI_DOG,u.ux,u.uy);
	if(!mtmp) return; /* dogs were genocided */
	initedog(mtmp);
}

initedog(mtmp) register struct monst *mtmp; {
	mtmp->mtame = mtmp->mpeaceful = 1;
	EDOG(mtmp)->hungrytime = 1000 + moves;
	EDOG(mtmp)->eattime = 0;
	EDOG(mtmp)->droptime = 0;
	EDOG(mtmp)->dropdist = 10000;
	EDOG(mtmp)->apport = 10;
	EDOG(mtmp)->whistletime = 0;
}

/* attach the monsters that went down (or up) together with @ */
struct monst *mydogs = 0;
struct monst *fallen_down = 0;	/* monsters that fell through a trapdoor */

losedogs(){
register struct monst *mtmp;
	while(mtmp = mydogs){
		mydogs = mtmp->nmon;
		mtmp->nmon = fmon;
		fmon = mtmp;
		mnexto(mtmp);
	}
	while(mtmp = fallen_down){
		fallen_down = mtmp->nmon;
		mtmp->nmon = fmon;
		fmon = mtmp;
		rloc(mtmp);
	}
}

keepdogs(){
register struct monst *mtmp;
	for(mtmp = fmon; mtmp; mtmp = mtmp->nmon) if(mtmp->mtame) {
		if(dist(mtmp->mx,mtmp->my) > 2) {
			mtmp->mtame = 0;	/* dog becomes wild */
			mtmp->mpeaceful = 0;
			continue;
		}
		relmon(mtmp);
		mtmp->nmon = mydogs;
		mydogs = mtmp;
		unpmon(mtmp);
		keepdogs();	/* we destroyed the link, so use recursion */
		return;		/* (admittedly somewhat primitive) */
	}
}

fall_down(mtmp) register struct monst *mtmp; {
	relmon(mtmp);
	mtmp->nmon = fallen_down;
	fallen_down = mtmp;
	unpmon(mtmp);
	mtmp->mtame = 0;
}

/* return quality of food; the lower the better */
#define	DOGFOOD	0
#define	CADAVER	1
#define	ACCFOOD	2
#define	MANFOOD	3
#define	APPORT	4
#define	POISON	5
#define	UNDEF	6
dogfood(obj) register struct obj *obj; {
	switch(obj->olet) {
	case FOOD_SYM:
	    return(
		(obj->otyp == TRIPE_RATION) ? DOGFOOD :
		(obj->otyp < CARROT) ? ACCFOOD :
		(obj->otyp < CORPSE) ? MANFOOD :
		(index(POISONOUS, obj->spe) || obj->age + 50 <= moves ||
		    obj->otyp == DEAD_COCKATRICE)
			? POISON : CADAVER
	    );
	default:
	    if(!obj->cursed) return(APPORT);
	    /* fall into next case */
	case BALL_SYM:
	case CHAIN_SYM:
	case ROCK_SYM:
	    return(UNDEF);
	}
}

/* return 0 (no move), 1 (move) or 2 (dead) */
dog_move(mtmp, after) register struct monst *mtmp; {
register int nx,ny,omx,omy,appr,nearer,j;
int udist,chi,i,whappr;
register struct monst *mtmp2;
register struct permonst *mdat = mtmp->data;
register struct edog *edog = EDOG(mtmp);
struct obj *obj;
struct gen *trap;
xchar cnt,chcnt,nix,niy;
schar dogroom,uroom;
xchar gx,gy,gtyp,otyp;	/* current goal */
coord poss[9];
int info[9];
#define GDIST(x,y) ((x-gx)*(x-gx) + (y-gy)*(y-gy))
#define DDIST(x,y) ((x-omx)*(x-omx) + (y-omy)*(y-omy))

	if(moves <= edog->eattime) return(0);	/* dog is still eating */
	omx = mtmp->mx;
	omy = mtmp->my;
	whappr = (moves - EDOG(mtmp)->whistletime < 5);
	if(moves > edog->hungrytime + 500 && !mtmp->mconf){
		mtmp->mconf = 1;
		mtmp->orig_hp /= 3;
		if(mtmp->mhp > mtmp->orig_hp)
			mtmp->mhp = mtmp->orig_hp;
		if(cansee(omx,omy))
			pline("%s is confused from hunger", Monnam(mtmp));
		else	pline("You feel worried about %s.", monnam(mtmp));
	} else
	if(moves > edog->hungrytime + 750 || mtmp->mhp < 1){
		if(cansee(omx,omy))
			pline("%s dies from hunger", Monnam(mtmp));
		else
		pline("You have a sad feeling for a moment, then it passes");
		mondied(mtmp);
		return(2);
	}
	dogroom = inroom(omx,omy);
	uroom = inroom(u.ux,u.uy);
	udist = dist(omx,omy);

	/* if we are carrying sth then we drop it (perhaps near @) */
	/* Note: if apport == 1 then our behaviour is independent of udist */
	if(mtmp->minvent){
		if(!rn2(udist) || !rn2((int) edog->apport))
		if(rn2(10) < edog->apport){
			relobj(mtmp,0);
			if(edog->apport > 1) edog->apport--;
		}
	} else {
		if(obj = o_at(omx,omy)) if(!index("0_", obj->olet)){
		    if((otyp = dogfood(obj)) <= CADAVER){
			nix = omx;
			niy = omy;
			goto eatobj;
		    }
		    if(obj->owt < 10*mtmp->data->mlevel)
		    if(rn2(20) < edog->apport+3)
		    if(rn2(udist) || !rn2((int) edog->apport)){
			freeobj(obj);
			unpobj(obj);
			/* if(levl[omx][omy].scrsym == obj->olet)
				newsym(omx,omy); */
			mpickobj(mtmp,obj);
		    }
		}
	}

	/* first we look for food */
	gtyp = UNDEF;	/* no goal as yet */
#ifdef LINT
	gx = gy = 0;
#endif LINT
	for(obj = fobj; obj; obj = obj->nobj) {
		otyp = dogfood(obj);
		if(otyp > gtyp || otyp == UNDEF) continue;
		if(inroom(obj->ox,obj->oy) != dogroom) continue;
		if(otyp < MANFOOD &&
		 (dogroom >= 0 || DDIST(obj->ox,obj->oy) < 10)) {
			if(otyp < gtyp || (otyp == gtyp &&
				DDIST(obj->ox,obj->oy) < DDIST(gx,gy))){
				gx = obj->ox;
				gy = obj->oy;
				gtyp = otyp;
			}
		} else
		if(gtyp == UNDEF && dogroom >= 0 &&
		   uroom == dogroom &&
		   !mtmp->minvent && edog->apport > rn2(8)){
			gx = obj->ox;
			gy = obj->oy;
			gtyp = APPORT;
		}
	}
	if(gtyp == UNDEF ||
	  (gtyp != DOGFOOD && gtyp != APPORT && moves < edog->hungrytime)){
		if(dogroom < 0 || dogroom == uroom){
			gx = u.ux;
			gy = u.uy;
#ifndef QUEST
		} else {
			int tmp = rooms[dogroom].fdoor;
			    cnt = rooms[dogroom].doorct;

			gx = gy = FAR;	/* random, far away */
			while(cnt--){
			    if(dist(gx,gy) >
				dist(doors[tmp].x, doors[tmp].y)){
					gx = doors[tmp].x;
					gy = doors[tmp].y;
				}
				tmp++;
			}
			/* here gx == FAR e.g. when dog is in a vault */
			if(gx == FAR || (gx == omx && gy == omy)){
				gx = u.ux;
				gy = u.uy;
			}
#endif QUEST
		}
		appr = (udist >= 9) ? 1 : (mtmp->mflee) ? -1 : 0;
		if(after && udist <= 4 && gx == u.ux && gy == u.uy)
			return(0);
		if(udist > 1){
			if(levl[u.ux][u.uy].typ < ROOM || !rn2(4) ||
			   whappr ||
			   (mtmp->minvent && rn2((int) edog->apport)))
				appr = 1;
		}
		/* if you have dog food he'll follow you more closely */
		if(appr == 0){
			obj = invent;
			while(obj){
				if(obj->otyp == TRIPE_RATION){
					appr = 1;
					break;
				}
				obj = obj->nobj;
			}
		}
	} else	appr = 1;	/* gtyp != UNDEF */
	if(mtmp->mconf) appr = 0;
#ifdef TRACK
	if(gx == u.ux && gy == u.uy && (dogroom != uroom || dogroom < 0)){
	extern coord *gettrack();
	register coord *cp;
		cp = gettrack(omx,omy);
		if(cp){
			gx = cp->x;
			gy = cp->y;
		}
	}
#endif TRACK
	nix = omx;
	niy = omy;
	cnt = mfndpos(mtmp,poss,info,ALLOW_M | ALLOW_TRAPS);
	chcnt = 0;
	chi = -1;
	for(i=0; i<cnt; i++){
		nx = poss[i].x;
		ny = poss[i].y;
		if(info[i] & ALLOW_M){
			mtmp2 = m_at(nx,ny);
			if(mtmp2->data->mlevel >= mdat->mlevel+2 ||
			  mtmp2->data->mlet == 'c')
				continue;
			if(after) return(0); /* hit only once each move */

			if(hitmm(mtmp, mtmp2) == 1 && rn2(4) &&
			  mtmp2->mlstmv != moves &&
			  hitmm(mtmp2,mtmp) == 2) return(2);
			return(0);
		}

		/* dog avoids traps */
		/* but perhaps we have to pass a trap in order to follow @ */
		if((info[i] & ALLOW_TRAPS) && (trap = g_at(nx,ny,ftrap))){
			if(!(trap->gflag & SEEN) && rn2(40)) continue;
			if(rn2(10)) continue;
		}

		/* dog eschewes cursed objects */
		/* but likes dog food */
		obj = fobj;
		while(obj){
		    if(obj->ox != nx || obj->oy != ny)
			goto nextobj;
		    if(obj->cursed) goto nxti;
		    if(obj->olet == FOOD_SYM &&
			(otyp = dogfood(obj)) < MANFOOD &&
			(otyp < ACCFOOD || edog->hungrytime <= moves)){
			/* Note: our dog likes the food so much that he
			might eat it even when it conceals a cursed object */
			nix = nx;
			niy = ny;
			chi = i;
		     eatobj:
			edog->eattime =
			    moves + obj->quan * objects[obj->otyp].oc_delay;
			edog->hungrytime =
			    moves + 5*obj->quan * objects[obj->otyp].nutrition;
			mtmp->mconf = 0;
			if(cansee(nix,niy))
			    pline("%s ate %s.", Monnam(mtmp), doname(obj));
			/* perhaps this was a reward */
			if(otyp != CADAVER)
			edog->apport += 200/(edog->dropdist+moves-edog->droptime);
			delobj(obj);
			goto newdogpos;
		    }
		nextobj:
		    obj = obj->nobj;
		}

		for(j=0; j<MTSZ && j<cnt-1; j++)
			if(nx == mtmp->mtrack[j].x && ny == mtmp->mtrack[j].y)
				if(rn2(4*(cnt-j))) goto nxti;

/* Some stupid C compilers cannot compute the whole expression at once. */
		nearer = GDIST(nx,ny);
		nearer -= GDIST(nix,niy);
		nearer *= appr;
		if((nearer == 0 && !rn2(++chcnt)) || nearer<0 ||
			(nearer > 0 && !whappr &&
				((omx == nix && omy == niy && !rn2(3))
				|| !rn2(12))
			)){
			nix = nx;
			niy = ny;
			if(nearer < 0) chcnt = 0;
			chi = i;
		}
	nxti:	;
	}
newdogpos:
	if(nix != omx || niy != omy){
		if(info[chi] & ALLOW_U){
			(void) hitu(mtmp, d(mdat->damn, mdat->damd)+1);
			return(0);
		}
		mtmp->mx = nix;
		mtmp->my = niy;
		for(j=MTSZ-1; j>0; j--) mtmp->mtrack[j] = mtmp->mtrack[j-1];
		mtmp->mtrack[0].x = omx;
		mtmp->mtrack[0].y = omy;
	}
	if(mintrap(mtmp) == 2)	/* he died */
		return(2);
	pmon(mtmp);
	return(1);
}

/* return roomnumber or -1 */
inroom(x,y) xchar x,y; {
#ifndef QUEST
	register struct mkroom *croom = &rooms[0];
	while(croom->hx >= 0){
		if(croom->hx >= x-1 && croom->lx <= x+1 &&
		   croom->hy >= y-1 && croom->ly <= y+1)
			return(croom - rooms);
		croom++;
	}
#endif QUEST
	return(-1);	/* not in room or on door */
}

tamedog(mtmp, obj)
register struct monst *mtmp;
register struct obj *obj;
{
register struct monst *mtmp2;
	if(mtmp->mtame || mtmp->mfroz ||
#ifndef NOWORM
		mtmp->wormno ||
#endif NOWORM
		mtmp->isshk || mtmp->isgd)
		return(0); /* no tame long worms? */
	if(obj) {
		if(dogfood(obj) >= MANFOOD) return(0);
		if(cansee(mtmp->mx,mtmp->my)){
			pline("%s devours the %s.", Monnam(mtmp),
				objects[obj->otyp].oc_name);
		}
		obfree(obj, (struct obj *) 0);
	}
	mtmp2 = newmonst(sizeof(struct edog) + mtmp->mnamelth);
	*mtmp2 = *mtmp;
	mtmp2->mxlth = sizeof(struct edog);
	if(mtmp->mnamelth) (void) strcpy(NAME(mtmp2), NAME(mtmp));
	initedog(mtmp2);
	replmon(mtmp,mtmp2);
	return(1);
}
#file hack.do_name.c
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* hack.do_name.c version 1.0.1 - correction in call of xname() */

#include "hack.h"
#include <stdio.h>

extern int mousex;
extern int mousey;

coord
getpos(force,goal) int force; char *goal; {
register int cx,cy,i,c;
extern char sdir[];      /* defined in hack.c */
extern schar xdir[], ydir[];   /* idem */
extern char *visctrl();      /* see below */
coord cc;
   pline("(For instructions type a ?)");
   cx = u.ux;
   cy = u.uy;
   curs(cx,cy+2);
   while((c = readchar()) != '.'){
      for(i=0; i<8; i++) if(sdir[i] == c){
         if(1 <= cx + xdir[i] && cx + xdir[i] <= COLNO)
            cx += xdir[i];
         if(0 <= cy + ydir[i] && cy + ydir[i] <= ROWNO-1)
            cy += ydir[i];
         goto nxtc;
      }
      if(c == '?'){
         pline("Use [hjkl] to move the cursor to %s.", goal);
         pline("Type a . when you are at the right place.");
      } else if (c == MDOWN) {
	cx = mousex;
	cy = mousey-2;
	} else if (c == MUP) {
	cx = mousex;
	cy = mousey-2;
	break;
	} else {
         pline("unknown direction: '%s' (%s)",
            visctrl(c),
            force ? "use hjkl or ." : "aborted");
         if(force) goto nxtc;
         cc.x = -1;
         cc.y = 0;
         return(cc);
      }
   nxtc:   ;
      curs(cx,cy+2);
   }
   cc.x = cx;
   cc.y = cy;
   return(cc);
}

do_mname(){
char buf[BUFSZ];
coord cc;
register int cx,cy,lth,i;
register struct monst *mtmp, *mtmp2;
extern char *lmonnam();
   cc = getpos(0, "the monster you want to name");
   cx = cc.x;
   cy = cc.y;
   if(cx < 0) return(0);
   mtmp = m_at(cx,cy);
   if(!mtmp){
       if(cx == u.ux && cy == u.uy){
      extern char plname[];
      pline("This ugly monster is called %s and cannot be renamed.",
          plname);
       } else   pline("There is no monster there.");
       return(1);
   }
   if(mtmp->mimic){
       pline("I see no monster there.");
       return(1);
   }
   if(!cansee(cx,cy)) {
       pline("I cannot see a monster there.");
       return(1);
   }
   pline("What do you want to call %s? ", lmonnam(mtmp));
   getlin(buf);
   clrlin();
   if(!*buf) return(1);
   lth = strlen(buf)+1;
   if(lth > 63){
      buf[62] = 0;
      lth = 63;
   }
   mtmp2 = newmonst(mtmp->mxlth + lth);
   *mtmp2 = *mtmp;
   for(i=0; i<mtmp->mxlth; i++)
      ((char *) mtmp2->mextra)[i] = ((char *) mtmp->mextra)[i];
   mtmp2->mnamelth = lth;
   (void) strcpy(NAME(mtmp2), buf);
   replmon(mtmp,mtmp2);
   if(mtmp2->isshk) setshk();   /* redefine shopkeeper and bill */
   if(mtmp2->isgd) setgd( /* mtmp2 */ );
   return(1);
}

/*
 * This routine changes the address of  obj . Be careful not to call it
 * when there might be pointers around in unknown places. For now: only
 * when  obj  is in the inventory.
 */
do_oname(obj) register struct obj *obj; {
register struct obj *otmp, *otmp2;
register int lth;
char buf[BUFSZ];
   pline("What do you want to name %s? ", doname(obj));
   getlin(buf);
   clrlin();
   if(!*buf) return;
   lth = strlen(buf)+1;
   if(lth > 63){
      buf[62] = 0;
      lth = 63;
   }
   otmp2 = newobj(lth);
   *otmp2 = *obj;
   otmp2->onamelth = lth;
   (void) strcpy(ONAME(otmp2), buf);

   setworn((struct obj *) 0, obj->owornmask);
   setworn(otmp2, otmp2->owornmask);

   /* do freeinv(obj); etc. by hand in order to preserve
      the position of this object in the inventory */
   if(obj == invent) invent = otmp2;
   else for(otmp = invent; ; otmp = otmp->nobj){
      if(!otmp)
         panic("Do_oname: cannot find obj.");
      if(otmp->nobj == obj){
         otmp->nobj = otmp2;
         break;
      }
   }
   /* obfree(obj, otmp2);   /* now unnecessary: no pointers on bill */
   free((char *) obj);   /* let us hope nobody else saved a pointer */
}

ddocall()
{
   register struct obj *obj;

   pline("Do you want to name an individual object? [yn] ");
   if(readchar() == 'y'){
      obj = getobj("#", "name");
      if(obj) do_oname(obj);
   } else {
      obj = getobj("?!=/", "call");
      if(obj) docall(obj);
   }
   return(0);
}

docall(obj)
register struct obj *obj;
{
   char buf[BUFSZ];
   register char **str1;
   extern char *xname();
	struct obj otemp;
   register char *str;

	otemp = *obj;
	otemp.quan = 1;
	str = xname(&otemp);
   pline("Call %s %s: ", index(vowels,*str) ? "an" : "a", str);
   getlin(buf);
   clrlin();
   if(!*buf) return;
   str = newstring(strlen(buf)+1);
   (void) strcpy(str,buf);
   str1 = &(objects[obj->otyp].oc_uname);
   if(*str1) free(*str1);
   *str1 = str;
}

char *
xmonnam(mtmp, vb) register struct monst *mtmp; int vb; {
static char buf[BUFSZ];      /* %% */
extern char *shkname();
	if(mtmp->mnamelth && !vb) {
		(void) strcpy(buf, NAME(mtmp));
		return(buf);
	}
   switch(mtmp->data->mlet) {
   case ' ':
      (void) sprintf(buf, "%s's ghost", (char *) mtmp->mextra);
      break;
   case '@':
      if(mtmp->isshk) {
         (void) strcpy(buf, shkname());
         break;
      }
      /* fall into next case */
   default:
      (void) sprintf(buf, "the %s%s",
         mtmp->minvis ? "invisible " : "",
         mtmp->data->mname);
   }
   if(vb && mtmp->mnamelth) {
      (void) strcat(buf, " called ");
      (void) strcat(buf, NAME(mtmp));
   }
   return(buf);
}

char *
lmonnam(mtmp) register struct monst *mtmp; {
   return(xmonnam(mtmp, 1));
}

char *
monnam(mtmp) register struct monst *mtmp; {
   return(xmonnam(mtmp, 0));
}

char *
Monnam(mtmp) register struct monst *mtmp; {
register char *bp = monnam(mtmp);
   if('a' <= *bp && *bp <= 'z') *bp += ('A' - 'a');
   return(bp);
}

char *
amonnam(mtmp,adj)
register struct monst *mtmp;
register char *adj;
{
   register char *bp = monnam(mtmp);
   static char buf[BUFSZ];      /* %% */

   if(!strncmp(bp, "the ", 4)) bp += 4;
   (void) sprintf(buf, "the %s %s", adj, bp);
   return(buf);
}

char *
Amonnam(mtmp, adj)
register struct monst *mtmp;
register char *adj;
{
   register char *bp = amonnam(mtmp,adj);

   *bp = 'T';
   return(bp);
}

char *
Xmonnam(mtmp) register struct monst *mtmp; {
register char *bp = Monnam(mtmp);
   if(!strncmp(bp, "The ", 4)) {
      bp += 2;
      *bp = 'A';
   }
   return(bp);
}

char *
visctrl(c)
char c;
{
static char ccc[3];
   if(c < 040) {
      ccc[0] = '^';
      ccc[1] = c + 0100;
      ccc[2] = 0;
   } else {
      ccc[0] = c;
      ccc[1] = 0;
   }
   return(ccc);
}
#file hack.do_wear.c
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* hack.do_wear.c version 1.0.1 - changed an int to long */

#include "hack.h"
#include <stdio.h>
extern char *nomovemsg;

off_msg(otmp) register struct obj *otmp; {
	pline("You were wearing %s.", doname(otmp));
}

doremarm() {
	register struct obj *otmp;
	if(!uarm && !uarmh && !uarms && !uarmg) {
		pline("Not wearing any armor.");
		return(0);
	}
	otmp = (!uarmh && !uarms && !uarmg) ? uarm :
		(!uarms && !uarm && !uarmg) ? uarmh :
		(!uarmh && !uarm && !uarmg) ? uarms :
		(!uarmh && !uarm && !uarms) ? uarmg :
		getobj("[", "take off");
	if(!otmp) return(0);
	if(!(otmp->owornmask & (W_ARMOR - W_ARM2))) {
		pline("You can't take that off.");
		return(0);
	}
	(void) armoroff(otmp);
	return(1);
}

doremring() {
	if(!uleft && !uright){
		pline("Not wearing any ring.");
		return(0);
	}
	if(!uleft)
		return(dorr(uright));
	if(!uright)
		return(dorr(uleft));
	if(uleft && uright) while(1) {
		pline("What ring, Right or Left? ");
		switch(readchar()) {
		case ' ':
		case '\n':
		case '\033':
			return(0);
		case 'l':
		case 'L':
			return(dorr(uleft));
		case 'r':
		case 'R':
			return(dorr(uright));
		}
	}
	/* NOTREACHED */
#ifdef lint
	return(0);
#endif lint
}

dorr(otmp) register struct obj *otmp; {
	if(cursed(otmp)) return(0);
	ringoff(otmp);
	off_msg(otmp);
	return(1);
}

cursed(otmp) register struct obj *otmp; {
	if(otmp->cursed){
		pline("You can't. It appears to be cursed.");
		return(1);
	}
	return(0);
}

armoroff(otmp) register struct obj *otmp; {
register int delay = -objects[otmp->otyp].oc_delay;
	if(cursed(otmp)) return(0);
	setworn((struct obj *) 0, otmp->owornmask & W_ARMOR);
	if(delay) {
		nomul(delay);
		switch(otmp->otyp) {
		case HELMET:
			nomovemsg = "You finished taking off your helmet.";
			break;
		case PAIR_OF_GLOVES:
			nomovemsg = "You finished taking off your gloves";
			break;
		default:
			nomovemsg = "You finished taking off your suit.";
		}
	} else {
		off_msg(otmp);
	}
	return(1);
}

doweararm() {
	register struct obj *otmp;
	register int delay;
	register int err = 0;
	long mask = 0;

	otmp = getobj("[", "wear");
	if(!otmp) return(0);
	if(otmp->owornmask & W_ARMOR) {
		pline("You are already wearing that!");
		return(0);
	}
	if(otmp->otyp == HELMET){
		if(uarmh) {
			pline("You are already wearing a helmet.");
			err++;
		} else
			mask = W_ARMH;
	} else if(otmp->otyp == SHIELD){
		if(uarms) pline("You are already wearing a shield."), err++;
		if(uwep && uwep->otyp == TWO_HANDED_SWORD)
	pline("You cannot wear a shield and wield a two-handed sword."), err++;
		if(!err) mask = W_ARMS;
	} else if(otmp->otyp == PAIR_OF_GLOVES){
		if(uarmg) pline("You are already wearing gloves."); else
		if(uwep && uwep->cursed)
			pline("You cannot wear gloves over your weapon.");
		else mask = W_ARMG;
	} else {
		if(uarm) {
			if(otmp->otyp != ELVEN_CLOAK || uarm2) {
				pline("You are already wearing some armor.");
				err++;
			}
		}
		if(!err) mask = W_ARM;
	}
	if(err) return(0);
	setworn(otmp, mask);
	if(otmp == uwep)
		setuwep((struct obj *) 0);
	delay = -objects[otmp->otyp].oc_delay;
	if(delay){
		nomul(delay);
		nomovemsg = "You finished your dressing manoeuvre.";
	}
	otmp->known = 1;
	return(1);
}

dowearring() {
	register struct obj *otmp;
	long mask = 0;
	long oldprop;

	if(uleft && uright){
		pline("There are no more ring-fingers to fill.");
		return(0);
	}
	otmp = getobj("=", "wear");
	if(!otmp) return(0);
	if(otmp->owornmask & W_RING) {
		pline("You are already wearing that!");
		return(0);
	}
	if(otmp == uleft || otmp == uright) {
		pline("You are already wearing that.");
		return(0);
	}
	if(uleft) mask = RIGHT_RING;
	else if(uright) mask = LEFT_RING;
	else do {
 		pline("What ring-finger, Right or Left? ");
		switch(readchar()){
		case 'l':
		case 'L':
			mask = LEFT_RING;
			break;
		case 'r':
		case 'R':
			mask = RIGHT_RING;
			break;
		case ' ':
		case '\n':
		case '\033':
			return(0);
		}
	} while(!mask);
	setworn(otmp, mask);
	if(otmp == uwep)
		setuwep((struct obj *) 0);
	oldprop = u.uprops[PROP(otmp->otyp)].p_flgs;
	u.uprops[PROP(otmp->otyp)].p_flgs |= mask;
	switch(otmp->otyp){
	case RIN_LEVITATION:
		if(!oldprop) float_up();
		break;
	case RIN_PROT_SHAPE_CHANGERS:
		rescham();
		break;
	case RIN_GAIN_STRENGTH:
		u.ustr += otmp->spe;
		u.ustrmax += otmp->spe;
		flags.botl=1;
		break;
	case RIN_INCREASE_DAMAGE:
		u.udaminc += otmp->spe;
		break;
	}
	prinv(otmp);
	return(1);
}

ringoff(obj)
register struct obj *obj;
{
register long mask;
	mask = obj->owornmask & W_RING;
	setworn((struct obj *) 0, obj->owornmask);
	if(!(u.uprops[PROP(obj->otyp)].p_flgs & mask)){
		pline("Strange... I didnt know you had that ring.");
		impossible();
	}
	u.uprops[PROP(obj->otyp)].p_flgs &= ~mask;
	switch(obj->otyp) {
	case RIN_LEVITATION:
		if(!Levitation) {	/* no longer floating */
			float_down();
		}
		break;
	case RIN_GAIN_STRENGTH:
		u.ustr -= obj->spe;
		u.ustrmax -= obj->spe;
		flags.botl = 1;
		break;
	case RIN_INCREASE_DAMAGE:
		u.udaminc -= obj->spe;
		break;
	}
}

find_ac(){
register int uac = 10;
	if(uarm) uac -= uarm->spe;
	if(uarm2) uac -= uarm2->spe;
	if(uarmh) uac -= uarmh->spe;
	if(uarms) uac -= uarms->spe;
	if(uarmg) uac -= uarmg->spe;
	if(uleft && uleft->otyp == RIN_PROTECTION) uac -= uleft->spe;
	if(uright && uright->otyp == RIN_PROTECTION) uac -= uright->spe;
	if(uac != u.uac){
		u.uac = uac;
		flags.botl = 1;
	}
}

glibr(){
register struct obj *otmp;
int xfl = 0;
	if(!uarmg) if(uleft || uright) {
		/* Note: at present also cursed rings fall off */
		pline("Your %s off your fingers.",
			(uleft && uright) ? "rings slip" : "ring slips");
		xfl++;
		if(otmp = uleft){
			ringoff(uleft);
			dropx(otmp);
		}
		if(otmp = uright){
			ringoff(uright);
			dropx(otmp);
		}
	}
	if(otmp = uwep){
		/* Note: at present also cursed weapons fall */
		setuwep((struct obj *) 0);
		dropx(otmp);
		pline("Your weapon %sslips from your hands.",
			xfl ? "also " : "");
	}
}

struct obj *
some_armor(){
register struct obj *otmph = uarm;
	if(uarmh && (!otmph || !rn2(4))) otmph = uarmh;
	if(uarmg && (!otmph || !rn2(4))) otmph = uarmg;
	if(uarms && (!otmph || !rn2(4))) otmph = uarms;
	return(otmph);
}

corrode_armor(){
register struct obj *otmph = some_armor();
	if(otmph){
		if(otmph->rustfree ||
		   otmph->otyp == ELVEN_CLOAK ||
		   otmph->otyp == LEATHER_ARMOR ||
		   otmph->otyp == STUDDED_LEATHER_ARMOR) {
			pline("Your %s not affected!",
				aobjnam(otmph, "are"));
			return;
		}
		pline("Your %s!", aobjnam(otmph, "corrode"));
		otmph->spe--;
	}
}
#file hack.eat.c
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* hack.eat.c version 1.0.1 - added morehungry() and FAINTED */

#include   "hack.h"
char POISONOUS[] = "ADKSVabhks";
extern char *nomovemsg;
extern int (*afternmv)();

/* hunger texts used on bottom line (each 8 chars long) */
#define SATIATED	0
#define NOT_HUNGRY	1
#define HUNGRY		2
#define WEAK		3
#define FAINTING	4
#define FAINTED		5
#define STARVED		6

char *hu_stat[] = {
	"Satiated",
	"        ",
	"Hungry  ",
	"Weak    ",
	"Fainting",
	"Fainted ",
	"Starved "
};

init_uhunger(){
   u.uhunger = 900;
   u.uhs = NOT_HUNGRY;
}

struct { char *txt; int nut; } tintxts[] = {
   {"It contains first quality peaches - what a surprise!",   40},
   {"It contains salmon - not bad!",   60},
   {"It contains apple juice - perhaps not what you hoped for.", 20},
   {"It contains some nondescript substance, tasting awfully.", 500},
   {"It contains rotten meat. You vomit.", -50},
   {"It turns out to be empty.",   0}
};

tinopen(){
#define   TTSZ   (sizeof(tintxts)/sizeof(tintxts[0]))
register int r = rn2(2*TTSZ);
   if(r < TTSZ){
       pline(tintxts[r].txt);
       lesshungry(tintxts[r].nut);
       if(r == 1)   /* SALMON */ {
      Glib = rnd(15);
      pline("Eating salmon made your fingers very slippery.");
       }
   } else {
       pline("It contains spinach - this makes you feel like Popeye!");
       lesshungry(600);
       if(u.ustr < 118)
      u.ustr += rnd( ((u.ustr < 17) ? 19 : 118) - u.ustr);
       if(u.ustr > u.ustrmax) u.ustrmax = u.ustr;
       flags.botl = 1;
   }
}

Meatdone(){
   u.usym = '@';
   prme();
}

doeat(){
   register struct obj *otmp;
   register struct objclass *ftmp;
   register int tmp;

   otmp = getobj("%", "eat");
   if(!otmp) return(0);
   if(otmp->otyp == TIN){
      if(uwep && (uwep->otyp == AXE || uwep->otyp == DAGGER ||
             uwep->otyp == CRYSKNIFE)){
         pline("Using your %s you try to open the tin",
            aobjnam(uwep, (char *) 0));
         tmp = 3;
      } else {
         pline("It is not so easy to open this tin.");
				if(Glib) {
					pline("The tin slips out of your hands.");
					dropx(otmp);
					return(1);
				}
				if(otmp->quan > 1) {
					register struct obj *obj;
					extern struct obj *splitobj();

					obj = splitobj(otmp, 1);
					if(otmp == uwep) setuwep(obj);
				}
         tmp = 2 + rn2(1 + 500/((int)(u.ulevel + u.ustr)));
      }
      if(tmp > 50){
         nomul(-50);
         nomovemsg="You give up your attempt to open the tin.";
      } else {
         nomul(-tmp);
         nomovemsg = "You succeed in opening the tin.";
         afternmv = tinopen;
         useup(otmp);
      }
      return(1);
   }
   ftmp = &objects[otmp->otyp];
   if(otmp->otyp >= CORPSE && eatcorpse(otmp)) goto eatx;
   if(!rn2(7) && otmp->otyp != FORTUNE_COOKIE) {
      pline("Blecch!  Rotten food!");
      if(!rn2(4)) {
         pline("You feel rather light headed.");
         Confusion += d(2,4);
      } else if(!rn2(4)&& !Blind) {
         pline("Everything suddenly goes dark.");
         Blind = d(2,10);
         seeoff(0);
      } else if(!rn2(3)) {
         if(Blind)
           pline("The world spins and you slap against the floor.");
         else
           pline("The world spins and goes dark.");
         nomul(-rnd(10));
         nomovemsg = "You are conscious again.";
      }
      lesshungry(ftmp->nutrition / 4);
   } else {
      multi = -ftmp->oc_delay;
      if(u.uhunger >= 1500) {
         pline("You choke over your food.");
         pline("You die...");
         killer = ftmp->oc_name;
         done("choked");
      }
      switch(otmp->otyp){
      case FOOD_RATION:
         if(u.uhunger <= 200)
            pline("That food really hit the spot!");
         else if(u.uhunger <= 700)
            pline("That satiated your stomach!");
         else {
	pline("You're having a hard time getting all that food down.");
            multi -= 2;
         }
         lesshungry(ftmp->nutrition);
         if(multi < 0) nomovemsg = "You finished your meal.";
         break;
      case TRIPE_RATION:
         pline("Yak - dog food!");
         u.uexp++;
         u.urexp += 4;
         flags.botl = 1;
			if(rn2(2)){
				pline("You vomit.");
				morehungry(20);
         } else   lesshungry(ftmp->nutrition);
         break;
      default:
         if(otmp->otyp >= CORPSE)
         pline("That %s tasted terrible!",ftmp->oc_name);
         else
         pline("That %s was delicious!",ftmp->oc_name);
         lesshungry(ftmp->nutrition);
#ifdef QUEST
         if(otmp->otyp == CARROT && !Blind){
            u.uhorizon++;
            setsee();
            pline("Your vision improves.");
         }
#endif QUEST
         if(otmp->otyp == FORTUNE_COOKIE) {
           if(Blind) {
             pline("This cookie has a scrap of paper inside!");
             pline("What a pity, that you cannot read it!");
           } else
             outrumor();
         }
         break;
      }
   }
eatx:
   if(multi<0 && !nomovemsg){
      static char msgbuf[BUFSZ];
      (void) sprintf(msgbuf, "You finished eating the %s.",
            ftmp->oc_name);
      nomovemsg = msgbuf;
   }
   useup(otmp);
   return(1);
}

/* called in hack.main.c */
gethungry(){
   --u.uhunger;
   if((Regeneration || Hunger) && moves%2) u.uhunger--;
	newuhs(TRUE);
}

/* called after vomiting and after performing feats of magic */
morehungry(num) register int num; {
	u.uhunger -= num;
	newuhs(TRUE);
}

/* called after eating something (and after drinking fruit juice) */
lesshungry(num) register int num; {
	u.uhunger += num;
	newuhs(FALSE);
}

unfaint(){
	u.uhs = FAINTING;
	flags.botl = 1;
}

newuhs(incr) boolean incr; {
	register int newhs, h = u.uhunger;

	newhs = (h > 1000) ? SATIATED :
		(h > 150) ? NOT_HUNGRY :
		(h > 50) ? HUNGRY :
		(h > 0) ? WEAK : FAINTING;

	if(newhs == FAINTING) {
		if(u.uhs == FAINTED)
			newhs = FAINTED;
		if(u.uhs <= WEAK || rn2(20-u.uhunger/10) >= 19) {
			if(u.uhs != FAINTED && multi >= 0 /* %% */) {
				pline("You faint from lack of food.");
				nomul(-10+(u.uhunger/10));
				nomovemsg = "You regain consciousness.";
				afternmv = unfaint;
				newhs = FAINTED;
			}
		} else
		if(u.uhunger < -(int)(200 + 25*u.ulevel)) {
			u.uhs = STARVED;
			flags.botl = 1;
			bot();
			pline("You die from starvation.");
			done("starved");
		}
	}

	if(newhs != u.uhs) {
		if(newhs >= WEAK && u.uhs < WEAK)
			losestr(1);
		else
		if(newhs < WEAK && u.uhs >= WEAK && u.ustr < u.ustrmax)
			losestr(-1);
		switch(newhs){
		case HUNGRY:
			pline((!incr) ? "You only feel hungry now." :
			      (u.uhunger < 145) ? "You feel hungry." :
				"You are beginning to feel hungry.");
			break;
		case WEAK:
			pline((!incr) ? "You feel weak now." :
			      (u.uhunger < 45) ? "You feel weak." :
				"You are beginning to feel weak.");
			break;
		}
		u.uhs = newhs;
		flags.botl = 1;
   }
}

/* returns 1 if some text was printed */
eatcorpse(otmp) register struct obj *otmp; {
register schar let = otmp->spe;
register int tp = 0;
   if(moves > otmp->age + 50 + rn2(100)) {
      tp++;
      pline("Ulch -- that meat was tainted!");
      pline("You get very sick.");
      Sick = 10 + rn2(10);
      u.usick_cause = objects[otmp->otyp].oc_name;
   } else if(index(POISONOUS, let) && rn2(5)){
      tp++;
      pline("Ecch -- that must have been poisonous!");
      if(!Poison_resistance){
         losehp(rnd(15), "poisonous corpse");
         losestr(rnd(4));
      } else
         pline("You don't seem affected by the poison.");
   } else if(index("ELNOPQRUuxz", let) && rn2(5)){
      tp++;
      pline("You feel sick.");
      losehp(rnd(8), "cadaver");
   }
   switch(let) {
   case 'L':
   case 'N':
   case 't':
      Teleportation |= INTRINSIC;
      break;
   case 'W':
      pluslvl();
      break;
   case 'n':
      u.uhp = u.uhpmax;
      flags.botl = 1;
      /* fall into next case */
   case '@':
      pline("You cannibal! You will be sorry for this!");
      /* not tp++; */
      /* fall into next case */
   case 'd':
      Aggravate_monster |= INTRINSIC;
      break;
   case 'I':
      See_invisible |= INTRINSIC;
      if(!Invis) newsym(u.ux, u.uy);
      Invis += 50;
      /* fall into next case */
   case 'y':
#ifdef QUEST
      u.uhorizon++;
#endif QUEST
      /* fall into next case */
   case 'B':
      Confusion = 50;
      break;
   case 'D':
      Fire_resistance |= INTRINSIC;
      break;
   case 'E':
      Telepat |= INTRINSIC;
      break;
   case 'F':
   case 'Y':
      Cold_resistance |= INTRINSIC;
      break;
   case 'k':
   case 's':
      Poison_resistance |= INTRINSIC;
      break;
   case 'c':
      pline("You turn to stone.");
      killer = "dead cockatrice";
      done("died");
   case 'M':
     pline("You cannot resist the temptation to mimic a treasure chest.");
     tp++;
     nomul(-30);
     afternmv = Meatdone;
     nomovemsg = "You now again prefer mimicking a human.";
     u.usym = '$';
     prme();
     break;
   }
   return(tp);
}

#file hack.end.c
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* hack.end.c version 1.0.1 - added "escaped with amulet" */

#include "hack.h"
#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#define   Sprintf   (void) sprintf
extern char plname[], pl_character[];
extern char *itoa(), *ordin(), *eos(), *getlogin();

extern char *index(); /* M.E.T. 11/20/85 */

xchar maxdlevel = 1;

done1()
{
   (void) signal(SIGINT,SIG_IGN);
   pline("Really quit?");
   if(readchar() != 'y')
      {
      (void) signal(SIGINT,done1);
      clrlin();
      (void) myfflush(stdout);
      if(multi > 0) nomul(0);
      return(0);
      }
   done("quit");
   /* NOTREACHED */
}

int done_stopprint;

done_intr(){
   done_stopprint++;
   (void) signal(SIGINT,SIG_IGN);
}

done_in_by(mtmp) register struct monst *mtmp; {
static char buf[BUFSZ];
   pline("You die ...");
   if(mtmp->data->mlet == ' ')
      {
      Sprintf(buf, "the ghost of %s", (char *) mtmp->mextra);
      killer = buf;
      }
   else if(mtmp->mnamelth)
      {
      Sprintf(buf, "%s called %s", mtmp->data->mname, NAME(mtmp));
      killer = buf;
      }
   else if(mtmp->minvis)
      {
      Sprintf(buf, "invisible %s", mtmp->data->mname);
      killer = buf;
      }
   else
      killer = mtmp->data->mname;
   done("died");
}

/* called with arg "died", "escaped", "quit", "choked", "panic"
   or "starved" */
/* Be careful not to call panic from here! */
done(st1)
register char *st1;
{

#ifdef WIZARD
   if(wizard && *st1 == 'd'){
      u.ustr = u.ustrmax += 2;
      u.uhp = u.uhpmax += 10;
      if(uarm) uarm->spe++;
      if(uwep) uwep->spe++; /* NB: uwep need not be a weapon! */
      u.uswldtim = 0;
      pline("For some reason you are still alive.");
      flags.move = 0;
      if(multi > 0) multi = 0; else multi = -1;
      flags.botl = 1;
      return;
   }
#endif WIZARD
   (void) signal(SIGINT, done_intr);
   if(*st1 == 'q' && u.uhp < 1)
      {
      st1 = "died";
      killer = "quit while already on Charon's boat";
      }
   if(*st1 == 's')
      killer = "starvation";
   paybill();
   clearlocks();
   if(index("cds", *st1))
		{
		savebones();
		if(!flags.notombstone)
			outrip();
		else
			more();
		}
   myprintf("Contents of your pack when you died:\n");
   myddoinv();
   settty((char *) 0);   /* does a cls() */
   if(!done_stopprint)
      myprintf("Goodbye %s %s...\n\n", pl_character, plname);
   {
      long int tmp;
      tmp = u.ugold - u.ugold0;
      if(tmp < 0) tmp = 0;
      if(*st1 == 'd')
         tmp -= tmp/10;
      else
         killer = st1;
      u.urexp += tmp;
   }
   if(*st1 == 'e')
      {
      extern struct monst *mydogs;
      register struct monst *mtmp = mydogs;
      register struct obj *otmp;
      register int i;
      register unsigned worthlessct = 0;

		killer = st1;
	u.urexp += 50 * maxdlevel;
      if(mtmp)
         {
         if(!done_stopprint) myprintf("You");
         while(mtmp)
            {
            if(!done_stopprint)
               myprintf(" and %s", monnam(mtmp));
            u.urexp += mtmp->mhp;
            mtmp = mtmp->nmon;
            }
         if(!done_stopprint)
          myprintf("\nescaped from the dungeon with %lu points,\n",
         u.urexp);
         }
      else if(!done_stopprint)
        myprintf("You escaped from the dungeon with %lu points,\n",
          u.urexp);
      for(otmp = invent; otmp; otmp = otmp->nobj) {
         if(otmp->olet == GEM_SYM){
            objects[otmp->otyp].oc_name_known = 1;
            i = otmp->quan*objects[otmp->otyp].g_val;
            if(i == 0) {
               worthlessct += otmp->quan;
               continue;
            }
            u.urexp += i;
            if(!done_stopprint)
              myprintf("\t%s (worth %d Zorkmids),\n",
                doname(otmp), i);
         } else if(otmp->olet == AMULET_SYM) {
            otmp->known = 1;
            i = (otmp->spe < 0) ? 2 : 5000;
            u.urexp += i;
            if(!done_stopprint)
              myprintf("\t%s (worth %d Zorkmids),\n",
                doname(otmp), i);
				if(otmp->spe >= 0) {
					u.urexp *= 2;
					killer = "escaped (with amulet)";
				}
         }
      }
      if(worthlessct) if(!done_stopprint)
        myprintf("\t%d worthless piece%s of coloured glass,\n",
        worthlessct, plur(worthlessct));
   } else
      if(!done_stopprint)
        myprintf("You %s on dungeon level %d with %lu points,\n",
          st1,dlevel,u.urexp);
   if(!done_stopprint)
     myprintf("and %lu piece%s of gold, after %lu move%s.\n",
       u.ugold, (u.ugold == 1) ? "" : "s",
       moves, (moves == 1) ? "" : "s");
   if(!done_stopprint)
  myprintf("You were level %d with a maximum of %d hit points when you %s.\n",
       u.ulevel, u.uhpmax, st1);
   if(*st1 == 'e'){
      getret();   /* all those pieces of coloured glass ... */
      cls();
   }
#ifdef WIZARD
   if(!wizard)
#endif WIZARD
      topten();
   if(done_stopprint) myprintf("\n\n");
   hackexit(0);
}

#define newttentry() (struct toptenentry *) alloc(sizeof(struct toptenentry))
#define   NAMSZ   8
#define   DTHSZ   40
#define   PERSMAX   1
#define   POINTSMIN   1   /* must be > 0 */
#define   ENTRYMAX   100   /* must be >= 10 */
struct toptenentry {
   struct toptenentry *tt_next;
   long int points;
   int level,maxlvl,hp,maxhp;
   char plchar;
   char str[NAMSZ+1];
   char death[DTHSZ+1];
} *tt_head;

topten(){
   int rank, rank0 = -1, rank1 = 0;
   int occ_cnt = PERSMAX;
   register struct toptenentry *t0, *t1, *tprev;
   char *recfile = "record";
   int  rfile;
   register int flg = 0;

   if((rfile = open(recfile,O_RDONLY)) < 0)
      {
      myputs("Cannot open record file!");
      return;
      }
   (void) myputchar('\n');

   /* create a new 'topten' entry */
   t0 = newttentry();
   t0->level = dlevel;
   t0->maxlvl = maxdlevel;
   t0->hp = u.uhp;
   t0->maxhp = u.uhpmax;
   t0->points = u.urexp;
   t0->plchar = pl_character[0];
   (void) strncpy(t0->str, plname, NAMSZ);
   (t0->str)[NAMSZ] = 0;
   (void) strncpy(t0->death, killer, DTHSZ);
   (t0->death)[DTHSZ] = 0;

   /* assure minimum number of points */
   if(t0->points < POINTSMIN)
      t0->points = 0;

   t1 = tt_head = newttentry();
   tprev = 0;
   /* rank0: -1 undefined, 0 not_on_list, n n_th on list */
   for(rank = 1; ; ) {
     if (read(rfile, t1, sizeof(struct toptenentry)) !=
           sizeof(struct toptenentry) || (t1->points < POINTSMIN))
        t1->points = 0;

     if(rank0 < 0 && t1->points < t0->points) {
      rank0 = rank++;
      if(tprev == 0)
         tt_head = t0;
      else
         tprev->tt_next = t0;
      t0->tt_next = t1;
      occ_cnt--;
      flg++;      /* ask for a rewrite */
     } else
      tprev = t1;
     if(t1->points == 0) break;
     if(strncmp(t1->str, t0->str, NAMSZ) == 0 &&
        t1->plchar == t0->plchar && --occ_cnt <= 0){
      if(rank0 < 0){
         rank0 = 0;
         rank1 = rank;
   myprintf("You didn't beat your previous score of %ld points.\n\n",
            t1->points);
      }
      if(occ_cnt < 0){
         flg++;
         continue;
      }
     }
     if(rank <= ENTRYMAX){
        t1 = t1->tt_next = newttentry();
        rank++;
     }
     if(rank > ENTRYMAX){
      t1->points = 0;
      break;
     }
   }
   if(flg) {   /* rewrite record file */
      (void) close(rfile);
      if((rfile=open(recfile,O_WRONLY)) < 0)
         {
         myputs("Cannot write record file\n");
         return;
         }

      if(!done_stopprint) if(rank0 > 0){
          if(rank0 <= 10)
         myputs("You made the top ten list!\n");
          else
      myprintf("You reached the %d%s place on the top %d list.\n\n",
         rank0, ordin(rank0), ENTRYMAX);
      }
   }
   if(rank0 == 0) rank0 = rank1;
   if(rank0 <= 0) rank0 = rank;
   if(!done_stopprint) outheader();
   t1 = tt_head;
   for(rank = 1; t1->points != 0; rank++, t1 = t1->tt_next) {
     if(flg) write(rfile, t1, sizeof(struct toptenentry));
     if(done_stopprint) continue;
	  if(rank > flags.end_top &&
	    (rank < rank0-flags.end_around || rank > rank0+flags.end_around)
	    && (!flags.end_own || strncmp(t1->str, t0->str, NAMSZ)))
	  	continue;
	  if(rank == rank0-flags.end_around &&
	     rank0 > flags.end_top+flags.end_around+1 &&
	     !flags.end_own)
		(void) putchar('\n');
     if(rank != rank0)
      (void) outentry(rank, t1, 0);
     else if(!rank1)
      (void) outentry(rank, t1, 1);
     else {
      int t0lth = outentry(0, t0, -1);
      int t1lth = outentry(rank, t1, t0lth);
      if(t1lth > t0lth) t0lth = t1lth;
      (void) outentry(0, t0, t0lth);
     }
   }
   if(rank0 >= rank)
      (void) outentry(0, t0, 1);
   (void) close(rfile);

/* 12nov85 djw */
   getret();
}

outheader() {
char linebuf[BUFSZ];
register char *bp;
   (void) strcpy(linebuf, "Number Points  Name");
   bp = eos(linebuf);
   while(bp < linebuf + COLNO - 9) *bp++ = ' ';
   (void) strcpy(bp, "Hp [max]");
   myputs(linebuf);
}

/* so>0: standout line; so=0: ordinary line; so<0: no output, return lth */
int
outentry(rank,t1,so) register struct toptenentry *t1; {
boolean quit = FALSE, killed = FALSE, starv = FALSE;
char linebuf[BUFSZ];
   linebuf[0] = 0;
   if(rank) Sprintf(eos(linebuf), "%3d", rank);
      else Sprintf(eos(linebuf), "   ");
   Sprintf(eos(linebuf), " %6ld %8s", t1->points, t1->str);
   if(t1->plchar == 'X') Sprintf(eos(linebuf), " ");
   else Sprintf(eos(linebuf), "-%c ", t1->plchar);
	if(!strncmp("escaped", t1->death, 7)) {
	  if(!strcmp(" (with amulet)", t1->death+7))
	    Sprintf(eos(linebuf), "escaped the dungeon with amulet");
	  else
	    Sprintf(eos(linebuf), "escaped the dungeon [max level %d]",
	      t1->maxlvl);
	} else {
     if(!strncmp(t1->death,"quit",4))
       Sprintf(eos(linebuf), "quit"), quit = TRUE;
     else if(!strcmp(t1->death,"choked"))
       Sprintf(eos(linebuf), "choked in his food");
     else if(!strncmp(t1->death,"starv",5))
       Sprintf(eos(linebuf), "starved to death"), starv = TRUE;
     else Sprintf(eos(linebuf), "was killed"), killed = TRUE;
     Sprintf(eos(linebuf), " on%s level %d",
       (killed || starv) ? "" : " dungeon", t1->level);
     if(t1->maxlvl != t1->level)
       Sprintf(eos(linebuf), " [max %d]", t1->maxlvl);
     if(quit && t1->death[4]) Sprintf(eos(linebuf), t1->death + 4);
   }
   if(killed) Sprintf(eos(linebuf), " by %s%s",
     !strncmp(t1->death, "the ", 4) ? "" :
     index(vowels,*t1->death) ? "an " : "a ",
     t1->death);
   Sprintf(eos(linebuf), ".");
   if(t1->maxhp) {
     register char *bp = eos(linebuf);
     char hpbuf[10];
     int hppos;
     Sprintf(hpbuf, (t1->hp > 0) ? itoa(t1->hp) : "-");
     hppos = COLNO - 7 - strlen(hpbuf);
     if(bp <= linebuf + hppos) {
       while(bp < linebuf + hppos) *bp++ = ' ';
       (void) strcpy(bp, hpbuf);
       Sprintf(eos(bp), " [%d]", t1->maxhp);
     }
   }
   if(so == 0) myputs(linebuf);
   else if(so > 0) {
     register char *bp = eos(linebuf);
     if(so >= COLNO) so = COLNO-1;
     while(bp < linebuf + so) *bp++ = ' ';
     *bp = 0;
     standoutbeg();
     myputs(linebuf);
     standoutend();
     (void) myputchar('\n');
   }
   return(strlen(linebuf));
}

char *
itoa(a) int a; {
static char buf[12];
   Sprintf(buf,"%d",a);
   return(buf);
}

char *
ordin(n) int n; {
register int d = n%10;
   return((d==0 || d>3 || n/10==1) ? "th" : (d==1) ? "st" :
      (d==2) ? "nd" : "rd");
}

clearlocks(){
register int x;
   (void) signal(SIGHUP,SIG_IGN);
   for(x = 1; x <= maxdlevel; x++) {
      glo(x);
      (void) unlink(lock);   /* not all levels need be present */
   }
   *index(lock,'.') = 0;
   (void) unlink(lock);
}

#ifdef NOSAVEONHANGUP
hangup(){
   (void) signal(SIGINT,SIG_IGN);
   clearlocks();
   hackexit(1);
}
#endif NOSAVEONHANGUP

char *
eos(s) register char *s; {
   while(*s) s++;
   return(s);
}

/* it is the callers responsibility to check that there is room for c */
charcat(s,c) register char *s, c; {
   while(*s) s++;
   *s++ = c;
   *s = 0;
}

prscore(argc,argv) int argc; char **argv; {
   extern char *hname;
   char *player0;
   char **players;
   int playerct;
   int rank;
   register struct toptenentry *t1;
   char *recfile = "record";
   int  rfile;
   register int flg = 0;
   register int i;

   if((rfile = open(recfile,O_RDONLY)) < 0)
      {
      myputs("Cannot open record file!");
      return;
      }

   if(argc > 1 && !strncmp(argv[1], "-s", 2)){
      if(!argv[1][2]){
         argc--;
         argv++;
      } else if(!argv[1][3] && index("CFKSTWX", argv[1][2])) {
         argv[1]++;
         argv[1][0] = '-';
      } else   argv[1] += 2;
   }
   if(argc <= 1){
      player0 = getlogin();
      if(!player0) player0 = "player";
      playerct = 1;
      players = &player0;
   } else {
      playerct = --argc;
      players = ++argv;
   }
   myputchar('\n');

   t1 = tt_head = newttentry();
   for(rank = 1; ; rank++) {
     if (read(rfile, t1, sizeof(struct toptenentry)) !=
           sizeof(struct toptenentry))
        t1->points = 0;
     if(t1->points == 0) break;
     for(i = 0; i < playerct; i++){
      if(strcmp(players[i], "all") == 0 ||
         strncmp(t1->str, players[i], NAMSZ) == 0 ||
        (players[i][0] == '-' &&
         players[i][1] == t1->plchar &&
         players[i][2] == 0) ||
        (digit(players[i][0]) && rank <= atoi(players[i])))
         flg++;
     }
     t1 = t1->tt_next = newttentry();
   }
   (void) close(rfile);
   if(!flg) {
      myprintf("Cannot find any entries for ");
      if(playerct > 1) myprintf("any of ");
      for(i=0; i<playerct; i++)
         myprintf("%s%s", players[i], (i<playerct-1)?", ":".\n");
      myprintf("Call is: %s -s [playernames]\n", hname);
      return;
   }

   outheader();
   t1 = tt_head;
   for(rank = 1; t1->points != 0; rank++, t1 = t1->tt_next) {
      for(i = 0; i < playerct; i++){
         if(strcmp(players[i], "all") == 0 ||
            strncmp(t1->str, players[i], NAMSZ) == 0 ||
           (players[i][0] == '-' &&
            players[i][1] == t1->plchar &&
            players[i][2] == 0) ||
           (digit(players[i][0]) && rank <= atoi(players[i])))
            goto out;
      }
      continue;
   out:
     (void) outentry(rank, t1, 0);
   }
}
