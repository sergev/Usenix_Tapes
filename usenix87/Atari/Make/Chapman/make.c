#define NDEBUG
#define MAKEINI  "MAKE.INI"
/*
 * make.c
 *
 *      An imitation of the Unix MAKE facility
 *
 *      Copyright (C) 1984 by Larry Campbell, 73 Concord St., Maynard, Mass.
 *      Rewritten w/modifications by Mike Hickey, University of DC
 *
 *      This software may be freely copied and disseminated for noncommercial 
 *      purposes, if and only if this entire copyright statement and notice 
 *      is included intact.  This software, and the information contained 
 *      herein, may not be used for commercial purposes without my prior 
 *      written permission.
 *
 *      This program runs a new shell (COMMAND.COM) for each command specified 
 *      in the makefile.  This, I recommend that you put a copy of the shell in
 *      ramdisk (if you have one).  Assuming your ramdisk is called, say, drive
 *      F:, you would:
 *      
 *
 *              COPY A:\COMMAND.COM F:
 *              SET COMSPEC=F:\COMMAND.COM
 *
 */

/* Certain portions of this software are Copyright (C) 1985 by Dan Grayson,
   2409 S. Vine St, Urbana, IL 61801, namely all those lines which differ
   between versions 2.10 and 2.11.  Qualitative descriptions of these 
   changes are described in the edit history below.
	Provided this copyright notice is not changed, and VERS211 below
   is not changed, and VERS211 is still prints when 'usage()' runs, these
   portions may be freely copied and used for any purpose, with one exception,
   namely:  those persons or corporations who claim a copyright on this
   program or a part of it may not use these portions for any commercial
   purpose whatsoever.  In particular, they may not collect royalties on
   any version of this program which includes these portions. */

/* Certain portions of this software are Copyright (c) 1986 by John Chapman,
   3635 Lozells Ave., Burnaby B.C., Canada, namely all those lines which
   differ between versions 2.11 and 3.00.  These changes were/are necessary
   to make this program function on the Atari 520 ST.  Anyone can do anything
   they want with the code I have included here with the exception of commercial
   use including, but not limited to, selling the software or charging any
   sort of fee for distributing it or any selection of software which 
   includes it.
*/

/*
 * Edit history:
 *
 *  3.00	Made it compilable by the Atari ST developement kit C
 *		compiler.  Made the necessary changes for it to function
 *		in a reasonable manner under the Atari operating system.
 *		No longer runs a shell to execute commands.
 *
 *  2.12	Made it compilable by Lattice C version 2.14. Use '-dLATTICE'
 *		on the command line for this. Default is Microsoft C.
 *  2.11        Fixed breakout_symbols, which tried to return a pointer to
			a local variable!
		Made symbol substitution occur in all lines, not just shell
			command lines.
		Fixed breakout_symbols, which blew up when a symbol was 
			undefined.
		Allowed blank lines, which are ignored now.
		Change command line length to 1000.
		Fixed it so MAKE, when no targets are specified on the command
			line, will simply make the first target in the makefile.
		Remove the command line symbol definition option.
		Changed the line continuation character to \ (it was - )
		Now symbol definition lines do NOT begin with $.
		Fixed numerous bugs dealing with character arrays and their
			lengths.
		Now a shell command line which begins with @ is not echoed.
		A shell command line beginning with + is executed through
			command.com ; this makes io redirection and pipes
			available, but the exit code of the program cannot
			be checked due to a misfeature of command.com.
		A shell command line beginning with - may return a nonzero
			exit code without halting 'make'.
		Fixed it so a target:prerequisite line followed by no how-to
			lines is interpreted not as an error, and not as 
			sharing the how-to lines following the next
			target:prerequisite line, but is considered fulfilled
			by no action other than making all the 
			prerequisites.
		Fixed the bug which meant the return code from the commmand
			was never dicovered.  This resulted from using 
			"system", which uses "command.com", which hides the
			return code of the program it runs.  Resident 
			commands can still be used, nevertheless.
		Error messages now include the line number of the makefile,
			if relevant.
		Made the return code of the command print out if nonzero.
		Now the copyright notice only prints when the usage appears.
		Convert to Microsoft vers 3.00, large memory model.
				- dan grayson
 *  2.10        Fix bug in abort routine, update copyright notice
 *  2.09        Set up for command line parsing of macros
 *  2.08        Remove edit 2.05; keep debug a compile-time option
 *  2.07        Finish macro parsing
 *  2.06        Add initial code for macro handling
 *  2.05        Add -d (debug) switch
 *  2.04        Add error message handling (doserror).
 *  2.03        Add -i (ignore errors) switch.
 *  2.02        Add -s (silent run) switch.
 *  2.01        Convert to Lattice 2.14.  Clean up code for faster execution.
 *  1.11        Make default makefilename be MAKEFILE.
 *  1.10        Add -n (trace) switch.
 *  1.09        Allow multiple targets before colon.
 *  1.08        Cleanup and speedup of makefile parsing.
 *  1.07        Add continuation lines (hyphen as last char of line)
 *  1.06        Don't need to make dummy FCBs, zero seems to work OK.
 *  1.05        Fix bug finding COMSPEC when it's not first env variable
 *  1.04        Wordier usage text, including copyright notice.
 *              Remove printf's (except when DEBUG on) to shrink EXE
 *  1.03        Don't uppercase shell command and fix datetime bug
 *  1.02        Random cleanup
 *  1.01        The beginning
 */


