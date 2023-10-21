
#file hack.pri.c
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* hack.pri.c version 1.0.1 - tiny change in mnewsym() - added time */

#include "hack.h"
#include <stdio.h>
xchar scrlx, scrhx, scrly, scrhy;   /* corners of new area on screen */

extern char *hu_stat[];   /* in eat.c */

swallowed()
{
   char *ulook = "|@|";
   ulook[1] = u.usym;

   cls();
   curs(u.ux-1, u.uy+1);
   myprintf("/-\\");
   curx = u.ux+2;
   curs(u.ux-1, u.uy+2);
   myprintf(ulook);
   curx = u.ux+2;
   curs(u.ux-1, u.uy+3);
   myprintf("\\-/");
   curx = u.ux+2;
   u.udispl = 1;
   u.udisx = u.ux;
   u.udisy = u.uy;
}


/*VARARGS1*/
boolean panicking;

panic(str,a1,a2,a3,a4,a5,a6)
char *str;
{
   if(panicking++) hackexit(1);   /* avoid loops */
   home();
   myprintf(" Suddenly, the dungeon collapses.\n");
   myprintf(" ERROR:  ");
   myprintf(str,a1,a2,a3,a4,a5,a6);
/* if(fork()) */
      done("panic");
/* else          */
/*    abort();   */   /* generate core dump */
}

atl(x,y,ch)
register int x,y;
{
   register struct rm *crm = &levl[x][y];

   if(x<0 || x>COLNO-1 || y<0 || y>ROWNO-1)
      panic("at(%d,%d,%c_%o_)",x,y,ch,ch);
   if(crm->seen && crm->scrsym == ch) return;
   crm->scrsym = ch;
   crm->new = 1;
   on_scr(x,y);
}

on_scr(x,y)
register int x,y;
{
   if(x<scrlx) scrlx = x;
   if(x>scrhx) scrhx = x;
   if(y<scrly) scrly = y;
   if(y>scrhy) scrhy = y;
}

/* call: (x,y) - display
   (-1,0) - close (leave last symbol)
   (-1,-1)- close (undo last symbol)
   (-1,let)-open: initialize symbol
   (-2,let)-change let
*/

tmp_at(x,y) schar x,y; {
static schar prevx, prevy;
static char let;
   if((int)x == -2){   /* change let call */
      let = y;
      return;
   }
   if((int)x == -1 && (int)y >= 0){   /* open or close call */
      let = y;
      prevx = -1;
      return;
   }
   if(prevx >= 0 && cansee(prevx,prevy)) {
      delay_output();
      prl(prevx, prevy);   /* in case there was a monster */
      at(prevx, prevy, levl[prevx][prevy].scrsym);
   }
   if(x >= 0){   /* normal call */
      if(cansee(x,y)) at(x,y,let);
      prevx = x;
      prevy = y;
   } else {   /* close call */
      let = 0;
      prevx = -1;
   }
}

/* like the previous, but the symbols are first erased on completion */
Tmp_at(x,y) schar x,y; {
static char let;
static xchar cnt;
static coord tc[COLNO];      /* but watch reflecting beams! */
register int xx,yy;
   if((int)x == -1) {
      if(y > 0) {   /* open call */
         let = y;
         cnt = 0;
         return;
      }
      /* close call (do not distinguish y==0 and y==-1) */
      while(cnt--) {
         xx = tc[cnt].x;
         yy = tc[cnt].y;
         prl(xx, yy);
         at(xx, yy, levl[xx][yy].scrsym);
      }
      cnt = let = 0;   /* superfluous */
      return;
   }
   if((int)x == -2) {   /* change let call */
      let = y;
      return;
   }
   /* normal call */
   if(cansee(x,y)) {
      if(cnt) delay_output();
      at(x,y,let);
      tc[cnt].x = x;
      tc[cnt].y = y;
      if(++cnt >= COLNO) panic("Tmp_at overflow?");
      levl[x][y].new = 0;   /* prevent pline-nscr erasing --- */
   }
}

#ifndef GRAPHICS
at(x,y,ch)
register xchar x,y;
char ch;
{
#ifndef lint
   /* if xchar is unsigned, lint will complain about  if(x < 0)  */
   if(x < 0 || x > COLNO-1 || y < 0 || y > ROWNO-1)
      panic("At gets 0%o at %d %d(%d %d)",ch,x,y,u.ux,u.uy);
#endif lint
   if(!ch) {
      home();
      myprintf("At gets null at %2d %2d.",x,y);
      curx = ROWNO+1;
      return;
   }
   y += 2;
   curs(x,y);
   myputchar(ch);
   curx++;
}
#endif

prme(){
   if(!Invis) at(u.ux,u.uy,u.usym);
}

docrt()
{
   register int x,y;
   register struct rm *room;
   register struct monst *mtmp;

   if(u.uswallow) {
      swallowed();
      return;
   }
   cls();
   if(!Invis){
      levl[(u.udisx = u.ux)][(u.udisy = u.uy)].scrsym = u.usym;
      levl[u.udisx][u.udisy].seen = 1;
      u.udispl = 1;
   } else   u.udispl = 0;

   /* %% - is this really necessary? */
   for(mtmp = fmon; mtmp; mtmp = mtmp->nmon)
      if(mtmp->mdispl && !(room = &levl[mtmp->mx][mtmp->my])->new &&
         !room->seen)
         mtmp->mdispl = 0;

   for(y = 0; y < ROWNO; y++)
      for(x = 0; x < COLNO; x++)
         if((room = &levl[x][y])->new) {
            room->new = 0;
            at(x,y,room->scrsym);
         } else if(room->seen) at(x,y,room->scrsym);
   scrlx = COLNO;
   scrly = ROWNO;
   scrhx = scrhy = 0;
   flags.botlx = 1;
   bot();
}

