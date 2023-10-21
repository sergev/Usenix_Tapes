/*
 *	Copyright 1987, Jonathan Biggar
 *
 *	Permission is given to copy and distribute for non-profit purposes.
 *
 */

#ifndef lint
static char *rcsh = "$header$ Copyright 1986 Jonathan Biggar";
#endif !lint

/*
 * $log$
 */

#define UPDATE_FREQ		100000	/* microsecs between updates */
#define ROT_SPEED		5	/* must divide 360 evenly */
#define MAX_VELOCITY		15	/* maximum speed */
#define MISSILE_VELOCITY	25	/* missile delta velocity */
#define KILL_RADIUS		10	/* radius of kill for missiles */
#define DIE_PERIOD		20	/* period a ship dies in */
#define MIS_P_PLAYER		5
#define SHIELD_TIME		20	/* time shield will protect */
#define SHIELD_RECHARGE		100	/* time between shield activations */
#define ASTEROID_MIN_PER	100	/* minimum time before new asteroid */
#define ASTEROID_MAX_PER	300	/* maximum time before new asteroid */

#define MAX_PACKET	(1500 - 20 - 8) /* for ip and udp headers */
#define MAX_OBJECTS	((MAX_PACKET - sizeof(struct header))/sizeof(Object))
#define OTHER_OBJECTS	10
#define MAX_PLAYERS	((MAX_OBJECTS-OTHER_OBJECTS)/(MIS_P_PLAYER+1))
#define MISSILE_START	(MAX_PLAYERS+OTHER_OBJECTS)
#define NUM_MISSILES	MAX_PLAYERS * MIS_P_PLAYER

#define S_X_MAX		1152
#define S_Y_MAX		900
#define BORDER		5
#define X_MAX		990
#define Y_MAX		(S_Y_MAX-BORDER*2)
#define P_X_MAX		(S_X_MAX-X_MAX-BORDER*3)
#define P_Y_MAX		Y_MAX

#define mysin(x)	sin_tab[x]
#define mycos(x)	cos_tab[x]

typedef struct object {
    u_char	type;
    u_char	flags;
    short	x_pos;
    short	y_pos;
    short	rot;
} Object;

/* object.type values */

#define SHIP_T		0
#define MISSILE_T	1
#define SUN_T		2
#define ASTEROID_T	3

/* objects.flags values */

#define ALIVE		0x1
#define DYING		0x2
#define THRUSTING	0x4
#define SHIELD		0x8

/* udp packet header */

struct header {
    u_char	type;
    u_char	ship_num;
};

/* header.type values */

#define HELLO		0
#define YOU_ARE		1
#define CONTROLS	2
#define DISPLAY		4
#define BYE		5
#define USERS		6

#define NAME_SIZE	40

typedef struct player {
    char	name[NAME_SIZE];
    int		score;
} Player;

typedef struct controls {
    struct header head;
    u_int	left:1;
    u_int	thrust:1;
    u_int	right:1;
    u_int	fire:1;
    u_int	shield:1;
    u_char	seq;
} Controls;
