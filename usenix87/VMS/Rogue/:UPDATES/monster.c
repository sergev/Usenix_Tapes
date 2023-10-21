#include curses.h
#include ctype.h
#include "monster.h"
#include "object.h"
#include "room.h"
#include "move.h"
#define TROLLMAX 64

extern short AMULET_LEVEL;
extern short current_level;
extern short current_room, party_room;
extern short detect_monster;
extern short blind, halluc;

char *monster_names[] = {
	"ant lion",
	"aquatar",
	"bat",
	"blood hawk",
	"centaur",
	"cloud giant",
	"baby dragon",
	"dragon",
	"emu",
	"ettin",
	"venus fly-trap",
	"frost giant",
	"griffin",
	"ghost",
	"grimlock",
	"hill giant",
	"hobgoblin",
	"ice monster",
	"imp",
	"jabberwock",
	"jackal",
	"kestrel",
	"kobold",
	"leopard",
	"leprechaun",
	"meazel",
	"medusa",
	"nightmare",
	"nymph",
	"orc",
	"ogre",
	"phantom",
	"pheonix",
	"quaggoth",
	"quasit",
	"ram",
	"rattlesnake",
	"snake",
	"spider",
	"storm giant",
	"titan",
	"troll",
	"umber hulk",
	"black unicorn",
	"vampire",
	"vilstrak",
	"wolf",
	"wraith",
	"xeroc",
	"xvart",
	"yeth hound",
	"yeti",
	"zombie"
};

