/*
 *
 * TREK73: globals.c
 *
 * Global variable declarations
 *
 */

#include "externs.h"

extern	int fire_phasers(), fire_tubes(), lock_phasers(), lock_tubes(),
	turn_phasers(), turn_tubes(), load_tubes(), phaser_status(),
	tube_status(), launch_probe(), probe_control(), pos_report(),
	pos_display(), pursue(), elude(), helm(), self_scan(), scan(),
	alter_power(), jettison_engineering(), detonate_engineering(),
	alterpntparams(), play_dead(), corbomite_bluff(), surrender_ship(),
	request_surrender(), self_destruct(), abort_self_destruct(),
	save_game(), survivors(), help(), vers();

extern 	int standard_strategy();

char encstr[] = "\211g\321_-\251b\324\237;\255\263\214g\"\327\224.,\252|9\265=\357+\343;\311]\341`\251\b\231)\266Y\325\251";

char version[] = "TREK73 Version 3.2 08/17/86";

char *sysname[] = {
	"Computer",		/* S_COMP */
	"Sensors",		/* S_SENSOR */
	"Probe launcher",	/* S_PROBE */
	"Warp Drive",		/* S_WARP */
};

char *statmsg[] = {			/* When this system is dead */
	"computer inoperable",		/* S_COMP */
	"sensors annihilated",		/* S_SENSOR */
	"probe launcher shot off",	/* S_PROBE */
	"warp drive disabled",		/* S_WARP */
	"engineering jettisoned",	/* S_ENG */
} ;

struct damage p_damage = {
	50, 2, 20, 10, 3,		/* eff, fuel, regen, crew, weapon */
	1000,	"Computer destroyed.",		/* S_COMP */
	500,	"Sensors demolished.",		/* S_SENSOR */
	100,	"Probe launcher crushed.",	/* S_PROBE */
	50,	"Warp drive destroyed.",	/* S_WARP */
} ;

struct damage a_damage = {
	100, 3, 10, 7, 6,		/* eff, fuel, regen, crew, weapon */
	1500,	"Computer banks pierced.",	/* S_COMP */
	750,	"Sensors smashed.",		/* S_SENSOR */
	150,	"Probe launcher shot off.",	/* S_PROBE */
	75,	"Warp drive disabled.",		/* S_WARP */
} ;


char *feds[] = {
	"Constitution", "Enterprise", "Hornet", "Lexington", "Potempkin",
	"Hood", "Kongo", "Republic", "Yorktown",
} ;