#define VERSION "MAKE ver. 2.10 Copyright (C) 1984 by Larry Campbell, Maynard Mass."
#define VERS211 "MAKE ver. 2.12 Portions copyright (C) 1985 by Dan Grayson, Urbana IL."
#define VERS300 "MAKE ver. 3.00 Portions copyright (c) 1986 by John Chapman, Vancouver B.C.\n"

#define EOS \0

#define perror error

#define assert(a) if (!  (a) ) printf("assert failed")

#ifdef NDEBUG
#undef assert
#define assert(x) {}
#endif

#include "define.h"
#include "osbind.h"
#include "tosdefs.h"
#include "stdio.h"
#include "ctype.h"
#include "mstring.h"
extern char *strchr();
typedef long ulong;


#ifdef SHELL
char *dos_commands[] = {
      "dir", "type", "rem", "pause", "date", "time",
       "ren", "rename", "ver", "vol", "break", "verify",
      "mkdir", "md", "exit", "ctty", "echo", "if", "cls",
      "chdir", "cd", "rmdir", "rd", "copy", "del", "erase",
			 NIL };

#endif

#define PREREQ_MAGIC 123
#define FILE_MAGIC 543
#define SHELL_MAGIC 678
#define TARG_MAGIC 987
#define SYMBOL_MAGIC 653
#define MAXLIN 1000
#define SYMLEN 1000
#define MAXTARGETS 100

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#define EOS '\0'                /* End Of String */
#endif

#define LINESIZE 1000            /* max length of input line */
#define MAXCOMMON 8             /* max no. of targets with common prereqs */
#define LONGNEGINF 0x00000000L	/* earliest possible file date */
#define LONGPOSINF 0x7fffffffL	/* latest possible file date */


#define tfree(x) if (x) free(x),x=NULL


extern int linenumber;   /* defined in mstring.c */
char  *talloc();
long getdatetime ();

    static int dontworry=0;


struct {
    char reserved[21];
    char attr;
    unsigned time, date;
    long f_size;
    char pname[13];
} 
find_buf;

/*
 * MAKE parses the make file and constructs a somewhat tangled
 * directed graph describing dependencies.  There is a list of
 * TargNode structures, each of which points to a list of
 * prereq_node structures.  These both point to FileNode structures,
 * which contain information about files such as date-time of write
 * and the filename.  To keep things simple, MAKE insures that only
 * one FileNode exists for any given file.
 */

