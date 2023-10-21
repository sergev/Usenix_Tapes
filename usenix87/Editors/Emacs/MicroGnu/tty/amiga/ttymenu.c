/*
 * ttymenu.c
 *
 * Incorporates the browser, for rummaging around on disks,
 * and the usual Emacs editing command menu
 *
 *	Copyright (c) 1986, Mike Meyer
 *	Manxification and Edit menu by Mic Kaczmarczik (no charge :-)
 *
 * Permission is hereby granted to distribute this program, so long as
 * this source file is distributed with it, and this copyright notice
 * is not removed from the file.
 *
 */

#include <exec/types.h>
#include <libraries/dos.h>
#include <libraries/dosextens.h>
#include <intuition/intuition.h>
#undef	TRUE
#undef	FALSE
#include "def.h"

extern struct Menu		*AutoMenu ;
extern struct Window		*EmW ;

#define MNUM(menu,item,sub) (SHIFTMENU(menu)|SHIFTITEM(item)|SHIFTSUB(sub))

#ifdef	BROWSER
#define LONGEST_NAME	80	/* Longest file name we can deal with	*/

# ifndef	MANX
char *strchr(char *, int);
# else
char *index();			/* find first instance of c in s	*/
#define	strchr(s, c) index(s, c)
# endif

# ifdef	MENU
#define	FIRSTMENU	1
# else
#define	FIRSTMENU	0
# endif

#endif	BROWSER

#ifdef	MENU
/*
 * When ttgetc() sees a menu selection event, it stuffs
 * the sequence <CSI>M~ into the input buffer, and
 * caches the menu number and item number for later.
 * This sequence is translated into the internal key code
 * KMENU, similar to KHELP and the other function keys.
 *
 * The menu item names are chosen to be relatively close
 * to the extended function names, so a user can usually
 * figure out the key binding of a menu item by searching
 * through the "display-bindings" buffer for something
 * that's close.
 */

/*
 * Commands for managing files and buffers
 */

static struct MenuBinding FileItems[] = {
	{ "Find File         C-x C-f",	"find-file"			},
	{ "Pop To File       C-x 4 f",	"find-file-other-window"	},
	{ "Insert File       C-x i",	"insert-file"			},
	{ "Save File         C-x C-s",	"save-buffer"			},
	{ "Write File        C-x C-w",	"write-file"			},
	{ "Switch To Buffer  C-x b",	"switch-to-buffer"		},
	{ "Pop To Buffer     C-x 4 b",	"switch-to-buffer-other-window"	},
	{ "Kill Buffer       C-x k",	"kill-buffer"			},
	{ "List Buffers      C-x C-b",	"list-buffers"			},
	{ "Save Buffers      C-x s",	"save-some-buffers"		},
	{ "Save And Exit     C-x C-c",	"save-buffers-kill-emacs"	}
};

/*
 * Commands for various editing functions
 */

static struct MenuBinding EditItems[] = {
	{ "Yank                 C-y",	"yank"				},
	{ "Blank Line           C-o ",	"open-line"			},
	{ "Kill Line            C-k",	"kill-line"			},
	{ "Delete Blank Lines   C-x C-o","delete-blank-lines"		},
	{ "Delete Blanks        M-SPC",	"just-one-space"		},
	{ "Newline And Indent   C-j",	"newline-and-indent"		},
	{ "Transpose Characters C-t",	"transpose-chars"		},
	{ "Quoted Insert        C-q",	"quoted-insert"			}
};

/*
 * Movement commands
 */

static struct MenuBinding MoveItems[] = {
	{ "Scroll Up       C-v",	"scroll-up"			},
	{ "Scroll Down     M-v",	"scroll-down"			},
	{ "Start Of Line   C-a",	"beginning-of-line"		},
	{ "Start Of Buffer M-<",	"beginning-of-buffer"		},
	{ "End Of Line     C-e",	"end-of-line"			},
	{ "End Of Buffer   M->",	"end-of-buffer"			},
	{ "Goto Line",			"goto-line"			},
	{ "Show Cursor     C-x =",	"what-cursor-position"		}
};

/*
 * Commands for searching and replacing
 */

static struct MenuBinding SearchItems[] = {
	{ "I-Search Forward  C-s",	"isearch-forward"	},
	{ "I-Search Backward C-r",	"isearch-backward"	},
	{ "Search Again",		"search-again"		},
	{ "Search Forward    M-s",	"search-forward"	},
	{ "Search Backward   M-r",	"search-backward"	},
	{ "Query Replace     M-%",	"query-replace"		}
};