object monster_tab[MONSTERS] = {
/* antlion*/	{(WANDERS),"5d4",64,'a',60,11,22,75,0,45},
/* aquatar*/	{(IS_ASLEEP|WAKENS|WANDERS),"1d5",25,'A',20,AQUATAR1,AQUATAR2,100,0,0},
/* bat    */	{(IS_ASLEEP|WANDERS|FLITS),"1d3",10,'B',2,1,8,60,0,0},
/* b. hawk*/	{(FLIES),"3d4/1d6",25,'b',15,6,17,85,0,25},
/* centaur*/	{(IS_ASLEEP|WANDERS),"3d4/2d6",30,'C',30,7,16,85,0,10},
/* c giant*/	{(IS_ASLEEP|WAKENS|WANDERS),"4d6/3d10",120,'c',900,15,126,85,0,80},
/* b.dragn*/	{(IS_ASLEEP|WAKENS|FLIES),"3d5/2d9",90,'d',2000,19,126,90,0,60},
/* dragon */	{(IS_ASLEEP|WAKENS),"4d5/3d9",128,'D',5000,21,126,100,0,90},
/* emu    */	{(IS_ASLEEP|WAKENS),"1d3",11,'E',2,1,7,65,0,0},
/* ettin  */	{(WANDERS),"2d8/3d6",80,'e',500,14,126,75,0,60},
/* fly trp*/	{(0),"1d3",32,'F',31,FLYTRAP1,FLYTRAP2,80,0,0},
/* f giant*/	{(IS_ASLEEP|WAKENS),"4d6/2d10",100,'f',700,14,126,85,0,60},
/* griffin*/	{(IS_ASLEEP|WAKENS|WANDERS|FLIES),"5d4/4d5",92,'G',2000,20,126,85,0,10},
/* ghost  */	{(0),"1d10",70,'.',75,12,126,85,0,50},
/* grimlck*/	{(WANDERS|FLIES),"3d10/6d3",105,'g',2000,15,126,100,0,70},
/* h giant*/	{(IS_ASLEEP|WANDERS),"2d8/2d8",80,'h',500,12,126,75,0,60},
/* hgoblin*/	{(IS_ASLEEP|WAKENS|WANDERS),"1d3/1d3",17,'H',3,1,10,67,0,0},
/* icemstr*/	{(IS_ASLEEP),"0d0",15,'I',5,ICEMSTR1,ICEMSTR2,68,0,0},
/* imp    */	{(WANDERS),"1d4",28,'i',3,1,7,50,0,0},
/* jabrwck*/	{(IS_ASLEEP|WANDERS),"3d10/3d4",125,'J',3000,21,126,100,0,0},
/* jackal */	{(WANDERS),"1d2",4,'j',2,1,4,50,0,0},
/* kestrel*/	{(IS_ASLEEP|WAKENS|WANDERS|FLIES),"1d4",10,'K',2,1,6,60,0,0},
/* kobold */ 	{(IS_ASLEEP|WAKENS|WANDERS),"2d4",8,'k',2,1,4,50,0,20},
/* leopard*/	{(WANDERS),"4d3/2d4",26,'l',20,8,17,75,0,20},
/* lepchan*/	{(IS_ASLEEP),"0d0",25,'L',18,LEPCHAN1,LEPCHAN2,75,0,0},
/* meazel */	{(IS_ASLEEP|WAKENS|WANDERS),"3d6",50,'m',50,7,20,75,0,10},
/* medusa */	{(IS_ASLEEP|WAKENS|WANDERS),"4d4/3d7",92,'M',250,18,126,85,0,25},
/* n. mare*/	{(WANDERS|FLIES),"2d4/3d4",54,'n',50,10,22,80,0,0},
/* nymph  */	{(IS_ASLEEP),"0d0",25,'N',37,NYMPH1,NYMPH2,75,0,100},
/* orc    */	{(IS_ASLEEP|WANDERS|WAKENS),"1d6",25,'O',10,4,13,70,0,10},
/* ogre   */	{(IS_ASLEEP|WANDERS|WAKENS),"2d5",41,'o',25,7,16,85,0,35},
/* phantom*/	{(IS_ASLEEP|IS_INVIS|WANDERS|FLITS),"5d4",76,'P',120,15,23,80,0,50},
/* pheonix*/	{(FLIES),"2d12/3d8",160,'p',2500,26,126,100,0,95},
/* quagoth*/	{(WANDERS),"2d10/3d4",100,'q',1000,14,126,100,0,60},
/* quasit */	{(IS_ASLEEP|WAKENS|WANDERS),"3d5",30,'Q',20,8,17,78,0,20},
/* ram	  */	{(IS_ASLEEP|WAKENS),"2d6",25,'r',10,4,12,65,0,0},
/* r snake*/	{(IS_ASLEEP|WAKENS|WANDERS),"2d5",19,'R',10,RSNAKE1,RSNAKE2,70,0,0},
/* snake  */	{(IS_ASLEEP|WAKENS|WANDERS),"1d3",8,'S',2,1,9,50,0,0},
/* spider */	{(IS_ASLEEP|WAKENS|FLITS),"1d2",9,'s',3,SPIDER1,SPIDER2,75,0,0},
/* s giant*/	{(WANDERS),"6d6/2d12",140,'s',1000,20,126,95,0,70},
/* titan  */	{(IS_ASLEEP|WAKENS|WANDERS),"6d10",150,'t',2500,24,126,97,0,80},
/* troll  */	{(IS_ASLEEP|WAKENS|WANDERS),"4d6",TROLLMAX,'T',300,13,126,75,0,33},
/* u hulk */	{(IS_ASLEEP|WAKENS),"3d4/2d5",64,'u',60,12,30,60,0,70},
/* unicorn*/	{(IS_ASLEEP|WAKENS|WANDERS),"4d9",88,'U',200,17,26,85,0,33},
/* vampire*/	{(IS_ASLEEP|WAKENS|WANDERS),"1d14",60,'V',350,VAMPIRE1,VAMPIRE2,85,0,18},
/* vilstrk*/	{(WANDERS),"2d5",15,'v',4,1,9,65,0,20},
/* wolf   */    {(IS_ASLEEP|WAKENS|WANDERS|FLIES),"1d4/2d3",16,'w',5,1,11,80,0,0},
/* wraith */	{(IS_ASLEEP|WANDERS),"2d7",42,'W',75,WRAITH1,WRAITH2,75,0,0},
/* xeroc  */	{(IS_ASLEEP),"4d6",42,'X',110,XEROC1,XEROC2,75,0,0},
/* xvart  */	{(IS_ASLEEP|WANDERS|FLITS),"1d8",20,'x',3,1,11,69,0,10},
/* yeth h.*/	{(WANDERS|FLIES),"2d8",27,'y',40,9,18,80,0,10},
/* yeti   */	{(IS_ASLEEP|WANDERS),"3d6",33,'Y',50,11,20,80,0,20},
/* zombie */	{(IS_ASLEEP|WAKENS|WANDERS),"1d7",20,'Z',8,5,14,69,0,0}
};