docorner(xmin,ymax) register int xmin,ymax; {
   register int x,y;
   register struct rm *room;
   if(u.uswallow) {   /* Can be done more efficiently */
      swallowed();
      return;
   }
   for(y = 0; y < ymax; y++) {
      curs(xmin,y+2);
      cl_end();
      for(x = xmin; x < COLNO; x++) {
         if((room = &levl[x][y])->new) {
            room->new = 0;
            at(x,y,room->scrsym);
         } else if(room->seen) at(x,y,room->scrsym);
      }
   }
}

pru()
{
   if(u.udispl && (Invis || u.udisx != u.ux || u.udisy != u.uy))
      /* if(! levl[u.udisx][u.udisy].new) */
         if(!vism_at(u.udisx, u.udisy))
            newsym(u.udisx, u.udisy);
   if(Invis) {
      u.udispl = 0;
      prl(u.ux,u.uy);
   } else
   if(!u.udispl || u.udisx != u.ux || u.udisy != u.uy) {
      atl(u.ux, u.uy, u.usym);
      u.udispl = 1;
      u.udisx = u.ux;
      u.udisy = u.uy;
   }
   levl[u.ux][u.uy].seen = 1;
}

#ifndef NOWORM
#include   "def.wseg.h"
extern struct wseg *m_atseg;
#endif NOWORM

/* print a position that is visible for @ */
prl(x,y)
{
   register struct rm *room;
   register struct monst *mtmp;
   register struct obj *otmp;

   if(x == u.ux && y == u.uy && !Invis) {
      pru();
      return;
   }
   room = &levl[x][y];
   if((!room->typ) || (room->typ<DOOR && levl[u.ux][u.uy].typ == CORR))
      return;
   if((mtmp = m_at(x,y)) && !mtmp->mhide &&
      (!mtmp->minvis || See_invisible)) {
#ifndef NOWORM
      if(m_atseg)
         pwseg(m_atseg);
      else
#endif NOWORM
      pmon(mtmp);
   }
   else if(otmp = o_at(x,y))
      atl(x,y,otmp->olet);
   else if(mtmp && (!mtmp->minvis || See_invisible)) {
      /* must be a hiding monster, but not hiding right now */
      /* assume for the moment that long worms do not hide */
      pmon(mtmp);
   }
   else if(g_at(x,y,fgold)) atl(x,y,'$');
   else if(!room->seen || room->scrsym == ' ') {
      room->new = room->seen = 1;
      newsym(x,y);
      on_scr(x,y);
   }
   room->seen = 1;
}

char
news0(x,y)
register xchar x,y;
{
   register struct obj *otmp;
   register struct gen *gtmp;
   struct rm *room;
   register char tmp;

   room = &levl[x][y];
   if(!room->seen) tmp = ' ';
   else if(!Blind && (otmp = o_at(x,y))) tmp = otmp->olet;
   else if(!Blind && g_at(x,y,fgold)) tmp = '$';
   else if(x == xupstair && y == yupstair) tmp = '<';
   else if(x == xdnstair && y == ydnstair) tmp = '>';
   else if((gtmp = g_at(x,y,ftrap)) && (gtmp->gflag & SEEN)) tmp = '^';
   else switch(room->typ) {
   case SCORR:
   case SDOOR:
      tmp = room->scrsym;   /* %% wrong after killing mimic ! */
      break;
   case HWALL:
      tmp = '-';
      break;
   case VWALL:
      tmp = '|';
      break;
   case LDOOR:
   case DOOR:
      tmp = '+';
      break;
   case CORR:
      tmp = CORR_SYM;
      break;
   case ROOM:
      if(room->lit || cansee(x,y) || Blind) tmp = '.';
      else tmp = ' ';
      break;
   default: tmp = ERRCHAR;
   }
   return(tmp);
}

newsym(x,y)
register int x,y;
{
   atl(x,y,news0(x,y));
}

/* used with wand of digging: fill scrsym and force display */
mnewsym(x,y)
register int x,y;
{
	register struct monst *mtmp = m_at(x,y);
	register struct rm *room;
	char newscrsym;

   if(!mtmp || (mtmp->minvis && !See_invisible) ||
          (mtmp->mhide && o_at(x,y))){
		room = &levl[x][y];
		newscrsym = news0(x,y);
		if(room->scrsym != newscrsym) {
			room->scrsym = newscrsym;
			room->seen = 0;
		}
   }
}

nosee(x,y)
register int x,y;
{
   register struct rm *room;

   room = &levl[x][y];
   if(room->scrsym == '.' && !room->lit && !Blind) {
      room->scrsym = ' ';
      room->new = 1;
      on_scr(x,y);
   }
}

#ifndef QUEST
prl1(x,y)
register int x,y;
{
   if(u.dx) {
      if(u.dy) {
         prl(x-(2*u.dx),y);
         prl(x-u.dx,y);
         prl(x,y);
         prl(x,y-u.dy);
         prl(x,y-(2*u.dy));
      } else {
         prl(x,y-1);
         prl(x,y);
         prl(x,y+1);
      }
   } else {
      prl(x-1,y);
      prl(x,y);
      prl(x+1,y);
   }
}

nose1(x,y)
register int x,y;
{
   if(u.dx) {
      if(u.dy) {
         nosee(x,u.uy);
         nosee(x,u.uy-u.dy);
         nosee(x,y);
         nosee(u.ux-u.dx,y);
         nosee(u.ux,y);
      } else {
         nosee(x,y-1);
         nosee(x,y);
         nosee(x,y+1);
      }
   } else {
      nosee(x-1,y);
      nosee(x,y);
      nosee(x+1,y);
   }
}
#endif QUEST

vism_at(x,y) register int x,y; {
register struct monst *mtmp;
register int csi = (See_invisible != 0);
   return((x == u.ux && y == u.uy && (!Invis || csi)) ? 1 :
      ((mtmp = m_at(x,y)) && (!mtmp->minvis || csi) &&
         (!mtmp->mhide || !o_at(mtmp->mx,mtmp->my)))
      ? cansee(x,y) : 0);
}

