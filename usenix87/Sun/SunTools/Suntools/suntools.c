#ifndef lint
static char sccsid[] = "@(#)suntools.c 1.2 85/03/13 SMI";
#endif

/*
* Sun Microsystems, Inc.
*/

/*
* Root window: Provides the background window for a screen.
*	Put up environment manager menu.
*/

/*

Modified by Don Libes, National Bureau of Standards, 9/25/85

Added the following:

Menu selections via keyboard
Su'd selections via shift key
User-specifiable primary menu name
User-specifiable root cursor
Various keywords: EXIT_NOCONFIRM, VERSION, HELP, KEYS

*/

#include <suntool/tool_hs.h>
#include <sys/ioctl.h>
#include <sys/dir.h>
#include <sys/file.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <pwd.h>
#include <suntool/menu.h>
#include <suntool/wmgr.h>
#include <sundev/kbd.h>		/* SHIFTMASK - DEL */
#include <ctype.h>
#include <suntool/icon_load.h>	/* icon_load_mpr for user cursor - DEL */

extern int      errno;

extern char    *malloc (), *calloc (), *getenv (), *strcpy (), *strncat (), *strncpy ();

static int      rootfd = 0,
                rootnumber;
static int      root_SIGCHLD,
                root_SIGWINCH;
static struct screen    screen;

static struct pixwin   *pixwin;

#define	ROOTMENUITEMS	20
#define	ROOTMENUFILE	"/usr/lib/rootmenu"
#define ROOTMENUNAME	"Suntools"

struct menuitem root_items[ROOTMENUITEMS];

struct menuitemstrings {
	char   *mis_prog;	/* program to call */
	char   *mis_args;	/* args to program */
}                       root_itemstrings[ROOTMENUITEMS];

char   *rootmenufile;
char   *rootmenuname;		/* DEL */

struct menu     wmgr_rootmenubody,
               *wmgr_rootmenu = &wmgr_rootmenubody;
struct stat_rec {
	char   *name;		/* Dynamically allocated menu file name */
	        time_t mftime;	/* Modified file time */
};
#define	MAX_MENUS	20
static struct stat_rec  stat_array[MAX_MENUS];
static int      menu_next;

#define	ROOTCOLOR_PATTERN	0
#define	ROOTCOLOR_FOREGROUND	1
#define	ROOTCOLOR_BACKGROUND	2
static  rootcolor = ROOTCOLOR_PATTERN;
/* Default cursor is a circle filled with stipple pattern. */
/* When or-ed against another stipple pattern, it changes constantly. */
/* Neat, huh?  - DEL */
static short    cursor_image[CUR_MAXIMAGEWORDS] = {
        0x0000,0x03E0,0x0E38,0x1224,0x288A,0x288A,0x6223,0x6223,
        0x4889,0x4889,0x6223,0x2222,0x288A,0x188C,0x0E38,0x03E0 };
mpr_static (cursor_pr, 8 * sizeof (cursor_image[0]),
	sizeof (cursor_image) / sizeof (cursor_image[0]), 1,
	cursor_image);

#define IS_ASCII_EVENT(x) (x>=ASCII_FIRST && x<=ASCII_LAST)
#define KEY_BUFFER_LENGTH	30
char    key_buffer[KEY_BUFFER_LENGTH] = "";
				/* keys coming directly to the root window */
int     key_buffer_length = 0;
#define KEYITEMS	50
struct menuitem key_items[KEYITEMS];
struct menuitemstrings  key_itemstrings[KEYITEMS];
struct menu     keymenubody = {
	0, 0, 0,
};

struct pixfont *sysfont;

#define SU_LABEL		"su"
int label_x, label_y;		/* where the label is on the screen */
struct rect su_rect;
struct pr_subregion su_bound;
struct pixrect *oldpr;		/* save old screen image here while we */
				/* scribble the SU_LABEL on it */

int     wants_su = FALSE;	/* true if user has requested setuid(0) */
int     oldruid, oldrgid;	/* save the originals here */
struct passwd *pwd;

