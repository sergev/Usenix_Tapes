/*
 * TREK73: externs.h
 *
 * External declarations for TREK73
 *
 */

/* UNIX include files needed for most (if not all) of the C files */
#include <stdio.h>
#ifdef SYSV
#include <string.h>
#endif SYSV
#ifdef BSD
#include <strings.h>
#endif BSD
#include <math.h>

/* UNIX extern declarations of functions used which do not
   return int plus any extern variable declarations    */
extern char	*getenv(), *malloc();
extern long	time();
extern unsigned	sleep(), alarm();
extern void	exit(), free(), perror();
#ifdef SYSV
extern void	srand();
#endif SYSV
extern char	*optarg;

/*
 * Now continue with declarations specific to TREK73
 */

#include "structs.h"

extern char	*Gets(), *vowelstr();
extern char	*sysname[S_NUMSYSTEMS];
extern char	*statmsg[S_NUMSYSTEMS + 1];
extern char	*feds[];
extern char	*options;

extern float	init_p_turn[MAXPHASERS][MAXPHASERS];
extern float	init_t_turn[MAXTUBES][MAXTUBES];
extern float	segment, timeperturn;
extern float	rectify(), bearing();

extern char	home[256];
extern char	savefile[256];
extern char	captain[30];
extern char	class[3];
extern char	com[30];
extern char	com_delay[6];
extern char	empire[30];
extern char	engineer[30];
extern char	foeclass[3];
extern char	foename[30];
extern char	foerace[30];
extern char	foestype[30];
extern char	helmsman[30];
extern char	nav[30];
extern char	racename[20];
extern char	savefile[256];
extern char	science[30];
extern char	sex[20];
extern char	shipname[30];
extern char	shutup[HIGHSHUTUP];
extern char	slots[HIGHSLOT];
extern char	title[9];

extern int	cmdarraysize;
extern int	corbomite;
extern int	defenseless;
extern int	enemynum;
extern int	global;
extern int	high_command;
extern int	reengaged;
extern int	restart;
extern int	shipnum;
extern int	silly;
extern int	surrender;
extern int	surrenderp;
extern int	teletype;
extern int	terse;
extern int	time_delay;
extern int	trace;
extern char	can_cloak;
extern double	o_bpv;
extern double	e_bpv;

extern struct ship_stat	us;
extern struct ship_stat	them;

extern int 	(*strategies[])();
extern int	rangefind();

extern struct cmd		*scancmd(), cmds[];
extern struct race_info		aliens[MAXFOERACES];
extern struct damage		p_damage, a_damage;
extern struct list		*newitem(), head, *tail;
extern struct ship		*shiplist[10], *ship_name();
extern struct ship_stat		stats[];