typedef struct FileNode {
#ifndef NDEBUG
    int magic;
#endif
    char *fname;
    char *chain;
    }    *fileptr;
fileptr  FileNodeList, NewFileNode ();

typedef struct TargNode {
#ifndef NDEBUG
    int magic;
#endif
    char *next;
    fileptr file;
    struct prereq_node *PreqList;
    struct shell_node *shell_list;
    }    *targptr;
targptr target_list, new_target (), lookup_target ();

typedef struct prereq_node {
#ifndef NDEBUG
    int magic;
#endif
    char   *next;
    fileptr file;
    } *preqptr;
preqptr NewPreqNode ();

typedef struct shell_node {
#ifndef NDEBUG
    int magic;
#endif
    char *next;
    char *command;
    int quiet, ignore, useshell;
    } *shellptr;

typedef struct symbol_node {
#ifndef NDEBUG
    int magic;
#endif
    char *token, *value;
    char *next;
    }    *symbptr;
symbptr SymbolList;

static char *makefilename;
static int status, tracing, quietly, ignore_errors;
static targptr first_targ;

#ifdef DEBUG
#define dumpsh(a) printf(" %8lx %8lx \n %s\n ",a,(a->next),(a->command))

dumppr(tar)
targptr tar;
{char *p;
 preqptr   *pp;
p=(tar);
printf("TARGET: %s First preq. at: %8lx\n",p->file->fname,p->PreqList);
pp= p->PreqList;
for ( ; pp; pp= pp->next)
	printf("Prereq %8lx to %8lx : %8lx %s\n",
		pp,(pp-1+(sizeof(struct prereq_node))),
		pp->next,pp->file->fname);

}
#endif

usage ()
{
    puts (VERS300);
    puts (VERS211);
    puts (VERSION);
    puts ("This program may be copied freely for noncommercial purposes.  It may");
    puts ("not be copied for commercial use without the author's written permission.\n");
    puts ("This  program is an imitation of the MAKE program supplied with Unix.");
    puts ("It works somewhat like Unix MAKE (and VAX/VMS MMS).\n");
    puts ("Default rules are read from the file MAKE.INI .\n");
    puts ("Usage: make [target ...] [options ...]");
    puts ("Options:");
    puts ("        -f filename specify makefile, default is MAKEFILE");
    puts ("        -i          ignore errors while processing");
    puts ("        -n          trace and print, but don't execute commands");
    puts ("        -s          suppress MAKE echoing commands");
    exit (1);
}

main (argc, argv)
int argc;
char **argv;
{
    int
	argi,
	targi,
	linlen;

    char  *targname[MAXTARGETS];

    sbrk(8192);
    makefilename = "MAKEFILE";
    tracing = quietly = ignore_errors = FALSE;
    target_list = 0;
    SymbolList = 0;
    FileNodeList = 0;
    targi = 0;
    for (argi = 1; argi < argc; argi++) {
	if (argv[argi][0] == '-')               /* switch */
	    switch (argv[argi][1]) {            /* switch character */
	    case 'f':
		if (argi < (argc - 1))      makefilename = argv[++argi];
		else                        usage ();
		break;
	    case 'i':                ignore_errors = TRUE;             break;
	    case 'n':                tracing = TRUE;                   break;
	    case 's':                quietly = TRUE;                   break;
	    default:                 usage ();
	    }
	else {                          /* target name */
	    if (targi == MAXTARGETS)    error ("Too many target files");
	    targname[targi] = strperm (argv[argi]);
	    targi++;
	}
    }
    if (tracing && quietly) quietly = 0;
{ FILE *fd, *fopen();
	if (fd = fopen(MAKEINI,"r")) parse(fd);
	first_targ=NULL;
	if (fd = fopen(makefilename,"r")) parse(fd); else /*no makefile*/;
}

    if (targi)
	    for (argi = 0; argi < targi; argi++) make (targname[argi]);
    else if (first_targ) make(first_targ->file->fname);
    else error("No targets specified");

    return 0;           /* need good return code */
}

