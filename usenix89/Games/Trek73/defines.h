/*
 * TREK73: defines.h
 *
 * Defines for TREK73
 *
 */

/* Globals externals */
extern char *malloc(), *strcpy(), *gets();
extern long random();

#define toradians(x) ((float)(x)*.0174533)
#define todegrees(x) ((float)(x)*57.2958)
#define randm(x) (((int)random() % (x)) + 1)

#define min(x, y) ((x) < (y) ? (x) : (y))
#define max(x, y) ((x) > (y) ? (x) : (y))

#ifndef NULL
#define NULL 0
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
#define S_NORMAL	000
#define S_COMP		001
#define S_SENSOR	002
#define S_PROBE		004
#define S_WARP		010
#define S_ENG		020
#define S_DEAD		040
#define S_SURRENDER	0100

/*
 * for the status message turn off array
 */
#define DISENGAGE	1
#define SHIELDSF	2
#define PHASERS		3
#define TUBES		7
#define SURRENDER	13
#define SURRENDERP	14
#define PLAYDEAD	15
#define CORBOMITE	16
#define BURNOUT		17
#define HIGHSHUTUP	27

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
 * Ship search routine directives
 */
#define ALL		0
#define ENEMYONLY	1

#define MKNODE(cast, star, number) (cast star)malloc(sizeof(cast) * number)

#define MAXBADS 9
#define MAXFEDS 9
#define MAXENCOMM 9
#define MAXFOERACES 9
