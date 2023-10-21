/*
 * TREK73: structs.h
 *
 * Struct Defs for TREK73
 *
 */

#include "defines.h"

struct phaser {
	struct ship *target;	/* who we're aimed at */
	float bearing;		/* direction aimed (if no target) */
	int drain;		/* Drain from engines (to if negative) */
	short load;		/* energy in phasers */
	char status;		/* Damaged, etc. */
} ;

struct tube {
	struct ship *target;	/* who we're aimed at */
	float bearing;		/* direction aimed (if no target) */
	int load;		/* energy in tubes */
	char status;		/* Damaged, etc */
} ;

struct shield {
	float eff;		/* efficiency from 0-1 */
	float drain;		/* Actual drain from engines */
	float attemp_drain;	/* Attempted drain from engines */
} ;

#define MAXWEAPONS 11
#define SHIELDS 4

struct ship {
	char name[30];		/* name of ship */
	char class[3];		/* Type of ship */
	int x, y;		/* location */
	float warp;		/* warp speed */
	float newwarp;		/* for speed changes */
	float course;		/* 0-360 */
	float newcourse;	/* for course changes */
	struct ship *target;	/* who we're pursuing */
	float relbear;		/* Relative bearing to keep */
	struct phaser phasers[MAXWEAPONS];	/* phaser banks */
	int p_spread;		/* phaser spread */
	int p_percent;		/* phaser firing percentage */
	int p_blind_left;	/* phaser blind area, left side angle */
	int p_blind_right;	/* phaser blind area, right side angle */
	struct tube tubes[MAXWEAPONS];		/* torpedo tubes */
	int t_prox;		/* proximity delay */
	int t_delay;		/* time delay to detonation */
	int t_lspeed;		/* launch speed */
	int t_blind_left;	/* tube blind area, left side angle */
	int t_blind_right;	/* tube blind area, right side angle */
	struct shield shields[SHIELDS]; /* shields */
	int probe_status;	/* Probe launcher status */
	float eff;		/* efficiency */
	float regen;		/* regeneration (energy per turn) */
	float energy;		/* amount of effective energy */
	float pods;		/* max energy level */
	int complement;		/* crew left alive */
	int status[MAXSYSTEMS];	/* Holds damage percentage of these systems */
	float delay;		/* how long 'till we blow up? */
	int id;			/* Unique identifier */
	int num_phasers;	/* Number of phasers */
	int num_tubes;		/* Number of tubes */
	float orig_max;		/* Maximum original warp */
	float max_speed;	/* Maximum warp */
	float deg_turn;		/* Degrees per warp turn */
	float ph_damage;	/* Damage divisor from phasers */
	float tu_damage;	/* Damage divisor from tubes */
	int cloaking;		/* Cloaking device status */
	int cloak_energy;	/* Energy needed to run cloak per segment */
	int cloak_delay;	/* Time until a cloak may be restarted */
	int (*strategy)();	/* Which strategy to use */
	struct pos {	/* Last known position (before a cloak) */
		int x,y;	/* Coordinates */
		float warp;	/* Warp speed */
		int range;	/* Distance to ship */
		float bearing;	/* Bearing */
		float course;	/* Course */
	} position ;
	int p_firing_delay;	/* Delay in segments for firing phasers */
	int t_firing_delay;	/* Delay in segments for firing torpedoes */
} ;

/*
 * note that probes act like torpedos
 * but have targets; torps only have
 * courses
 */
struct torpedo {
	struct ship *from;	/* pointer to ship they're from */
	int x, y;		/* coords of object */
	float course;		/* where it's going */
	float speed;		/* how fast we're moving */
	float newspeed;		/* what our target speed is */
	struct ship *target;	/* who we're aimed at */
	int fuel;		/* how many antimatter pods it has */
	float timedelay;	/* seconds until detonation */
	int prox;		/* proximity fuse */
	int id;			/* Unique identifier */
	int type;		/* torpedo, probe, or engineering */
} ;

/*
 * the list of what's in space -- depending on the type, we use
 * differing parts of the union (data) structure.  it's a linked
 * list of all the stuff in space.
 */
struct list {
	int type;
	struct list *back, *fwd;
	union {
		struct torpedo *tp;
		struct ship *sp;
	} data;
} ;

struct cmd {
	int (*routine)();
	char *word1;
	char *word2;
	int turns;
} ;

/*
 * for the phaser and anti-matter damage lists
 */

struct dam {
	int roll;
	char *mesg;
};

struct damage {
	float eff;
	float fuel;
	float regen;
	float crew;
	float weapon;
	struct dam stats[S_NUMSYSTEMS];
};

struct score {
	int score;
	int ships;
	char captain[30];
	char username[10];
}; 

struct rates {
	char *rate;
	int points;
};

struct ship_stat {
	char abbr[4];		/* Abbreviation */
	int class_num;		/* Index into array */
	int num_phaser;		/* Number of phasers */
	int num_torp;		/* Number of tubes */
	int o_warpmax;		/* Own max speed */
	int e_warpmax;		/* Enemy max speed */
	float o_eff;		/* Own efficiency */
	float e_eff;		/* Enemy efficiency */
	float regen;		/* Regeneration */
	float energy;		/* Starting fuel */
	float pods;		/* Max pods */
	int o_crew;		/* Own crew */
	int e_crew;		/* Enemy crew */
	float ph_shield;	/* Divisor for phaser damage */
	float tp_shield;	/* Divisor for torp damage */
	int turn_rate;		/* Degrees per warp-second */
	int cloaking_energy;	/* Energy to run cloaking device */
	int t_blind_left;	/* Start of tube blind area left */
	int t_blind_right;	/* Start of tube blind area right */
	int p_blind_left;	/* Start of phaser blind area left */
	int p_blind_right;	/* Start of phaser blind area right */
	/* Must change to absolute time */
	int p_firing_delay;	/* Delay in segments for firing phasers */
	int t_firing_delay;	/* Delay in segments for firing torpedoes */
};

struct race_info {
	char race_name[30];	/* Name of the race */
	char empire_name[30];	/* What they call themselves */
	int id;			/* Identifier number */
	int surrender;		/* Chance they will accept a surrender */
	int surrenderp;		/* Chance they will surrender to you */
	int corbomite;		/* Chance they fall for a corbomite bluff */
	int defenseless;	/* Chance they fall for a defenseless ruse */
	int attitude;		/* Attitude factor for strategies */
	char *ship_names[MAXESHIPS];	/* Ship names */
	char *ship_types[MAXSHIPCLASS];	/* Ship types */
	char *captains[MAXECAPS];	/* Some exemplary captains */
	/*
	 * For the strategic game that is to come.  -Deep Thought
	 */
	int relation;		/* Diplomatic relation with other races */
};

struct planet {
	char name[30];		/* Planetary name */
	int id;			/* Unique identifier number */
	int x;			/* Location in the X plane */
	int y;			/* Location in the Y plane */
	int radius;		/* Planetary radius */
	int mass;		/* Planetary mass */
	int race;		/* Owner's race */
	/*
	 * planetary weaponry goes here
	 */
};
