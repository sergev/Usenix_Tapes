/*
 * types.h: Rog-O-Matic XIV (CMU) Sat Feb 16 09:44:54 1985 - mlm
 * Copyright (C) 1985 by A. Appel, G. Jacobson, L. Hamey, and M. Mauldin
 *
 * Miscellaneous Macros and Constants 
 */

/* The unctrl macro for systems where curses doesn't define it */

# ifndef unctrl
extern char	*_unctrl[];

# define	unctrl(ch)	(_unctrl[ch & 0177])
# endif

/* Global Preprocessor constants */

# define ill		";'"
# define status		"Str:"
# define MAXATTEMPTS	(10)
# define ROGQUIT	(-2)
# define DIED		(1)
# define FINISHED	(0)
# define SAVED		(2)
# define MAXINV		(26)
# define NONE		(-1)
# define MAXSTUFF	(30)
# define MAXMONST	(40)
# define SAVEARROWS	(30)
# define NAMSIZ		(64)
# define MAXLEV		(30)
# define MAXMON		(128)
# define SUCCESS	(1)
# define FAILURE	(0)

# define ISPRT(c)	((c) >= ' ' && (c) <= '~')
# define GETROGUECHAR	fgetc(frogue)
# define GETLOGCHAR	fgetc(logfile)

/*
 * Magic numbers for Invisible stalker strategies
 */

# define INVDAM		(16)
# define INVHIT		(1000)
# define INVPRES	(INVHIT-100)
# define INVLURK	(INVPRES-200)

/*
 * Attribute bits for the screen map: If you change this list, be sure
 * to change the flag names at the end of debug.c.   MLM
 */

# define BEEN		(0000000001)
# define CANGO		(0000000002)
# define DOOR		(0000000004)
# define HALL		(0000000010)
# define PSD		(0000000020)
# define ROOM		(0000000040)
# define SAFE		(0000000100)
# define SEEN		(0000000200)
# define SEENPOS	(7)
# define DEADEND	(0000000400)
# define STUFF		(0000001000)
# define TRAP		(0000002000)
# define ARROW		(0000004000)
# define TRAPDOR	(0000010000)
# define TELTRAP	(0000020000)
# define GASTRAP	(0000040000)
# define BEARTRP	(0000100000)
# define DARTRAP	(0000200000)
# define WATERAP	(0000400000)
# define MONSTER	(0001000000)
# define WALL		(0002000000)
# define USELESS	(0004000000)
# define SCAREM		(0010000000)
# define STAIRS		(0020000000)
# define RUNOK		(0040000000)
# define BOUNDARY	(0100000000)
# define SLEEPER	(0200000000)
# define EVERCLR	(0400000000)

# define TOPW		(0)
# define BOTW		(1)
# define LEFTW		(2)
# define RIGHTW		(3)
# define NOTW		(-1)
# define DOORW		(-2)
# define CORNERW	(-3)

# define INFINITY	(10000)

# define NOTAMOVE	(-1)

# define FORCE		(1)
# define OPTIONAL	(0)

# define NEAR		(0)
# define HERE		(1)

# define NOPRINT	(0)
# define DOPRINT	(1)

# define NOTRUNNING	(0)
# define RUNNING	(1)

# define ANY		(0)
# define RUSTPROOF	(1)

/* Types of moves planned by makemove */
# define REUSE		(0)
# define REEVAL		(1)

# define EXPLORE	(1)
# define EXPLORERUN	(2)
# define RUNTODOOR	(3)
# define RUNAWAY	(4)
# define SECRETDOOR	(5)
# define FINDSAFE	(6)
# define GOTOMOVE	(7)
# define ATTACKSLEEP	(8)
# define ARCHERYMOVE	(9)
# define UNPIN		(10)
# define UNPINEXP	(11)
# define EXPLOREROOM	(12)
# define FINDROOM	(13)
# define RESTMOVE	(14)
# define DOWNMOVE	(15)
# define RUNDOWN	(16)
# define NUMMOVES	(17)

/* Version numbers */
# define RV36A		(361)	/* OLDROG: Rogue 3.6 w/o wands */
# define RV36B		(362)	/* CURROG: Rogue 3.6 with wands */
# define RV52A		(521)	/* NEWROG: Rogue 5.2 */
# define RV52B		(522)	/* Rogue 5.2 with maze rooms */
# define RV53A		(531)	/* Rogue 5.3 new monsters */

/* Ways to spend time */

# define T_OTHER	(0)
# define T_HANDLING	(1)
# define T_FIGHTING	(2)
# define T_RESTING	(3)
# define T_MOVING	(4)
# define T_EXPLORING	(5)
# define T_RUNNING	(6)
# define T_GROPING	(7)
# define T_SEARCHING	(8)
# define T_DOORSRCH	(9)
# define T_LISTLEN	(10)

/* Bit value for debugging types (for debugging function dwait) */

# define D_FATAL	(00001)
# define D_ERROR	(00002)
# define D_WARNING	(00004)
# define D_INFORM	(00010)
# define D_SEARCH	(00020)
# define D_BATTLE	(00040)
# define D_MESSAGE	(00100)
# define D_PACK		(00200)
# define D_CONTROL	(00400)
# define D_SCREEN	(01000)
# define D_MONSTER	(02000)
# define D_SAY		(04000)
# define D_ALL		(01777)
# define D_NORMAL	(D_FATAL | D_ERROR)

# define debugon(mask)	(debugging|=(mask))
# define debugoff(mask)	(debugging&=(~(mask)))
# define debug(mask)	(debugging&(mask))