#ifdef NEWSCR
pobj(obj) register struct obj *obj; {
register int show = (!obj->oinvis || See_invisible) &&
      cansee(obj->ox,obj->oy);
   if(obj->odispl){
      if(obj->odx != obj->ox || obj->ody != obj->oy || !show)
      if(!vism_at(obj->odx,obj->ody)){
         newsym(obj->odx, obj->ody);
         obj->odispl = 0;
      }
   }
   if(show && !vism_at(obj->ox,obj->oy)){
      atl(obj->ox,obj->oy,obj->olet);
      obj->odispl = 1;
      obj->odx = obj->ox;
      obj->ody = obj->oy;
   }
}
#endif NEWSCR

unpobj(obj) register struct obj *obj; {
/*    if(obj->odispl){
      if(!vism_at(obj->odx, obj->ody))
         newsym(obj->odx, obj->ody);
      obj->odispl = 0;
   }
*/
   if(!vism_at(obj->ox,obj->oy))
      newsym(obj->ox,obj->oy);
}

seeobjs(){
register struct obj *obj, *obj2;
   for(obj = fobj; obj; obj = obj2) {
      obj2 = obj->nobj;
      if(obj->olet == FOOD_SYM && obj->otyp >= CORPSE
         && obj->age + 250 < moves)
            delobj(obj);
   }
   for(obj = invent; obj; obj = obj2) {
      obj2 = obj->nobj;
      if(obj->olet == FOOD_SYM && obj->otyp >= CORPSE
         && obj->age + 250 < moves)
            useup(obj);
   }
}

seemons(){
register struct monst *mtmp;
   for(mtmp = fmon; mtmp; mtmp = mtmp->nmon){
      pmon(mtmp);
#ifndef NOWORM
      if(mtmp->wormno) wormsee(mtmp->wormno);
#endif NOWORM
   }
}

pmon(mon) register struct monst *mon; {
register int show =
   ((!mon->minvis || See_invisible) &&
      (!mon->mhide || !o_at(mon->mx,mon->my)) &&
      cansee(mon->mx,mon->my))
    || (Blind && Telepat);
   if(mon->mdispl){
      if(mon->mdx != mon->mx || mon->mdy != mon->my || !show)
         unpmon(mon);
   }
   if(show && !mon->mdispl){
      atl(mon->mx,mon->my,
        mon->mimic ? mon->mimic : mon->data->mlet);
      mon->mdispl = 1;
      mon->mdx = mon->mx;
      mon->mdy = mon->my;
   }
}

unpmon(mon) register struct monst *mon; {
   if(mon->mdispl){
      newsym(mon->mdx, mon->mdy);
      mon->mdispl = 0;
   }
}

nscr()
{
   register int x,y;
   register struct rm *room;

   if(u.uswallow || u.ux == FAR || flags.nscrinh) return;
   pru();
   for(y = scrly; y <= scrhy; y++)
      for(x = scrlx; x <= scrhx; x++)
         if((room = &levl[x][y])->new) {
            room->new = 0;
            at(x,y,room->scrsym);
         }
   scrhx = scrhy = 0;
   scrlx = COLNO;
   scrly = ROWNO;
}

char oldbot[100], newbot[100];      /* 100 >= COLNO */
extern char *eos();
bot()
{
register char *ob = oldbot, *nb = newbot;
register int i;
	if(flags.botlx) *ob = 0;
	flags.botl = flags.botlx = 0;
	(void) sprintf(newbot,
		"Level %-2d  Gold %-5lu  Hp %3d(%d)  Ac %-2d  Str ",
		dlevel, u.ugold, u.uhp, u.uhpmax, u.uac);
	if(u.ustr>18) {
	    if(u.ustr>117)
		(void) strcat(newbot,"18/**");
	    else
		(void) sprintf(eos(newbot), "18/%02d",u.ustr-18);
	} else
	    (void) sprintf(eos(newbot), "%-2d   ",u.ustr);
	(void) sprintf(eos(newbot), "  Exp %2d/%-5lu ", u.ulevel,u.uexp);
	(void) strcat(newbot, hu_stat[u.uhs]);
	if(flags.time)
	    (void) sprintf(eos(newbot), "  %ld", moves);
	if(strlen(newbot) >= COLNO) {
		register char *bp0, *bp1;
		bp0 = bp1 = newbot;
		do {
			if(*bp0 != ' ' || bp0[1] != ' ' || bp0[2] != ' ')
				*bp1++ = *bp0;
		} while(*bp0++);
	}
	for(i = 1; i<COLNO; i++) {
      if(*ob != *nb){
         curs(i,ROWNO+2);
         (void) myputchar(*nb ? *nb : ' ');
         curx++;
      }
      if(*ob) ob++;
      if(*nb) nb++;
   }
   (void) strcpy(oldbot, newbot);
}

#ifdef WAN_PROBING
mstatusline(mtmp) register struct monst *mtmp; {
   pline("Status of %s: ", monnam(mtmp));
   pline("Level %-2d  Gold %-5lu  Hp %3d(%d)  Ac %-2d  Dam %d",
       mtmp->data->mlevel, mtmp->mgold, mtmp->mhp, mtmp->orig_hp,
       mtmp->data->ac, (mtmp->data->damn + 1) * (mtmp->data->damd + 1));
}
#endif WAN_PROBING

cls(){
   if(flags.topl == 1)
      more();
   flags.topl = 0;

   clear_screen();

   flags.botlx = 1;
}

#file hack.read.c
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* hack.read.c version 1.0.1 - tiny correction in SCR_MAGIC_MAPPING */

#include "hack.h"

extern struct monst *makemon();
int identify();

