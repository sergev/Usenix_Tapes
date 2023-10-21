/*
 *  Top - a top users display for Berkeley Unix
 *
 *  This file defines the locations on tne screen for various parts of the
 *  display.  These definitions are used by the routines in "display.c" for
 *  cursor addressing.
 */

#define  x_lastpid	10
#define  y_lastpid	0
#define  x_loadave	33
#define  y_loadave	0
#define  x_procstate	0
#define  y_procstate	1
#define  x_brkdn	14
#define  y_brkdn	1
#define  x_realmem	8
#define  x_virtmem	28
#define  x_free		51
#define  y_mem		3
#define  x_header	0
#define  y_header	5
#define  x_idlecursor	0
#define  y_idlecursor	4
#define  y_procs	6
#define  x_p_pid	0
#define  x_p_user	6
#define  x_p_pri	15
#define  x_p_nice	20
#define  x_p_size	25
#define  x_p_res	31
#define  x_p_state	37
#define  x_p_time	43
#define  x_p_wcpu	50
#define  x_p_cpu	57
#define  x_p_command	64

#define  y_cpustates	2
