#include curses.h
#include "object.h"
#include "monster.h"
#include "room.h"
#include file.h
extern char *player_name;
extern char *monster_names[];
extern short has_amulet, max_level;

killed_by(monster, other)
object *monster;
short other;
{
	char buf[80];
	short i;


	if (other != QUIT) {
		rogue.gold = ((rogue.gold * 9) / 10);
	}

	if (other) {
		switch(other) {
		case HYPOTHERMIA:
			strcpy(buf, "died of hypothermia");
			break;
		case STARVATION:
			strcpy(buf, "died of starvation");
			break;
		case QUIT:
			strcpy(buf, "quit");
			break;
		}
	} else {
		strcpy(buf, "Killed by ");
		if (is_vowel(monster_names[monster->ichar - 'A'])) {
			strcat(buf, "an ");
		} else {
			strcat(buf, "a ");
		}
		strcat(buf, monster_names[monster->ichar - 'A']);
	}
	strcat(buf, " with ");
	sprintf(buf+strlen(buf), "%d gold", rogue.gold);
	message(buf, 0);
	message("");
	score(monster, other);
}

win()
{
	rogue.armor = 0;	/* disarm and relax */
	rogue.weapon = 0;

	clear();
	mvaddstr(10, 11, "@   @  @@@   @   @      @  @  @   @@@   @   @   @");
	mvaddstr(11, 11, " @ @  @   @  @   @      @  @  @  @   @  @@  @   @");
	mvaddstr(12, 11, "  @   @   @  @   @      @  @  @  @   @  @ @ @   @");
	mvaddstr(13, 11, "  @   @   @  @   @      @  @  @  @   @  @  @@");
	mvaddstr(14, 11, "  @    @@@    @@@        @@ @@    @@@   @   @   @");
	mvaddstr(17, 11, "Congratulations,  you have  been admitted  to  the");
	mvaddstr(18, 11, "Fighter's Guild.   You return home,  sell all your");
	mvaddstr(19, 11, "treasures at great profit and retire into comfort.");
	message("");
	message("");
	id_all();
	sell_pack();
	score((object *) 0, WIN);
}

quit()
{
char getchartt();
	message("really quit?", 1);
	if (getchartt() != 'y') {
		check_message();
		return;
	}
	check_message();

	killed_by(0, QUIT);
}

score(monster, other)
object *monster;
{
	int fd;

	while (access(SCOREFILE, 2) || access(SCOREFILE, 4)) {
		umask(0);
		if ((fd = creat(SCOREFILE, 0666)) < 0) {
			message("cannot find/read/write score file");
			message(SCOREFILE);
			message("I'll wait, you go off and try to fix it");
			message("hit space when finished fixing the problem");
			wait_for_ack(0);
		} else {
			close(fd);
		}
	}
	put_scores(monster, other);
}

put_scores(monster, other)
object *monster;
{
	short i, j, n, rank = 10, x, dont_insert = 0;
	char scores[10][82];
	char getchartt();
	int fd, s;

	fd = open(SCOREFILE, O_RDWR, 0);

	for (i = 0; i < 10; i++) {
L:		if (((n = read(fd, &scores[i][0], 80)) < 80) && (n != 0)) {
			message("error in score file format");
			message("");
        		printf("space to continue, n=%d",n);
			getchartt();
			clean_up("sorry, score file is out of order");
		} else if (n == 0) {
			break;
		}
		if (!ncmp(scores[i]+16, player_name)) {
			x = 7;
			while (scores[i][x] == ' ') x++;
			s = get_number(scores[i] + x);
			if (s <= rogue.gold) {
				goto L;
			} else {
				dont_insert = 1;
			}
		}
	}
	if (dont_insert) goto DI;

	for (j = 0; j < i; j++) {
		if (rank > 9) {

			x = 7;
			while (scores[j][x] == ' ') x++;
			s = get_number(scores[j] + x);

			if (s <= rogue.gold) {
				rank = j;
			}
		}
	}
	if (i == 0) {
		rank = 0;
	} else if ((i < 10) && (rank > 9)) {
		rank = i;
	}
	if (rank <= 9) {
		insert_score(scores, rank, i, monster, other);

		if (i < 10) {
			i++;
		}
	}
	lseek(fd, 0, 0);
DI:
	clear();
	mvaddstr(3, 30, "Top  Ten  Rogueists");
	mvaddstr(8, 0, "Rank\tScore\tName");