parse (fd)
FILE *fd;

{

    int targi=0,   i;
    char  c,  *sp,  *dp;
    int	  wlen;
    mstring input=NULL;
    targptr targ[MAXTARGETS];


    while (1) {
#ifdef DEBUG
printf("TOP of major loop in parse\n");
#endif
	tfree(input);
	input = mfgets(fd);
	if (input==NULL) break;
#ifdef DEBUG
	printf ("Makefile Input: \"%s\"\n", input);
#endif
	sp = input;
	passpace(&sp);
#ifdef DEBUG
if (targi>0) dumppr(targ[0]);
#endif
	if (*sp==0 || *sp=='!' || *sp=='#') continue;
	   /* ignore comment lines and blank lines */

	if (isspace(*input)) {   /* if leading space, then this is a shell line */
		if (targi == 0) error("Target line must come before shell lines");
		sp = input; passpace(&sp);
		for (i = 0; i < targi; i++)  NewShellLine (targ[i], sp);
		continue;
	}

	{               /* substitute for symbols - this will be done later
			   for shell lines, to take special symbols like $*
			   into account, which can only be known at run time
			   */
	      breakout_symbols(&input);
	}

	{       /*** check for the form 'name = value'   ***/
		char *endword;
		sp=input;
		password(&sp);    endword = sp;
		passpace(&sp);
		if (*sp == '=') {
		    targi=0;
		    sp++;
		    *endword = EOS;
		    SetSymbol (input, sp) ;
		    continue;
		}
	}                               /* end of macro parsing */


	/**** now we know this is a 'targets : prerequisite' line ***/


	targi=0;
	for ( dp = sp = input; 1 ; wlen++, sp++)   /*** collect the targets ***/
	    if (*sp == ':'
#ifdef DRIVES
		&& (wlen !=2 || isspace(sp[1])) /* allow drive specifiers */
#endif
		|| isspace (*sp)) { /* space or colon ends target name */
		if (targi == MAXTARGETS) error ("Too many targets in one line");
		c = *sp;  *sp = EOS;
		targ[targi] = new_target (dp);
		*sp = c;
		if (first_targ == NULL && *dp != '*') first_targ=targ[targi];
		targi++;
		passpace(&sp);
		if (*sp == ':') break;
		dp = sp;
		wlen = 0;
	    }
	    else if (*sp == EOS) error ("no colon");

	sp++;
	if (targi == 0) error ("No target file before ':' ");
	while(1) {              /*** collect the prerequisites ***/
	    passpace(&sp);
	    if (*sp == EOS) break;            /* end of line */
	    dp = sp;                    /* beginning of prereq word */
	    passnonsp(&sp);
	    c = *sp;
	    *sp = EOS ;                /* end of prereq word */
	    for (i = 0; i < targi; i++) {
		LinkPreq (targ[i], NewFileNode(dp) );
#ifdef DEBUG
dumppr(targ[i]);
#endif
	    };
	    *sp = c;
	}
    }   /* end while */
    tfree(input);
    fclose (fd);
    linenumber = 0;
#ifdef DEBUG
printf("Leave Make\n");
#endif

}

/*
 * new_target
 *
 *      We think we have a new target;  this routine scans the
 *      target list and if we've already seen this name returns
 *      the node we already built.  Otherwise, a new node is
 *      allocated and linked.
 */

targptr new_target (name)
char *name;
{
    targptr targ ;

#ifdef DEBUG
    printf ("new_target (\"%s\")\n", name);
#endif
    for ( targ = target_list; targ ; targ = targ->next ) {
	assert (targ->magic == TARG_MAGIC);
    }
    targ = (targptr) talloc (sizeof (struct TargNode));
#ifndef NDEBUG
    targ->magic = TARG_MAGIC;
#endif
    targ->file = NewFileNode (name);
    targ->next = target_list;
    targ->shell_list = NULL;
    targ->PreqList = NULL;
    target_list = targ;

#ifdef DEBUG
   printf("new_target returns\n");
#endif
    return targ;
}