doread() {
	register struct obj *scroll;
	register boolean confused = (Confusion != 0);
	register boolean known = FALSE;

	scroll = getobj("?", "read");
	if(!scroll) return(0);
	if(!scroll->dknown && Blind) {
	    pline("Being blind, you cannot read the formula on the scroll.");
	    return(0);
	}
	if(Blind)
	  pline("As you pronounce the formula on it, the scroll disappears.");
	else
	  pline("As you read the scroll, it disappears.");
	if(confused)
	  pline("Being confused, you mispronounce the magic words ... ");

	switch(scroll->otyp) {

	case SCR_ENCHANT_ARMOR:
	    {	extern struct obj *some_armor();
		register struct obj *otmp = some_armor();
		if(!otmp) {
			strange_feeling(scroll);
			return(1);
		}
		if(confused) {
			pline("Your %s glows silver for a moment.",
				objects[otmp->otyp].oc_name);
			otmp->rustfree = 1;
			break;
		}
		if(otmp->spe*2 + objects[otmp->otyp].a_ac > 23 &&
			!rn2(3)) {
	pline("Your %s glows violently green for a while, then evaporates.",
			objects[otmp->otyp].oc_name);
			useup(otmp);
			break;
		}
		pline("Your %s glows green for a moment.",
			objects[otmp->otyp].oc_name);
		otmp->cursed = 0;
		otmp->spe++;
		break;
	    }
	case SCR_DESTROY_ARMOR:
		if(confused) {
			register struct obj *otmp = some_armor();
			if(!otmp) {
				strange_feeling(scroll);
				return(1);
			}
			pline("Your %s glows purple for a moment.",
				objects[otmp->otyp].oc_name);
			otmp->rustfree = 0;
			break;
		}
		if(uarm) {
		    pline("Your armor turns to dust and falls to the floor!");
		    useup(uarm);
		} else if(uarmh) {
		    pline("Your helmet turns to dust and is blown away!");
		    useup(uarmh);
		} else if(uarmg) {
			pline("Your gloves vanish!");
			useup(uarmg);
			selftouch("You");
		} else {
			strange_feeling(scroll);
			return(1);
		}
		break;
	case SCR_CONFUSE_MONSTER:
		if(confused) {
			pline("Your hands begin to glow purple.");
			Confusion += rnd(100);
		} else {
			pline("Your hands begin to glow blue.");
			u.umconf = 1;
		}
		break;
	case SCR_SCARE_MONSTER:
	    {	register int ct = 0;
		register struct monst *mtmp;

		for(mtmp = fmon; mtmp; mtmp = mtmp->nmon)
			if(cansee(mtmp->mx,mtmp->my)) {
				if(confused)
					mtmp->mflee = mtmp->mfroz =
					mtmp->msleep = 0;
				else
					mtmp->mflee = 1;
				ct++;
			}
		if(!ct) {
		    if(confused)
			pline("You hear sad wailing in the distance.");
		    else
			pline("You hear maniacal laughter in the distance.");
		}
		break;
	    }
	case SCR_BLANK_PAPER:
		if(confused)
		    pline("You see strange patterns on this scroll.");
		else
		    pline("This scroll seems to be blank.");
		break;
	case SCR_REMOVE_CURSE:
	    {	register struct obj *obj;
		if(confused)
		  pline("You feel like you need some help.");
		else
		  pline("You feel like someone is helping you.");
		for(obj = invent; obj ; obj = obj->nobj)
			if(obj->owornmask)
				obj->cursed = confused;
		if(Punished && !confused) {
			Punished = 0;
			freeobj(uchain);
			unpobj(uchain);
			free((char *) uchain);
			uball->spe = 0;
			uball->owornmask &= ~W_BALL;
			uchain = uball = (struct obj *) 0;
		}
		break;
	    }
	case SCR_CREATE_MONSTER:
	    {	register int cnt = 1;

		if(!rn2(73)) cnt += rn2(4) + 1;
		if(confused) cnt += 12;
		while(cnt--)
		    (void) makemon(confused ? PM_ACIDBLOB :
			(struct permonst *) 0, u.ux, u.uy);
		break;
	    }
	case SCR_ENCHANT_WEAPON:
		if(!uwep) {
			strange_feeling(scroll);
			return(1);
		}
		if(confused) {
			pline("Your %s glows silver for a moment.",
				objects[uwep->otyp].oc_name);
			uwep->rustfree = 1;
		} else
			if(!chwepon(scroll, 1)) return(1);
		break;
	case SCR_DAMAGE_WEAPON:
		if(confused) {
			pline("Your %s glows purple for a moment.",
				objects[uwep->otyp].oc_name);
			uwep->rustfree = 0;
		} else
			if(!chwepon(scroll, -1)) return(1);
		break;
	case SCR_TAMING:
	    {	register int i,j;
		register int bd = confused ? 5 : 1;
		register struct monst *mtmp;

		for(i = -bd; i <= bd; i++) for(j = -bd; j <= bd; j++)
		if(mtmp = m_at(u.ux+i, u.uy+j))
			(void) tamedog(mtmp, (struct obj *) 0);
		break;
	    }
	case SCR_GENOCIDE:
	    {	extern char genocided[], fut_geno[];
		char buf[BUFSZ];
		register struct monst *mtmp, *mtmp2;

		pline("You have found a scroll of genocide!");
		known = TRUE;
		if(confused)
			*buf = u.usym;
		else do {
	    pline("What monster do you want to genocide (Type the letter)? ");
			getlin(buf);
		} while(strlen(buf) != 1 || !letter(*buf));
		if(!index(fut_geno, *buf))
			charcat(fut_geno, *buf);
		if(!index(genocided, *buf))
			charcat(genocided, *buf);
		else {
			pline("Such monsters do not exist in this world.");
			break;
		}
		for(mtmp = fmon; mtmp; mtmp = mtmp2){
			mtmp2 = mtmp->nmon;
			if(mtmp->data->mlet == *buf)
				mondead(mtmp);
		}
		pline("Wiped out all %c's.", *buf);
		if(*buf == u.usym) {
			killer = "scroll of genocide";
			u.uhp = -1;
		}
		break;
		}
	case SCR_LIGHT:
		if(!Blind) known = TRUE;
		litroom(!confused);
		break;
	case SCR_TELEPORTATION:
		if(confused)
			level_tele();
		else {
#ifdef QUEST
			register int oux = u.ux, ouy = u.uy;
			tele();
			if(dist(oux, ouy) > 100) known = TRUE;
#else QUEST
			register int uroom = inroom(u.ux, u.uy);
			tele();
			if(uroom != inroom(u.ux, u.uy)) known = TRUE;
#endif QUEST
		}
		break;
	case SCR_GOLD_DETECTION:
	    {	register struct gen *head = confused ? ftrap : fgold;
		register struct gen *gtmp;

		if(!head) {
			strange_feeling(scroll);
			return(1);
		} else {
			known = TRUE;
			for(gtmp = head; gtmp; gtmp = gtmp->ngen)
				if(gtmp->gx != u.ux || gtmp->gy != u.uy)
					goto outgoldmap;
			/* only under me - no separate display required */
			if(confused)
			    pline("You feel very giddy!");
			else
			    pline("You notice some gold between your feet.");
			break;
		outgoldmap:
			cls();
			for(gtmp = head; gtmp; gtmp = gtmp->ngen)
				at(gtmp->gx, gtmp->gy, '$');
			prme();
			if(confused)
			    pline("You feel very greedy!");
			else
			    pline("You feel very greedy, and sense gold!");
			more();
			docrt();
		}
		break;
	    }
	case SCR_FOOD_DETECTION:
	    {	register int ct = 0, ctu = 0;
		register struct obj *obj;
		register char foodsym = confused ? POTION_SYM : FOOD_SYM;

		for(obj = fobj; obj; obj = obj->nobj)
			if(obj->olet == FOOD_SYM) {
				if(obj->ox == u.ux && obj->oy == u.uy) ctu++;
				else ct++;
			}
		if(!ct && !ctu) {
			strange_feeling(scroll);
			return(1);
		} else if(!ct) {
			known = TRUE;
			pline("You smell %s close nearby.",
				confused ? "something" : "food");
			
		} else {
			known = TRUE;
			cls();
			for(obj = fobj; obj; obj = obj->nobj)
			    if(obj->olet == foodsym)
				at(obj->ox, obj->oy, FOOD_SYM);
			prme();
			pline("Your nose tingles and you smell %s!",
				confused ? "something" : "food");
			more();
			docrt();
		}
		break;
	    }
	case SCR_IDENTIFY:
		/* known = TRUE; */
		if(confused)
			pline("You identify this as an identify scroll.");
		else
			pline("This is an identify scroll.");
		useup(scroll);
		objects[SCR_IDENTIFY].oc_name_known = 1;
		if(!confused)
		    while(
			!ggetobj("identify", identify, rn2(5) ? 1 : rn2(5))
			&& invent
		    );
		return(1);
	case SCR_MAGIC_MAPPING:
	    {	register struct rm *lev;
		register int num, zx, zy;

		known = TRUE;
		pline("On this scroll %s a map!",
			confused ? "was" : "is");
		for(zy = 0; zy < ROWNO; zy++)
			for(zx = 0; zx < COLNO; zx++) {
				if(confused && rn2(7)) continue;
				lev = &(levl[zx][zy]);
				if((num = lev->typ) == 0)
					continue;
				if(num == SCORR) {
					lev->typ = CORR;
					lev->scrsym = CORR_SYM;
				} else
				if(num == SDOOR) {
					lev->typ = DOOR;
					lev->scrsym = '+';
					/* do sth in doors ? */
				} else if(lev->seen) continue;
#ifndef QUEST
				if(num != ROOM)
#endif QUEST
				{
				  lev->seen = lev->new = 1;
				  if(lev->scrsym == ' ')
				    newsym(zx,zy);
				  else
				    on_scr(zx,zy);
				}
			}
		break;
	    }
	case SCR_AMNESIA:
	    {	register int zx, zy;

		known = TRUE;
		for(zx = 0; zx < COLNO; zx++) for(zy = 0; zy < ROWNO; zy++)
		    if(!confused || rn2(7))
			if(!cansee(zx,zy))
			    levl[zx][zy].seen = 0;
		docrt();
		pline("Thinking of Maud you forget everything else.");
		break;
	    }
	case SCR_FIRE:
	    {	register int num;

		known = TRUE;
		if(confused) {
		    pline("The scroll catches fire and you burn your hands.");
		    losehp(1, "scroll of fire");
		} else {
		    pline("The scroll erupts in a tower of flame!");
		    if(Fire_resistance)
			pline("You are uninjured.");
		    else {
			num = rnd(6);
			u.uhpmax -= num;
			losehp(num, "scroll of fire");
		    }
		}
		break;
	    }
	case SCR_PUNISHMENT:
		known = TRUE;
		if(confused) {
			pline("You feel guilty.");
			break;
		}
		pline("You are being punished for your misbehaviour!");
		if(Punished){
			pline("Your iron ball gets heavier.");
			uball->owt += 15;
			break;
		}
		Punished = INTRINSIC;
		mkobj_at(CHAIN_SYM, u.ux, u.uy);
		setworn(fobj, W_CHAIN);
		mkobj_at(BALL_SYM, u.ux, u.uy);
		setworn(fobj, W_BALL);
		uball->spe = 1;		/* special ball (see save) */
		break;
	default:
		pline("What weird language is this written in? (%d)",
			scroll->otyp);
		impossible();
	}
	if(!objects[scroll->otyp].oc_name_known) {
		if(known && !confused) {
			objects[scroll->otyp].oc_name_known = 1;
			u.urexp += 10;
		} else if(!objects[scroll->otyp].oc_uname)
			docall(scroll);
	}
	useup(scroll);
	return(1);
}

