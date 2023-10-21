/*
 * TREK73: defines.h
 *
 * Defines for TREK73
 *
 */

/* Globals externals */
extern char *strcpy(), *gets();
extern char *Gets();
extern float rectify(), bearing();


#ifdef BSD
extern long random();
#endif
#ifdef SYSV
#define random()	((long)(rand()))
#define srandom(seed)	(srand((unsigned)(seed)))
#endif

#define is_dead(sp, sys)	((sp)->status[sys] == 100)
#define randm(x)		((int)random() % (x) + 1)
#define syswork(sp, sys)	(randm(100) > (sp)->status[sys])
#define toradians(x)		((double)(x)*.0174533)
#define todegrees(x)		((double)(x)*57.2958)

#define betw(i, j, k)	(((j) < (i)) && ((i) < (k)))
#define min(x, y)	((x) < (y) ? (x) : (y))
#define max(x, y)	((x) > (y) ? (x) : (y))

#define cansee(x)	((x)->cloaking != C_ON)
#define cantsee(x)	((x)->cloaking == C_ON)

#define plural(n)	(((n) > 1) ? "s" : "")

#ifndef NULL
#define NULL	0
#endif

/*
 * for the item linked list
 */
#define I_UNDEFINED	0
#define I_SHIP		1
#define I_TORPEDO	2
#define I_PROBE		3
#define I_ENG		4

/*
 * for the ship status word
 */
#define S_COMP		0
#define S_SENSOR	1
#define S_PROBE		2
#define S_WARP		3
#define S_ENG		4
#define S_DEAD		5
#define S_SURRENDER	6
/* The following define must also be in structs.h */
#define S_NUMSYSTEMS	4	/* Number of systems with damage descriptions */
#define MAXSYSTEMS	7

/*
 * for the status message turn off array (shutup[])
 */
#define DISENGAGE	1			/* Autopilot disengaging */
#define SHIELDSF	2			/* Shields fluctuating */
#define PHASERS		3		/* Phasers disengaging */
#define TUBES		(PHASERS + MAXPHASERS)	/* Tubes disengaging */
#define SURRENDER	(TUBES + MAXTUBES)	/* Flag for enemy surrender */
#define SURRENDERP	20			/* Flag for our surrender */
#define PLAYDEAD	21			/* Flag for playing dead */
#define CORBOMITE	22			/* Flag for corbomite bluff */
#define BURNOUT		23			/* Flag for warp burnout */
#define HIGHSHUTUP	(BURNOUT + 10)		/* Burnout + 10 */

/*
 * Multiplier for shield 1
 */
#define SHIELD1		1.5

/*
 * Defines for the play status word
 */
#define NORMAL		000
#define F_SURRENDER	001
#define E_SURRENDER	002

/*
 * Phaser statuses
 */
#define P_NORMAL	000
#define P_DAMAGED	001
#define P_FIRING	002

/*
 * Tube statuses
 */
#define T_NORMAL	000
#define T_DAMAGED	001
#define T_FIRING	002

/*
 * Probe launcher status
 */
#define PR_NORMAL	000
#define PR_LAUNCHING	001
#define PR_DETONATE	002
#define PR_LOCK		004

/*
 * Cloaking device status / capability
 */
#define C_NONE		000
#define C_OFF		001
#define C_ON		002
#define CLOAK_DELAY	2	/*
				 * Number of turns after cloak is
				 * dropped before it can be reactivated
				 */

/*
 * For the damage routine
 */
#define D_PHASER	0
#define D_ANTIMATTER	1

/*
 * Some necessary constants
 */
#define HIT_PER_POD	5
#define PROX_PER_POD	50

/*
 * A handy little routine
 */
#define MKNODE(cast, star, number) (cast star)malloc(sizeof(cast) * number)

/*
 * Definitions for the bad guys
 */
#define MAXESHIPS	9
#define MAXFEDS		9
#define MAXECAPS	8
#define MAXFOERACES	9

/*
 * For the different ship classes
 */
#define MAXSHIPCLASS	4
#define MAXPHASERS	11
#define MAXTUBES	11

/*
 * Turn costs (either takes a turn or is a free command)
 */
#define TURN	1
#define	FREE	0

/*
 * Type of torpedo object (can be probe, torpedo, or engineering)
 */
#define TP_TORPEDO	0
#define	TP_PROBE	1
#define	TP_ENGINEERING	2

/*
 * Defines for the end of game calls to warn and final
 */
#define FIN_F_LOSE	0
#define FIN_E_LOSE	1
#define FIN_TACTICAL	2
#define FIN_F_SURRENDER	3
#define FIN_E_SURRENDER	4
#define FIN_COMPLETE	5
#define QUIT		6

/*
 * Number of items we allow in space.
 */
#define HIGHSLOT	300

/*
 * Trace flags
 */
#define TR_OFF		0
#define TR_ON		1

/*
 * Some hard constants
 */
#define MIN_PHASER_SPREAD	10
#define MAX_PHASER_SPREAD	45
#define MAX_PHASER_RANGE	1000
#define MAX_PHASER_CHARGE	10.
#define MIN_PHASER_DRAIN	-MAX_PHASER_CHARGE
#define MAX_PHASER_DRAIN	MAX_PHASER_CHARGE
#define MAX_TUBE_CHARGE		10
#define MAX_TUBE_PROX		500	/* 50 times the number of pods */
#define MAX_TUBE_SPEED		12
#define MAX_TUBE_TIME		10.
#define MAX_PROBE_DELAY		15
#define MIN_PROBE_CHARGE	10
#define MIN_PROBE_PROX		50
#define MIN_SENSOR_RANGE	100
#define MAX_SENSOR_RANGE	50000

#define INIT_P_SPREAD		MIN_PHASER_SPREAD
#define INIT_P_LOAD		MAX_PHASER_CHARGE
#define INIT_P_DRAIN		MAX_PHASER_DRAIN
#define INIT_P_PERCENT		100
#define INIT_T_LOAD		0
#define INIT_T_PROX		200
#define INIT_T_TIME		MAX_TUBE_TIME
#define INIT_T_SPEED		MAX_TUBE_SPEED