main (argc, argv)
int     argc;
char  **argv;
{
	char    name[WIN_NAMESIZE],
	        setupfile[MAXNAMLEN];
	int     _root_sigchldcatcher (), _root_sigwinchcatcher ();
	int     donosetup = 0,
	        printname = 0;
	unsigned char   red[256],
	                green[256],
	                blue[256];
	struct pixrect *fb_pixrect;
	int     fullplanes = 255;

	/* 
	 * Parse cmd line.
	 */
	setupfile[0] = NULL;
	win_initscreenfromargv (&screen, argv);
	if (argv) {
		char  **args;

		for (args = ++argv; *args; args++) {
			if ((strcmp (*args, "-s") == 0) && *(args + 1)) {
				(void) strcpy (setupfile, *(args + 1));
				args++;
			}
			else if (strcmp (*args, "-F") == 0)
				rootcolor = ROOTCOLOR_FOREGROUND;
			else if (strcmp (*args, "-B") == 0)
				rootcolor = ROOTCOLOR_BACKGROUND;
			else if (strcmp (*args, "-P") == 0)
				rootcolor = ROOTCOLOR_PATTERN;
			else if (strcmp (*args, "-n") == 0)
				donosetup = 1;
			else if (strcmp (*args, "-p") == 0)
				printname = 1;
			else if (strcmp (*args, "-S") == 0)
				wants_su = 1;
			else if (argc == 2 && *args[0] != '-')/* 
				 * If only arg and not a flag then treat as
				 * setupfile (backward compatibility with 1.0).
				 */
				(void) strcpy (setupfile, *args);
		}
	}

	oldrgid = getgid ();
	oldruid = getuid ();

	if (NULL == (pwd = getpwnam("root"))) {
		fprintf(stderr,"couldn't find root in passwd file?\n");
		exit(1);
	}
	if (*pwd->pw_passwd != '\0') {
		if (wants_su) {
			if (geteuid() != 0) {
				fprintf(stderr,"suntools must be setuid(0)\n");
				exit(1);
			}
			if (strcmp(pwd->pw_passwd,
					crypt(getpass("Password: "),
						pwd->pw_passwd))) {
				fprintf(stderr,"Sorry\n");
				exit(1);
			}
		} else disable_su();
	}
	wants_su = FALSE;

	/* 
	 * Initialize root menu from menu file.
	 */
	if ((rootmenufile = getenv ("ROOTMENU")) == NULL)
		rootmenufile = ROOTMENUFILE;
	/* added changeable root menu name - DEL */
	if ((rootmenuname = getenv ("ROOTMENUNAME")) == NULL)
		rootmenuname = ROOTMENUNAME;
	if (wmgr_getrootmenu (rootmenuname, wmgr_rootmenu, rootmenufile,
				root_items, root_itemstrings, ROOTMENUITEMS) <= 0) {
		fprintf (stderr, "suntools: invalid root menu\n");
		exit (1);
	}
	/* 
	 * Set up signal catchers.
	 */
	(void) signal (SIGCHLD, _root_sigchldcatcher);
	(void) signal (SIGWINCH, _root_sigwinchcatcher);
	/* 
	 * Find out what colormap is so can restore later.
	 * Do now before call win_screennew which changes colormap.
	 */
	if (screen.scr_fbname[0] == NULL)
		strcpy (screen.scr_fbname, "/dev/fb");
	if ((fb_pixrect = pr_open (screen.scr_fbname)) == (struct pixrect      *) 0) {
		fprintf (stderr, "suntools: invalid frame buffer %s\n",
			screen.scr_fbname);
		exit (1);
	}
	                                                                        pr_getcolormap (fb_pixrect, 0, 256, red, green, blue);
	/* 
	 * Create root window
	 */
	if ((rootfd = win_screennew (&screen)) == -1) {
		perror ("suntools");
		exit (1);
	}
	
	init_cursor();

	if (rootcolor != ROOTCOLOR_PATTERN) {
		struct cursor   cursor;

		cursor.cur_shape = &cursor_pr;
		win_getcursor (rootfd, &cursor);
		cursor.cur_function = PIX_SRC ^ PIX_DST;

		win_setcursor (rootfd, &cursor);
	}
	win_screenget (rootfd, &screen);
	/* 
	 * Open pixwin.
	 */
	if ((pixwin = pw_open (rootfd)) == 0) {
		fprintf (stderr, "%s not available for window system usage\n",
				screen.scr_fbname);
		perror ("suntools");
		exit (1);
	}
	/* 
	 * Set up root's name in environment
	 */
	win_fdtoname (rootfd, name);
	rootnumber = win_nametonumber (name);
	we_setparentwindow (name);
	if (printname)
		fprintf (stderr, "suntools window name is %s\n", name);
	/* 
	 * Set up tool slot allocator
	 */
	wmgr_setrectalloc (rootfd, 200, 40,
			0, pixwin -> pw_pixrect -> pr_height - TOOL_ICONHEIGHT);
	/* 
	 * Setup tty parameters for all terminal emulators that will start.
	 */
	{
		int     tty_fd;
		tty_fd = open ("/dev/tty", O_RDWR, 0);
		if (tty_fd < 0)
			ttysw_saveparms (2);/* Try stderr */
		else {
			ttysw_saveparms (tty_fd);
			(void) close (tty_fd);
		}
	}

	/*
         * setup su hack - DEL
	 */

	/* find bounding box */
	su_bound.pr = pixwin -> pw_pixrect;
	su_bound.pos.x = 0;
	su_bound.pos.y = 0;

	sysfont = pw_pfsysopen ();
	pf_textbound (&su_bound, strlen (SU_LABEL), sysfont, SU_LABEL);
	su_rect.r_width = su_bound.size.x;
	su_rect.r_height = su_bound.size.y;

	/* create pixrect to save screen while scribbling on it */
	oldpr = mem_create (su_bound.size.x, su_bound.size.y, 1);

	/* 
	 * Draw background.
	 */
	_root_sigwinchhandler ();

	/* 
	 * Do initial window setup.
	 */
	if (!donosetup)
		_root_initialsetup (setupfile);

	/* 
	 * Do window management loop.
	 */
	_root_winmgr ();
	/* 
	 * Destroy screen sends SIGTERMs to all existing windows and
	 * wouldn't let any windows install themselves in the window tree.
	 * Calling process of win_screedestroy is spared SIGTERM.
	 */
	win_screendestroy (rootfd);
	/* 
	 * Lock screen before clear so don't clobber frame buffer while
	 * cursor moving.
	 */
	pw_lock (pixwin, &screen.scr_rect);
	/* 
	 * Enable writing to entire depth of frame buffer.
	 */
	pr_putattributes (fb_pixrect, &fullplanes);
	/* 
	 * Clear entire frame buffer.
	 */
	pr_rop (fb_pixrect, screen.scr_rect.r_left, screen.scr_rect.r_top,
			screen.scr_rect.r_width, screen.scr_rect.r_height, PIX_CLR, 0, 0, 0);
	/* 
	 * Reset previous colormap.
	 */
	pr_putcolormap (fb_pixrect, 0, 256, red, green, blue);
	/* 
	 * Unlock screen.
	 */
	pw_unlock (pixwin);
	exit (0);
}

