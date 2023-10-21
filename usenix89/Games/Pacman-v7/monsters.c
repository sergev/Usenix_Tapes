/*
** monsters.c -
**  This file contains the necessary functions
**  to deal with the monsters and how they move.
**  If the flag variable is TRUE, then they are after
**  you, else you are after them.
**
**
**	[pm by Peter Costantinidis, Jr. @ University of California at Davis]
*/
#include "pm.h"

/*
** harpo	- will try his best to get you, but is slow
**		- smart, slow
**
** groucho  	- is always behind you, it is hard to shake him
**		- smart, fast
**
** chico	- he's fast
**		- fast
**
** zeppo	- terribly shy and will actually run away from you
**		- medium, dumb
*/

/*
** p_monsters()	- place and initialize the monsters for new screens
*/
void	p_monsters ()
{
	reg	int	i;

	for (i = 0; i < MAX_MONS; i++)
		m_init(&ghosts[i]);
	h->mo_pos.x = 26;
	h->mo_pos.y = 11;
	g->mo_pos.x = 26;
	g->mo_pos.y = 9;
	c->mo_pos.x = 28;
	c->mo_pos.y = 11;
	z->mo_pos.x = 24;
	z->mo_pos.y = 11;
	h->mo_name = HARPO;
	g->mo_name = GROUCHO;
	c->mo_name = CHICO;
	z->mo_name = ZEPPO;
	g->mo_inside = FALSE;
	for (i = 0; i < MAX_MONS; i++)
	{
		move(ghosts[i].mo_pos.y, ghosts[i].mo_pos.x);
		addch(ghosts[i].mo_name);
	}
}

/*
** m_move()	- move the monsters
**		- if they are eatable, then skip them half the time
**		- if they are inside, skip them a third of the time
*/
void	m_move ()
{
	reg	int	i;

	for (i = 0; i < MAX_MONS; i++)
	{
		if (ghosts[i].mo_run && (demon % 3l))
			continue;
		mv_mon(&ghosts[i]);
		if (pm_eaten)
			return;
	}
}

/*
** mv_mon()	- make the given monster move
**		- maybe change the "mod 5"'s to randomness
**		- intelligence is simulated by choosing the best
**		  possible move, and then occasionally doing some-
**		  thing random.  occasionally is defined relative
**		  to the intelligence of the monster!
**		- when a monster is in the tunnel, it continues
**		  moving in its original direction
**		- returns FALSE if pm is eaten (everything must stop!)
*/
void	mv_mon (m)
reg	mons	*m;
{
#ifdef	FASTER
	reg	int	once = FALSE;
#endif

#ifdef	FASTER
top:
#endif
	if (!get_move(m))
	{
		m->mo_ch = moves[rnd(0, 3)];
		m->mo_cnt = 0;
	}
	/*
	** take care of slow monsters
	*/
	if ((!(demon % SPEED)) &&
	    (m->mo_attrib & SLOW) &&
	    !m->mo_run && !m->mo_tunn)
		return;
#ifdef	FASTER
	/*
	** take care of fast monsters
	*/
	if (!(demon % SPEED) &&
	    (m->mo_attrib & FAST) &&
	    !once && !m->mo_run && !m->mo_tunn)
		once = TRUE;
	else if (once)
		once = FALSE;
#endif
	/*
	** take care of dumb monsters
	** -	25% of time they move randomly, and if they are inside,
	**	then 50% of the time they move wrong
	*/
	if ((m->mo_attrib & DUMB) && !m->mo_tunn)
	{
		if (rnd(1,100) < 25)
		{
			if ((m->mo_ch = gen_mv(m)) != MSTOP)
				m->mo_cnt = DIST();
			else
				m->mo_cnt = 0;
		}
		else if (m->mo_inside && (rnd(1, 100) < 45))
		{
			m->mo_ch = opposite(m->mo_ch);
			m->mo_cnt = 2;
		}
	}
	/*
	** take care of medium monsters
	*/
	else if ((m->mo_attrib & NORMAL) && !m->mo_tunn)
	{
		if (rnd(1,100) < 10)
		{
			if ((m->mo_ch = gen_mv(m)) != MSTOP)
				m->mo_cnt = DIST();
			else
				m->mo_cnt = 0;
		}
		else if (m->mo_inside && (rnd(1, 100) < 37))
		{
			m->mo_ch = opposite(m->mo_ch);
			m->mo_cnt = 2;
		}
		else if ((rnd(1, 100) < 15) && (m->mo_ch != MSTOP))
			m->mo_cnt = rnd(2, 4);
	}
	else if (m->mo_attrib & SMART)
	{
		if (m->mo_inside && (rnd(1, 100) < 27))
		{
			m->mo_ch = opposite(m->mo_ch);
			m->mo_cnt = 2;
		}
	}
	if (!_mv_mon(m))
		return;
#ifdef	FASTER
	if (once)
		goto top;
#endif
}

