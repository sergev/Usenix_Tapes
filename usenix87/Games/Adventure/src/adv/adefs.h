/*
**		    Copyright (c) 1985	Ken Wellsch
**
**     Permission is hereby granted to all users to possess, use, copy,
**     distribute, and modify the programs and files in this package
**     provided it is not for direct commercial benefit and secondly,
**     that this notice and all modification information be kept and
**     maintained in the package.
**
*/

#define ADV		"adv"
#define FREEZER		"freezer"
#define	INITIAL		0
#define	REPEAT		500
#define	ERROR		(-1)
#define	OBJECTS		120
#define	PLACES		307
#define	VARS		50
#define	SYMTABREC	13000
#define MAXKEY		14000
#define	MAXOPS		58
#define	BUFSIZE		500
#define	TBUFSIZE	3000
#define CACHESIZE	100
#define MAXCACHE	4096
#define	TABSIZ		128
#define	LEXSIZ		20
#define	MAXLINE		134

#define	YES		1
#define	NO		0
#define	EOS		0
#define NIL		((struct symstr *)0)
#define	OK		1
#define MATCH		6

#define ADD		 0
#define AND		 1
#define ANYOF		 2
#define APPORT		 3
#define AT		 4
#define BIC		 5
#define BIS		 6
#define BIT		 7
#define CALL		 8
#define CHANCE		 9
#define DEFAULT		10
#define DEPOSIT		11
#define DIVIDE		12
#define DROP		13
#define ELSE		14
#define EOF		15
#define EOI		16
#define EOR		17
#define EVAL		18
#define EXEC		19
#define FIN		20
#define GET		21
#define GOTO		22
#define HAVE		23
#define IFAT		24
#define IFEQ		25
#define IFGE		26
#define IFGT		27
#define IFHAVE		28
#define IFKEY		29
#define IFLE		30
#define IFLOC		31
#define IFLT		32
#define IFNEAR		33
#define INPUT		34
#define ITLIST		35
#define ITOBJ		36
#define ITPLACE		37
#define KEYWORD		38
#define LDA		39
#define LOCATE		40
#define MOVE		41
#define MULT		42
#define NAME		43
#define NEAR		44
#define NOT		45
#define OR		46
#define PROCEED		47
#define QUERY		48
#define QUIT		49
#define RANDOM		50
#define SAY		51
#define SET		52
#define SMOVE		53
#define STOP		54
#define SUB		55
#define SVAR		56
#define VALUE		57

#define	LINELEN		2
#define LEXLEN		5
#define	MAXCALLS	10
#define	MAXDOS		2

#define OBJKEY		8000
#define MAXOBJECTS	120
#define MAXOTEXT	20
#define MAXOCODE	4
#define PLACEKEY	4000
#define MAXPLACES	350
#define MAXPCODE	8
#define VERBKEY		2000
#define MAXVERBS	160
#define MAXVCODE	12

#define	MOVED		001
#define	BRIEF		002
#define	FAST		004
#define	LOOKING		010
#define	BEEN		002
#define	DUAL		010

#define	XOBJECT		0100000
#define	XVERB		0040000
#define	XPLACE		0020000
#define	BADWORD		0010000

#define	INIT		0
#define	LABEL		1
#define	VERB		2
#define	PLACE		3
#define	TEXT		4
#define	OBJECT		5
#define	VARIABLE	6
#define	NULL		7

#define	INHAND		(-1)
#define	REPLACE		'#'
#define	NULLTXT		7000
#define	BLANKTXT	7001
#define	OKTXT		7002

#define forever		for(;;)

#define class(key)	(clss[(key)/1000])
#define indx(key)	((key)%1000)
#define logical(instr)	(ltab[(instr)])
#define opnum(opcode)	(opn[(opcode)])
#define where(key)	(objloc[indx(key)])

#define CACHE				/* see `cache.c' for details */

struct symstr
{
	char		*s_nam ;
	struct symstr	*s_nxt ;
	short int	 s_val ;
} ;

extern char opn [] ;
extern struct symstr *symtab [] ;

extern char ltab[] ;

extern char token [] ;

extern int linlen ;
extern int linewd [] ;

extern char lex[LEXLEN][LEXSIZ] ;

extern short int objval[], objbit[], objloc[] ;

extern short int plcbit[] ;

extern short int varval[], varbit[] ;

extern short int codebuf[] ;
extern char textbuf[] ;

extern int nrep, ninit, nvars, nobj, nplace ;
extern int here, there, status, argwd[] ;

extern int dbunit ;
extern char clss[] ;