_root_winmgr () {
	struct inputmask        im;
	struct inputevent       event;
	struct menuitem *mi;
	extern struct menuitem *menu_display ();
	int     keyexit = 0,
	        exit;

	/* 
	 * Set up input mask so can do menu stuff
	 */
	input_imnull (&im);
	im.im_flags |= IM_NEGEVENT;
	im.im_flags |= IM_ASCII;
	win_setinputcodebit (&im, SELECT_BUT);
	win_setinputcodebit (&im, MENU_BUT);
	/* by enabling LOC_MOVE, we can discard the input buffer when the */
	/* mouse moves. */
	win_setinputcodebit (&im, LOC_MOVE);
	win_setinputmask (rootfd, &im, (struct inputmask *) 0, WIN_NULLLINK);
	/* 
	 * Read and invoke menu items
	 */
	for (;;) {
		int     ibits,
		        nfds;

		/* 
		 * Use select (to see if have input) so will return on
		 * SIGWINCH or SIGCHLD.
		 */
		ibits = 1 << rootfd;
		do {
			if (root_SIGCHLD)
				_root_sigchldhandler ();
			if (root_SIGWINCH)
				_root_sigwinchhandler ();
		} while (root_SIGCHLD || root_SIGWINCH);
		nfds = select (8 * sizeof (ibits), &ibits, (int *) 0, (int *) 0,
				(struct timeval *) 0);
		if (nfds == -1) {
			if (errno == EINTR)/* 
				 * Go around again so that signals can be
				 * handled.  ibits may be non-zero but should
				 * be ignored in this case and they will be
				 * selected again.
				 */
				continue;
			else {
				perror ("suntools");
				break;
			}
		}
		if (ibits & (1 << rootfd)) {
			/* 
			 * Read will not block.
			 */
			if (input_readevent (rootfd, &event) < 0) {
				if (errno != EWOULDBLOCK) {
					perror ("suntools");
					break;
				}
			}
		}
		else
			continue;

		if (win_inputnegevent (&event))
			continue;

		if (IS_ASCII_EVENT (event.ie_code)) {
			int     i;

			/* add to key input buffer */
			if (isupper (event.ie_code)) {
				event.ie_code = tolower (event.ie_code);
				wants_su = TRUE;
			}
			key_buffer[key_buffer_length] = event.ie_code;
			key_buffer_length += (key_buffer_length > KEY_BUFFER_LENGTH ? 0 : 1);
			key_buffer[key_buffer_length] = '\0';
			for (i = 0; i < keymenubody.m_itemcount; i++) {
				if (strcmp (keymenubody.m_items[i].mi_imagedata,
							key_buffer) == 0)
					break;
			}
			if (i >= keymenubody.m_itemcount)
				continue;/* not found */

			exit = wmgr_handlerootmenuitem ((struct menu   *) NULL /* not used! */ ,
				                                               &keymenubody.m_items[i], rootfd) == -1;
			if (exit)
				break;/* exit suntools */
			/* zap buffer now that we have used it */
			/* for now this occurs simply by falling through */
		}
		key_buffer_length = 0;
		key_buffer[0] = '\0';
		wants_su = FALSE;

		if (event.ie_code != MENU_BUT)
			continue;

		if (wants_su = (event.ie_shiftmask & SHIFTMASK)) {
			struct pr_prpos textpos;

			/* move to the left by its width, so its right flush 
			*/
			/* against the menu */
			su_rect.r_left = event.ie_locx - su_bound.size.x;
			/* origin for text is from the bottom, so subtract its
			   /* its height */
			su_rect.r_top = event.ie_locy - sysfont -> pf_defaultsize.y;
			/* ignore going off the screen, since clipping is
			   enabled */

			pw_lock (pixwin, &su_rect);
			/* save whats lying on the screen */
			pr_rop (oldpr, 0, 0, su_rect.r_width, su_rect.r_height,
					PIX_SRC, pixwin -> pw_pixrect,
					su_rect.r_left, su_rect.r_top);
			textpos.pr = pixwin -> pw_pixrect;
			textpos.pos.x = su_rect.r_left;
			textpos.pos.y = su_rect.r_top
				- /* distance to baseline */ sysfont -> pf_char['a'].pc_home.y;
			/* textpos is a structure passed by value!?! - DEL */
			pf_text (textpos, PIX_SRC, sysfont, SU_LABEL);
			pw_unlock (pixwin);
		}

		/* 
		 * Do menus
		 */
		if (wmgr_getrootmenu (rootmenuname, wmgr_rootmenu, rootmenufile,
					root_items, root_itemstrings, ROOTMENUITEMS) <= 0) {
			fprintf (stderr, "suntools: invalid root menu\n");
			continue;
		}
		for (;;) {
			struct inputevent       tevent;

			exit = 0;
			tevent = event;
			mi = menu_display (&wmgr_rootmenu, &event, rootfd);
			if (mi)
				exit = wmgr_handlerootmenuitem (wmgr_rootmenu, mi, rootfd) == -1;
			if (event.ie_code == MS_LEFT && !exit) {
				event = tevent;
				/* 		
				   win_setmouseposition(rootfd, event.ie_locx,
				   event.ie_locy); */
			}
			else {
				break;
			}
		}
		if (exit)
			break;	/* exit suntools */
		if (wants_su) {
			/* restore what was lying on screen */
			pw_lock (pixwin, &su_rect);
			pr_rop (pixwin -> pw_pixrect,
					su_rect.r_left, su_rect.r_top,
					su_rect.r_width, su_rect.r_height,
					PIX_SRC, oldpr, 0, 0);
			pw_unlock (pixwin);
			wants_su = FALSE;
		}
	}
}

