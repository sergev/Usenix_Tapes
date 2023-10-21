/*
 * TREK73: structs.h
 *
 * Struct Defs for TREK73
 *
 */


struct phaser {
	struct ship *target;	/* who we're aimed at */
	int bearing;		/* direction aimed (if no target) */
	char load;		/* energy in phasers */
	char status;		/* Damaged, etc. */
	int drain;		/* Drain from engines (to if negative) */
} ;

struct tube {
	struct ship *target;	/* who we're aimed at */
	int bearing;		/* direction aimed (if no target) */
	char load;		/* energy in tubes */
	char status;		/* Damaged, etc */
} ;

struct shield {
	float eff;		/* efficiency from 0-1 */
	float drain;		/* Actual drain from engines */
	float attemp_drain;	/* Attempted drain from engines */
} ;

struct ship {
	char name[30];		/* name of ship */
	char nat;		/* nationality */
	int x, y;		/* location */
	float warp;		/* warp speed */
	float newwarp;		/* for speed changes */
	int course;		/* 0-360 */
	int newcourse;		/* for course changes */
	struct ship *target;	/* who we're pursuing */
	int eluding;		/* Flag for eluding */
	struct phaser phasers[4]; /* phaser banks */
	int p_spread;		/* phaser spread */
	int p_percent;		/* phaser firing percentage */
	struct tube tubes[6];	/* torpedo tubes */
	int t_prox;		/* proximity delay */
	int t_delay;		/* time delay */
	int t_lspeed;		/* launch speed */
	struct shield shields[4]; /* shields */
	float eff;		/* efficiency */
	float regen;		/* regeneration (energy per turn) */
	int energy;		/* amount of effective energy */
	int pods;		/* max energy level */
	int crew;		/* crew left alive */
	int status;		/* computer, probe l, warp, sensors, eng */
	int delay;		/* how long 'till we blow up? */
	int id;			/* Unique identifier */
	int (*strategy)();	/* Which strategy to use */
} ;

/*
 * note that probes act like torpedos
 * but have targets; torps only have
 * courses
 */
struct torpedo {
	struct ship *from;	/* pointer to ship they're from */
	int x, y;		/* coords of object */
	int course;		/* where it's going */
	float speed;		/* how fast we're moving */
	float newspeed;		/* what our target speed is */
	struct ship *target;	/* who we're aimed at */
	int fuel;		/* how many antimatter pods it has */
	int timedelay;		/* time clicks left before detonation */
	int prox;		/* proximity fuse */
	int id;			/* Unique identifier */
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
	float shield;
	float crew;
	float weapon;
	struct dam stats[4];
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