identify(otmp)
register struct obj *otmp;
{
	objects[otmp->otyp].oc_name_known = 1;
	otmp->known = otmp->dknown = 1;
	prinv(otmp);
	return(1);
}

litroom(on)
register boolean on;
{
	register int num,zx,zy;

	/* first produce the text (provided he is not blind) */
	if(Blind) goto do_it;
	if(!on) {
		if(u.uswallow || !xdnstair || levl[u.ux][u.uy].typ == CORR ||
		    !levl[u.ux][u.uy].lit) {
			pline("It seems even darker in here than before.");
			return;
		} else
			pline("It suddenly becomes dark in here.");
	} else {
		if(u.uswallow){
			pline("%s's stomach is lit.", Monnam(u.ustuck));
			return;
		}
		if(!xdnstair){
			pline("Nothing Happens");
			return;
		}
#ifdef QUEST
		pline("The cave lights up around you, then fades.");
		return;
#else QUEST
		if(levl[u.ux][u.uy].typ == CORR) {
		    pline("The corridor lights up around you, then fades.");
		    return;
		} else if(levl[u.ux][u.uy].lit) {
		    pline("The light here seems better now.");
		    return;
		} else
		    pline("The room is lit.");
#endif QUEST
	}

do_it:
#ifdef QUEST
	return;
#else QUEST
	if(levl[u.ux][u.uy].lit == on)
		return;
	if(levl[u.ux][u.uy].typ == DOOR) {
		if(levl[u.ux][u.uy+1].typ >= ROOM) zy = u.uy+1;
		else if(levl[u.ux][u.uy-1].typ >= ROOM) zy = u.uy-1;
		else zy = u.uy;
		if(levl[u.ux+1][u.uy].typ >= ROOM) zx = u.ux+1;
		else if(levl[u.ux-1][u.uy].typ >= ROOM) zx = u.ux-1;
		else zx = u.ux;
	} else {
		zx = u.ux;
		zy = u.uy;
	}
	for(seelx = u.ux; (num = levl[seelx-1][zy].typ) != CORR && num != 0;
		seelx--);
	for(seehx = u.ux; (num = levl[seehx+1][zy].typ) != CORR && num != 0;
		seehx++);
	for(seely = u.uy; (num = levl[zx][seely-1].typ) != CORR && num != 0;
		seely--);
	for(seehy = u.uy; (num = levl[zx][seehy+1].typ) != CORR && num != 0;
		seehy++);
	for(zy = seely; zy <= seehy; zy++)
		for(zx = seelx; zx <= seehx; zx++) {
			levl[zx][zy].lit = on;
			if(!Blind && dist(zx,zy) > 2)
				if(on) prl(zx,zy); else nosee(zx,zy);
		}
	if(!on) seehx = 0;
#endif	QUEST
}
#file hack.rip.c
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1984. */