/*
 * Commands that manipulate words
 */
static struct MenuBinding WordItems[] = {
	{ "Forward Word       M-f",	"forward-word"			},
	{ "Backward Word      M-b",	"backward-word"			},
	{ "Kill Word          M-d",	"kill-word"	 		},
	{ "Backward Kill Word M-DEL",	"backward-kill-word" 		},
	{ "Capitalize Word    M-c",	"capitalize-word"		},
	{ "Downcase Word      M-l",	"downcase-word"			},
	{ "Upcase Word        M-u",	"upcase-word"			}
};

static struct MenuBinding ParaItems[] = {
	{ "Forward Paragraph  M-]",	"forward-paragraph"		},
	{ "Backward Paragraph M-[",	"backward-paragraph"		},
	{ "Fill Paragraph     M-q",	"fill-paragraph"		},
	{ "Set Fill Column    C-x f",	"set-fill-column"		},
	{ "Kill Paragraph",		"kill-paragraph"		},
	{ "Auto Fill Mode",		"auto-fill-mode"		}
};

/*
 * Region stuff
 */

static struct MenuBinding RegionItems[] = {
	{ "Set Mark            C-@",	"set-mark-command"		},
	{ "Exch Point And Mark C-x C-x","exchange-point-and-mark"	},
	{ "Kill Region         C-w",	"kill-region"			},
	{ "Copy Region As Kill M-w",	"copy-region-as-kill"		},
	{ "Downcase Region     C-x C-l","downcase-region"		},
	{ "Upcase Region       C-x C-u","upcase-region"			}
};

/*
 * Commands for manipulating windows
 */

static struct MenuBinding WindowItems[] = {
	{ "Split Window         C-x 2", "split-window-vertically"	},
	{ "Delete Window        C-x 0",	"delete-window"			},
	{ "Delete Other Windows C-x 1",	"delete-other-windows"		},
	{ "Down Window          C-x o",	"next-window"			},
	{ "Up Window",			"previous-window"		},
	{ "Enlarge Window       C-x ^",	"enlarge-window"		},
	{ "Shrink Window",		"shrink-window"			},
	{ "Redraw Display",		"redraw-display"		},
	{ "Recenter             C-l",	"recenter"			},
	{ "Toggle Border",		"toggle-window-hack"		},
#ifdef	CHANGE_FONT
	{ "Set Font",			"set-font"			}
#endif
};

/*
 * Miscellaneous commands
 */

static struct MenuBinding MiscItems[] = {
	{ "Start Kbd Macro   C-x (",	"start-kbd-macro"		},
	{ "End Kbd Macro     C-x )",	"end-kbd-macro"			},
	{ "Call Kbd Macro    C-x e",	"call-last-kbd-macro"		},
	{ "Execute Command   M-x",	"execute-extended-command"	},
	{ "Global Set Key",		"global-set-key"		},
	{ "Global Unset Key",		"global-unset-key"		},
	{ "Describe Key      C-h c",	"describe-key-briefly",		},
	{ "Describe Bindings C-h b",	"describe-bindings"		},
	{ "Emacs Version",		"emacs-version"			},
	{ "New CLI           C-z",	"suspend-emacs"			}
};

/*
 * The following table contains the titles, number of
 * items, and pointers to, the individual menus.
 */

static struct MenuInfo EMInfo[] = {
	{ "File  ",		NITEMS(FileItems),	&FileItems[0]	},
	{ "Edit  ",		NITEMS(EditItems),	&EditItems[0]	},
	{ "Move  ", 		NITEMS(MoveItems),	&MoveItems[0]	},
	{ "Search  ",		NITEMS(SearchItems),	&SearchItems[0] },
	{ "Word  ",		NITEMS(WordItems),	&WordItems[0]	},
	{ "Paragraph  ",	NITEMS(ParaItems),	&ParaItems[0]	},
	{ "Region  ",		NITEMS(RegionItems),	&RegionItems[0]	},
	{ "Window  ",		NITEMS(WindowItems),	&WindowItems[0] },
	{ "Miscellaneous  ",	NITEMS(MiscItems),	&MiscItems[0]	}
};