_root_sigchldhandler () {
	union wait status;

	root_SIGCHLD = 0;
	while (wait3 (&status, WNOHANG, (struct rusage *) 0) > 0) {
	}
}

_root_sigwinchhandler()
{
	root_SIGWINCH = 0;
	pw_damaged (pixwin);
	switch (rootcolor) {
	case ROOTCOLOR_PATTERN: 
		pw_replrop(pixwin,
			screen.scr_rect.r_left, screen.scr_rect.r_top,
			screen.scr_rect.r_width, screen.scr_rect.r_height,
			PIX_SRC, tool_bkgrd, 0, 0);
		break;
	default: 
		pw_writebackground(pixwin,
			screen.scr_rect.r_left, screen.scr_rect.r_top,
			screen.scr_rect.r_width, screen.scr_rect.r_height,
			(rootcolor == ROOTCOLOR_BACKGROUND)?PIX_CLR:PIX_SET);
	}

	pw_donedamaged (pixwin);
	return;
}

static
        _root_sigchldcatcher () {
	        root_SIGCHLD = 1;
}

static
        _root_sigwinchcatcher () {
	        root_SIGWINCH = 1;
}

char   *
        _get_home_dir () {
	extern char    *getlogin ();
	extern struct passwd   *getpwnam (), *getpwuid ();
	struct passwd  *passwdent;
	char   *home_dir = getenv ("HOME"), *loginname;

	if (home_dir != NULL)
		return (home_dir);
	loginname = getlogin ();
	if (loginname == NULL) {
		passwdent = getpwuid (getuid ());
	}
	else {
		passwdent = getpwnam (loginname);
	}
	if (passwdent == NULL) {
		fprintf(stderr,
			"suntools: couldn't find user in password file.\n");
		return (NULL);
	}
	if (passwdent -> pw_dir == NULL) {
		fprintf (stderr,
			"suntools: no home directory in password file.\n");
		return (NULL);
	}
	return (passwdent -> pw_dir);
}