SetSymbol (name, value)
char *name, *value;
{
    symbptr sym;
#ifdef DEBUG
    printf("enter SetSymbol\n");
#endif
    for (sym = SymbolList; sym; sym = sym->next)
	 if (0==strcmp(sym->token,name)) {
		free(sym->value);
			sym->value = strperm(value) ;
		return;
		}
    sym = (symbptr) talloc (sizeof (struct symbol_node));
#ifndef NDEBUG
    sym->magic = SYMBOL_MAGIC;
#endif
    sym->token = strperm (name);
    sym->value = strperm (value);
    sym->next = SymbolList;
    SymbolList = sym;
#ifdef DEBUG
    printf("leave SetSymbol\n");
#endif
}

/*
 * NewShellLine
 *
 *      Add a shell command to the list of commands needed to update
 *      a target file
 */

NewShellLine (targ, line)
targptr targ;
char *line;
{
    shellptr snode, new;

#ifdef DEBUG
    printf ("NewShellLine (%lx, \"%s\")\n", targ, line);
#endif
    new = (shellptr) talloc (sizeof (struct shell_node));
    new->next = 0;

#ifndef NDEBUG
    new->magic = SHELL_MAGIC;
#endif
    new->useshell = new->ignore = 0;
    new -> quiet = quietly ;
    for ( ; 1 ; line++, passpace(&line) )
       if      (line[0] == '@')  new->quiet = 1;
       else if (line[0] == '+')  new->useshell = 1;
       else if (line[0] == '-')  new->ignore = 1;
       else break;

    new->command = strperm(line);
    snode = targ->shell_list;
    if (snode) {
	assert (snode->magic == SHELL_MAGIC);
	while (snode->next) {

		snode = snode->next;
		assert (snode->magic == SHELL_MAGIC);
		}

	snode->next = new;

    }
    else
	targ->shell_list = new;
#ifdef DEBUG
printf("Result:\n");
#endif
}

/*
 * LinkPreq	
 *
 *      Link a new prerequisite file onto prereq list for a target.
 */

LinkPreq (targ, fnode)
   targptr targ;  fileptr fnode;
{
preqptr p,np=NewPreqNode(fnode);

#ifdef DEBUG
    printf ("LinkPreq (\"%s\")\n", fnode->fname );
#endif

    p = targ->PreqList;
if (p) {
	assert(p->magic == PREREQ_MAGIC);
	while (p->next) {p=p->next; assert(p->magic == PREREQ_MAGIC);};
	p->next = np;
	}
else {
    targ->PreqList = np;
     };

#ifdef DEBUG
    printf("leave LinkPreq\n");
#endif
    }

/*
 * NewPreqNode
 *
 *      Allocate and return a new prerequisite node
 */

	preqptr 
NewPreqNode (fnode)
	fileptr fnode;
{
    preqptr new;

#ifdef DEBUG
    printf ("NewPreqNode (struct FileNode *%lx, \"%s\")\n",fnode,fnode->fname);
#endif
    new = (preqptr) talloc (sizeof (struct prereq_node));
    new->next = NULL;
#ifndef NDEBUG
    new->magic = PREREQ_MAGIC;
#endif
    new->file = fnode;

#ifdef DEBUG
    printf("leave NewPreqNode\n");
#endif
    return new;

}

/*
 * NewFileNode
 *
 *      Return FileNode pointer for a file;  returns pointer to
 *      existing FileNode if this file already seen, else allocates
 *      and inits a new node
 */

	fileptr
