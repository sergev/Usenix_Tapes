
#include <curses.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <utmp.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/un.h>

#define	MAX_USERS		12
#define ESC			27
#define RET			'\n'
#define SPC			' '
#define BEGIN_SECRET		19
#define END_SECRET		20

WINDOW	*SCREEN;		/* a full-size work area	*/
WINDOW	*LINE;			/* bottom line of screen	*/

FILE	*fp, *fopen();

struct	pw__user		/* pwrite user structure	*/
{
	WINDOW	*__win1;	/* user's border window		*/
	WINDOW	*__win2;	/* user's inner window		*/
	char	__file[32];	/* user's socket file name	*/
	char	__tty[8];	/* user's tty number (tty??)	*/
	char	__username[32];	/* user's username		*/
	int	__sock;		/* user's socket descriptor	*/
	int	__connected;	/* is this user connected?	*/
	int	__dead;		/* is this connection dead?	*/
	int	__secret;	/* is currently sending secret  */
};
struct	pw__user T[MAX_USERS];	/* I am T[0]			*/

struct	corners
{
	int	C_y;		/* y coordinate or this corner	*/
	int	C_x;		/* x coordinate or this corner	*/
	int	C_under;	/* what was under this corner	*/
};
struct	corners C[4];
int	CURRENT_WINDOW;		/* The current window		*/

struct	def__loc		/* default locations of wins	*/
{
	int	D_y;		/* default row of this window	*/
	int	D_x;		/* default col			*/
	int	D_ysize;	/* default y size		*/
	int	D_xsize;	/* default x size		*/
};
struct	def__loc D[MAX_USERS];	/* I am D[0]			*/

struct	who__list		/* who_list structure		*/
{
	char	W_tty[8];	/* tty				*/
	char	W_username[32];	/* username			*/
};
struct	who__list W[64];	/* The who_list			*/
int	NUM_W;			/* # of users on the system	*/

char	STRING[BUFSIZ];		/* General purpose string	*/
int	NUM_T;			/* # of users you have		*/
int	NUM_C;			/* # of users connectd to you	*/
int	ACTIVE_MASK;		/* mask of active descriptors	*/
int	SMART;			/* TRUE if terminal is smart	*/
int	SNOOPER;		/* T element for the Snooper	*/

int	errno;			/* standard error noumber	*/
int	sock_io();		/* a place to go on SIGURG	*/
int	goodbye();		/* a place to go and die	*/
int	_pipe_();		/* "The Universe is defective"	*/
int	_death_();		/* program bombed badley!	*/

struct	timeval TIMEOUT;	/* timeout structure		*/

extern	char *enter_line();	/* input routine		*/