struct race_info aliens[MAXFOERACES] = {
	{
		"Klingon", "Empire", 0, 25, 25, 75, 75, 0,
		{ "Annihilation", "Crusher", "Devastator", "Merciless",
		  "Nemesis", "Pitiliess", "Ruthless", "Savage", "Vengeance",
		},
		{ "C-9 Dreadnought", "D-7 Battle Cruiser",
		  "D-6 Light Battlecruiser", "F-5L Destroyer",
		},
		{ "Koloth", "Kang", "Kor", "Krulix", "Korax", "Karg",
		  "Kron", "Kumerian",
		}, 0,
	},
	{
		"Romulan", "Star Empire", 0, 5, 5, 80, 50, 0,
		{ "Avenger", "Defiance", "Fearless", "Harrower", "Intrepid",
		  "Relentless", "Seeker", "Torch", "Vigilant",
		},
		{ "Condor Dreadnought", "Firehawk Heavy Cruiser",
		  "Sparrowhawk Light Cruiser", "Skyhawk Destroyer",
		},
		{ "Tal", "Tiercellus", "Diana", "Tama", "Subeus", "Turm",
		  "Strell", "Scipio",
		}, 0,
	},
	{
		"Kzinti", "Hegemony", 0, 50, 50, 50, 50, 0,
		{ "Black Hole", "Comet", "Ecliptic", "Galaxy", "Meteor",
		  "Nova", "Pulsar", "Quasar", "Satellite",
		},
		{ "Space Control Ship", "Strike Cruiser", "Light Cruiser",
		  "Destroyer",
		},
		{ "Hunter", "\"Cat Who Sleeps With Dogs\"", "Fellus", "Corda",
		  "\"Cat Who Fought Fuzzy Bear\"", "", "", "",
		}, 0,
	},
	{
		"Gorn", "Confederation", 0, 80, 50, 50, 50, 0,
		{ "Chimericon", "Dragonicon", "Ornithocon", "Predatoricon",
		  "Reptilicon", "Serpenticon", "Tyranicon", "Vipericon",
		  "Wyvericon",
		},
		{ "Tyrannosaurus Rex Dreadnought", "Allosaurus Heavy Cruiser",
		  "Megalosaurus Light Cruiser", "Carnosaurus Destroyer",
		},
		{ "Sslith", "Dardiss", "Ssor", "Sslitz", "S'Arnath",
		  "Zor", "", "",
		}, 0,
	},
	{
		"Orion", "Pirates", 0, 95, 5, 50, 60, 0,
		{ "Deuce Coupe", "Final Jeopardy", "Long John Dilithium",
		  "Millennium Pelican", "Omega Race", "Penzance",
		  "Road Warrior", "Scarlet Pimpernel", "Thunderduck",
		},
		{ "Battle Raider", "Heavy Cruiser", "Raider Cruiser",
		  "Light Raider",
		},
		{ "Daniel \"Deth\" O'Kay", "Neil Ricca", "Delilah Smith",
		  "Hamilcar", "Pharoah", "Felna Greymane", "Hacker", 
		  "Credenza",
		}, 0,
	},
	{
		"Hydran", "Monarchy", 0, 50, 50, 50, 50, 0,
		{ "Bravery", "Chivalry", "Devotion", "Fortitude", "Loyalty",
		  "Modesty", "Purity", "Resolution", "Tenacity",
		},
		{ "Paladin Dreadnought", "Ranger-class Cruiser",
		  "Horseman Light Cruiser", "Lancer Destroyer",
		},
		{ "Hypantspts", "S'Lenthna", "Hydraxan", "", "", "",
		  "", "",
		}, 0,
	},
	{
		"Lyran", "Empire", 0, 50, 50, 50, 50, 0,
		{ "Bandit", "Claw", "Dangerous", "Fury", "Mysterious",
		  "Sleek", "Tiger", "Vicious", "Wildcat",
		},
		{ "Lion Dreadnought", "Tiger Cruiser",
		  "Panther Light Cruiser", "Leopard Destroyer",
		},
		{ "Kleave", "Leyraf", "Kuhla", "Nashar",
		  "Prekor", "Ffarric", "Rippke", "Larkahn",
		}, 0,
	},
	{
		"Tholian", "Holdfast", 0, 75, 25, 50, 50, 0,
		{ "Bismark", "Centaur", "Draddock", "Forbin", "Kreiger",
		  "Shlurg", "Trakka", "Varnor", "Warrior",
		},
		{ "Tarantula Dreadnought", "Cruiser", "Improved Patrol Cruiser",
		  "Patrol Cruiser",
		},
		{ "Secthane", "Kotheme", "Sectin", "Brezgonne", "", "",
		  "", "",
		}, 0,
	},
	{
		"Monty Python", "Flying Circus", 0, -1, -1, -1, -1, 0,
		{ "Blancmange", "Spam", "R.J. Gumby", "Lumberjack",
		  "Dennis Moore", "Ministry of Silly Walks", "Argument Clinic",
		  "Piranha Brothers", "Upper Class Twit of the Year",
		},
		{ "Thingee", "Thingee", "Thingee", "Thingee",
		},
		{ "Cleese", "Chapman", "Idle", "Jones", "Gilliam", "Bruce",
		  "Throatwobblermangrove", "Arthur \"Two Sheds\" Jackson",
		}, 0,
	}
};


float init_p_turn[MAXPHASERS][MAXPHASERS] = {
	{ 666.666 },
	{ 0.0 },
	{ 0.0, 0.0 },
	{ 90.0, 0.0, 270.0 },
	{ 90.0, 0.0, 0.0, 270.0 },
	{ 90.0, 0.0, 0.0, 0.0, 270.0 },
	{ 90.0, 90.0, 0.0, 0.0, 270.0, 270.0 },
	{ 90.0, 90.0, 0.0, 0.0, 0.0, 270.0, 270.0 },
	{ 90.0, 90.0, 0.0, 0.0, 0.0, 0.0, 270.0, 270.0 },
	{ 90.0, 90.0, 90.0, 0.0, 0.0, 0.0, 270.0, 270.0, 270.0 },
	{ 90.0, 90.0, 90.0, 0.0, 0.0, 0.0, 0.0, 270.0, 270.0, 270.0 },
};

float init_t_turn[MAXTUBES][MAXTUBES] = {
	{ 666.666 },
	{ 0.0 },
	{ 0.0, 0.0 },
	{ 60.0, 0.0, 300.0 },
	{ 60.0, 0.0, 0.0, 300.0 },
	{ 60.0, 0.0, 0.0, 0.0, 300.0 },
	{ 120.0, 60.0, 0.0, 0.0, 300.0, 240.0 },
	{ 120.0, 60.0, 0.0, 0.0, 0.0, 300.0, 240.0 },
	{ 120.0, 60.0, 60.0, 0.0, 0.0, 300.0, 300.0, 240.0 },
	{ 120.0, 60.0, 60.0, 0.0, 0.0, 0.0, 300.0, 300.0, 240.0 },
	{ 120.0, 120.0, 60.0, 60.0, 0.0, 0.0, 300.0,300.0, 240.0, 240.0 },
};

/*
 * for the linked list of items in space
 */
struct	list head;
struct	list *tail;

/*
 * Global definitions
 */
float	segment = 0.05;		/* Segment time */
float	timeperturn = 2.0;	/* Seconds per turn */