#include <stdio.h>
#include "hack.h"
/* #include   <libraries/dos.h> */

extern char plname[];

static char *rip[] = {
"                       ----------",
"                      /          \\",
"                     /    REST    \\",
"                    /      IN      \\",
"                   /     PEACE      \\",
"                  /                  \\",
"                  |                  |",
"                  |                  |",
"                  |                  |",
"                  |                  |",
"                  |                  |",
"                  |       1001       |",
"                 *|     *  *  *      | *",
"        _________)/\\\\_//(\\/(/\\)/\\//\\/|_)_______\n",
0
};

outrip(){
   register char **dp = rip;
   register char *dpx;
/*   struct DateStamp now, *DateStamp(); */
   char buf[BUFSZ];
   register int x,y;
   int thisyear;

   cls();

/*   now = DateStamp(&now);  */
   thisyear = 85;
   (void) strcpy(buf, plname);
   buf[16] = 0;
   center(6, buf);
   (void) sprintf(buf, "%ld AU", u.ugold);
   center(7, buf);
   (void) sprintf(buf, "killed by%s",
      !strncmp(killer, "the ", 4) ? "" :
      !strcmp(killer, "starvation") ? "" :
      index(vowels, *killer) ? " an" : " a");
   center(8, buf);
   (void) strcpy(buf, killer);
   if(strlen(buf) > 16) {
       register int i,i0,i1;
      i0 = i1 = 0;
      for(i = 0; i <= 16; i++)
         if(buf[i] == ' ') i0 = i, i1 = i+1;
      if(!i0) i0 = i1 = 16;
      buf[i1 + 16] = 0;
      center(10, buf+i1);
      buf[i0] = 0;
   }
   center(9, buf);
   (void) sprintf(buf, "19%2d", thisyear);
   center(11, buf);
   for(y=8; *dp; y++,dp++){
      x = 0;
      dpx = *dp;
      while(dpx[x]) {
         while(dpx[x] == ' ') x++;
         curs(x,y);
         while(dpx[x] && dpx[x] != ' '){
            extern int done_stopprint;
            if(done_stopprint)
               return;
            curx++;
            (void) myputchar(dpx[x++]);
         }
      }
   }
   getret();
}

center(line, text) int line; char *text; {
register char *ip,*op;
   ip = text;
   op = &rip[line][28 - ((strlen(text)+1)/2)];
   while(*ip) *op++ = *ip++;
}
#file hack.rumors.c
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1984. */



#include	<stdio.h>

#include	"config.h"

#define	CHARSZ	8	/* number of bits in a char */

#define	RUMORFILE	"rumors"

extern long *alloc();

extern char *index();

int n_rumors = 0;

int n_used_rumors = -1;

char *usedbits;



init_rumors(rumf) register FILE *rumf; {

register int i;

	n_used_rumors = 0;

	while(skipline(rumf)) n_rumors++;

	rewind(rumf);

	i = n_rumors/CHARSZ;

	usedbits = (char *) alloc((unsigned)(i+1));

	for( ; i>=0; i--) usedbits[i] = 0;

}



skipline(rumf) register FILE *rumf; {

char line[COLNO];

	while(1) {

		if(!fgets(line, sizeof(line), rumf)) return(0);

		if(index(line, '\n')) return(1);

	}

}



outline(rumf) register FILE *rumf; {

char line[COLNO];

register char *ep;

	if(!fgets(line, sizeof(line), rumf)) return;

	if((ep = index(line, '\n')) != 0) *ep = 0;

	pline("This cookie has a scrap of paper inside! It reads: ");

	pline(line);

}



