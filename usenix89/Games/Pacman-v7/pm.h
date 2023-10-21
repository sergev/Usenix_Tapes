/*
** pm.h -	all pm source files include this
**
**	[pm by Peter Costantinidis, Jr. @ University of California at Davis]
*/

/*
** defines that control things
*/
#define	PATTERNS		/* let there be patterns!!!		*/
#define	FASTER			/* let the monsters move fast (monsters.c) */
#define	MAX_UPDATE		/* more refreshes			*/
#ifdef	BSD42|BSD43|BSD29
#	define	BAD_OVERLAY	/* bug in curses (overlay.c)		*/
#endif
#define	MYTIMER			/* quite reliable			*/

#define	BYTE_SIZE	8	/* # of bits in a byte			*/


#include <curses.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/timeb.h>
#ifdef	BSD42|BSD43
#	include <sys/time.h>
#else
#	include <time.h>
#endif
#ifdef	MYTIMER
#	undef	ITIMER_REAL
#endif

/*
** bound defines
*/
#define	TOP		1
#define	LEFT		0
#define	RIGHT		52

/*
** function status returns
*/
#define	ERROR		-1
#define	STOP		-2
#define	QUIT		-3
#define	FINE		-4
#define	BAD		-5

/*
** input defines
*/
#define	MUP		'k'		/* move up			*/
#define	MDOWN		'j'		/* move down			*/
#define	MLEFT		'h'		/* move left			*/
#define	MRIGHT		'l'		/* move right			*/
#define	MSTOP		' '		/* stop movement		*/
#define	MKP_UP		'8'		/* key pad move up		*/
#define	MKP_DOWN	'2'		/* key pad move down		*/
#define	MKP_LEFT	'4'		/* key pad move left		*/
#define	MKP_RIGHT	'6'		/* key pad move right		*/
#define	MKP_STOP	'5'		/* key pad stop movement	*/
#define	MQUIT		'q'		/* quit game			*/
#define	MREDRAW		CTRL(L)		/* redraw the screen		*/
#define	MSHELL		'!'		/* shell escape			*/
#define	MHELP		'?'		/* give list of commands	*/
#define	MFAST		'f'		/* toggle looping in make_move	*/
#define	MQUIET		'b'		/* toggle beeping		*/
#define	MPAUSE		'p'		/* hang on getchar()		*/
#define	MHUH		CTRL(R)		/* reprint last message		*/
#define	MNULL		'\0'
/*
** special input defines (cheating)
*/
#define	MWIZARD		CTRL(P)		/* request to become wizard	*/
#define	MSTATUS		's'		/* print debugging info		*/
#define	MMONS		'm'		/* print monster  ""    ""	*/
#define	MPM		'@'		/* request for more pm's	*/
#define	MUP_LVL		CTRL(U)		/* up a level			*/
#define	MDN_LVL		CTRL(D)		/* down a level			*/
#define	MEAT		CTRL(E)		/* make them eatable		*/
#define	MMEAN		CTRL(M)		/* make them non-eatable	*/
#define	MSLOW		'r'		/* change null padding		*/

/*
** wizardly defines
*/
#define	W_PASSWD	"mTEsRrd1.h/"	/* any guesses?			*/
#define	SALT		"mT"		/* look familiar anyone?	*/

/*
** various characters on the screen
** the underscores mean the "submissive" ghost associated with *
*/
#define	PM		'@'
#define HARPO		'H'
#define _HARPO		'h'
#define GROUCHO		'G'
#define _GROUCHO	'g'
#define ZEPPO		'Z'
#define _ZEPPO		'z'
#define CHICO		'C'
#define _CHICO		'c'
#define	BLOCK		'#'
#define	DOT		'.'
#define	TUNNEL		'-'
#define	ENERGY		'*'
#define	EMPTY		' '
#define	DOOR		'='

/*
** monster attributes
*/
#define	FAST		001		/* three different (relative) speeds */
#define	MED		002
#define	SLOW		004
#define	SMART		010		/* three different (relative) smarts */
#define	NORMAL		020
#define	DUMB		040

/*
** to keep lint quiet
** used primarily for the "Hit return to continue"'s
*/
#ifdef	LINT
#	define	trash(x)	_trash_ = x
#else
#	define	trash(x)	x
#endif

/*
** miscellaneous definition
*/
#define	BELL		'\07'
#define	SPEED		15
#define	MIN_BAUD	1200		/* must alter bauds[] if this value is
					** changed */
#define	EAT_PAUSE	((unss) 20)	/* used when pm is eaten */
#define	MAX_DIRS	4
#define	MAX_LEVEL	13
#define	MAX_ENERGY	4
#define	MAX_DOTS	208
#define	MAX_MONS	4
#define	MAX_PMS		4
#define	MAX_BLINKS	30		/* number of warning blinks	*/
#define	MAX_CNT		50		/* kind of useless, (see get_move) */
#define	TUNN_TIME	2
#define TUNN_ROW	11
#define	DOOR_COL	26		/* column where door to monsters is */
#define	V_DOT		10
#define	V_ENERGY	50
#define	ALARM_TIME	10		/* messages get erased every 10 sec */
#define	BONUS		10000		/* get a free pm with 10000 score*/
#define	CMASK		0177		/* used to strip garbage from what
					** inch() returns (when in stand...)
					*/
/*
** useful defines to make core more terse
*/
#define	when		break; case
#define otherwise	break; default

#ifdef	PATTERNS
#	define	SEED		(uid+level)	/* thier uid */
#else
#	define	SEED		get_seed()	/* thier uid */
#endif