#define	ROOT_ARGBUFSIZE		1000
#define	ROOT_SETUPFILE		"/.suntools"
#define	ROOT_MAXTOOLDELAY	10

_root_initialsetup (requestedfilename)
char   *requestedfilename;
{
	register        i;
	FILE * file;
	char    filename[MAXNAMLEN],
	        programname[MAXNAMLEN],
	        otherargs[ROOT_ARGBUFSIZE];
	struct rect     rectnormal,
	                recticonic;
	int     iconic,
	        topchild,
	        bottomchild,
	        seconds,
	        j;
	char    line[200];

	if (requestedfilename[0] == NULL) {
		char   *home_dir = _get_home_dir ();
		if (home_dir == NULL)
			return;
		(void) strcpy (filename, home_dir);
		(void) strncat (filename, ROOT_SETUPFILE, sizeof (filename) - 1 -
				strlen (filename) - strlen (ROOT_SETUPFILE));
	}
	else
		(void) strncpy (filename, requestedfilename, sizeof (filename) - 1);
	if ((file = fopen (filename, "r")) == 0) {
		if (requestedfilename[0] == NULL)
				/* 
				 * No message if was trying to open default.
				 */
			return;
		fprintf (stderr, "suntools: couldn't open %s\n", filename);
		return;
	}
	while (fgets (line, sizeof (line), file)) {
		if (line[0] == '#')
			continue;
		otherargs[0] = '\0';
		programname[0] = '\0';
		i = sscanf (line, "%s%hd%hd%hd%hd%hd%hd%hd%hd%hD%[^\n]\n",
				programname,
				&rectnormal.r_left, &rectnormal.r_top,
				&rectnormal.r_width, &rectnormal.r_height,
				&recticonic.r_left, &recticonic.r_top,
				&recticonic.r_width, &recticonic.r_height,
				&iconic, otherargs);
		if (i == EOF)
			break;
		if (i < 10 || i > 11) {
			/* 
			 * Just get progname and args.
			 */
			otherargs[0] = '\0';
			programname[0] = '\0';
			j = sscanf (line, "%s%[^\n]\n", programname, otherargs);
			if (j > 0) {
				iconic = 0;
				rect_construct (&recticonic, WMGR_SETPOS, WMGR_SETPOS,
						WMGR_SETPOS, WMGR_SETPOS);
				rect_construct (&rectnormal, WMGR_SETPOS, WMGR_SETPOS,
						WMGR_SETPOS, WMGR_SETPOS);
			}
			else {
				fprintf (stderr,
						"suntools: in file=%s fscanf gave %D, correct format is:\n",
						filename, i);
				fprintf (stderr,
						"program open-left open-top open-width open-height close-left close-top close-width close-height iconicflag [args] <newline>\n OR\nprogram [args] <newline>\n");
				continue;
			}
		}
		/* 
		 * Handle WMGR_SETPOS requests.
		 */
		wmgr_figuretoolrect (rootfd, &rectnormal);
		wmgr_figureiconrect (rootfd, &recticonic);
		/* 
		 * Remember who top and bottom children windows are for use when
		 * trying to determine when tool is installed.
		 */
		topchild = win_getlink (rootfd, WL_TOPCHILD);
		bottomchild = win_getlink (rootfd, WL_BOTTOMCHILD);
		/* 
		 * Fork tool.
		 */
		/* 
		 (void) wmgr_forktool (programname, otherargs,
		 &rectnormal, &recticonic, iconic);
		 */

		del_forktool (programname, otherargs, &rectnormal, &recticonic, iconic);
		/* 
		 * Give tool chance to intall self in tree before starting next.
		 */
		for (seconds = 0; seconds < ROOT_MAXTOOLDELAY; seconds++) {
			sleep (1);
			if (topchild != win_getlink (rootfd, WL_TOPCHILD) ||
					bottomchild != win_getlink (rootfd, WL_BOTTOMCHILD))
				break;
		}
	}
	(void) fclose (file);
}