NewFileNode (name)
	char *name;
{
    fileptr fnode;

#ifdef DEBUG
    printf ("NewFileNode (\"%s\")\n", name);
#endif
    for ( fnode = FileNodeList; fnode; fnode = fnode->chain) {
	assert (fnode->magic == FILE_MAGIC);
	if (strcmp (name, fnode->fname) == 0) {
#ifdef DEBUG
	    printf ("NewFileNode returning existing node\n");
#endif
	    return fnode;
	}
	
    }

    fnode = (fileptr) talloc (sizeof (struct FileNode));
    fnode->fname = strperm (name);

#ifndef NDEBUG
    fnode->magic = FILE_MAGIC;
#endif
    fnode -> chain = FileNodeList;
    FileNodeList = fnode;
#ifdef DEBUG
    printf("NewFileNode returning new node\n");
#endif
    return fnode;
}

/*
 * getdatetime
 *
 *      Return date-time of a file squished into a ulong so compares
 *      are easy
 */

ulong getdatetime (name,def)
	char *name;
	ulong def;
{
    ulong datetime;
    ulong dma_old;
    WORD attr;

#ifdef DEBUG
    printf("getdatetime(\"%s\")\n",name);
#endif

    /* get current DMA address */
    dma_old = Fgetdta();           /* and save for later restoration */
    {  char *p = (char *) & find_buf;
	Fsetdta(p);  /* set DMA to GNJFN block */
    }
    attr=0;
    status = Fsfirst(name,attr);
    if (status & 1) {
#ifdef DEBUG
	printf ("File \"%s\" does not exist\n", name);
#endif
	return def;
    }
     /* restore DMA address */
     Fsetdta(dma_old);

#ifdef DEBUG
    printf ("filespec = \"%s\", date = %4x, time = %4x, size = %ld\n",
	find_buf.pname, find_buf.date, find_buf.time, find_buf.f_size);
#endif
    datetime = (ulong) find_buf.date;
    datetime = datetime << 16;
    datetime = datetime + ( (ulong ) find_buf.time);

    return datetime;
}

/*
 * make (name)
 *
 *      This routine actually does the work.  It scans the list of
 *      targets parsed from the makefile, and checks the target's
 *      prerequisites date/time values against the target's.  If
 *      the prerequisite is itself a target (present in target_list),
 *      we call make recursively to check it.  Then, if any of our
 *      prerequisites are newer than we are, we execute all our shell
 *      commands.  If there are no prerequisites specified at all, then
 *      also execute all our shell commands.
 */

	int
make (targname)         /* use fnode instead of fname here */
    char *targname;
{          
    targptr targ,lookup_target();
    int	    t,TryDefault();
    preqptr prereq;
    ulong getdatetime(),NewestPreq=LONGNEGINF+1, targtime=LONGNEGINF;

#ifdef DEBUG
    printf ("Making %s\n", targname);
#endif

    if ((targ = lookup_target (targname)) == 0)
	return TryDefault( targname, 0 );
#ifdef DEBUG
printf("make target is: %8lx %s\n",targ,targ->file->fname);
printf("prerequisites start at %8lx\n",targ->PreqList);
#endif
    prereq = targ->PreqList; 
    if (prereq)
	{
	for ( ; prereq; prereq = prereq->next) {
	    ulong date;
	    make (prereq->file->fname);             /* recursively make */
	    date =  getdatetime(prereq->file->fname,LONGPOSINF);
	    if (date > NewestPreq) NewestPreq = date;
	}
	}
targtime = getdatetime(targ->file->fname, LONGNEGINF);

#ifdef DEBUG
	printf ("Target \"%s\" datetime is %08lx, newest prereq is %08lx\n",
		targ->file->fname, getdatetime(targ->file->fname,LONGNEGINF),
		 NewestPreq);
#endif
if (targ->shell_list)
    if (targtime < NewestPreq) build(targ);
    else;
else { /* no shell lines specified, look for *. lines */

	int i;
	dontworry++;
	i= TryDefault( targname, targtime < NewestPreq );
	dontworry--;
	return i;
     }
return 1;


}

	int
