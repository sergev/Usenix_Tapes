/*
 *	openpty - open a pseudo-terminal
 *
 * The first time that the routine is called, the device directory is
 * searched and a list of all candidate pseudo-terminals is compiled.
 * Candidates are defined to be those entries in "/dev" whose names
 * (1) are the same length as PTY_PROTO and (2) start with the
 * initial string PTY_PREFIX.  Further, the master and slave sides
 * must both exist.
 *
 * openpty() attempts to find an unused pseudo-terminal from the list
 * of candidates.  If one is found, the master and slave sides are
 * opened and the file descriptors and names of these two devices are
 * returned in a "ptydesc" structure.  (The address of this structure
 * is supplied by the caller.  Zero is returned if openpty() was
 * successful, -1 is returned if no pty could be found.
 *
 *    written by John Bruner <unmvax!unm-la!lanl!cmcl2!seismo!mordor!jdb>
 */

#include <sys/types.h>
#include <sys/dir.h>
#include <fcntl.h>
#include <strings.h>
#include "openpty.h"

#define	DEV_DIR		"/dev"		/* directory where devices live */
#define	PT_INDEX	(sizeof DEV_DIR)	/* location of 'p' in "pty" */

#define	PTY_PROTO	"ptyp0"		/* prototype for pty names */
#define	PTY_PREFIX	"pty"		/* prefix required for name of pty */

struct ptyinfo {
	struct ptyinfo	*pi_next;
	char		*pi_pty;
	char		*pi_tty;
};

static struct ptyinfo *ptylist;

extern char *malloc();

static char * devname(name)
char *name;
{
	register char *fullname;

	/*
	 * Construct the full name of a device in DEV_DIR.  Returns
	 * NULL if it failed (because malloc() failed).
	 */

	fullname = malloc((unsigned)(sizeof DEV_DIR + 1 + strlen(name)));
	if (fullname != NULL) {
		(void)strcpy(fullname, DEV_DIR);
		(void)strcat(fullname, "/");
		(void)strcat(fullname, name);
	}
	return(fullname);
}

static isapty(dp)
struct direct *dp;
{
	static struct ptyinfo *pi;

	/*
	 * We don't care about the gory details of the directory entry.
	 * Instead, what we really want is an array of pointers to
	 * device names (with DEV_DIR prepended).  Therefore, we create
	 * this array ourselves and tell scandir() to ignore every
	 * directory entry.
	 *
	 * If malloc() fails, the current directory entry is ignored.
	 */

	if (pi == NULL &&
	    (pi = (struct ptyinfo *)malloc((unsigned)sizeof *pi)) == NULL)
		return(0);
		
	if (strlen(dp->d_name) == sizeof PTY_PROTO - 1 &&
	    strncmp(dp->d_name, PTY_PREFIX, sizeof PTY_PREFIX - 1) == 0) {
		pi->pi_pty = devname(dp->d_name);
		if (pi->pi_pty == NULL)
			return(0);
		pi->pi_tty = malloc((unsigned)(strlen(pi->pi_pty) + 1));
		if (pi->pi_tty == NULL) {
			free(pi->pi_pty);
			return(0);
		}
		(void)strcpy(pi->pi_tty, pi->pi_pty);
		pi->pi_tty[PT_INDEX] = 't';
		if (access(pi->pi_pty, 0) == 0 && access(pi->pi_tty, 0) == 0) {
			pi->pi_next = ptylist;
			ptylist = pi;
			pi = NULL;
		} else {
			free(pi->pi_pty);
			free(pi->pi_tty);
		}
	}
	return(0);
}

_openpty(pt)
struct ptydesc *pt;
{
	register struct ptyinfo *pi;
	static int fail;
	auto struct direct **dirlist;
	extern char *re_comp();
	extern int alphasort();

	/*
	 * If scandir() fails or no possible pty's are found, then "fail"
	 * is set non-zero.  If "fail" is non-zero then the routine bombs
	 * out immediately.  Otherwise, the list of candidates is examined
	 * starting with the entry following the last one chosen.
	 */

	if (fail)
		return(-1);

	if (!ptylist) {		/* first time */
		if (scandir(DEV_DIR, &dirlist, isapty, alphasort) < 0 ||
		    ptylist == NULL) {
			fail = 1;
			return(-1);
		}
		for (pi=ptylist; pi->pi_next; pi=pi->pi_next)
			;
		pi->pi_next = ptylist;	/* make the list circular */
	}

	pi = ptylist;
	do {
		if ((pt->pt_pfd = open(pi->pi_pty, O_RDWR)) >= 0) {
			if ((pt->pt_tfd = open(pi->pi_tty, O_RDWR)) >= 0) {
				ptylist = pi->pi_next;
				pt->pt_pname = pi->pi_pty;
				pt->pt_tname = pi->pi_tty;
				return(0);
			} else
				(void)close(pt->pt_pfd);
		}
		pi = pi->pi_next;
	} while (pi != ptylist);
	return(-1);
}