/*
** p_info()	- prints information about the given monster
*/
void	p_info (name)
reg	char	name;
{
	reg	mons	*m;

	m = wh_mons(name);
	if (m == NULL)
		msg("No such monster: %s", punctrl(name));
	_puts(CL);
	fprintf(stderr, "Name:          %c\n", m->mo_name);
	fprintf(stderr, "Place:         (%d,%d)\n", m->mo_pos.x, m->mo_pos.y);
	fprintf(stderr, "Inch:          %c\n", m->mo_inch);
	fprintf(stderr, "Run:           %s\n", TF(m->mo_run));
	fprintf(stderr, "Tunn:          %s\n", TF(m->mo_tunn));
	fprintf(stderr, "Eaten:         %s\n", TF(m->mo_eaten));
	fprintf(stderr, "Inside:        %s\n", TF(m->mo_inside));
	fprintf(stderr, "Move:          %c\n", m->mo_ch);
	fprintf(stderr, "Count:         %d\n", m->mo_cnt);
	fprintf(stderr, "Attrib:        %o\n\n", m->mo_attrib);
	trash(getchar());
	redraw();
}

/*
** _mv_mon()	- lower level routine to move a monster
**		- checks for barriers and stuff...
**		- returns FALSE if pm is eaten (everything must stop!)
**		- if in tunnel, it will only move half as fast
*/
_mv_mon (m)
reg	mons	*m;
{
	reg	char	what;
	auto	coord	pos;
Above:
	if (m->mo_tunn)
	{
		if (!(demon % 4))
		{
			pos.x = m->mo_pos.x;
			pos.y = m->mo_pos.y;
			switch (m->mo_ch)
			{
				when MLEFT:
					if (--pos.x < LEFT)
						pos.x = RIGHT;
				when MRIGHT:
					if (++pos.x > RIGHT)
						pos.x = LEFT;
				otherwise:
					msg("Bad move in tunn");
			}
		}
		else	/* not moving this time around	*/
			return(TRUE);
		switch (what = tunn_look(&pos))
		{
			case PM:
				m->mo_pos.x = pos.x;
				m->mo_pos.y = pos.y;
				if (OUTOFTUNN(&m->mo_pos))
					m->mo_tunn = FALSE;
				if (m->mo_run)
				{
					pm_eat_m(m->mo_name);
					return(FALSE);
				}
				m_eat_pm(m);
				return(FALSE);
			case EMPTY:
				if (!OUTOFTUNN(&pos))
					msg("Lost out of tunn");
				m->mo_tunn = FALSE;
				goto ok;
			case TUNNEL:
				m->mo_pos.x = pos.x;
				m->mo_pos.y = pos.y;
				m->mo_inch = what;
				return(TRUE);
			default:
				/*
				** ran into another monster?
				*/
				m->mo_ch = opposite(m->mo_ch);
				msg("Ran into another monster");
				return(TRUE);
		}
	}
	else
	{
		if (move_to(m->mo_ch, &m->mo_pos, &pos) == -1)
			return(-1);
		move(pos.y, pos.x);
		what = INCH();
	}
	switch (what)
	{
		when PM:
			mvaddch(m->mo_pos.y, m->mo_pos.x, m->mo_inch);
			draw();
			m->mo_pos.x = pos.x;
			m->mo_pos.y = pos.y;
			if (m->mo_run)
				return(pm_eat_m(m->mo_name), FALSE);
			mvaddch(pos.y, pos.x, m->mo_name);
			draw();
			m->mo_inch = EMPTY;
			m_eat_pm(m);
			return(FALSE);
		when BLOCK:
			m->mo_cnt = 0;
		when DOT:
		case ENERGY:
		case EMPTY:
ok:
			mvaddch(m->mo_pos.y, m->mo_pos.x, m->mo_inch);
#ifdef	MAX_UPDATE
			draw();
#endif
			mvaddch(pos.y, pos.x, m->mo_name);
			m->mo_pos.x = pos.x;
			m->mo_pos.y = pos.y;
			m->mo_inch = what;
		when DOOR:
			if (!m->mo_inside)	/*  can only exit, not enter */
			{
				m->mo_cnt = 0;
				break;
			}
			mvaddch(m->mo_pos.y, m->mo_pos.x, m->mo_inch);
#ifdef	MAX_UPDATE
			draw();
#endif
			mvaddch(pos.y, pos.x, m->mo_name);
			m->mo_pos.x = pos.x;
			m->mo_pos.y = pos.y;
			m->mo_inch = what;
			m->mo_inside = FALSE;
		when TUNNEL:
			if (tunn_look(&pos) != TUNNEL)
				break;
			mvaddch(m->mo_pos.y, m->mo_pos.x, m->mo_inch);
			m->mo_inch = TUNNEL;
			m->mo_tunn = TRUE;
			m->mo_pos.x = pos.x;
			m->mo_pos.y = pos.y;
			goto Above;
		otherwise:
			if (IS_FRUIT(what))
				goto ok;
			if (!is_mons(what))
			{
				msg("_mv_mon(): default");
				break;
			}
			m->mo_cnt = 0;
	}
	draw();
	return(TRUE);
}