object level_monsters;

identify_monst(monst)
char monst;{
int i;

for(i=0;i!=MONSTERS;i++)
	if(monster_tab[i].ichar==monst

	 && 
	current_level >= monster_tab[i].is_protected && 
	current_level <= monster_tab[i].is_cursed ){ 

		message(monster_names[i],1);
		return;
	}	
}

put_monsters()
{
	short i;
	short n;
	object *monster, *get_rand_monster();

	n = get_rand(3, 7);

	for (i = 0; i < n; i++) {
		monster = get_rand_monster();
		if ((monster->m_flags & WANDERS) && rand_percent(50)) {
			wake_up(monster);
		}
		put_monster_rand_location(monster);
		add_to_pack(monster, &level_monsters, 0);
	}
}

object *get_rand_monster()
{
	object *monster, *get_an_object();
	short mn;

	monster = get_an_object();

	for (;;) {
		mn = get_rand(0, MAXMONSTER-1);
		if ((current_level >= monster_tab[mn].is_protected) &&
		(current_level <= monster_tab[mn].is_cursed)) {
			break;
		}
	}
	*monster = monster_tab[mn];
	monster->what_is = MONSTER;
	if (monster->ichar == 'X') {
		monster->identified = get_rand_obj_char();
	}
	if (monster->ichar == 'D'|| monster->ichar=='d') {
		monster->to_hit_enchantment= rand()%6;
	}
	if (current_level > (AMULET_LEVEL + 2)) {
		monster->m_flags |= HASTED;
	}
	monster->trow = -1;
	return(monster);
}

move_monsters()
{
	object *monster;
	short flew;

	monster = level_monsters.next_object;

	while (monster) {
		if (monster->m_flags & HASTED) {
			mv_monster(monster, rogue.row, rogue.col);
		} else if (monster->m_flags & SLOWED) {
			monster->quiver = !monster->quiver;
			if (monster->quiver) {
				goto NM;
			}
		}
		flew = 0;
		if ((monster->m_flags & FLIES) &&
		     !monster_can_go(monster, rogue.row, rogue.col)) {
			flew = 1;
			mv_monster(monster, rogue.row, rogue.col);
		}
		if (!flew || !monster_can_go(monster, rogue.row, rogue.col)) {
			mv_monster(monster, rogue.row, rogue.col);
		}
NM:		monster = monster->next_object;
	}
}

fill_room_with_monsters(rn, n)
{
	short i;
	short row, col;
	object *monster, *get_rand_monster();

	n += n/2;

	for (i = 0; i < n; i++) {
		if (no_room_for_monster(rn)) break;
		do {
			row = get_rand(rooms[rn].top_row+1,
				       rooms[rn].bottom_row-1);
			col = get_rand(rooms[rn].left_col+1,
				       rooms[rn].right_col-1);
		} while (screen[row][col] & MONSTER);

		monster = get_rand_monster();
		put_monster_at(row, col, monster);
	}
}

get_monster_char_row_col(row, col)
{
	object *monster, *object_at();
	short retval;

	monster = object_at(&level_monsters, row, col);
	if ((!detect_monster && (monster->m_flags & IS_INVIS)) || blind) {
		retval = get_room_char((screen[row][col] & ~MONSTER),row,col);
		return(retval);
	}
	if ((monster->ichar == 'X') && monster->identified) {
		return(monster->identified);
	}
	return(monster->ichar);
}

get_monster_char(monster)
object *monster;
{
	short retval;

	if ((!detect_monster && (monster->m_flags & IS_INVIS)) || blind) {
		retval = get_room_char((screen[monster->row][monster->col] & ~MONSTER), monster->row, monster->col);
		return(retval);
	}
	if ((monster->ichar == 'X') && monster->identified) {
		return(monster->identified);
	}
	return(monster->ichar);
}

