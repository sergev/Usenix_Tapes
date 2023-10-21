/*
 * fight.c   Phantasia monster fighting routine
 */

/*
 * The code exists here for fight to the finish.  Simply add code to
 * set 'fgttofin = TRUE' as an option.	Everything else is here.
 */
#include "phant.h"

void	fight(stat,particular)		/* monster fighting routine */
struct	stats	*stat;
int		particular;
{

	bool	checkhit = TRUE, fghttofin = FALSE, luckout = FALSE,
			firsthit = stat->bls;
	char	aline[80];
	double	monhit, mdamage, sdamage, monspd, maxspd, inflict, monstr, temp,
			shield;
	int		ch, whichm, size, howmany, lines;
	struct	mstats	monster;

	fghting = changed = TRUE;

	stat->status = INBATTLE;
	update(stat,fileloc);

	shield = 0.0;
	if (setjmp(fightenv) == 2)
		shield = roll(100 + (stat->mxn + stat->shd)*6.2,3000);
	howmany = 1;
	size = (valhala) ? stat->lvl/5 : circ(stat->x,stat->y);
	if (particular >= 0)
		whichm = particular;
	else if (marsh)
		whichm = roll(0,15);
	else if (size > 24)
		whichm = roll(14,86);
	else if (size > 15)
		whichm = roll(0,50) + roll(14,37);
	else if (size > 8)
		whichm = roll(0,50) + roll(14,26);
	else if (size > 3)
		whichm = roll(14,50);
	else 
		whichm = roll(14,25);
	move(6,0);
	clrtobot();

CALL: move(6,0);
	lines = 9;
	callmonster(whichm,size,&monster);
	if (stat->blind)
		strcpy(monster.mname,"a monster");
	if (monster.mtyp == 1)   /* unicorn */
		if (stat->vrg)
		{
			printw("You just subdued %s, thanks to the virgin.\n",
					monster.mname);
			stat->vrg = FALSE;
			if (stat->sin > 1)
				stat->sin -= 1;
			if (stat->psn > 1)
				stat->psn -= 1;
			goto FINISH;
		}
		else
		{
			printw("You just saw %s running away!\n",monster.mname);
			goto LEAVE;
		}
	if (monster.mtyp == 2 && stat->typ > 20)
	{
		strcpy(monster.mname,"Morgoth");
		monster.mstr = rnd()*(stat->mxn + stat->shd)/1.4 + rnd()*(stat->mxn + stat->shd)/1.5;
		monster.mbrn = stat->brn;
		monster.mhit = stat->str*30;
		monster.mtyp = 23;
		monster.mspd = speed*1.1 + speed*(stat->typ == 90);
		monster.mflk = monster.mtrs = monster.mexp = 0;
		mvprintw(4,0,"You've encountered %s, Bane of the Council and Valar.\n",monster.mname);
	}
	fghttofin = luckout = FALSE;
	monstr = monster.mstr;
	monhit = monster.mhit;
	mdamage = sdamage = 0;
	monspd = maxspd = monster.mspd;
	if (speed <= 0)
	{
		monster.mspd += - speed;
		speed = 1;
	}
	move(8,0);
	clrtobot();

TOP: mvprintw(6,0,"You are being attacked by %s,   Exp: %.0f   (Size: %d)\n",
				monster.mname,monster.mexp,size);
	printstats(stat);
	mesg();
#ifdef SHELL
	ch='x';			/* set to garbage char to fix up shell escapes */
#endif
	mvprintw(1,26,"%9.0f",stat->energy + shield);
	if (monster.mtyp == 4 && stat->bls && stat->chm)
	{
		mvprintw(7,0,"You just overpowered %s!",monster.mname);
		lines = 8;
		stat->bls = FALSE;
		--stat->chm;
		goto FINISH;
	}
	monster.mspd = min(monster.mspd + 1,maxspd);
	if (rnd()*monster.mspd > rnd()*speed && monster.mtyp != 4 &&
			monster.mtyp != 16 && !firsthit && checkhit)
	{
		if (monster.mtyp)
			switch (monster.mtyp)    /* do special things */
			{
			case 5: /* Leanan-Sidhe */
				if (rnd() > 0.25)
					goto NORMALHIT;
				inflict = roll(1,(size - 1)/2);
				inflict = min(stat->str,inflict);
				mvprintw(lines++,0,"%s sapped %0.f of your strength!",
							monster.mname,inflict);
				stat->str -= inflict;
				strength -= inflict;
				break;
			case 6: /* Saruman */
				if (stat->pal)
				{
					mvprintw(lines++,0,"Wormtongue stole your palantir!");
					stat->pal = FALSE;
				}
				else if (rnd() > 0.2)
					goto NORMALHIT;
				else if (rnd() > 0.5)
				{
					mvprintw(lines++,0,"%s transformed your gems to gold!",
							monster.mname);
					stat->gld += stat->gem;
					stat->gem = 0.0;
				}
				else
				{
					mvprintw(lines++,0,"%s scrambled your stats!",
							monster.mname);
					scramble(stat);
				}
				break;
			case 7: /* Thaumaturgist */
				if (rnd() > 0.15)
					goto NORMALHIT;
				mvprintw(lines++,0,"%s transported you!",monster.mname);
				stat->x += sgn(stat->x)*roll(50*size,250*size);
				stat->y += sgn(stat->y)*roll(50*size,250*size);
				goto LEAVE;
			case 8: /* Balrog */
				inflict = roll(10,monster.mstr);
				inflict = min(stat->exp,inflict);
				mvprintw(lines++,0,"%s took away %.0f experience points.",
						monster.mname,inflict);
				stat->exp -= inflict;
				break;
			case 9: /* Vortex */
				if (rnd() > 0.2)
					goto NORMALHIT;
				inflict = roll(0,7.5*size);
				inflict = min(stat->man,floor(inflict));
				mvprintw(lines++,0,"%s sucked up %.0f of your mana!",
						monster.mname,inflict);
				stat->man -= inflict;
				break;
			case 10:    /* Nazgul */
				if (rnd() > 0.3)
					goto NORMALHIT;
				if (stat->rng.type && stat->rng.type < 10)
				{
					mvaddstr(lines++,0,"Will you relinquish your ring? ");
					ch = getans("YN", FALSE);
					if (ch == 'Y')
					{
						stat->rng.type = NONE;
						goto LEAVE;
					}
				}
				mvprintw(lines++,0,"%s neutralized 1/5 of your brain!",
						monster.mname);
				stat->brn *= 0.8;
				break;
			case 11:    /* Tiamat */
				if (rnd() > 0.6)
					goto NORMALHIT;
				mvprintw(lines++,0,"%s took half your gold and gems and flew off.",monster.mname);
				stat->gld = floor(stat->gld/2);
				stat->gem = floor(stat->gem/2);
				goto LEAVE;
			case 12:    /* Kobold */
				if (rnd() >.7)
					goto NORMALHIT;
				mvprintw(lines++,0,"%s stole one gold piece and ran away.",
						monster.mname);
				stat->gld = max(0,stat->gld-1);
				goto LEAVE;
			case 13:    /* Shelob */
				if (rnd() > 0.5)
					goto NORMALHIT;
				mvprintw(lines++,0,"%s has bitten and poisoned you!",
						monster.mname);
				++stat->psn;
				break;
			case 14:    /* Faeries */
				if (!stat->hw)
					goto NORMALHIT;
				mvprintw(lines++,0,"Your holy water killed it!");
				--stat->hw;
				goto FINISH;
			case 15:    /* Lamprey */
				if (rnd() > 0.7)
					goto NORMALHIT;
				mvprintw(lines++,0,"%s bit and poisoned you!",
						monster.mname);
				stat->psn += 0.25;
				break;
			case 17:    /* Bonnacon */
				if (rnd() > 0.1)
					goto NORMALHIT;
				mvprintw(lines++,0,"%s farted and scampered off.",
						monster.mname);
				stat->energy /= 2;
				goto LEAVE;
			case 18:    /* Smeagol */
				if (rnd() > 0.5 || !stat->rng.type)
					goto NORMALHIT;
				mvprintw(lines++,0,"%s tried to steal your ring, ",
						monster.mname);
				if (rnd() > 0.1)
					addstr("but was unsuccessful.");
				else
				{
					addstr("and ran away with it!");
					stat->rng.type = NONE;
					goto LEAVE;
				}
				break;
			case 19:    /* Succubus */
				if (rnd() > 0.3)
					goto NORMALHIT;
				inflict = roll(15,size*10);
				inflict = min(inflict,stat->energy);
				mvprintw(lines++,0,"%s sapped %.0f of your energy.",
						monster.mname,inflict);
				stat->energy -= inflict;
				break;
			case 20:    /* Cerberus */
				if (rnd() > 0.25)
					goto NORMALHIT;
				mvprintw(lines++,0,"%s took all your metal treasures!",
						monster.mname);
				stat->swd = stat->shd =stat->gld = stat->crn = 0;
				goto LEAVE;
			case 21:    /* Ungoliant */
				if (rnd() > 0.1)
					goto NORMALHIT;
				mvprintw(lines++,0,"%s poisoned you, and took one quick.",
						monster.mname);
				stat->psn += 5;
				--stat->quk;
				break;
			case 22:    /* Jabberwock */
				if (rnd() > 0.1)
					goto NORMALHIT;
				mvprintw(lines++,0,"%s flew away, and left you to contend with one of its friends.",monster.mname);
				whichm = 55 + 22*(rnd() > 0.5);
				goto CALL;
			case 24:    /* Troll */
				if (rnd() > 0.5)
					goto NORMALHIT;
				mvprintw(lines++,0,"%s partially regenerated his energy.!",
						monster.mname);
				monster.mhit += floor((monhit*size - monster.mhit)/2);
				monster.mstr = monstr;
				mdamage = sdamage = 0;
				maxspd = monspd;
				break;
			case 25:    /* wraith */
				if (rnd() > 0.3 || stat->blind)
					goto NORMALHIT;
				mvprintw(lines++,0,"%s blinded you!",monster.mname);
				stat->blind = TRUE;
				break;
			default:
				goto NORMALHIT;
		}
		else
NORMALHIT:
		{
			inflict = rnd()*monster.mstr + 0.5;
			mvprintw(lines++,0,"%s hit you %.0f times!",monster.mname,inflict);
SPECIALHIT:	if ((shield -= inflict) < 0)
			{
				stat->energy += shield;
				shield = 0;
			}
		}
	}
	else if (fghttofin)
		goto MELEE;
	mvaddstr(7,0,"1:Melee  2:Skirmish  3:Evade  4:Spell  5:Nick  ");
	if (!luckout)
		if (monster.mtyp == 23)
			addstr("6:Ally  ");
		else
			addstr("6:Luckout  ");
	if (stat->rng.type > 0)
		addstr("7:Use Ring  ");
	else
		clrtoeol();
GET: ch = gch(stat->rng.type);
	if (stat->lvl > 5)			/* will re-set after a ^S */
		timeout = TRUE;
	firsthit = FALSE;
	checkhit = TRUE;
	move(8,0);
	clrtobot();
	lines = 9;
	mvaddstr(4,0,"\n\n");
	switch (ch)
	{
		case '\020':			/* ^P -- suspend the game temporarily */
			if (timeout)
				timeout = FALSE;
			goto GET;
			/*NOTREACHED*/
		case 'T':	/* timeout; lose turn */
			break;
		case ' ':
		case '1':	/* melee */
MELEE:	    inflict = roll(strength/2 + 5,1.3*strength) +
				(stat->rng.type < 0 ? strength : 0);
			mdamage += inflict;
			monster.mstr = monstr - mdamage/monhit*monstr/4;
			goto HITMONSTER;
		case '2':	/* skirmish */
			inflict = roll(strength/3 + 3,1.1*strength) +
					(stat->rng.type < 0 ? strength : 0);
			sdamage += inflict;
			maxspd = monspd - sdamage/monhit*monspd/4;
			goto HITMONSTER;
		case '3':	/* evade */
			if ((monster.mtyp == 4 || monster.mtyp == 16
					|| rnd()*speed*stat->brn > rnd()*monster.mspd*monster.mbrn)
					&& (monster.mtyp != 23))
			{
				mvaddstr(lines++,0,"You got away!");
				stat->x += roll(-2,5);
				stat->y += roll(-2,5);
				goto LEAVE;
			}
			else
				mvprintw(lines++,0,"%s is still after you!",monster.mname);
			break;
		case 'M':	/* spell */
		case '4':
			mvaddstr(7,0,"\n\n");
			mvaddstr(7,0,"1:All or Nothing  ");
			if (stat->mag >= 5)
				addstr("2:Magic Bolt  ");
			if (stat->mag >= 15)
				addstr("3:Force Field  ");
			if (stat->mag >= 25)
				addstr("4:Transform  ");
			if(stat->mag >= 35)
				addstr("5:Increase Might\n");
			if (stat->mag >= 45)
				mvaddstr(8,0,"6:Invisibility  ");
			if (stat->mag >= 60)
				addstr("7:Transport  ");
			if (stat->mag >= 75)
				addstr("8:Paralyze  ");
			if (stat->typ > 20)
				addstr("9:Specify");
			mvaddstr(4,0,"Spell? ");
			ch = getans(" ", TRUE);
			mvaddstr(7,0,"\n\n");
			if (monster.mtyp == 23 && ch != '3')
				illspell();
			else
				switch (ch)
				{
					case '1':   /* all or nothing */
						inflict = (rnd() < 0.25) ? (monster.mhit*1.0001 + 1) : 0;
						if (monster.mtyp == 4)
							inflict *= .9;
						if (stat->man)
							--stat->man;
						maxspd *= 2;
						monspd *= 2;
						monster.mspd = max(1,monster.mspd * 2);
						monstr = monster.mstr *= 2;
						goto HITMONSTER;
						break;
					case '2':   /* magic bolt */
						if (stat->mag < 5)
							illspell();
						else
						{
							do
							{
								mvaddstr(4,0,"How much mana for bolt? ");
								getstring(aline,80);
								sscanf(aline,"%F",&temp);
							} while (temp < 0 || temp > stat->man);
							stat->man -= floor(temp);
							inflict = temp*roll(10,sqrt(stat->mag/3.0 + 1.0));
							mvaddstr(5,0,"Magic Bolt fired!\n");
							if (monster.mtyp == 4)
								inflict = 0.0;
							goto HITMONSTER;
						}
						break;
					case '5':   /* increase might */
						if (stat->mag < 45)
							illspell();
						else if (stat->man < 75)
							nomana();
						else
						{
							stat->man -= 75;
							strength +=(1.2*(stat->str+stat->swd)+5-strength)/2;
							mvprintw(5,0,"New strength:  %.0f\n",strength);
						}
						break;
					case '3':   /* force field */
						if (stat->mag < 15)
							illspell();
						else if (stat->man < 30)
							nomana();
						else
						{
							shield = (stat->mxn + stat->shd)*4.2 + 45;
							stat->man -= 30;
							mvaddstr(5,0,"Force Field up.\n");
						}
						break;
					case '4':   /* transform */
						if (stat->mag < 25)
							illspell();
						else if (stat->man < 50)
							nomana();
						else
						{
							stat->man -= 50;
							whichm = roll(0,100);
							goto CALL;
						}
						break;
					case '6':   /* invisible */
						if (stat->mag < 45)
							illspell();
						else if (stat->man < 90)
							nomana();
						else
						{
							stat->man -= 90;
							speed += (1.2*(stat->quk+stat->quks)+5-speed)/2;
							mvprintw(5,0,"New quick :  %.0f\n",speed);
						}
						break;
					case '7':   /* transport */
						if (stat->mag < 60)
							illspell();
						else if (stat->man < 125)
							nomana();
						else
						{
							stat->man -= 125;
							if (stat->brn + stat->mag < monster.mexp/200*rnd())
							{
								mvaddstr(5,0,"Transport backfired!\n");
								stat->x += (250*size*rnd() +
											50*size)*sgn(stat->x);
								stat->y += (250*size*rnd() +
											50*size)*sgn(stat->y);
								goto LEAVE;
							}
							else
							{
								mvprintw(5,0,"%s is transported.\n",
											monster.mname);
								if (rnd() < 0.3)
								monster.mtrs = 0;
								goto FINISH;
							}
						}
						break;
					case '8':   /* paralyze */
						if (stat->mag < 75)
							illspell();
						else if (stat->man < 150)
							nomana();
						else
						{
							stat->man -= 150;
							if (stat->mag > monster.mexp/1000*rnd())
							{
								mvprintw(5,0,"%s is held.\n",monster.mname);
								monster.mspd = -2;
							}
							else
								mvaddstr(5,0,"Monster unaffected.\n");
						}
						break;
					case '9':   /* specify */
						if (stat->typ < 20)
							illspell();
						else if (stat->man < 1000)
							nomana();
						else
						{
							mvaddstr(5,0,"Which monster do you want [0-99]? ");
							whichm = inflt();
							whichm = max(0,min(99,whichm));
							stat->man -= 1000;
							goto CALL;
						}
						break;
				}
				break;
			case '5':
				inflict = 1 + stat->swd;
				stat->exp += floor(monster.mexp/10);
				monster.mexp *= 0.92;
				maxspd += 2;
				monster.mspd = (monster.mspd < 0) ? 0 : monster.mspd + 2;
				if (monster.mtyp == 4)
				{
					mvprintw(lines++,0,"You hit %s %.0f times, and made him mad!",monster.mname,inflict);
					stat->quk /= 2;
					stat->x += sgn(stat->x)*roll(50*size,250*size);
					stat->y += sgn(stat->y)*roll(50*size,250*size);
					stat->y += (250*size*rnd() + 50*size)*sgn(stat->y);
					goto LEAVE;
				}
				else
					goto HITMONSTER;
			case 'B':	/* luckout */
			case '6':
				if (luckout)
					mvaddstr(lines++,0,"You already tried that.");
				else
					if (monster.mtyp == 23)
						if (rnd() < stat->sin/100)
						{
							mvprintw(lines++,0,"%s accepted!",monster.mname);
							goto LEAVE;
						}
						else
						{
							luckout = TRUE;
							mvaddstr(lines++,0,"Nope, he's not interested.");
						}
				else
					if ((rnd() + .333)*stat->brn < (rnd() + .333)*monster.mbrn)
					{
						luckout = TRUE;
						mvprintw(lines++,0,"You blew it, %s.",stat->name);
					}
					else
					{
						mvaddstr(lines++,0,"You made it!");
						goto FINISH;
					}
				break;
			case '\014':    /* clear screen */
				clear();
				break;
			case '7':	/* use ring */
				if (stat->rng.type > 0)
				{
					mvaddstr(lines++,0,"Now using ring.");
					stat->rng.type = -stat->rng.type;
					if (abs(stat->rng.type) != DLREG)
						--stat->rng.duration;
					goto HITMONSTER;
				}
				break;
#ifdef SHELL
			case '!':	/* shell escape */
				shellcmd();
				/* FALLTHROUGH */
#endif
			default:
				checkhit = FALSE; /* only SPACE and digits work for fighting */
				goto TOP;
			}
		goto BOT;
HITMONSTER:	{
		inflict = floor(inflict);
		mvprintw(lines++,0,"You hit %s %.0f times!",monster.mname,inflict);
		if ((monster.mhit -= inflict) >0)
			switch (monster.mtyp)
			{
				case 4: /* dark lord */
					inflict = stat->energy + shield +1;
					goto SPECIALHIT;
				case 16:	/* shrieker */
					mvaddstr(lines++,0,"Shrieeek!!  You scared it, and it called one of its friends.");
					more(lines);
					whichm = roll(70,30);
					goto CALL;
			}
		else
		{
			if (monster.mtyp == 23)	/* morgoth */
				mvaddstr(lines++,0,"You have defeated Morgoth, but he may return...");
			else
			{
				mvprintw(lines++,0,"You killed it.  Good work, %s.",stat->name);
				if (monster.mtyp == 3 && (strcmp(monster.mname,"Mimic") != 0))
					mvaddstr(lines++,0,"The body slowly changes shape -- it must have been a Mimic!");
			}
			goto FINISH;
		}
	}
BOT:	refresh();
	if (lines > LINES - 2)
	{
		more(lines);
		move(lines = 8,0);
		clrtobot();
	}
	if (stat->energy <= 0)
	{
		more(lines);
		death(stat, monster.mname);
		goto LEAVE;
	}
	goto TOP;
FINISH: stat->exp += monster.mexp;
	if (rnd() < monster.mflk/100.0)  /* flock monster */
	{
		more(lines);
		fghttofin = FALSE;
		++howmany;
		goto CALL;
	}
	else if (size > 1 && monster.mtrs && rnd() > 0.2 +
			pow(0.4,(double) (howmany/3.0 + size/3.0)))
	{		   /* this takes # of flocks and size into account */
		more(lines);
		treasure(stat,monster.mtrs,size);
	}
	else
LEAVE:	    more(lines);
	stat->rng.type = abs(stat->rng.type);
	move(4,0);
	clrtobot();
	fghting = FALSE;
	stat->status = PLAYING;
	update(stat,fileloc);
}
