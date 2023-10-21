/*
 *
 * TREK73: globals.c
 *
 * Global variable declarations
 *
 */
#include "defines.h"
#include "structs.h"

extern	int fire_phasers(), fire_tubes(), lock_phasers(), lock_tubes(),
	turn_phasers(), turn_tubes(), load_tubes(), phaser_status(),
	tube_status(), launch_probe(), probe_control(), pos_report(),
	pos_display(), pursue(), elude(), helm(), self_scan(), scan(),
	alter_power(), jettison_engineering(), detonate_engineering(),
	alterpntparams(), play_dead(), corbomite_bluff(), surrender_ship(),
	request_surrender(), self_destruct(), abort_self_destruct(),
	survivors(), help();

extern 	int
	standard_strategy();


char *statmsg[] = {
	"computer inoperable",
	"sensors annihilated",
	"probe launcher shot off",
	"warp drive disabled",
	"engineering jettisoned",
} ;

struct damage p_damage = {
	50, 2, 20, 3, 10, 3,
	1000,	"Computer destroyed.",
	500,	"Sensors demolished.",
	100,	"Probe launcher crushed.",
	50,	"Warp drive destroyed.",
} ;

struct damage a_damage = {
	100, 3, 10, 2, 7, 6,
	1500,	"Computer banks pierced.",
	750,	"Sensors smashed.",
	150,	"Probe launcher shot off.",
	75,	"Warp drive disabled.",
} ;


char *baddies[MAXFOERACES][MAXBADS] = {
	/* Klingons*/
	"Annihilation", "Crusher", "Devastator", "Merciless", "Nemesis",
	"Pitiliess", "Ruthless", "Savage", "Vengeance",
	/* Romulan */
	"Avenger", "Defiance", "Fearless", "Harrower", "Intrepid",
	"Relentless", "Seeker", "Torch", "Vigilant",
	/* Kzinti */
	"Black Hole", "Comet", "Ecliptic", "Galaxy", "Meteor",
	"Nova", "Pulsar", "Quasar", "Satellite",
	/* Gorn */
	"Chimericon", "Dragonicon", "Ornithocon", "Predatoricon", "Reptilicon",
	"Serpenticon", "Tyranicon", "Vipericon", "Wyvericon",
	/* Orion */
	"Arrogant", "Boisterous", "Daring", "Flamboyant", "Heavensent",
	"Jolly Tar", "Magnificent", "Resplendent", "Stupendous",
	/* Hydran */
	"Baron", "Chancellor", "Dictator", "Emperor", "Lord",
	"Monarch", "President", "Shogun", "Viscount",
	/* Lyran */
	"Bandit", "Claw", "Dangerous", "Fury", "Mysterious",
	"Sleek", "Tiger", "Vicious", "Wildcat",
	/* Tholian */
	"Bismark", "Centaur", "Draddock", "Forbin", "Kreiger",
	"Shlurg", "Trakka", "Varnor", "Warrior",
	/* Monty Python */
	"Blancmange", "Spam", "R.J. Gumby", "Lumberjack", "Dennis Moore",
	"Ministry of Silly Walks", "Argument Clinic", "Piranha Brothers",
	"Upper Class Twit of the Year",
} ;

char *feds[] = {
	"Constitution", "Enterprise", "Hornet", "Lexington", "Potempkin",
	"Hood", "Kongo", "Republic", "Yorktown",
} ;

char *foeraces[] = {
	"Klingon", "Romulan", "Kzinti", "Gorn", "Orion", "Hydran",
	"Lyran", "Tholian", "Monty Python",
} ;

char *foeempire[] = {
	"Empire", "Star Empire", "Hegemony", "Confederation", "Pirates",
	"Monarchy", "Something", "Holdfast", "Flying Circus",
};

char *foeshiptype[] = {
	"D-7 Battle Cruiser", "Sparrowhawk", "Strike Cruiser",
	"Heavy Cruiser", "Raider Cruiser", "Ranger-class Cruiser",
	"Tiger-class Cruiser", "Patrol Cruiser", "Thingee",
};

char *foecaps[] = {
	"Bolak", "Kang", "Koloth", "Kor", "Korax", "Krulix", "Quarlo",
	"Tal", "Troblak"
};