/*
** get_move()	- find a smart move for the given monster
**		- return FALSE if none was determined
*/
int	get_move (m)
reg	mons	*m;
{
	reg	int	i, dist = 0;
	auto	char	tmp;

	if (m->mo_cnt > 0)
		m->mo_cnt--;
	/*
	** must always move up when on the door!!!
	*/
	if (m->mo_inch == DOOR)
	{
		m->mo_ch = MUP;
		m->mo_cnt = 0;
		return(TRUE);
	}
	if (m->mo_tunn)
	{	/* if in tunnel, keep moving	*/
#ifdef	DEBUG
		if (!m->mo_ch)
			msg("m's in tunn, & mo_ch is null");
#endif
		return(TRUE);
	}
	if (m->mo_inside)
	{
		/*
		** still inside thier little cave (box)
		** make them move to space under the door if they
		** are not there already
		*/
		if (m->mo_pos.x > DOOR_COL)
		{
			m->mo_ch = MLEFT;
			m->mo_cnt = 1;
			return(TRUE);
		}
		if (m->mo_pos.x < DOOR_COL)
		{
			m->mo_ch = MRIGHT;
			m->mo_cnt = 1;
			return(TRUE);
		}
		/*
		** they're under the door, try to move up!
		*/
		m->mo_ch = MUP;
		m->mo_cnt = 1;
		return(TRUE);
	}
	tmp = m->mo_ch;
	for (i = 0; i < 4; i++)
	{
		m->mo_ch = int_dir(i);
		if (dist = can_see(m, &pm_pos))
			break;
	}
	if (m->mo_run && dist)	/* running away from him		*/
	{
		m->mo_cnt = 0;
		if (m_is_safe(m, opposite(m->mo_ch)))
			return(m->mo_ch = opposite(m->mo_ch), TRUE);
		m->mo_ch = tmp;
		if ((m->mo_ch = gen_mv(m)) != MSTOP)
			return(TRUE);
		m->mo_ch = (rnd(0, 1) ? lturn(m->mo_ch) : rturn(m->mo_ch));
		return(FALSE);
	}
	if (dist)
		return(m->mo_cnt = dist, TRUE);
	m->mo_ch = tmp;
	if (m->mo_cnt)	/* check for predetermination		*/
		return(TRUE);
	m->mo_cnt = 0;
	if ((m->mo_ch = gen_mv(m)) != MSTOP)
		return(TRUE);
	m->mo_ch = tmp;
	if (m_is_safe(m, m->mo_ch))
		return(TRUE);
	return(FALSE);
}