mv_monster(monster, row, col)
register object *monster;
short row, col;
{
	short i, n;
	char tried[6];

	if (monster->m_flags & IS_ASLEEP) {
		if ((monster->m_flags & WAKENS) &&
		     rogue_is_around(monster->row, monster->col) &&
		     rand_percent(WAKE_PERCENT)) {
			wake_up(monster);
		}
		return;
	}
	if ((monster->m_flags & FLITS) && flit(monster)) {
		return;
	}
	if (((current_level >= FLYTRAP1) && (current_level <= FLYTRAP2) &&
	     (monster->ichar == 'F')) && 
	     !monster_can_go(monster, rogue.row, rogue.col)) {
		return;
	}
	if (((current_level >= ICEMSTR1) && (current_level <= ICEMSTR2) &&
	     (monster->ichar == 'I')) && monster->identified) {
		return;
	}
	if ((monster->ichar == 'M') && paralyze(monster)) {
		return;
	}
	if (monster->ichar =='T') {
		monster->quantity+= 10;
		if (monster->quantity > TROLLMAX) 
			monster->quantity = TROLLMAX;
	}
	if ((monster->ichar == 'u') && m_confuse(monster)) {
		return;
	}
	if (monster_can_go(monster, rogue.row, rogue.col)) {
		monster_hit(monster, 0);
		return;
	}
	if (((monster->ichar == 'd') || (monster->ichar == 'D')) 
			     && breath(monster,monster->to_hit_enchantment)) {
		return;
	}
	if ((monster->ichar == 'O') && orc_gold(monster)) {
		return;
	}
	if ((monster->trow == monster->row) &&
		   (monster->tcol == monster->col)) {
		monster->trow = -1;
	} else if (monster->trow != -1) {
		row = monster->trow;
		col = monster->tcol;
	}
	if (monster->row > row) {
		row = monster->row - 1;
	} else if (monster->row < row) {
		row = monster->row + 1;
	}
	if ((screen[row][monster->col] & DOOR) &&
	     mtry(monster, row, monster->col)) {
		return;
	}
	if (monster->col > col) {
		col = monster->col - 1;
	} else if (monster->col < col) {
		col = monster->col + 1;
	}
	if ((screen[monster->row][col] & DOOR) &&
	     mtry(monster, monster->row, col)) {
		return;
	}
	if (mtry(monster, row, col)) {
		return;
	}
	for (i = 0; i <= 5; i++) tried[i] = 0;

	for (i = 0; i < 6; i++) {
NEXT_TRY:	n = get_rand(0, 5);
		switch(n) {
		case 0:
			if (!tried[n] && mtry(monster, row, monster->col-1)) {
				return;
			}
			break;
		case 1:
			if (!tried[n] && mtry(monster, row, monster->col)) {
				return;
			}
			break;
		case 2:
			if (!tried[n] && mtry(monster, row, monster->col+1)) {
				return;
			}
			break;
		case 3:
			if (!tried[n] && mtry(monster, monster->row-1, col)) {
				return;
			}
			break;
		case 4:
			if (!tried[n] && mtry(monster, monster->row, col)) {
				return;
			}
			break;
		case 5:
			if (!tried[n] && mtry(monster, monster->row+1, col)) {
				return;
			}
			break;
		}
		if (!tried[n]) {
			tried[n] = 1;
		} else {
			goto NEXT_TRY;
		}
	}
}

mtry(monster, row, col)
register object *monster;
register short row, col;
{
	if (monster_can_go(monster, row, col)) {
		move_monster_to(monster, row, col);
		return(1);
	}
	return(0);
}

move_monster_to(monster, row, col)
register object *monster;
register short row, col;
{
	short c;

	add_mask(row, col, MONSTER);
	remove_mask(monster->row, monster->col, MONSTER);

	c = mvinch(monster->row, monster->col);

	if (isupper(c)||islower(c)) {
		mvaddch(monster->row, monster->col,
		get_room_char(screen[monster->row][monster->col],
		monster->row, monster->col));
	}
	if (!blind && (detect_monster || can_see(row, col))) {
		if ((!(monster->m_flags & IS_INVIS) || detect_monster)) {
			mvaddch(row, col, get_monster_char(monster));
		}
	}
	if ((screen[row][col] & DOOR) &&
	    (get_room_number(row, col) != current_room) &&
	    (screen[monster->row][monster->col] == FLOOR)) {
		if (!blind)
			mvaddch(monster->row, monster->col, ' ');
	}
	if (screen[row][col] & DOOR) {
			door_course(monster,
			(screen[monster->row][monster->col] & TUNNEL),
			row, col);
	} else {
		monster->row = row;
		monster->col = col;
	}
}