int init_p_turn[] = {
	-90, 0, 0, 90
};

int init_t_turn[] = {
	-120, -60, 0, 0, 60, 120
};

/*
 * for the linked list of items in space
 */
struct	list head;
struct	list *tail;

/*
 * Global definitions
 */
float	segment = 0.2;		/* Segment time */
float	timeperturn = 2.0;	/* Seconds per turn */

struct	ship *shiplist[10];	/* All the ships in the battle */

char	captain[30];		/* captain's name */
char	science[30];		/* science officer's name */
char	engineer[30];		/* engineer's name */
char	com[30];		/* communications officer's name */
char	nav[30];		/* navigation officer's name */
char	helmsman[30];		/* helmsman's name */
char	title[9];		/* captain's title */
char	foerace[11];		/* enemy's race */
char	foename[9];		/* enemy's captain's name */
char	foestype[30];		/* enemy's ship type */
char	empire[30];		/* What the enemy's empire is called */
int	shipnum;		/* number of ships this time out */
int	terse = 0;		/* print out initial description? */
int	silly = 0;		/* Use the Monty Python's Flying Curcus? */
int	defenseless = 0;	/* defensless ruse status */
int	corbomite = 0;		/* corbomite bluff status */
int	surrender = 0;		/* surrender offered by federation? */
int	surrenderp = 0;		/* Did we request that the enemy surrenders? */
char	shutup[HIGHSHUTUP];	/* Turn off messages after first printing */
char	slots[300];		/* Id slots */
int	global = NORMAL;	/* Situation status */
char	**argp = NULL;		/* Argument list for parsit() routine */
char	options[100];		/* Environment variable */
char	sex[20];		/* From environment */
char	shipbuf[10];		/* From environment */
char	shipname[20];		/* From environment */
char	racename[20];		/* From environment */
int	reengaged = 0;		/* Re-engaging far-off ships? */

struct  cmd cmds[] = {
	{ fire_phasers,		"1",	"Fire phasers",			1 },
	{ fire_tubes,		"2",	"Fire photon torpedoes",	1 },
	{ lock_phasers, 	"3",	"Lock phasers onto target",	1 },
	{ lock_tubes,		"4",	"Lock tubes onto target",	1 },
	{ turn_phasers,		"5",	"Manually rotate phasers",	1 },
	{ turn_tubes,		"6",	"Manually rotate tubes",	1 },
	{ phaser_status,	"7",	"Phaser status",		1 },
	{ tube_status,		"8",	"Tube status",			1 },
	{ load_tubes,		"9",	"Load/unload torpedo tubes",	1 },
	{ launch_probe,		"10",	"Launch antimatter probe",	1 },
	{ probe_control,	"11",	"Probe control",		1 },
	{ pos_report,		"12",	"Position report",		0 },
	{ pos_display,		"13",	"Position display",		0 },
	{ pursue,		"14",	"Pursue an enemy vessel",	1 },
	{ elude,		"15",	"Elude an enemy vessel",	1 },
	{ helm,			"16",	"Manually change course and speed",	1 },
	{ self_scan,		"17",	"Damage report",		1 },
	{ scan,			"18",	"Scan enemy (enemy Damage report)",	1 },
	{ alter_power,		"19",	"Alter power distribution",	1 },
	{ jettison_engineering,	"20",   "Jettison engineering",		1 },
	{ detonate_engineering,	"21",  	"Detonate engineering",		1 },
	{ alterpntparams,	"22",	"Alter firing parameters", 	1},
	{ play_dead,		"23",	"Attempt defenseless ruse",	1 },
	{ corbomite_bluff,	"24",	"Attempt corbomite bluff(s)",	1 },
	{ surrender_ship,	"25",	"Surrender",			1 },
	{ request_surrender,	"26",	"Ask enemy to surrender",	1 },
	{ self_destruct,	"27",	"Initiate self-destruct sequence",	1 },
	{ abort_self_destruct,	"28",	"Abort self-destruct",		1 },
	{ survivors,		"29",	"Survivors report",		0 },
	{ help,			"30",	"Reprints above list",		0 },
	{ NULL,			NULL,	NULL,				1 },
} ;

/*	Strategy table */
int (*strategies[])() = {
	standard_strategy
};