int
        wmgr_rootmenuschanged (menu)
struct menu    *menu;
{
	struct stat     statb;
	int     sa_count;

	/* Whenever existing menu going up, stat menu files */
	for (sa_count = 0; sa_count < menu_next; sa_count++) {
		if (stat (stat_array[sa_count].name, &statb) < 0) {
			if (errno == ENOENT)
				return (1);
			fprintf (stderr, "suntools: ");
			perror (stat_array[sa_count].name);
			return (-1);
		}
		if (statb.st_mtime > stat_array[sa_count].mftime)
			return (1);
	}
	return (0);
}

wmgr_freerootmenus (menu)
struct menu    *menu;
{
	int     sa_count = 0;
	struct menu    *next = menu -> m_next,
	               *nnext;

	while (next) {
		nnext = next -> m_next;
		if (next -> m_items) {
			free (next -> m_items -> mi_data);
				/* free string storage */
			free (next -> m_items);/* item storage */
		}
		free (next);	/* menu storage */
		next = nnext;
	}
	while (sa_count < menu_next) {
		free (stat_array[sa_count].name);/* file name */
		stat_array[sa_count].name = NULL;
		stat_array[sa_count].mftime = 0;
		sa_count++;
	}
	menu_next = 0;
}

wmgr_getrootmenu (mn, menu, mf, mi, mis, maxitems)
char   *mn,
       *mf;