/*
** gen_mv()	- generate a move when no hueristics are available
**		- turn 80% of the time, 50% of that left, 50% right
**		- 20% of the time just keep going
*/
char	gen_mv (m)
reg	mons	*m;
{
	if (rnd(0, 99)<40)
	{
		if (m_is_safe(m, rturn(m->mo_ch)))
			return(rturn(m->mo_ch));
		if (m_is_safe(m, lturn(m->mo_ch)))
			return(lturn(m->mo_ch));
		return(MSTOP);
	}
	if (rnd(0, 59) < 40)
	{
		if (m_is_safe(m, lturn(m->mo_ch)))
			return(lturn(m->mo_ch));
		if (m_is_safe(m, rturn(m->mo_ch)))
			return(rturn(m->mo_ch));
		return(MSTOP);
	}
	if (m_is_safe(m, m->mo_ch))
		return(m->mo_ch);
	if (rnd(0, 1))
	{
		if (m_is_safe(m, rturn(m->mo_ch)))
			return(rturn(m->mo_ch));
		if (m_is_safe(m, lturn(m->mo_ch)))
			return(lturn(m->mo_ch));
		return(MSTOP);
	}
	if (m_is_safe(m, lturn(m->mo_ch)))
		return(lturn(m->mo_ch));
	if (m_is_safe(m, rturn(m->mo_ch)))
		return(rturn(m->mo_ch));
	return(MSTOP);
}

/*
** m_is_safe()	- return TRUE if monster can move in the indicated
**		  direction
*/
int	m_is_safe (m, dir)
reg	mons	*m;
reg	char	dir;
{
	reg	char	what;
	auto	coord	pos;

	if (move_to(dir, &m->mo_pos, &pos))
		return(FALSE);
	move(pos.y, pos.x);
	switch (what = INCH())
	{
		case HARPO:
		case _HARPO:
		case GROUCHO:
		case _GROUCHO:
		case CHICO:
		case _CHICO:
		case ZEPPO:
		case _ZEPPO:
		case BLOCK:
		case DOOR:
			return(FALSE);
		case DOT:
		case ENERGY:
		case EMPTY:
		case TUNNEL:
			return(TRUE);
		case PM:
			if (m->mo_run)
				return(FALSE);
			return(TRUE);
		default:
			if (IS_FRUIT(what))
				return(TRUE);
			msg("m_is_safe default");
			return(FALSE);
	}
}

/*
** can_see()	- returns distance the monster is from the pm if
**		  the pm can be seen
**		- look in the direction the monster is facing
**		- special case for when pm is in tunnel, then the
**		  monster must be facing the tunnel
*/
int	can_see (m, pos)
reg	mons	*m;
reg	coord	*pos;
{
	auto	int	dist = 0;
	auto	coord	mv;

	switch (m->mo_ch)
	{
		when MUP:
			if (m->mo_pos.x != pos->x)	/* not in line	*/
				return(FALSE);
			if ((m->mo_pos.y == TUNN_ROW) && pm_tunn)
				break;
			if (m->mo_pos.y < pos->y)	/* not facing	*/
				return(FALSE);
		when MDOWN:
			if (m->mo_pos.x != pos->x)	/* not in line	*/
				return(FALSE);
			if ((m->mo_pos.y == TUNN_ROW) && pm_tunn)
				break;
			if (m->mo_pos.y > pos->y)	/* not facing	*/
				return(FALSE);
		when MLEFT:
			if (m->mo_pos.y != pos->y)	/* not in line	*/
				return(FALSE);
			if ((m->mo_pos.y == TUNN_ROW) && pm_tunn)
				break;
			if (m->mo_pos.x < pos->x)	/* not facing	*/
				return(FALSE);
		when MRIGHT:
			if (m->mo_pos.y != pos->y)	/* not in line	*/
				return(FALSE);
			if ((m->mo_pos.y == TUNN_ROW) && pm_tunn)
				break;
			if (m->mo_pos.x > pos->x)	/* not facing	*/
				return(FALSE);
		otherwise:
			msg("default in can_see: \%03o", m->mo_ch);
			return(FALSE);
	}
	/*
	** at this point, they are in direct line and
	** facing in the right direction, we need to
	** see if anything is in the way
	*/
	mv.x = m->mo_pos.x;
	mv.y = m->mo_pos.y;
	do
	{
		reg	char	what;
			
		if (move_to(m->mo_ch, &mv, &mv) == -1)
			return(msg("move_to=-1"), FALSE);
		move(mv.y, mv.x);
		switch (what = INCH())
		{
			case PM:
			case HARPO:
			case _HARPO:
			case GROUCHO:
			case _GROUCHO:
			case CHICO:
			case _CHICO:
			case ZEPPO:
			case _ZEPPO:
			case DOT:
			case ENERGY:
			case DOOR:
			case EMPTY:
				break;
			case TUNNEL:
				if (pm_tunn)
					return(dist);
				else
					msg("can_see(): what???");
			case BLOCK:
				return(0);
			default:
				if (IS_FRUIT(what))	/* its the fruit */
					break;
				msg("can_see(): case (2)");
				return(0);
		}
		dist++;
	} while ((mv.x != pos->x) || (mv.y != pos->y));
	return(dist);
}