TryDefault(targname, outofdate)
	char *targname;
	int  outofdate; /* !=0 if target out of date with some prerequisite */
{
	targptr targ;
	long getdatetime();
	char * ext = strchr (targname, '.');
	char  ext1[12],  ext2[12];

#ifdef DEBUG
printf("TryDefault: %s\n",targname);
if (outofdate) printf("out of date\n");
#endif

	dontworry ++;
	if (ext != NULL) {
	    strcpy(ext1,ext);
	    strupper(ext1);  /* make all comparisons upper case */
	    for (targ = target_list ; targ ; targ = targ -> next ) {
		strcpy(ext2,targ->file->fname);
		strupper(ext2);
		if (targ->file->fname[0] == '*' &&
		    0 == strcmp ( ext1 , ext2+1 ) ) {
			char *root; 
			char *cname;
			int worked;
			ulong cnamedate;
			root = msubstr( targname,0,((int)(ext-targname)));
			cname = mstrcat( root ,targ->PreqList->file->fname+1 );
			worked = make ( cname ) ;
			SetSymbol ( "*" , root ) ;
			if (worked && !outofdate)
			  cnamedate= getdatetime(cname,LONGNEGINF);
			free(cname);
			free(root);
			if (!worked) continue; /* try next*/
			if (outofdate || 
			    cnamedate > getdatetime(targname,LONGNEGINF))
				build(targ);
			goto ret1;
			}
	    }
	}

	if (getdatetime(targname,LONGNEGINF) > LONGNEGINF) goto ret1;

	ret0:                   /* unsuccessful return */
	    if (--dontworry) return 0;
	    else error (mstrcat("Don't know how to make ",targname));

	ret1:                   /* successful return */
	    dontworry--;
	    return 1;
	}




/*
 * build
 *
 *      Invoke shell commands to build a target file
 */

build (targ)
    targptr targ;
    {
    shellptr snode;
    char *cmd;
    int  runsts = 0;
    WORD drv = 0;


#ifdef DEBUG
    printf ("Building \"%s\"\n", targ->file->fname,
				 targ->PreqList->file->fname);
#endif
    for ( snode = targ->shell_list; snode; snode = snode->next, free(cmd) ) {
	char *p, **q, *cmdname;

	assert (snode->magic == SHELL_MAGIC);
	cmd = strperm(snode->command);

	breakout_symbols(&cmd);
       /* notice that this may introduce a space
	at the beginning of the command line */
	cmdname = cmd; passpace(&cmdname);
	
	/*if (!snode->quiet)*/   fputs (cmdname, stdout);
	if (tracing) {
	       puts ("");   /* EXEC does newline, so must be faked */
	       continue;
	       }

	p = cmdname ; passnonsp(&p);
	if (*p)  *p++ = EOS ;       /* terminate the name of the cmd */

	/* The following converts the command to lower case and checks
	   to see if it is a builtin shell command.  This is not much
	   use with command.com on the atari since it will not execute
	   commands given on the invokation line - however there will
	   (presumably) be a better shell one day so I've left the code
	   in place. Note that this also means you should not use the
	   make option of forcing a command to be executed by the shell.
	   JC
	*/
#ifdef SHELL
        strlwr(cmdname);          /* lower  case for comparison */
	for (q=dos_commands ; *q ; q++) if (0==strcmp(*q,cmdname)) break;

	if (*q || snode->useshell)        /* must we use command.com ? */
#endif
#ifndef SHELL

	if (snode->useshell)
#endif
		if (0==strcmp(cmdname,"chdir") || 0==strcmp(cmdname,"cd"))
			if (passpace (&p) , *p) {   /* chdir with arg */
			    char *q=p;
			    passnonsp(&q);  *q = EOS;
			    runsts = Dsetpath(p);
			    }
			else {                      /* chdir without arg */
			    char name[200];
			    if (Dgetpath(name,drv)) {
				if (!snode->quiet) putchar('\n');
				fputs(name,stdout);
				}
			    else error("path name too long");
			     }
		else

			{                           /* resident command */
			WORD mode = 0;
			if (*p) *--p = ' ';         /* splice command line */
			if (strlen(cmdname) > 128) error("shell command line too long");
			runsts = Pexec(mode,"command.com",cmdname,NULL);
			}
	else

		{                                   /* transient command */
		WORD mode = 0;
		char *index();
		int i;
		char cmdprg[128],parms[128];

		if (strlen(p)+1 > 128) error("shell command line too long");
		strcpy(parms,"  ");
		parms[0]= (char ) (1+strlen(p));
		strcat(parms,p);
		if (!snode->quiet) putchar ('\n');
		strcpy(cmdprg,cmdname);
		strupper(cmdprg);
		/* add .prg suffix if command name has no suffix */
		if (0 != index(cmdprg,'.')) strcat(cmdprg,".PRG");
		runsts = Pexec(mode,cmdprg,parms,NULL);


		/* can't use 'system()' here, because command.com does not
		   return the exit code of the program */
		}
	putchar('\n');      /* some programs do not end with return */
	
	if (runsts != 0 && !snode->ignore && !ignore_errors)
	      printf ( " --- return code %d ---\7", runsts),
	      exit(runsts);
		     
    }

}

	targptr 