outrumor(){

register int rn,i;

register FILE *rumf;

	if(n_rumors <= n_used_rumors ||

	  (rumf = fopen(RUMORFILE, "r")) == NULL) return;

	if(n_used_rumors < 0) init_rumors(rumf);

	if(!n_rumors) goto none;

	rn = rn2(n_rumors - n_used_rumors);

	i = 0;

	while(rn || used(i)) {

		(void) skipline(rumf);

		if(!used(i)) rn--;

		i++;

	}

	usedbits[i/CHARSZ] |= (1 << (i % CHARSZ));

	n_used_rumors++;

	outline(rumf);

none:

	(void) fclose(rumf);

}



used(i) register int i; {

	return(usedbits[i/CHARSZ] & (1 << (i % CHARSZ)));

}

#file hack.save.c
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1984. */

#include "hack.h"
extern char genocided[60];	/* defined in Decl.c */
extern char fut_geno[60];	/* idem */
#include <signal.h>

extern char SAVEF[], nul[];
extern char pl_character[PL_CSIZ];
extern long lseek();
extern struct obj *restobjchn();
extern struct monst *restmonchn();

extern char *index();		/* M.E.T.  11/20/85 */

dosave(){
	if(dosave0(0)) {
		settty("Be seeing you ...\n");
		hackexit(0);
	}
#ifdef lint
	return(0);
#endif lint
}

#ifndef NOSAVEONHANGUP
hangup(){
	(void) dosave0(1);
	hackexit(1);
}
#endif NOSAVEONHANGUP

/* returns 1 if save successful */
dosave0(hu) int hu; {
	register int fd, ofd;
	int tmp;      /* not register ! */
	(void) signal(SIGHUP, SIG_IGN);
	(void) signal(SIGINT, SIG_IGN);
	if((fd = creat(SAVEF, FMASK)) < 0)
		{
		if(!hu) pline("Cannot open save file. (Continue or Quit)");
			return(0);
		}
	makeicon(SAVEF, pl_character[0]);
	savelev(fd);
	saveobjchn(fd, invent);
	saveobjchn(fd, fcobj);
	savemonchn(fd, fallen_down);
	bwrite(fd, (char *) &flags, sizeof(struct flag));
	bwrite(fd, (char *) &dlevel, sizeof dlevel);
	bwrite(fd, (char *) &maxdlevel, sizeof maxdlevel);
	bwrite(fd, (char *) &moves, sizeof moves);
	bwrite(fd, (char *) &u, sizeof(struct you));
	bwrite(fd, (char *) pl_character, sizeof pl_character);
	bwrite(fd, (char *) genocided, sizeof genocided);
	bwrite(fd, (char *) fut_geno, sizeof fut_geno);
	savenames(fd);
	for(tmp = 1; tmp <= maxdlevel; tmp++)
		{
		glo(tmp);
		if((ofd = open(lock, 0)) < 0)
			continue;
		(void) getlev(ofd);
		(void) close(ofd);
		bwrite(fd, (char *) &tmp, sizeof tmp);   /* level number */
		savelev(fd);            /* actual level */
		(void) unlink(lock);
		}
	(void) close(fd);
	*index(lock, '.') = 0;
	(void) unlink(lock);
	return(1);
}

dorecover(fd)
register int fd;
{
	register int nfd;
	int tmp;      /* not a register ! */
	struct obj *otmp;
	 (void) getlev(fd);
	invent = restobjchn(fd);
	for(otmp = invent; otmp; otmp = otmp->nobj)
		if(otmp->owornmask)
			setworn(otmp, otmp->owornmask);
	fcobj = restobjchn(fd);
	fallen_down = restmonchn(fd);
	mread(fd, (char *) &flags, sizeof(struct flag));
	mread(fd, (char *) &dlevel, sizeof dlevel);
	mread(fd, (char *) &maxdlevel, sizeof maxdlevel);
	mread(fd, (char *) &moves, sizeof moves);
	mread(fd, (char *) &u, sizeof(struct you));
	mread(fd, (char *) pl_character, sizeof pl_character);
	mread(fd, (char *) genocided, sizeof genocided);
	mread(fd, (char *) fut_geno, sizeof fut_geno);
	restnames(fd);
	while(1)
		{
		if(read(fd, (char *) &tmp, sizeof tmp) != sizeof tmp)
			break;
		if(getlev(fd))
			break;      /* this is actually an error */
		glo(tmp);
		if((nfd = creat(lock, FMASK)) < 0)
			panic("Cannot open temp file %s!\n", lock);
		savelev(nfd);
		(void) close(nfd);
		}
	(void) lseek(fd, 0L, 0);
	(void) getlev(fd);
	(void) close(fd);
	(void) unlink(SAVEF);
	(void) delicon(SAVEF);
	if(Punished) {
		for(otmp = fobj; otmp; otmp = otmp->nobj)
			if(otmp->olet == CHAIN_SYM) goto chainfnd;
		panic("Cannot find the iron chain?");
	chainfnd:
		uchain = otmp;
		if(!uball)
			{
			for(otmp = fobj; otmp; otmp = otmp->nobj)
				if(otmp->olet == BALL_SYM && otmp->spe)
					goto ballfnd;
			panic("Cannot find the iron ball?");
			ballfnd:
			uball = otmp;
			}
	}
#ifndef QUEST
	setsee();  /* only to recompute seelx etc. - these weren't saved */
#endif QUEST
	docrt();
}

struct obj *
restobjchn(fd)
register int fd;
{
	register struct obj *otmp, *otmp2;
	register struct obj *first = 0;
	int xl;
#ifdef lint
	/* suppress "used before set" warning from lint */
	otmp2 = 0;
#endif lint
	while(1)
		{
		mread(fd, (char *) &xl, sizeof(xl));
		if(xl == -1) break;
		otmp = newobj(xl);
		if(!first) first = otmp;
		else otmp2->nobj = otmp;
		mread(fd, (char *) otmp, (unsigned) xl + sizeof(struct obj));
		if(!otmp->o_id)   /* from MKLEV */
			otmp->o_id = flags.ident++;
		otmp2 = otmp;
		}
	if(first && otmp2->nobj)
		{
		pline("Restobjchn: error reading objchn.");
		impossible();
		otmp2->nobj = 0;
		}
	return(first);
}