/*
** move_to()	-
*/
int	move_to (dir, pos1, pos2)
reg	int	dir;			/* declared int to pacify lint	*/
reg	coord	*pos1, *pos2;
{
	static	int	offset[2][4] =
	{	{0, 0, -1, 1},
		{-1, 1, 0, 0}
	};

	if ((dir = dir_int((char) dir)) == -1)
		return(-1);
	pos2->x = pos1->x + offset[0][dir];
	pos2->y = pos1->y + offset[1][dir];
	return(0);
}

/*
** wh_mons()	- return pointer to ch's monster struct
*/
mons	*wh_mons (mch)
reg	char	mch;
{
	switch (mch)
	{
		case HARPO:
		case _HARPO:
			return(h);
		case GROUCHO:
		case _GROUCHO:
			return(g);
		case CHICO:
		case _CHICO:
			return(c);
		case ZEPPO:
		case _ZEPPO:
			return(z);
		default:
			msg("Unknown monster!!!");
			return((mons *) NULL);
	}
}

/*
** m_eat_pm()	- a monster has eaten the pm
*/
void	m_eat_pm (m)
reg	mons	*m;
{
	eat_pm();
	sleep(1);
	if (!--pms_left)	/* if no more pm's then quit immediately */
		die(m->mo_name);
	flush();
	old_screen();
	pm_eaten = FALSE;
	ch = ' ';
	oldch = '\0';
	newch = '\0';
}

/*
** eat_pm()	- make the pm look eaten
*/
void	eat_pm ()
{
	reg	int	i;

	if (pm_tunn)
		return;
	for (i = 0; i < 5; i++)
	{
		mvaddch(pm_pos.y, pm_pos.x, 'O');
		draw();
		msleep(EAT_PAUSE);
		mvaddch(pm_pos.y, pm_pos.x, '=');
		draw();
		msleep(EAT_PAUSE);
		mvaddch(pm_pos.y, pm_pos.x, 'O');
		draw();
		msleep(EAT_PAUSE);
		mvaddch(pm_pos.y, pm_pos.x, '=');
		draw();
		msleep(EAT_PAUSE);
	}
	mvaddch(pm_pos.y, pm_pos.x, EMPTY);
	draw();
	sleep(2);
}

/*
** is_mons()	- return TRUE if ch is a monster
*/
int	is_mons (whoru)
reg	char	whoru;
{
	switch (whoru)
	{
		case HARPO:
		case _HARPO:
		case GROUCHO:
		case _GROUCHO:
		case CHICO:
		case _CHICO:
		case ZEPPO:
		case _ZEPPO:
			return(TRUE);
	}
	return(FALSE);
}

/*
** place_m()	- put a monster back in its box
**		- an infinite loop can occur if the box is full!
**		- the box should never be full!!!
*/
void	place_m (m)
reg	mons	*m;
{
	reg	int	xx;

	m->mo_pos.y = 11;
	while (TRUE)
	{
		move(m->mo_pos.y, (xx = rnd(19, 33)));
		if (INCH() == EMPTY)
			break;
	}
	m->mo_pos.x = xx;
	move(m->mo_pos.y, m->mo_pos.x);
	addch(m->mo_name);
}