struct menu    *menu;
struct menuitem *mi;
struct menuitemstrings *mis;
int     maxitems;
{
	FILE * f;
	int     lineno;
	char    line[256];
	char    tag[256],	/* was 32 - DEL */
	        prog[256],
	        args[256];
	char   *savestr ();
	struct stat     statb;
	struct menu    *menunext = wmgr_rootmenu;
	int     nitems = 0;
	static  time_t mftime = 0;
	static char    *nqformat = "%[^ \t\n]%*[ \t]%[^ \t\n]%*[ \t]%[^\n]\n";
	static char    *qformat = "\"%[^\"]\"%*[ \t]%[^ \t\n]%*[ \t]%[^\n]\n";

	if (menu == wmgr_rootmenu && menu_next != 0) {
		if (wmgr_rootmenuschanged (wmgr_rootmenu) != 0)
			wmgr_freerootmenus (wmgr_rootmenu);
		else
			return menu -> m_itemcount;
	}
	if (menu_next >= MAX_MENUS - 1) {
		fprintf (stderr,
				"suntools: max number of menus is %D\n", MAX_MENUS);
		return - 1;
	}
	if ((f = fopen (mf, "r")) == NULL) {
		fprintf (stderr, "suntools: can't open menu file %s\n", mf);
		return - 1;
	}
	if (stat (mf, &statb) < 0) {
		fprintf (stderr, "suntools: ");
		perror (mf);
		return - 1;
	}
	stat_array[menu_next].mftime = statb.st_mtime;
	stat_array[menu_next].name = savestr (mf);
	++menu_next;
	menu -> m_imagetype = MENU_IMAGESTRING;
	menu -> m_imagedata = mn;
	menu -> m_items = mi;
	for (nitems = 0, lineno = 1; nitems < maxitems &&
			fgets (line, sizeof (line), f); lineno++) {
		if (line[0] == '#')
			continue;
		args[0] = '\0';
		if (sscanf (line, line[0] == '"' ? qformat : nqformat,
					tag, prog, args) < 2) {
			fprintf (stderr,
					"suntools: format error in %s: line %d\n",
					mf, lineno);
			mftime = 0;/* complain every time */
			continue;
		}
		if (strcmp (prog, "KEYS") == 0) {
			/* check for calling only from root menu...bah! */
			if (wmgr_getrootmenu ("keys" /* menu name */ ,
						&keymenubody, /* key file name */ args,
						&key_items[0], &key_itemstrings[0],
						KEYITEMS) <= 0) {
				fprintf (stderr, "suntools: bad key menu %s\n", args);
				continue;
			}
			/* do not link this menu in with others!!! */
			/* it will not be displayed! */
		}
		else if (strcmp (prog, "MENU") == 0) {
			struct menu    *m;
			char   *mi,
			       *ms;
			if (menu != wmgr_rootmenu) {
				fprintf (stderr,
						"suntools: MENU command illegal in secondary menu file %s: line %d\n",
						mf, lineno);
				continue;
			}
			if (wmgr_getrootmenu (
						savestr (tag),/* menu name */
						m = (struct menu       *)
					                        calloc (1, sizeof (struct menu)), args,
					 /* file name */                                        mi = calloc (ROOTMENUITEMS, sizeof (struct menuitem))  ,
					                                                                                                                ms = calloc (ROOTMENUITEMS, sizeof (struct menuitemstrings))   ,
					                                                                                                                                                                                ROOTMENUITEMS) <= 0) {
				fprintf (stderr,
					"suntools: invalid secondary menu %s\n", args);
				free (m);
				free (mi), free (ms);
				continue;
			}
			else {
				                                                                                                                                                                                menunext -> m_next = m;
				menunext = m;
			}
		}
		else {
			if (mi -> mi_imagedata)
				free ((char *) mi -> mi_imagedata);
			mi -> mi_imagetype = MENU_IMAGESTRING;
			mi -> mi_imagedata = (caddr_t) savestr (tag);
			mi -> mi_data = (caddr_t) mis;
			if (mis -> mis_prog)
				free (mis -> mis_prog);
			if (mis -> mis_args)
				free (mis -> mis_args);
			mis -> mis_prog = savestr (prog);
			if (args[0] == '\0')
				mis -> mis_args = (char *) NULL;
			else
				mis -> mis_args = savestr (args);
			mi++;
			mis++;
			nitems++;
		}
	}
	fclose (f);
	return menu -> m_itemcount = nitems;
}

char   *
        savestr (s)
register char  *s;
{
	register char  *p;

	if ((p = malloc (strlen (s) + 1)) == NULL) {
		if (rootfd)
			win_screendestroy (rootfd);
		fprintf (stderr, "suntools: out of memory for menu strings\n");
		exit (1);
	}
	strcpy (p, s);
	return (p);
}

wmgr_handlerootmenuitem (menu, mi, rootfd)
struct menu    *menu;
struct menuitem *mi;
int     rootfd;
{
	int     returncode = 0;
	struct rect     recticon,
	                rectnormal;
	struct menuitemstrings *mis;

	/* 
	 * Get next default tool positions
	 */
	rect_construct (&recticon, WMGR_SETPOS, WMGR_SETPOS,
			WMGR_SETPOS, WMGR_SETPOS);
	rectnormal = recticon;
	mis = (struct menuitemstrings  *) mi -> mi_data;
	if (strcmp (mis -> mis_prog, "EXIT_NOCONFIRM") == 0) {
		returncode = -1;
	}
	else if (strcmp (mis -> mis_prog, "EXIT") == 0) {
		returncode = wmgr_confirm (rootfd,
				"Press the left mouse button to confirm Exit.  \
To cancel, press the right mouse button now.");
	}
	else if (strcmp (mis -> mis_prog, "REFRESH") == 0) {
		wmgr_refreshwindow (rootfd);
	}
	else if (strcmp (mis -> mis_prog, "HELP") == 0) {
		wmgr_confirm (rootfd, "Right mouse down displays menus.  \
Right mouse up selects.  \
Left mouse down selects without losing menus.  \
Holding shift key down before right mouse enables superuser.  \
Press any key to remove this garbage.");
	}
	else if (strcmp (mis -> mis_prog, "VERSION") == 0) {
		wmgr_confirm(rootfd,"suntools 1.2.4 (9/26/85) by Don Libes \
at NBS (nbs-amrf!libes).  Based on suntools 1.2 (3/13/85) by unknown \
at SMI.  Press any key to remove this garbage.");
	}
	else {
		wmgr_figureiconrect (rootfd, &recticon);
		wmgr_figuretoolrect (rootfd, &rectnormal);
		del_forktool (mis -> mis_prog, mis -> mis_args,
				&rectnormal, &recticon, 0 /* not iconic */ );
	}
	return (returncode);
}