/*
** stuff to do with the score routines
*/
#define	DEFAULT_SH	"/bin/sh"	/* default shell */
#define	MAX_SCORES	10		/*  maximum scores on pm roll */
#define	SCR_SIZE	(sizeof(score))
#define	NAME_SIZE	50		/* biggest name allowed on roll	*/
#define	MODE		0644		/* the mode of the PM_ROLL	*/
			/* you devil */
#define	MAX_BYTES	4096		/* byte size of user file	*/
#define	MAX_USERS	(MAX_BYTES*BYTE_SIZE)/* number of uid's (bit size of
					** PM_USER)			*/

/*
** flag stuff for the above
*/
#define	FL_DIE		0		/* indicates he died		*/
#define	FL_QUIT		1		/* indicates he quit		*/

/*
** misc definitions
*/
#define	unss	unsigned short

/*
** structure definitions
*/
typedef	struct
{
	int	x, y;
} coord;

typedef	struct				/* monster description		*/
{
	coord	mo_pos;			/* where it is at		*/
	char	mo_inch;		/* what it is on top of		*/
	bool	mo_run,			/* TRUE if it is eatable	*/
		mo_tunn,		/* TRUE if it is in a tunnel	*/
		mo_eaten,		/* TRUE if eaten		*/
		mo_inside;		/* TRUE if inside		*/
	char	mo_ch,			/* current move			*/
		mo_name;		/* name (letter) of monster	*/
	int	mo_cnt,			/* how many times to do mo_ch	*/
		mo_extunn,		/* how long left in tunnel	*/
		mo_attrib;		/* monsters characteristics	*/
} mons;

typedef	struct
{
	int	sc_uid;			/* player's uid			*/
	long	sc_score;		/* player's score		*/
	int	sc_level;		/* how deep the player went	*/
	int	sc_flags;		/* misc. info			*/
	char	sc_name[NAME_SIZE],	/* player's name		*/
		sc_mons;		/* monster's name		*/
} score;

/*
** psuedo functions
*/
/*
** need to strip out the garbage that inch() returns, i only want
** the single character, and i think that it gets messed up when
** that character is in background
*/
#define	INCH()		(inch() & CMASK)
#define	abs(x)		(x < 0 ? - x : x)
#define	MVADDCH(p, c)	mvaddch(p.y, p.x, c)
#define	TF(x)		(x ? "True" : "False")
#define	DIST()		rnd(4, 15)
#define	IS_FRUIT(c)	(c == fr_ch)	/* ...			*/
#define	SLOWER()	slow(FALSE)
#ifndef	CTRL
#	define	CTRL(ch)	('ch' & '\037')
#endif
#define	AT(pos1, pos2)	(((pos1)->x == (pos2)->x) && ((pos1)->y == (pos2)->y))
#define	OUTOFTUNN(pos)	(((pos)->y != 0) && ((pos)->y != 52))
#define	beep()		putchar(BELL)
#define	RN		(((seed = (seed * 11109) + 13849) & 0xfff) >> 1)
#define	randomize(i)	seed = i
#define	flush()		raw(),noraw()
#define	m_erase(mon)	mvaddch(mon.mo_pos.y, mon.mo_pos.x, mon.mo_inch)
#define	draw()		ftime(&_tp), refresh(), delay()

extern	coord	pm_pos;
extern	struct	timeb	_tp;
extern	int	pm_tunn, pm_extunn, d_left, e_left, level, fr_val,
		fruit_val[], pms_left, pm_bonus, pm_eaten, pm_run,
		mons_eaten, mons_val[], eat_times[], timer, was_wiz, is_wiz,
		timeit, quiet, fast, uid, seed, bauds[], wizard_uid;
extern	char	*argv0, fruit[], fruit_eaten[], fr_ch, ch, oldch,
		newch, moves[], *mesg, **environ, *pm_user, *pm_roll,
		baud;
#ifdef	LINT
extern	char	_trash_;
#endif
extern	long	thescore, hi_score, demon, move_cntr, chcnt;
extern	mons	ghosts[];
extern	mons	*h, *g, *c, *z;
extern	char	_putchar(), *crypt(), *strcpy();
extern	int	getuid();
extern	off_t	lseek();

/*
** local functions
*/
extern	void	add_fruit(), aggressive(), chg_lvl(), check_scrs(),
		commands(), delay(), die(), directions(),
		doadd(), draw_screen(), eat_pm(), init(),
		m_eat_pm(),
		mons_init(), m_move(), msg(), msg_erase(), msleep(),
		mv_mon(), new_screen(), old_screen(), p_barriers(), p_dots(),
		p_energizers(), p_fruits(), p_info(), p_monsters(),
		p_pm(), p_pms(), p_scores(), place_m(), pm_eat_m(),
		pmers(),
		print_scrs(), quit_it(), quitit(),
		re_msg(), redraw(), scores(), scrcpy(),
		shell(), slow(), slowness(), status(), strucpy(),
		submissive(), tombstone(), trap(), usage(), warning();
extern	int	_mv_mon(), can_see(), chk_pm_user(), dir_int(), get_move(),
		is_mons(), is_safe(), m_is_safe(),
		make_moves(), moveit(), move_to(), pending(), rnd();
extern	char	gen_mv(), *get_pass(), int_dir(), lturn(), *mons_str(),
		opposite(), *punctrl(), rturn(), to_baud(), toletter(),
		tunn_look();
extern	long	get_hi_scr();

extern	mons	*wh_mons();

#ifdef	PATTERNS
extern	int	get_seed();
#endif