struct	ship *shiplist[10];	/* All the ships in the battle */

char	home[256];		/* Path to user's home directory */
char	savefile[256];		/* Path to save file */
char	captain[30];		/* captain's name */
char	science[30];		/* science officer's name */
char	engineer[30];		/* engineer's name */
char	com[30];		/* communications officer's name */
char	nav[30];		/* navigation officer's name */
char	helmsman[30];		/* helmsman's name */
char	title[9];		/* captain's title */
char	foerace[30];		/* enemy's race */
char	foename[30];		/* enemy's captain's name */
char	foestype[30];		/* enemy's ship type */
char	empire[30];		/* What the enemy's empire is called */
char	class[3];		/* Class of own ship */
char	foeclass[3];		/* Class of enemy ship(s) */
int	shipnum;		/* number of ships this time out */
int	enemynum;		/* Enemy identifier */
int	terse = 0;		/* print out initial description? */
int	silly = 0;		/* Use the Monty Python's Flying Curcus? */
int	defenseless = 0;	/* defensless ruse status */
int	corbomite = 0;		/* corbomite bluff status */
int	surrender = 0;		/* surrender offered by federation? */
int	surrenderp = 0;		/* Did we request that the enemy surrenders? */
int	restart = 0;		/* Should we restart the game? */
char	shutup[HIGHSHUTUP];	/* Turn off messages after first printing */
char	slots[HIGHSLOT];	/* Id slots */
int	global = NORMAL;	/* Situation status */
char	*options;		/* Environment variable */
char	sex[20];		/* From environment */
char	shipname[30];		/* From environment */
char	racename[20];		/* From environment */
int	reengaged = 0;		/* Re-engaging far-off ships? */
char    com_delay[6];		/* Number of seconds per real-time turn */
int	teletype = 0;		/* Flag for special teletype options */
int	time_delay = 30;	/* Time to enter command */
int	trace = TR_OFF;		/* Trace flag for debugging and records */
char	can_cloak = 0;		/* Can enemy ship cloak? */
double	o_bpv;			/* BPV of us */
double	e_bpv;			/* BPV of enemy */

struct ship_stat	us;	/* Our ship definition */
struct ship_stat	them;	/* Their ship definition */

struct  cmd cmds[] = {
	{ fire_phasers,		"1",	"Fire phasers",			TURN },
	{ fire_tubes,		"2",	"Fire photon torpedoes",	TURN },
	{ lock_phasers, 	"3",	"Lock phasers onto target",	TURN },
	{ lock_tubes,		"4",	"Lock tubes onto target",	TURN },
	{ turn_phasers,		"5",	"Manually rotate phasers",	TURN },
	{ turn_tubes,		"6",	"Manually rotate tubes",	TURN },
	{ phaser_status,	"7",	"Phaser status",		FREE },
	{ tube_status,		"8",	"Tube status",			FREE },
	{ load_tubes,		"9",	"Load/unload torpedo tubes",	TURN },
	{ launch_probe,		"10",	"Launch antimatter probe",	TURN },
	{ probe_control,	"11",	"Probe control",		TURN },
	{ pos_report,		"12",	"Position report",		FREE },
	{ pos_display,		"13",	"Position display",		FREE },
	{ pursue,		"14",	"Pursue an enemy vessel",	TURN },
	{ elude,		"15",	"Elude an enemy vessel",	TURN },
	{ helm,			"16",	"Change course and speed",	TURN },
	{ self_scan,		"17",	"Damage report",		FREE },
	{ scan,			"18",	"Scan enemy",			TURN },
	{ alter_power,		"19",	"Alter power distribution",	TURN },
	{ jettison_engineering,	"20",   "Jettison engineering",		TURN },
	{ detonate_engineering,	"21",  	"Detonate engineering",		TURN },
	{ alterpntparams,	"22",	"Alter firing parameters", 	TURN},
	{ play_dead,		"23",	"Attempt defenseless ruse",	TURN },
	{ corbomite_bluff,	"24",	"Attempt corbomite bluff(s)",	TURN },
	{ surrender_ship,	"25",	"Surrender",			TURN },
	{ request_surrender,	"26",	"Ask enemy to surrender",	TURN },
	{ self_destruct,	"27",	"Initiate self-destruct",	TURN },
	{ abort_self_destruct,	"28",	"Abort self-destruct",		TURN },
	{ survivors,		"29",	"Survivors report",		FREE },
	{ vers,			"30",	"Print version number",		FREE },
	{ save_game,		"31",	"Saves game",			FREE },
	{ help,			"32",	"Reprints above list",		FREE },
	{ NULL,			NULL,	NULL,				TURN },
} ;

int	high_command = 32;	/* XXX */

	/* used to print cmd list */
int	cmdarraysize = sizeof(cmds) / sizeof (struct cmd) -1; 

/*	Strategy table */
int (*strategies[])() = {
	standard_strategy,
};