monster_can_go(monster, row, col)
register object *monster;
register short row, col;
{
	object *obj, *object_at();
	short dr, dc;

	dr = monster->row - row;	/* check if move distance > 1 */
	if ((dr >= 2) || (dr <= -2)) return(0);
	dc = monster->col - col;
	if ((dc >= 2) || (dc <= -2)) return(0);

	if ((!screen[monster->row][col]) || (!screen[row][monster->col])) {
		return(0);
	}

	if ((!is_passable(row, col)) || (screen[row][col] & MONSTER)) {
		return(0);
	}
	if ((monster->row!=row)&&(monster->col!=col)&&((screen[row][col]&DOOR)
	|| (screen[monster->row][monster->col]&DOOR))) {
		return(0);
	}
	if (!(monster->m_flags & FLITS) && !(monster->m_flags & CAN_GO) &&
	     (monster->trow == -1)) {
	if ((monster->row < rogue.row) && (row < monster->row)) return(0);
	if ((monster->row > rogue.row) && (row > monster->row)) return(0);
	if ((monster->col < rogue.col) && (col < monster->col)) return(0);
	if ((monster->col > rogue.col) && (col > monster->col)) return(0);
	}

	if (screen[row][col] & SCROLL) {
		obj = object_at(&level_objects, row, col);
		if (obj->which_kind == SCARE_MONSTER) {
			return(0);
		}
	}

	return(1);
}

wake_up(monster)
object *monster;
{
	monster->m_flags &= (~IS_ASLEEP);
}

wake_room(rn, entering, row, col)
{
	object *monster;
	short wake_percent;

	wake_percent = (rn == party_room) ? PARTY_WAKE_PERCENT : WAKE_PERCENT;

	monster = level_monsters.next_object;

	while (monster) {
		if (((monster->m_flags & WAKENS) || (rn == party_room)) &&
		   (rn == get_room_number(monster->row, monster->col))) {
			if ((monster->ichar != 'X') && (rn == party_room)) {
				monster->m_flags |= WAKENS;
			}
			if (entering) {
				monster->trow = -1;
			} else {
				monster->trow = row;
				monster->tcol = col;
			}
			if (rand_percent(wake_percent) &&
			   (monster->m_flags & WAKENS)) {
				if (monster->ichar != 'X') {
					wake_up(monster);
				}
			}
		}
		monster = monster->next_object;
	}
}

char *monster_name(monster)
object *monster;
{
	int i;

	if (blind || ((monster->m_flags & IS_INVIS) && !detect_monster)) {
		return("something");
	}
	if (halluc) {
		i=rand()%MONSTERS;
		return(monster_names[i]);
	}
for(i=0;i!=MONSTERS;i++)
	if(monster_tab[i].ichar==monster->ichar

	 && 
	current_level >= monster_tab[i].is_protected && 
	current_level <= monster_tab[i].is_cursed )
			return(monster_names[i]);

/*	for(i=0;i!=MONSTERS;i++)
		if(monster_tab[i].ichar==monster->ichar)
			return(monster_names[i]);
*/
}

rogue_is_around(row, col)
{
	short rdif, cdif, retval;

	rdif = row - rogue.row;
	cdif = col - rogue.col;

	retval = (rdif >= -1) && (rdif <= 1) && (cdif >= -1) && (cdif <= 1);
	return(retval);
}

start_wanderer()
{
	object *monster, *get_rand_monster();
	short row, col, i;
ANOTHER:
	monster = get_rand_monster();
	if ((!(monster->m_flags & WAKENS)) &&
	    (!(monster->m_flags & WANDERS))) {
		free(monster);
		goto ANOTHER;
	}
	wake_up(monster);

	for (i = 0; i < 12; i++) {
		get_rand_row_col(&row, &col, (FLOOR | TUNNEL | IS_OBJECT));
		if (!can_see(row, col)) {
			put_monster_at(row, col, monster);
			return;
		}
	}
}

show_monsters()
{
	object *monster;

	if (blind) return;

	monster = level_monsters.next_object;

	while (monster) {
		mvaddch(monster->row, monster->col, monster->ichar);
		if (monster->ichar == 'X') {
			monster->identified = 0;
		}
		monster = monster->next_object;
	}
}