/* Parameters for genetic learning, knobs */
# define K_SRCH		(0)
# define K_DOOR		(1)
# define K_REST		(2)
# define K_ARCH		(3)
# define K_EXPER	(4)
# define K_RUN		(5)
# define K_WAKE		(6)
# define K_FOOD		(7)
# define MAXKNOB	(8)

/* Monster attributes */
# define AWAKE		(1)
# define ASLEEP		(2)
# define ALL		(9)
# define HELD		(10)

/* Constants for handling inventory */
# define UNKNOWN	-99999
# define INVMAX		25

/* Pack item attributes  DR UTexas 25 Jan 84 */
# define KNOWN                 (0000000001)
# define CURSED                (0000000002)
# define ENCHANTED             (0000000004)
# define PROTECTED             (0000000010)
# define UNCURSED              (0000000020)
# define INUSE                 (0000000040)
# define WORTHLESS             (0000000100)

/* Miscellaneous macros */

# define LETTER(i)	((char)((i)+'a'))
# define DIGIT(c)	((int)((c)-'a'))
# define OBJECT(c)	DIGIT(c)
# define ISDIGIT(c)	((c) >= '0' && (c) <= '9')
# define plural(n)	((n)==1 ? "" : "s")
# define ordinal(n)	((((n)/10)%10==1) ? "th": \
			      ((n)%10==1) ? "st": \
			      ((n)%10==2) ? "nd": \
			      ((n)%10==3) ? "rd":"th")
# define ctrl(c)	((c)&037)
# define ESC		ctrl('[')
# define NEWLINE	ctrl('J')
# define abs(a)		((a) >= 0 ? (a) : -(a))
# define max(a,b)	(((a) > (b)) ? (a) : (b))
# define min(a,b)	(((a) < (b)) ? (a) : (b))
# define between(v,a,b)	((v) < (a) ? (a) : (v) > (b) ? (b) : (v))
# define percent(v,p)	(((v)*(p))/100)
# define SKIPARG	while (*++(*argv)); --(*argv)
# define when	        break; case
# define orwhen	        case
# define otherwise      break; default
# define SKIPDIG(s)	while (ISDIGIT(*(s))) (s)++
# define SKIPCHAR(c,s)	while (*(s)==(c)) (s)++
# define SKIPTO(c,s)	\
	{ while (*(s) && *(s)!=(c)) (s)++; if (*(s)==(c)) (s)++; }

/* Utility Macros */

/* onrc - tell if row and col have the proper attributes */
# define onrc(type,r,c) ((type)&scrmap[r][c])

/* on - tell if current position has correct attributes */
# define on(type) onrc(type,atrow,atcol)

/* seerc - is this character at row,col */
# define seerc(ch,r,c) ((ch)==screen[r][c])

/* see - is this character at current position */
# define see(ch) seerc(atrow,atcol)

/* setrc - set attribute at <r,c> */
# define setrc(type,r,c) scrmap[r][c]|=(type)

/* set - set attribute at current position */
# define set(type) setrc(type,atrow,atcol)

/* unsetrc - unset attribute at <r,c> */
# define unsetrc(type,r,c) scrmap[r][c]&= ~(type)

/* unset - unset attribute at current position */
# define unset(type) unsetrc(type,atrow,atcol)

/* Direc - give the vector from an xy difference */
# define direc(r,c) (r>0?(c>0?7:(c<0?5:6)):(r<0?(c>0?1:(c<0?3:2)):(c>0?0:4)))

/* atdrow - gives row of adjacent square given direction */
# define atdrow(dir) (atrow+deltr[(dir)])

/* atdcol - gives col of adjacent square given direction */
# define atdcol(dir) (atcol+deltc[(dir)])

/* Define a more mnemonic string comparison */
# define streq(s1,s2) (strcmp ((s1),(s2)) == 0)

/* Monster value macros */
# define maxhitchar(m) (cosmic ? Level*3/2+6 : monatt[(m)-'A'].maxdam)
# define avghitchar(m) (cosmic ? Level*2/3+4 : monatt[(m)-'A'].expdam)
# define maxhit(m) maxhitchar(mlist[m].chr)
# define avghit(m) avghitchar(mlist[m].chr)     /* times 10 */

/* Item knowledge macros   DR UTexas 25 Jan 84 */

/* itemis - test pack item for traits */
# define itemis(obj,trait) ((obj>=0) ? ((trait)&inven[obj].traits) : 0)

/* remember - set traits for pack item */
# define remember(obj,trait) ((obj>=0) ? (inven[obj].traits|=(trait)) : 0)

/* forget - clear traits for pack item */
# define forget(obj,trait) ((obj>=0) ? (inven[obj].traits&= ~(trait)) : 0)

/* The types of objects */
typedef enum { strange, food, potion, scroll, wand, ring, hitter,
               thrower, missile, armor, amulet, gold, none} stuff;

typedef struct { int   fail, win; } probability;

typedef struct { int   count;
		 double sum, sumsq, low, high; } statistic;

typedef struct { int   scol, srow;
                 stuff what; } stuffrec;

typedef struct { int   mcol, mrow;
                 char  chr;
                 int   q; } monrec;

typedef struct { int   expdam, maxdam, mtokill; } attrec;

typedef struct { int gamecnt, gamesum, inittime, timeswritten; } lrnrec;

typedef struct { char m_name[NAMSIZ];
		 probability wehit, theyhit, arrowhit;
		 statistic htokill, damage, atokill; } ltmrec;

typedef struct { stuff type; 
                 int   count, phit, pdam, charges, traits;
		 char  *str;  } invrec;

typedef struct { int activity[T_LISTLEN];
		 int timestamp; } timerec;