struct monst *
restmonchn(fd)
register int fd;
{
	register struct monst *mtmp, *mtmp2;
	register struct monst *first = 0;
	int xl;
    
#ifdef FUNNYRELOC
	struct permonst *monbegin;

	mread(fd, (char *)&monbegin, sizeof(monbegin));
#endif

#ifdef lint
	/* suppress "used before set" warning from lint */
	mtmp2 = 0;
#endif lint
	while(1)
		{
		mread(fd, (char *) &xl, sizeof(xl));
		if(xl == -1) break;
		mtmp = newmonst(xl);
		if(!first) first = mtmp;
		else mtmp2->nmon = mtmp;
		mread(fd, (char *) mtmp, (unsigned) xl + sizeof(struct monst));
#ifdef DEBUGMON
		myprintf("Read Monster #%d", mtmp->data);
#endif
		mtmp->data = &mons[ (int) mtmp->data ];
		if(!mtmp->m_id) {         /* from MKLEV */
			mtmp->m_id = flags.ident++;
#ifndef NOWORM
			if(mtmp->data->mlet == 'w' && getwn(mtmp)){
			initworm(mtmp);
			mtmp->msleep = 0;
			}
#endif NOWORM
		}
		if(mtmp->minvent)
			mtmp->minvent = restobjchn(fd);
		mtmp2 = mtmp;
		}
	if(first && mtmp2->nmon){
		pline("Restmonchn: error reading monchn.");
		impossible();
		mtmp2->nmon = 0;
	}
	return(first);
}
#file hack.search.c
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* hack.search.c version 1.0.1 - small correction in findit() */

#include "hack.h"
#include "def.trap.h"

extern struct monst *makemon();

findit()   /* returns number of things found */
{
   int num;
   register xchar zx,zy;
   register struct gen *gtmp;
   register struct monst *mtmp;
   xchar lx,hx,ly,hy;

   if(u.uswallow) return(0);
   for(lx = u.ux;(num = levl[lx-1][u.uy].typ) && num != CORR;lx--) ;
   for(hx = u.ux;(num = levl[hx+1][u.uy].typ) && num != CORR;hx++) ;
   for(ly = u.uy;(num = levl[u.ux][ly-1].typ) && num != CORR;ly--) ;
   for(hy = u.uy;(num = levl[u.ux][hy+1].typ) && num != CORR;hy++) ;
   num = 0;
   for(zy = ly;zy <= hy;zy++)
      for(zx = lx;zx <= hx;zx++) {
         if(levl[zx][zy].typ == SDOOR) {
            levl[zx][zy].typ = DOOR;
            atl(zx,zy,'+');
            num++;
         } else if(levl[zx][zy].typ == SCORR) {
            levl[zx][zy].typ = CORR;
            atl(zx,zy,CORR_SYM);
            num++;
         } else if(gtmp = g_at(zx,zy,ftrap)) {
            if(gtmp->gflag == PIERC){
               (void) makemon(PM_PIERC,zx,zy);
               num++;
               deltrap(gtmp);
				} else if(!(gtmp->gflag & SEEN)) {
					gtmp->gflag |= SEEN;
					if(!vism_at(zx,zy)) atl(zx,zy,'^');
					num++;
				}
         } else if(mtmp = m_at(zx,zy)) if(mtmp->mimic){
            seemimic(mtmp);
            num++;
         }
      }
   return(num);
}

dosearch()
{
   register xchar x,y;
   register struct gen *tgen;
   register struct monst *mtmp;

   for(x = u.ux-1; x < u.ux+2; x++)
   for(y = u.uy-1; y < u.uy+2; y++) if(x != u.ux || y != u.uy) {
      if(levl[x][y].typ == SDOOR && !rn2(7)) {
         levl[x][y].typ = DOOR;
         levl[x][y].seen = 0;   /* force prl */
         prl(x,y);
         nomul(0);
      } else if(levl[x][y].typ == SCORR && !rn2(7)) {
         levl[x][y].typ = CORR;
         levl[x][y].seen = 0;   /* force prl */
         prl(x,y);
         nomul(0);
      } else {
         if(mtmp = m_at(x,y)) if(mtmp->mimic){
            seemimic(mtmp);
            pline("You find a mimic.");
            return(1);
         }
         for(tgen = ftrap;tgen;tgen = tgen->ngen)
         if(tgen->gx == x && tgen->gy == y &&
            !(tgen->gflag & SEEN) && !rn2(8)) {
            nomul(0);
            pline("You find a%s.",
					traps[tgen->gflag & TRAPTYPE]);
            if(tgen->gflag == PIERC) {
               deltrap(tgen);
               (void) makemon(PM_PIERC,x,y);
               return(1);
            }
            tgen->gflag |= SEEN;
            if(!vism_at(x,y)) atl(x,y,'^');
         }
      }
   }
   return(1);
}

/* ARGSUSED */
doidtrap(str) /* register */ char *str; {
register struct gen *tgen;
register int x,y;
   if(!getdir()) return(0);
   x = u.ux + u.dx;
   y = u.uy + u.dy;
   for(tgen = ftrap; tgen; tgen = tgen->ngen)
      if(tgen->gx == x && tgen->gy == y &&
         (tgen->gflag & SEEN)) {
			pline("That is a%s.", traps[tgen->gflag & TRAPTYPE]);
         return(0);
      }
   pline("I can't see a trap there.");
   return(0);
}

wakeup(mtmp)
register struct monst *mtmp;
{
   mtmp->msleep = 0;
   setmangry(mtmp);
   if(mtmp->mimic) seemimic(mtmp);
}

/* NOTE: we must check if(mtmp->mimic) before calling this routine */
seemimic(mtmp)
register struct monst *mtmp;
{
      mtmp->mimic = 0;
      unpmon(mtmp);
      pmon(mtmp);
}
