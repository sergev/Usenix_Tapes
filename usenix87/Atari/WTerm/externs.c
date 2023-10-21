#define EXTERN 1
#include "wterm.h"

int	Handle;
int	Ix, Iy, Iw, Ih; /* inner size of window */

int	Xdesk, Ydesk, Wdesk, Hdesk;
int	CharW, CharH;	/* character cell width and height */
int	Baseline;	/* Bottom line relative to baseline */

char	Screen[N_ROW][1+N_COL];		/* image of whole screen */
int	Scr_col, Scr_row;
int	Sysl_row, Sysl_col, Sysline;

int	command, comd_pos;		/* used in screen.c mostly */
int	Half;				/* Half/full duplex */
int	T_mode, T_draw, T_count;	/* used in TEK 4010 emulation */
int	T_x, T_y;			/* current X and Y graphics pos */

int	contrl[12];
int	intin[128];
int	ptsin[128];
int	intout[128];
int	ptsout[128];