lookup_target (name)
char *name;
{
    targptr targ;

#ifdef DEBUG
printf("lookup_target: %s\n",name);
#endif
    for ( targ = target_list; targ ; targ = targ->next) {
#ifdef DEBUG
printf(" %8lx %s \n",targ,targ->file->fname);
#endif
	if (strcmp (name, targ->file->fname) == 0) break;
    };
#ifdef DEBUG
printf("\n lookup_target returns %lx\n",targ);
#endif
    return targ;

}

breakout_symbols (cmdlinptr)
char **cmdlinptr;
{
    char *cmdlin = *cmdlinptr, *cmd = talloc(LINESIZE+100);
    symbptr sym;
    char   symcmp[SYMLEN];
    int i, paren, cmdptr;

#ifdef DEBUG
    printf("breakout_symbols (\"%s\")\n", cmdlin);
#endif
    /* this routine doesn't check for overflow of the line ! */

    strcpy ( cmd, "");
    cmdptr = 0;
    while (1) {
	while (*cmdlin != '$' && *cmdlin != EOS) {
		if (cmdptr >= LINESIZE) error ("Line too long after symbol substitution");
		cmd[cmdptr++] = *cmdlin++;
		}
	if (cmdptr >= LINESIZE) error ("Line too long after symbol substitution");
	cmd[cmdptr] = EOS;
	if (0==*cmdlin) break;            /* end of line */
	cmdlin++;               /* pass the $ */
	/* now we know we are looking at a symbol */
	if (*cmdlin == '(') paren = 1, cmdlin++; else paren=0;
	for (i = 0; i < SYMLEN-1 && (*cmdlin == '*' || isalnum (*cmdlin)); )
	    symcmp[i++] = *cmdlin++;
	symcmp[i] = EOS;
	if (paren)
	   if (*cmdlin == ')') cmdlin++;
	   else puts ("No closing paren on shell line macro");
	for ( sym = SymbolList; 1 ; sym = sym->next) {
	    if (sym==NULL)  error ("Undefined symbol %s", symcmp );
	    assert (sym->magic == SYMBOL_MAGIC);
	    if (strcmp (sym->token, symcmp) == 0) break;
	}
	strcpy ( cmd + cmdptr , sym->value );
	cmdptr = strlen ( cmd ) ;
    }

    free(*cmdlinptr);
    *cmdlinptr = strperm(cmd);
    free(cmd);

#ifdef DEBUG
    printf ("breakout_symbols returning (\"%s\")\n", *cmdlinptr);
#endif
}

strlwr(p)
char *p;
{
	while (*p)
	{	*p= toascii(*p);
		if (isupper(*p)) *p = tolower(*p);
		p++;
	}
}

strupper(p)
char *p;
{
	while (*p)
	{	*p= toascii(*p);
		if (islower(*p)) *p = toupper(*p);
		p++;
	}
}