create_monster()
{
	short row, col;
	short i, j, inc1, inc2;
	short found = 0;
	object *monster;

	inc1 = get_rand(0, 1) ? 1 : -1;
	inc2 = get_rand(0, 1) ? 1 : -1;

	for (i = inc1; i != (2 * -inc1); i -= inc1) {
		for (j = inc2; j != (2 * -inc2); j -= inc2) {
			if (!i && !j) continue;
			row = rogue.row + i;
			col = rogue.col + j;
			if ((row < MIN_ROW) || (row > (LINES-2)) ||
			    (col < 0) || (col > (COLS-1))) {
				continue;
			}
			if ((!(screen[row][col] & MONSTER)) &&
			      (screen[row][col] & (FLOOR|TUNNEL|STAIRS))) {
				found = 1;
				goto FOUND;
			}
		}
	}
FOUND:
	if (found) {
		monster = get_rand_monster();
		put_monster_at(row, col, monster);
		mvaddch(row, col, get_monster_char(monster));
		if (monster->m_flags & WANDERS) {
			wake_up(monster);
		}
	} else {
		message("you hear a faint cry of anguish in the distance",0);
	}
}

put_monster_at(row, col, monster)
object *monster;
{
		monster->row = row; monster->col = col;
		add_mask(row, col, MONSTER);
		add_to_pack(monster, &level_monsters, 0);
}

can_see(row, col)
{
	short retval;

	retval = !blind && ((get_room_number(row, col) == current_room) ||
	   (rogue_is_around(row, col)));

	return(retval);
}

flit(monster)
object *monster;
{
	short inc1, inc2;
	short i, j, row, col;

	if (!rand_percent(FLIT_PERCENT)) {
		return(0);
	}
	inc1 = get_rand(0, 1) ? 1 : -1;
	inc2 = get_rand(0, 1) ? 1 : -1;

	if (rand_percent(10)) {
		return(1);
	}
	for (i = inc1; i != (2*(0-inc1)); i+=(0-inc1)) {
		for (j = inc2; j != (2*(0-inc2)); j+=(0-inc2)) {

			row = monster->row + i;
			col = monster->col + j;
			if ((row == rogue.row) && (col == rogue.col)) {
				continue;
			}

			if (mtry(monster, row, col)) {
				return(1);
			}
		}
	}
	return(1);
}

put_monster_rand_location(monster)
object *monster;
{
	short row, col;

	get_rand_row_col(&row, &col, (FLOOR | TUNNEL | IS_OBJECT));
	add_mask(row, col, MONSTER);
	monster->row = row;
	monster->col = col;

}

get_rand_obj_char()
{
	short r;
	char *rs = "%!?]/):*";

	r = get_rand(0, 7);

	return(rs[r]);
}

no_room_for_monster(rn)
{
	short i, j;

	for (i = rooms[rn].left_col+1; i < rooms[rn].right_col; i++) {
		for (j = rooms[rn].top_row+1; j < rooms[rn].bottom_row; j++) {
			if (!(screen[i][j] & MONSTER)) {
				return(0);
			}
		}
	}
	return(1);
}

aggravate()
{
	struct object *monster;

	message("you hear a high pitched humming noise");

	monster = level_monsters.next_object;

	while (monster) {
		wake_up(monster);
		if (monster->ichar == 'X') {
			monster->identified = 0;
		}
		monster = monster->next_object;
	}
}

monster_can_see(monster, row, col)
object *monster;
{
	short rn, rdif, cdif, retval;

	rn = get_room_number(row, col);

	if ((rn != NO_ROOM) && (rn ==
	    get_room_number(monster->row, monster->col))) {
		return(1);
	}
	rdif = row - monster->row;
	cdif = col - monster->col;

	retval = (rdif >= -1) && (rdif <= 1) && (cdif >= -1) && (cdif <= 1);
	return(retval);
}

mv_aquatars()
{
	object *monster;

	monster = level_monsters.next_object;

	while (monster) {
		if (monster->ichar == 'A') {
			mv_monster(monster, rogue.row, rogue.col);
		}
		monster = monster->next_object;
	}
}