enable_su () {
	if (setuid (0)) perror ("setuid(0)");
	setgid (1);
}

disable_su () {
	setgid (oldrgid);
	if (setuid (oldruid)) perror ("setuid(oldruid)");
}

del_forktool (program, args, open_rect, closed_rect, iconic)
char   *program, *args;
struct rect     open_rect, closed_rect;
int     iconic;
{
	if (0 == fork ()) {	/* child */
		char    buffer[5000];/* space for program name and args */
		/* if doesn't have rectnormal, add it */
		/* if doesn't have recticon, add it */
		/* if it doesn't have iconic, add it if iconic */
		if (wants_su && (geteuid() == 0))
			enable_su ();
		else
			disable_su ();
		if (args)
			sprintf (buffer, "%s %s", program, args);
		else
			strcpy (buffer, program);

		/*
		   now exec the program.  Only problem is we have the
		   prog name and args in a character string.  We have
		   to tokenize them in order to call exec or exec sh
		   and have that do it for us.  But that will create
		   two processes for every one.
		*/
		/* execl ("/bin/sh", "sh", "-c", buffer, 0);*/
		del_exec(buffer);
		_exit (127);
	}
}

#define end_token if (in_token) {		\
	/* terminate current token */	*cp = '\0';		\
	/* advance to next token */	args[++argi] = cp = ++s;\
	/* not sure if there is anything valid here, so */ in_token = FALSE; }\
	else s++;

/* exec a program and args that are all stored as one string */
del_exec(s)
char *s;
{
	char *args[50];
	char *cp;
	int argi = 0;
	int quoting = FALSE;
	int in_token = FALSE;	/* TRUE if we are reading a token */

	args[0] = cp = s;
	while (*s) {
		if (quoting) {
			if (*s == '\\' && *(s+1) == '"') { /* quoted quote */
				s++;	/* get past " */
				*cp++ = *s++;
			} else 	if (*s == '\"') { /* close quote */
				end_token
				quoting = FALSE;
			} else *cp++ = *s++; /* suck up anything */
		} else if (*s == '\"') { /* open quote */
			in_token = TRUE;
			quoting = TRUE;
			s++;
		} else if (isspace(*s)) {
			end_token
		} else {
			*cp++ = *s++;
			in_token = TRUE;
		}
	}
	end_token
	args[argi] = (char *) 0; /* terminate argv */
	execvp(args[0],args);
}

/* allow user to set cursor attributes - DEL */
init_cursor()
{
	struct cursor cursor;
	char *cursorfile, *x_hot, *y_hot, *cursor_rop;	/* environment parms */

	cursor.cur_shape = &cursor_pr;

	/* default cursor is a weird looking dot */
	cursor.cur_xhot = 8;
	cursor.cur_yhot = 8;
	cursor.cur_function = PIX_SRC | PIX_DST;
	win_setcursor(rootfd,&cursor);

	/* win_getcursor(rootfd, &cursor); */
	if (cursorfile = getenv("ROOT_CURSOR")) {
		char errmsg[1000];

		if (cursor.cur_shape = icon_load_mpr(cursorfile,errmsg)) {
			if (x_hot = getenv("ROOT_CURSOR_HOTSPOT_X"))
				cursor.cur_xhot = atoi(x_hot);
			if (y_hot = getenv("ROOT_CURSOR_HOTSPOT_Y"))
				cursor.cur_yhot = atoi(y_hot);
			if (cursor_rop = getenv("ROOT_CURSOR_ROP"))
				cursor.cur_function = atoi(cursor_rop);
			win_setcursor(rootfd,&cursor);
		} else printf(errmsg);
	}
}
