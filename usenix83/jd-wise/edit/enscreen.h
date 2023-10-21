/*
 * return conditions for enscreen
 */

#define offthetop	257
#define offthebottom	258
#define offtheright	259
#define offtheleft	260
#define en_eof	261


/* the cursor position on return */
int lastcol,lastline;

/*
 * return conditions for commands
 */
#define co_normal	0
#define co_inserting	1
#define co_shell	2
#define co_find		3
#define co_esc		4
#define co_help		5
#define co_more		6

int	*insertadr;	/* address of line at which insertion is taking place */