	for (j = 0; j < i; j++) {
		if (j == rank) {
			standout();
		}
		if (j == 9) {
			scores[j][0] = '1';
			scores[j][1] = '0';
		} else {
			scores[j][0] = ' ';
			scores[j][1] = j + '1';
		}
		mvaddstr(j+10, 0, scores[j]);
		if (rank < 10) {
			write(fd, scores[j], 80);
		}
		if (j == rank) {
			standend();
		}
	}
	close(fd);
        mvaddstr(21,20,"space to continue");
	refresh_vms();
	getchartt();
	clean_up("");
}

insert_score(scores, rank, n, monster, other)
char scores[][82];
short rank, n;
object *monster;
{
	short i, k;
	char buf[82];

	for (i = (n - 1); i >= rank; i--) {
		if (i < 9) {
			strcpy(scores[i+1], scores[i]);
		}
	}
	sprintf(buf, "%2d      %5d   %s: ", rank+1, rogue.gold, player_name);

	if (other) {
		switch(other) {
		case HYPOTHERMIA:
			strcat(buf, "died of hypothermia");
			break;
		case STARVATION:
			strcat(buf, "died of starvation");
			break;
		case QUIT:
			strcat(buf, "quit");
			break;
		case WIN:
			strcat(buf, "a total winner");
			break;
		}
	} else {
		strcat(buf, "killed by ");
		if (is_vowel(monster_names[monster->ichar - 'A'])) {
			strcat(buf, "an ");
		} else {
			strcat(buf, "a ");
		}
		strcat(buf, monster_names[monster->ichar - 'A']);
	}
	sprintf(buf+strlen(buf), " on level %d ",  max_level);
	if ((other != WIN) && has_amulet) {
		strcat(buf, "with amulet");
	}
	for (i = strlen(buf); i < 79; i++) {
		buf[i] = ' ';
	}
	buf[79] = 0;
	strcpy(scores[rank], buf);
}

is_vowel(ch)
short ch;
{
	return( (ch == 'a') ||
		(ch == 'e') ||
		(ch == 'i') ||
		(ch == 'o') ||
		(ch == 'u') );
}

sell_pack()
{
	object *obj;
	short row = 2, val;
	char buf[80];

	obj = rogue.pack.next_object;

	clear();

	while (obj) {
		mvaddstr(1, 0, "Value      Item");
		if (obj->what_is == FOOD) {
			goto NEXT;
		}
		obj->identified = 1;
		val = get_value(obj);
		rogue.gold += val;

		if (row < SROWS) {
			sprintf(buf, "%5d      ", val);
			get_description(obj, buf+11);
			mvaddstr(row++, 0, buf);
		}
NEXT:		obj = obj->next_object;
	}
	refresh_vms();
	message("");
}

get_value(obj)
object *obj;
{
	short wc;
	int val;

	wc = obj->which_kind;

	switch(obj->what_is) {
	case WEAPON:
		val = id_weapons[wc].value;
		if ((wc == ARROW) || (wc == SHURIKEN)) {
			val *= obj->quantity;
		}
		val += (obj->damage_enchantment * 85);
		val += (obj->to_hit_enchantment * 85);
		break;
	case ARMOR:
		val = id_armors[wc].value;
		val += (obj->damage_enchantment * 75);
		if (obj->is_protected) {
			val += 200;
		}
		break;
	case WAND:
		val = id_wands[wc].value * obj->class;
		break;
	case SCROLL:
		val = id_scrolls[wc].value * obj->quantity;
		break;
	case POTION:
		val = id_potions[wc].value * obj->quantity;
		break;
	case AMULET:
		val = 5000;
		break;
	}
	if (val <= 0) {
		val = 10;
	}
	return(val);
}

id_all()
{
	short i;

	for (i = 0; i < SCROLLS; i++) {
		id_scrolls[i].id_status = IDENTIFIED;
	}
	for (i = 0; i < WEAPONS; i++) {
		id_weapons[i].id_status = IDENTIFIED;
	}
	for (i = 0; i < ARMORS; i++) {
		id_armors[i].id_status = IDENTIFIED;
	}
	for (i = 0; i < WANDS; i++) {
		id_wands[i].id_status = IDENTIFIED;
	}
	for (i = 0; i < POTIONS; i++) {
		id_potions[i].id_status = IDENTIFIED;
	}
}

ncmp(s1, s2)
char *s1, *s2;
{
	short i = 0;
	int r;

	while(s1[i] != ':') i++;
	s1[i] = 0;
	r = strcmp(s1, s2);
	s1[i] = ':';
	return(r);
}