/* There are three cases to deal with; the menu alone, the Browser
 * alone, and both of them together.  We #define some things to make
 * life a little easier to deal with
 */
# ifdef	BROWSER
#  define Edit_Menu_Init() Menu_Add("Edit ", TRUE) 
#  define Edit_Menu_Add(n) Menu_Item_Add(n,(USHORT)ITEMENABLED,0L,(BYTE)0)
#  define Edit_Item_Add(n) Menu_SubItem_Add(n,(USHORT)ITEMENABLED,0L,(BYTE)0)
# else
#  define Edit_Menu_Init() cinf = NULL	/* harmless */
#  define Edit_Menu_Add(n) n[strlen(n)-1] = '\0'; Menu_Add(n, TRUE)
#  define Edit_Item_Add(n) Menu_Item_Add(n,(USHORT)ITEMENABLED,0L,(BYTE)0)
# endif	BROWSER

#endif	MENU

/*
 * Heeere we go!!!!
 */

struct Menu * InitEmacsMenu(EmW)
struct Window *EmW;
{
#ifdef	MENU
	register struct MenuInfo *cinf, *lastinfo;
	register struct MenuBinding *cb, *lastbinding;
#endif

	Menu_Init() ;			/* init the menu		*/

#ifdef	MENU

	Edit_Menu_Init() ;		/* Set up for editing menu	*/
	lastinfo = &EMInfo[NITEMS(EMInfo)];	/* loop sentinel	*/	
	for (cinf = EMInfo; cinf < lastinfo; cinf++) {
		Edit_Menu_Add(cinf->Name);
		lastbinding = &cinf->Items[cinf->NumItems];
		for (cb = cinf->Items; cb < lastbinding; cb++)
			Edit_Item_Add(cb->Command);
	}
#endif	MENU

#ifdef	BROWSER
	Menu_Add("Disks ", TRUE) ;
	Menu_Item_Add("Df0:", (USHORT) ITEMENABLED, 0L, (BYTE) 0) ;
	Menu_Item_Add("Df1:", (USHORT) ITEMENABLED, 0L, (BYTE) 0) ;
	Menu_Item_Add("Ram:", (USHORT) ITEMENABLED, 0L, (BYTE) 0) ;
#endif	BROWSER

	return 	AutoMenu ;
}

/*
 * amigamenu() -- handles a menu pick.
 */

amigamenu(f, n, k) {
	unsigned short		Menu_Number;
	char			*name;
	int			ttmenu(); /* in ttyio.c */

#ifdef	BROWSER
	register unsigned short	level, i, dirp;
	register char		*cp;
	int			stat;
	struct MenuItem		*ItemAddress() ;

	/* State variables that describe the current directory */
	static char		Dir_Name[LONGEST_NAME] ;
	static unsigned short	Menu_Level = 0 ;
#endif
#ifdef	MENU
	SYMBOL	*sp;
#endif

	if (!ttmenu(&Menu_Number)) return FALSE;	/* get menu number */

#ifndef	BROWSER
# ifdef	MENU
	name = EMInfo[MENUNUM(Menu_Number)].Items[ITEMNUM(Menu_Number)].Binding;
	if ((sp=symlookup(name)) != NULL)
		return ((*sp->s_funcp)(f, n, KRANDOM));
	panic("Unknown menu command!");	/* trouble!	*/
# endif
#else	/* we're using the Browser */
# ifdef	MENU
	/* Handle commands from the Edit menu when using the Browser */
	if (0 == MENUNUM(Menu_Number)) {
		name = EMInfo[ITEMNUM(Menu_Number)].Items[SUBNUM(Menu_Number)].Binding;
		if ((sp=symlookup(name)) != NULL)
			return ((*sp->s_funcp)(f, n, KRANDOM));
		panic("Unknown menu command!");	/* trouble!	*/
	}
# endif
	/* Here when a selection was made in a Browser menu */
	name = (char *)((struct IntuiText *)
		(ItemAddress(AutoMenu,(ULONG) Menu_Number) -> ItemFill))
		-> IText ;
	level = MENUNUM(Menu_Number) - FIRSTMENU;

	/* Got what we want, so clear the menu to avoid confusing the user */
	ClearMenuStrip(EmW) ;

	/* set dirp to FALSE if the name is not a directory or disk */
	dirp = (strchr(name, '/') != NULL || strchr(name, ':') != NULL) ;

	/* First, set the directory name right */
	if (level > Menu_Level)			/* Not possible, die */
		panic("impossible menu_level in amigamenu");
	else if (level == 0)			/* picked a new disk */
		Dir_Name[0] = '\0' ;
	else if (level < Menu_Level) {		/* Throw away some levels */
		for (i = 1, cp = strchr(Dir_Name, ':'); i < level; i++) {
			if (cp == NULL) return FALSE;
			cp = strchr(cp, '/') ;
			}
		if (cp == NULL) panic("broken file name in amigamenu");
		*++cp = '\0' ;
		}
	/* else Menu_Level == level, chose a file a current level */

	/* Now, fix up the menu and it's state variable */
	while (Menu_Level > level) {
		Menu_Level-- ;
		Menu_Pop() ;
		}

	/* If we added a directory, tack it onto the name */
	if (dirp) {
		Menu_Level++ ;
		(void) strncat(Dir_Name, name,
			LONGEST_NAME - strlen(Dir_Name) - 1) ;
		}

	/* Now, tell the user all about it */
	if (dirp) stat = Add_Dir(Dir_Name, name) ;
	else stat = Display_File(Dir_Name, name) ;
	SetMenuStrip(EmW, AutoMenu) ;
	return stat ;
#endif	BROWSER
}

