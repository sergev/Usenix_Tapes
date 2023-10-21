#define BLANK ((unsigned short) 0)
#define ARMOR ((unsigned short) 01)
#define WEAPON ((unsigned short) 02)
#define SCROLL ((unsigned short) 04)
#define POTION ((unsigned short) 010)
#define GOLD ((unsigned short) 020)
#define FOOD ((unsigned short) 040)
#define WAND ((unsigned short) 0100)
#define STAIRS ((unsigned short) 0200)
#define AMULET ((unsigned short) 0400)
#define MONSTER ((unsigned short) 01000)
#define HORWALL ((unsigned short) 02000)
#define VERTWALL ((unsigned short) 04000)
#define DOOR ((unsigned short) 010000)
#define FLOOR ((unsigned short) 020000)
#define TUNNEL ((unsigned short) 040000)
#define UNUSED ((unsigned short) 0100000)
#define IS_OBJECT ((unsigned short) 0777)  /* all object masks or'd together */
#define CAN_PICK_UP ((unsigned short) 0577)

#define LEATHER 0
#define RING 1
#define SCALE 2
#define CHAIN 3
#define BANDED 4
#define SPLINT 5
#define PLATE 6
#define ARMORS 7

#define BOW 0
#define ARROW 1
#define SHURIKEN 2
#define MACE 3
#define LONG_SWORD 4
#define TWO_HANDED_SWORD 5
#define WEAPONS 6

#define MAX_PACK_COUNT 24

#define PROTECT_ARMOR 0
#define HOLD_MONSTER 1
#define ENCHANT_WEAPON 2
#define ENCHANT_ARMOR 3
#define IDENTIFY 4
#define TELEPORT 5
#define SLEEP 6
#define SCARE_MONSTER 7
#define REMOVE_CURSE 8
#define CREATE_MONSTER 9
#define AGGRAVATE_MONSTER 10
#define SCROLLS 11

#define INCREASE_STRENGTH 0
#define RESTORE_STRENGTH 1
#define HEALING 2
#define EXTRA_HEALING 3
#define POISON 4
#define RAISE_LEVEL 5
#define BLINDNESS 6
#define HALLUCINATION 7
#define DETECT_MONSTER 8
#define DETECT_OBJECTS 9
#define CONFUSION 10
#define POTIONS 11

#define TELEPORT_AWAY 0
#define SLOW_MONSTER 1
#define KILL_MONSTER 2
#define INVISIBILITY 3
#define POLYMORPH 4
#define HASTE_MONSTER 5
#define PUT_TO_SLEEP 6
#define DO_NOTHING 7
#define WANDS 8

#define UNIDENTIFIED ((unsigned char) 0)	/* MUST BE ZERO! */
#define IDENTIFIED ((unsigned char) 01)
#define CALLED ((unsigned char) 02)

#define SROWS 24
#define SCOLS 80
#define MAX_TITLE_LENGTH 30
#define MORE "-more-"
#define MAXSYLLABLES 40
#define MAXMETALS 15

#define GOLD_PERCENT 46

struct identify {
	short value;
	char *title;
	char *real;
	unsigned char id_status;
};

struct object {				/* comment is monster meaning */
	unsigned short m_flags;		/* monster flags */
	char *damage;			/* damage it does */
	short quantity;			/* hit points to kill */
	char ichar;			/* 'A' is for aquatar */
	short kill_exp;			/* exp for killing it */
	char is_protected;		/* level starts */
	char is_cursed;			/* level ends */
	char class;			/* chance of hitting you */
	char identified;		/* F%d/Arwarn/Og/If/Mc/Xc */
	unsigned char which_kind;	/* item carry/drop % */
	char row, col;			/* current row,col */
	char damage_enchantment;	/* fly-trap,medusa,etc */
	char quiver;			/* monster slowed toggle */
	char trow, tcol;		/* target row, col */
	char to_hit_enchantment;
	unsigned short what_is;
	char picked_up;
	struct object *next_object;	/* next monster */
};

typedef struct object object;

struct fighter {
	object *armor;
	object *weapon;
	short hp_current;
	short hp_max;
	char strength_current;
	char strength_max;
	object pack;
	int gold;
	char exp;
	int exp_points;
	short row, col;
	char fchar;
	short moves_left;
};

typedef struct fighter fighter;

struct door {
	char other_room;
	char other_row,
	     other_col;
};

typedef struct door door;

struct room {
	char bottom_row, right_col, left_col, top_row;
	char width, height;
	door doors[4];
	char is_room;
};

typedef struct room room;

extern fighter rogue;
extern room rooms[];
unsigned extern short screen[SROWS][SCOLS];
extern object level_objects;

extern struct identify id_scrolls[];
extern struct identify id_potions[];
extern struct identify id_wands[];
extern struct identify id_weapons[];
extern struct identify id_armors[];

extern object monster_tab[];
extern object level_monsters;
