/*
** vn news reader.
**
** tty_set.c - interface to ioctl (system tty interface)
**
** see copyright disclaimer / history in vn.c source file
*/

#ifdef SYSV
#include <termio.h>
#else
#include <sgtty.h>
#endif

#include "tty.h"

extern char Erasekey,Killkey;

#ifdef SYSV
static struct termio C_tp, O_tp;
#else
static struct sgttyb C_tp;
static unsigned short O_lflag;
#endif

static unsigned S_flag=0;
static int R_ignore=0;		/* up/down counter of reset calls to ignore */

#define IO_GOT 1	/* have polled for original terminal mode */
#define IO_RAW 2	/* in RAW (CBREAK actually) mode */

/*
	tty_set handles ioctl calls.  SAVEMODE, RESTORE are used around
	system calls and interrupts to assure cooked mode, and restore
	raw if raw on SAVEMODE.  The pair results in no calls to ioctl
	if we are cooked already when SAVEMODE is called, and may be nested,
	provided we desire no "restore" of cooked mode after restoring raw.

	When we get the original terminal mode, we also save erase and kill.

	sig_set makes an ioctl call to get process group leader.  Otherwise
	ioctl calls should come through here.
*/
tty_set(cmd)
int cmd;
{
	int rc;
	unsigned mask;

	switch (cmd)
	{
	case BACKSTOP:
#ifdef JOBCONTROL
		if ((rc = ioctl(1,TIOCLGET,&mask)) != 0)
			break;
		mask |= LTOSTOP;
		rc = ioctl(1,TIOCLSET,&mask);
#else
		rc = 0;
#endif
		break;
	case RAWMODE:
		if ((S_flag & IO_RAW) != 0)
		{
			rc = 0;
			break;
		}
		if ((S_flag & IO_GOT) == 0)
		{
			/* Save original modes, get erase / kill */
#ifdef SYSV
			rc = ioctl(0,TCGETA,&C_tp);
			O_tp = C_tp;
			Erasekey = C_tp.c_cc[VERASE];
			Killkey = C_tp.c_cc[VKILL];
#else
			rc = ioctl(0,TIOCGETP,&C_tp);
			O_lflag = C_tp.sg_flags;
			Erasekey = C_tp.sg_erase;
			Killkey = C_tp.sg_kill;
#endif
		}
#ifdef SYSV
		C_tp.c_lflag &= ~(ECHO | ICANON);
		C_tp.c_cc[VMIN] = 1;
		rc = ioctl(0,TCSETAW,&C_tp);
#else
		C_tp.sg_flags |= CBREAK;
		C_tp.sg_flags &= ~ECHO;
		rc = ioctl(0,TIOCSETP,&C_tp);
#endif
		S_flag = IO_GOT|IO_RAW;
		break;
	case COOKED:
		if ((S_flag & IO_RAW) != 0)
		{
#ifdef SYSV
			C_tp = O_tp;
			rc = ioctl(0,TCSETAW,&C_tp);
#else
			C_tp.sg_flags = O_lflag;
			rc = ioctl(0,TIOCSETP,&C_tp);
#endif
			S_flag &= ~IO_RAW;
		}
		else
			rc = 0;
		break;
	case SAVEMODE:
		if ((S_flag & IO_RAW) != 0)
		{
			tty_set(COOKED);
			R_ignore = 0;
		}
		else
			++R_ignore;
		rc = 0;
		break;
	case RESTORE:
		if (R_ignore <= 0)
		{
			tty_set(RAWMODE);
		}
		else
			--R_ignore;
		rc = 0;
		break;
	default:
		rc = -1;
	}
	if (rc < 0)
		printex ("ioctl failure, tty_set: %d",cmd);
}