#ifdef	BROWSER
/*
 * Display_File - Go fetch a the requested file into a window.
 */
Display_File(dir, file) char *dir, *file; {
	register BUFFER	*bp, *findbuffer();
	int		s;
	char		File_Name[LONGEST_NAME];

	(void) strcpy(File_Name, dir);
	(void) strncat(File_Name, file, LONGEST_NAME - strlen(File_Name) - 1) ;
	if ((bp = findbuffer(File_Name, &s)) == NULL) return s;
	curbp = bp;
	if (showbuffer(bp, curwp, WFHARD) != TRUE) return FALSE;
	if (bp->b_fname[0] == 0)
		return (readin(File_Name));		/* Read it in.	*/
	return TRUE;
	}
/*
 * Add_Dir - given a dir and a name, add the menu name with the files in
 *	dir as entries.  Use AllocMem() in order to make
 *      sure the file info block is on a longword boundary.
 */
static
Add_Dir(dir, name) char *dir, *name; {
	register char			*last_char ;
	register struct FileLock	*my_lock, *Lock() ;
	unsigned short			count ;
	int				stat = FALSE;
	static char			Name_Buf[LONGEST_NAME] ;
	char				*AllocMem();
	struct	FileInfoBlock		*File_Info;

	if ((File_Info = (struct FileInfoBlock *)
		AllocMem((LONG)sizeof(struct FileInfoBlock), 0L)) == NULL)
		return (FALSE);

	/* Fix up the trailing / if it needs it */
	last_char = &dir[strlen(dir) - 1] ;
	if (*last_char == '/') *last_char = '\0' ;

	/* Now, start on the directory */
	if ((my_lock = Lock(dir, ACCESS_READ)) == NULL) goto out;

	if (!Examine(my_lock, File_Info)) goto out;
	if (File_Info -> fib_DirEntryType < 0L)
		goto out;

	if (Menu_Add(name, TRUE) == 0) return NULL;
	for (count = 0; ExNext(my_lock, File_Info) 
			|| IoErr() != ERROR_NO_MORE_ENTRIES; count++)
		if (File_Info -> fib_DirEntryType < 0L) {
			if (Menu_Item_Add(File_Info -> fib_FileName,
				(USHORT)ITEMENABLED, 0L, (BYTE)0)
					== MNUM(NOMENU, NOITEM, NOSUB))
					break ;
			}
		else {
			(void) strcpy(Name_Buf, File_Info -> fib_FileName) ;
			(void) strcat(Name_Buf, "/") ;
			if (Menu_Item_Add(Name_Buf,
				(USHORT) ITEMENABLED, 0L, (BYTE)0)
					 == MNUM(NOMENU, NOITEM, NOSUB))
				break ;
			}
	if (count == 0) Menu_Item_Add("EMPTY", (USHORT)0, 0L, (BYTE)0) ;

	/* Put everything back */
	if (*last_char == '\0') *last_char = '/' ;
	stat = TRUE;
out:
	UnLock(my_lock) ;
	FreeMem(File_Info, (LONG) sizeof(struct FileInfoBlock));
	return stat;
	}
#endif	BROWSER
