#include <stdio.h>
#include "make.h"

/*
 * Macro processing
 */


/*
 * Perform macro substitution from 'orig' to 'dest'.
 * Return number of macro substitutions made.
 * A macro reference is in one of two forms:
 *		<MACCHAR>(macro-name)
 *  	or	<MACCHAR><single-character>
 *
 * "<MACCHAR><MACCHAR>" expands to a single '<MACCHAR>'
 */
mexpand(orig, dest, destsiz, macchar)
char *orig, *dest;
int destsiz;
char macchar;
{
	char *s, *d, mname[STRSIZ];
	int di, count;
/*	MACRO *m; */

	di = count = 0;
	for(s=orig; *s;)
		if(*s == macchar)
		{
			if(*++s == macchar)
			{
				if(di < destsiz-1) dest[di++] = *s++;
				continue;
			}

			if(!*s) break;
			d = mname;
			if(*s != '(') *d++ = *s++;
			else
			{
				for(++s; *s && *s!=')';) *d++ = *s++;
				if(*s != ')') puts("Missed matching ')'");
				else ++s;
			}
			*d = 0;
			if((d = gmacro(mname)) == NULL) {
				fputs("Undefined macro: ", stderr);
				fputs(mname, stderr);
				fputc('\n', stderr);
			}
			else {
				while(*d && di < (destsiz - 1))
					dest[di++] = *d++;
				++count;
			}
		} else if(di < destsiz-1)
			dest[di++] = *s++;

	dest[di]=0;
	return count;
}


/*
 * Define a macro.
 * Give the macro called 'name' the string expansion 'def'.
 * Old macro-names are superseded, NOT replaced.
 * Return ERROR if can't define the macro.
 */
defmac(name, def)
char *name, *def;
{
	MACRO *m;

	if((m = (MACRO *)malloc(sizeof(MACRO))) == (MACRO *) NULL) allerr();
	if((m->mname = (char *)malloc(strlen(name)+1)) == NULL) allerr();
	if((m->mvalue = (char *)malloc(strlen(def)+1)) == NULL) allerr();

	strcpy(m->mname, name);
	strcpy(m->mvalue, def);
	m->mnext = mroot;
	mroot = m;
}


/*
 * undefmac - undefine a macro.
 * Return 0 if macro was succesfully undefined, -1 if not found.
 */
undefmac(name)
char *name;
{
	MACRO *m = mroot;
	MACRO *prev = (MACRO *)NULL;

	while(m != (MACRO *) NULL && strcmp(name, m->mname))
	{
		prev = m;
		m = m->mnext;
	}

	if(m == (MACRO *) NULL) return -1;
	if(prev == (MACRO *) NULL) mroot = m->mnext;
	    else prev->mnext = m->mnext;

	free(m->mname);
	free(m->mvalue);
	free(m);
	return 0;
}


/*
 * Lookup a macro called 'name'.
 * Return a pointer to its definition,
 * or NULL if it does not exist.
 */
char *gmacro(name)
char *name;
{
	MACRO *m;

	for(m=mroot; m != (MACRO *) NULL; m=m->mnext)
		if(!strcmp(name, m->mname)) return m->mvalue;
	return NULL;
}
 //E*O*F macro.c//

echo x - make.c
cat > "make.c" << '//E*O*F make.c//'
#include <stdio.h>
#include <ctype.h>
#include "make.h"

/*
 *    MAKE - Maintain separate source files
 *
 *    SYNOPSIS
 *        MK [-f file] [-a] [-n] [-d] [-i] [-k] [name] ...
 *           f: use 'file' instead of default makefile
 *           a: assume all modules are obsolete (recompile everything)
 *           n: don't recompile, just list steps to recompile
 *           d: debugging (print tree, file info)
 *	     i: ignore return statuses from execution
 *	     k: if errors occur, propagate error status up tree; continue.
 *           name: module name to recompile
 *
 *    AUTHOR
 *        Landon M. Dyer, Atari Inc.
 *
 *    INCREDIBLY HACKED OVER BY
 *        Eric C. Brown University of Utah.
 *
 *    HACKS
 *        Added object library support (dummy names that inherit dates)
 *	  Added source library support (real parents)
 *	  Added direct execution capability.
 *	  Removed script file.
 */

#define INIT    "~INIT"              /* initialization macro */
#define DEINIT    "~DEINIT"          /* de-init macro */
#define BEFORE    "~BEFORE"          /* the per-root 'startup' method */
#define AFTER    "~AFTER"            /* the per-root 'wrapup' method */


char *mfiles[] = {            /* default makefiles */
    "makefile",

#ifdef VAXVMS
    "[-]makefile",
    "sys$login:makefile",
#endif

#ifdef MSDOS
    "..\makefile",
#endif
    ""
};


MACRO *mroot = (MACRO *) NULL;          /* root of macro-list */
FILENODE *froot = (FILENODE *) NULL;    /* root of filenode-list */
FILENODE *firstf =(FILENODE *) NULL;    /* the very first filenode */
char *modnames[MAXMODS];    /* module-names mentioned in commandline */
int execstat = 0;	    /* nonzero if started executing */
int modcount = 0;           /* #of module-names */
int debug = 0;              /* nonzero: turn on debugging */
int obsolete = 0;           /* nonzero: every file should be recompiled */
int noscript = 0;           /* nonzero: print methods on standard output */
int ignore_errors = 0;       /* nonzero: ignore error return codes */
int prop_errors = 0;         /* nonzero: propagate error status up tree */
DATE bigbang;               /* a date, the very earliest possible */
DATE endoftime;             /* a date, the very last possible */


main(argc, argv)
int argc;
char **argv;
{
    int arg, i;
    char *mfile = NULL;
    DATE adate();

    initrootdates();

    for (arg = 1; arg < argc; ++arg)
    if (*argv[arg] == '-') 
        switch (tolower(argv[arg][1])) {
            case 'f':
                if (++arg >= argc) {
                    fputs("-f needs filename argument.\n", stderr);
                    return;
                }
                mfile = argv[arg];
                break;
           case 'a':
               obsolete = 1;
               break;
           case 'n':
               noscript = 1;
               break;
           case 'd':
               debug = 1;
               break;
           case 'i':
               ignore_errors = 1;
               break;
           case 'k':
               prop_errors = 1;
               break;
           default:
               fputs("Unknown switch: ", stderr);
	       fputc(argv[arg][1], stderr);
	       fputc('\n', stderr);
               break;
        } 
    else if (modcount < MAXMODS)
         modnames[modcount++] = argv[arg];
    else {
        fputs("Too many module names.\n", stderr);
        return;
    }

     if (mfile != NULL)  {
         if (fmake(mfile) == -1) {
             fputs("Cannot open makefile '", stderr);
	     fputs(mfile, stderr);
	     fputs("'.\n", stderr);
	 }
     } else {
         for (i = 0; *mfiles[i]; ++i)
             if (fmake(mfiles[i]) != -1) 
	         break;
         if (!*mfiles[i])
             fputs("Cannot open makefile.\n", stderr);
    }
    if (debug) 
        prtree();
}


/*
 * Construct dependency tree from the makefile 'fn'.
 * Figure out what has to be recompiled, and write a script file to do that.
 */
fmake(fn)
char *fn;
{
    FILE *fp;

    if ((fp = fopen(fn, "r")) == (FILE *) NULL) 
        return -1;

    fparse(fp);
    determ();
    fclose(fp);
    return 0;
}


/*
 * Parse the input file, defining macros and building the dependency tree.
 */
fparse(fp)
FILE *fp;
{
    char ibuf[STRSIZ], ebuf[STRSIZ];
    char *strp, *tok1, *tok2, *s, *fgets();
    FILENODE *lastf = (FILENODE *) NULL, *addfile();

    for (;;)
    {
        if (fgets(ibuf, STRSIZ, fp) == NULL) break;
        mexpand(ibuf, ebuf, STRSIZ, MACCHAR);
        escape(ebuf, COMCHAR);
        s = ebuf + strlen(ebuf) - 1;       /* clobber last newline in string */
        if (s >= ebuf && *s == '\n') 
	    *s = '\0';
        if (*ebuf == '\t') {
            addmeth(lastf, ebuf+1);
            continue;
        }
        strp = ebuf;
        if ((tok1 = token(&strp)) == NULL) 
	    continue;
        if ((tok2 = token(&strp)) != NULL)
            if (!strcmp(tok2, DEFMAC)) {
                if (*strp) 
		    defmac(tok1, strp);
                else 
		    if (undefmac(tok1) < 0) {
             		fputs("Can't undefine macro '", stderr);
			fputs(tok1, stderr);
	   	        fputs("'.\n", stderr);
		    }
                continue;
            }
            else if (!strcmp(tok2, DEPEND)) {
                addmeth(lastf, gmacro(AFTER));	/* terminate last method */

                lastf = filenode(tok1);		/* init lastf */
                if (firstf == (FILENODE *)NULL) 
		    firstf = lastf;
                lastf->fmake = NULL;
                addmeth(lastf, gmacro(BEFORE));
                lastf->fflag |= ROOTP;

                addmanydepend(strp, lastf);
                continue;
            }
#ifndef FUNNYLIBS
            else if (!strcmp(tok2, ISLIB))
            {
                addmeth(lastf, gmacro(AFTER));

                lastf = filenode(tok1);
                if (firstf == (FILENODE *) NULL) firstf = lastf;
                lastf->fmake = NULL;
                addmeth(lastf, gmacro(BEFORE));
                lastf->fflag |= LIBRARY;
                lastf->fflag |= ROOTP;

                addtolibrary(lastf);
/* no archives here -- archives and libraries are mutually exclusive */
                while((tok1 = token(&strp)) != NULL)
                    (void) addfile(lastf, tok1);
                continue;
            }
#endif
            else addmanydepend(strp, lastf);
    }
    addmeth(lastf, gmacro(AFTER));
}

/*
 * scan tokens from strbuf and search for libraries and archives.
 * libraries look like foo [ bar baz mumble ]
 * archives look like foo ( bar baz mumble )
 * in either case, bar, baz, and mumble have parents of foo.
 * foo is added to the parentlist, if not already on the list.
 * bar, baz, and mumble are added to the dependency list of depend.
 * the command *cannot* be split across newlines without causing errors.
 * if you don't like that, well, life's a bitch and then you die.
 */
addmanydepend(strbuf, depend)
char *strbuf;
FILENODE *depend;
{
    char *tok1, *tok2;
    FILENODE *parent, *child, *addfile(), *addparent();
    
    tok1 = token(&strbuf);
    if (tok1 == NULL)
        return;
    tok2 = token(&strbuf);
    while (tok2 != NULL) {
#ifdef FUNNYLIBS
        if (!strcmp(tok2, BGNLIB)) {
             parent = addparent(tok1);	/* add tok1 to parent list */
    	     for (tok1 = token(&strbuf); /* skip over token in tok2 */
                  tok1 != NULL && strcmp(tok1, ENDLIB); /* go eol or end */
                  tok1 = token(&strbuf)) {	/* get next token */
                 if (tok1 == NULL) {
                     fputs("MAKE: Error in library defn.\n", stderr);
                     exit(2);
                 };
                 child = addfile(depend, tok1);
                 child -> fflag = LIBRARY;
                 child -> parent = parent;
             }					/* for */
	     tok1 = token(&strbuf);
	     tok2 = token(&strbuf);
	     continue;	/* the while */
	 }					/* if islib */
#endif
        if (!strcmp(tok2, BGNARC)) {
             parent = addparent(tok1);	/* add tok1 to parent list */
    	     for (tok1 = token(&strbuf); /* skip over token in tok2 */
                  tok1 != NULL && strcmp(tok1, ENDARC); /* go eol or end */
                  tok1 = token(&strbuf)) {	/* get next token */
                 if (tok1 == NULL) {
                     fputs("MAKE: Error in archive defn.\n", stderr);
                     exit(2);
                 };
                 child = addfile(depend, tok1);
                 child -> fflag = ARCHIVE;
                 child -> parent = parent;
             }					/* for */
	     tok1 = token(&strbuf);		/* get current token */
	     tok2 = token(&strbuf);		/* get lookahead token */
	     continue;	/* the while */
	 }					/* if isarc */
	 else {				/* nothing special -- */
	     (void) addfile(depend, tok1);	/* add dependency */
	     tok1 = tok2;		/* shift token */
	     tok2 = token(&strbuf);
	}
    }						/* while */
    if (tok2 == NULL && tok1 != NULL)	/* last token = not special */
        (void) addfile(depend, tok1);
}

/*
 * Determine sequence of recompiles from the creation dates.
 * If have anything to recompile, then create a script file full of commands.
 */
determ()
{
    FILENODE *f;
    int i;
    char *m;

    if (firstf ==(FILENODE *) NULL)            /* empty tree */
    {
        puts("No changes.");
        return;
    }

    if (modcount == 0) {
    examine(firstf, endoftime);
    }
    else {
        for (i = 0; i < modcount; ++i) {
            if ((f = gfile(modnames[i])) == (FILENODE *) NULL) {
           	fputs("Don't know how to make ", stderr);
		fputs(modnames[i], stderr);
	   	fputs(".\n", stderr);
                continue;
            }
            if ((f->fflag & ROOTP) == 0) {
             	fputc('\'', stderr);
		fputs(f->fname, stderr);
	   	fputs("' is not a root!\n", stderr);
                continue;
            }
            examine(f, endoftime);
        }
    }
    if (execstat) {
        if ((m = gmacro(DEINIT)) != NULL) {
            execute (m, noscript);
        }
	cleanuparchives();
    } else puts("No changes.");
}


/*
 * Examine filenode 'fnd' and see if it has to be recompiled.
 * 'date' is the last-touched date of the node's father
 * (or 'endoftime' if its a root file.)
 * Root files with NO dependencies are assumed not to be up to date.
 */
examine(fnd, date)
FILENODE *fnd;
DATE date;
{
    int rebuildp = 0, rval, errcode = 0;
    NODE *n;

#if DEBUG
    printf("Examining %s\n", fnd->fname);
#endif
    getdate(fnd);
    if (fnd->fnode == (NODE *) NULL && fnd->fflag & ROOTP)
        rebuildp = 1;        /* always rebuild root with no dependents */
    else             /* see if dependents need to be recompiled */
    for (n = fnd->fnode; n != (NODE *) NULL; n = n->nnext) {
        if ((rval = examine(n->nfile, fnd->fdate)) != 0) {
	    if (rval == ERROR) {
	        errcode = ERROR;	/* if error occurred, propagate up */
		fnd->fflag |= ERROR;
		fputs("Couldn't compile ", stderr);
		fputs(fnd->fname, stderr);
		fputs(" because of errors.\n", stderr);
	    }
            rebuildp = 1;
        }
    }

/* if ancestor recompiled or root, recompile, but not if error in ancestor */
    if (rebuildp && (fnd->fflag & ERROR) == 0) {
        recomp(fnd);
	if (fnd->fflag & ERROR) {
#if DEBUG
printf("Golly.  I got an error compiling %s.\n", fnd->fname);
#endif
	    return (ERROR);
        }
    }

    if (obsolete || laterdt(fnd->fdate, date) >= 0) {
        rebuildp = 1;
    }
    if (errcode)
        return errcode;
    else 
        return rebuildp;
}

/*
 * Make sure a filenode gets recompiled.
 */
recomp(f)
FILENODE *f;
{
    char *m;

    if (!execstat) {
	execstat = 1;
        if ((m = gmacro(INIT)) != NULL) {
            execute(m, noscript);
        }
    }

    if (f->fflag & REBUILT) return;
    if (!noscript) {			/* don't extract if printing steps */
        yankdependents(f);
    };
    if (f->fmake != NULL) {
        if (execute(f->fmake, noscript) != 0) {
	    if (!ignore_errors && !prop_errors)
	        exit(2);
	    else if (prop_errors) 
	        f->fflag |= ERROR;
	}
    }
    f->fflag |= REBUILT;
}

yankdependents(fnd)
FILENODE *fnd;
{
    NODE *n;

    for (n = fnd->fnode; n != (NODE *) NULL; n = n->nnext) {
#ifdef YANKDESCENDANTS
        yankdependents(n->nfile);
#endif

#if DEBUG
printf("yanking %s, flags are %d\n", n->nfile->fname, n->nfile->fflag);
#endif
        if ((n->nfile->fflag & ARCHIVE) && ((n->nfile->fflag & EXTRACT) == 0)){
                    /* if archived and not extracted */
            fputs("Extracting ", stdout);
            puts(n->nfile->fname);
            if (!noscript) {
                if (extract(n->nfile) == FAILURE) {
                    fputs("Extract failed -- I think I'll die now.\n", stderr);
                    exit(1);
                }
            };
            n->nfile->fflag |= EXTRACT;
        }
    }
}

/*
 * Complain about being out of memory, and then die.
 */
allerr() {
    fputs("Can't alloc -- no space left (I give up!)\n", stderr);
    exit(1);
}
//E*O*F make.c//

echo x - make.h
cat > "make.h" << '//E*O*F make.h//'
/* #define	VAXVMS	1 */		/* uncomment for VAX/VMS */
#define	MSDOS	1			/* uncomment for MSDOS */

#ifdef VAXVMS
#define ESCCHAR	`\\`		/* ok to use backslash on VMS */
#endif

#ifdef MSDOS
#define	ESCCHAR	'`'		/* since pathname char is backslash (yech) */
#endif

#define	MACCHAR '$'		/* macro-definition char */
#define	COMCHAR	'#'		/* comment char */
#define	DEFMAC	"="		/* macro-definition token */
#define	DEPEND	":"		/* dependency-definition token */
#ifdef FUNNYLIBS
#define BGNLIB  "["		/* start library token */
#define ENDLIB  "]"		/* end library token */
#else
#define ISLIB   "|"		/* Library definition token */
#endif
#define BGNARC  "("		/* start archive token */
#define ENDARC  ")"		/* end archive token */

#define	DEBUG	0
#define	STRSIZ	1024
#define	MAXMODS	50

#if DEBUG
#define NOREALEXECUTE		/* if set, don't really execute commands */
#define NOREALEXTRACT		/* if set, don't really extract files */
#define NOREALDELETE		/* if set, don't really delete files */
#endif

#define SUCCEED 0
#define FAILURE -1

/* file attributes */
#define	REBUILT	0x01		/* file has been reconstructed */
#define	ROOTP	0x02		/* file was named on left side of DEPEND */
#define LIBRARY 0x04		/* file is library; inherit parent's time */
#define ARCHIVE 0x08		/* file is archive; search through parent */
				/* archive directory for time */
#define EXTRACT 0x10		/* extract from archive when rebuilding */
#define ERROR   0x20		/* error occurred while rebuilding */

#ifdef VAXVMS
struct date_str {
	unsigned ds_low, ds_high;
};

typedef struct date_str *DATE;
#endif
#ifdef MSDOS
#ifndef FA_ARCH
#include <bdos.h>
#endif
typedef struct ftime *DATE;
#endif

struct node {
	struct filenode *nfile;	/* this node's file */
	struct node *nnext;	/* the next node */
};
typedef struct node NODE;


struct filenode {
	char *fname;		/* the filename */
	char *fmake;		/* remake string for file */
	DATE fdate;		/* 32 bit last-modification date */
	NODE *fnode;		/* files this file depends on */
	char fflag;		/* magic flag bits */
	struct filenode *parent;/* pointer to parent for archives, libraries */
	struct filenode *fnext;	/* the next file */
};
typedef struct filenode FILENODE;


struct macro {
	char *mname;		/* the macro's name */
	char *mvalue;		/* the macro's definition */
	struct macro *mnext;	/* the next macro */
};
typedef struct macro MACRO;


extern MACRO *mroot;
extern FILENODE *froot;
extern DATE bigbang;		/* Far, far in the past */
extern DATE endoftime;		/* Far, far in the future */
char *gmacro();
FILENODE *filenode(), *gfile();
char *token();
//E*O*F make.h//

echo x - osdate.c
cat > "osdate.c" << '//E*O*F osdate.c//'
/*
; OSDATE - return file's creation-date (called from Lattice), or -1
;	   if can't find the file.
; Synopsis:
;		int osdate(filename, time1, time2)
;			char *filename;
;			int *time1, *time2;
;
*/

#include "lar.h"
#undef ERROR
#define FILERR -1
#include "make.h"

DATE osdate(filename, success)
char *filename;
int *success;
{
	DATE adate(), newdate;
	register int handle;
	if ((handle = _open(filename, O_RDONLY)) == FILERR) {
		*success = FAILURE;
		return (DATE) NULL;
	}
 	newdate = adate();
	getftime(handle, newdate);
	_close(handle);
	*success = SUCCEED;
	return newdate;
}

struct ludir ldir[MAXFILES];
int nslots;
char *malloc(), *getname();

DATE getarchdate(archname, filename)
char *archname, *filename;
{
    fildesc lfd;
    register int    i;
    register struct ludir *lptr;
    union timing {
    	DATE ftimeptr;
	long *longptr;
	} unionptr;
    DATE adate(), newdate;
    char   *realname;

#if DEBUG
printf("Inside getarchdate; looking for %s inside %s.\n", filename, archname);
#endif
    if ((lfd = _open (archname, O_RDONLY)) == FILERR)
	cant (archname);
    getdir (lfd);

    for (i = 1, lptr = ldir+1; i < nslots; i++, lptr++) {
	if(lptr->l_stat != ACTIVE)
		continue;
	realname = getname (lptr->l_name, lptr->l_ext);
	if (strcmp (realname, filename) != 0)
	    continue;
#if DEBUG
printf("Found file.. name %s time %lx\n", realname, lwtol(lptr->l_datetime));
#endif
	newdate = adate();
	unionptr.ftimeptr = newdate;
	*(unionptr.longptr) = lwtol(lptr->l_datetime);
	break;
    }
    VOID _close (lfd);
#if DEBUG
printf("About to return; date is ");
printdate(newdate);
#endif
    return newdate;
}

copyfile(archname, filename)
char   *archname, *filename;
{
    fildesc lfd, ofd;
    register int    i;
    register struct ludir *lptr;
    union timer timeunion;
    extern int errno;
    char   *realname, outname[64], *tmpptr, *strrchr();

#if DEBUG
printf("Inside copyfile; looking for %s inside %s.\n", filename, archname);
#endif
    if ((lfd = _open (archname, O_RDWR)) == FILERR)
	cant (archname);

    getdir (lfd);

    for (i = 1, lptr = &ldir[1]; i < nslots; i++, lptr++) {
	if(lptr->l_stat != ACTIVE)
		continue;
	realname = getname (lptr->l_name, lptr->l_ext);
	if (strcmp (realname, filename) != 0)
	    continue;
/* generate real filename */
	tmpptr = strrchr(archname, '/');	/* should be path chr */
	if (tmpptr != NULL)  {
            i = (int) (tmpptr - archname);
	    i++;
#if DEBUG
printf("tmpptr was not NULL; i is %d.\n",i);
#endif
            strncpy(outname, archname, i);
	} 
	else {
#if DEBUG
printf("tmpptr was NULL.\n");
#endif
	    i = 0;
	    *outname = '\0';
	}
	strcat(outname, realname);
#if DEBUG
printf("Got it; about to extract %s.\n", outname);
#endif
	ofd = _creat(outname, 0);
	if (ofd == FILERR) {
	    fputs(outname, stderr);
	    fputs ("  - can't create\n", stderr);
	    return;
	}
	else {
	    VOID lseek (lfd, (long) lwtol (lptr->l_off), 0);
	    acopy (lfd, ofd, lwtol (lptr->l_len));
	    timeunion.realtime = lwtol(lptr->l_datetime);
	    if (ofd != fileno(stdout)) {
                VOID setftime(ofd, &(timeunion.ftimep));
		VOID _close (ofd);
	    }
	    break;			/* exit after copy */
	}
    }
    VOID _close (lfd);
}

static acopy (fdi, fdo, nbytes)		/* copy nbytes from fdi to fdo */
fildesc fdi, fdo;
long nbytes;
{
    register int btr, retval;
    char blockbuf[BLOCK];
    
    for (btr = (nbytes > BLOCK) ? BLOCK : (int) nbytes; btr > 0; 
    	 nbytes -= BLOCK, btr = (nbytes > BLOCK) ? BLOCK : (int) nbytes)  {
        if ((retval = _read(fdi, blockbuf, btr)) != btr) {
	    if( retval == 0 ) {
		error("Premature EOF\n");
	     }
	     if( retval == FILERR)
	        error ("Can't read");
	}
	if ((retval = _write(fdo, blockbuf, btr)) != btr) {
	     if( retval == FILERR )
	        error ("Write Error");
	}
    }
}

//E*O*F osdate.c//

echo x - parsedir.c
cat > "parsedir.c" << '//E*O*F parsedir.c//'
#include <stdio.h>
#include "make.h"
#ifdef VAXVMS
#include <rms.h>
#endif

extern DATE bigbang, endoftime;

/*
 * Get a file's creation date.
 */
int getdate(f)
FILENODE *f;
{
	/* return if already got date or if filedate succeeds. */
	if (f->fdate != (DATE) NULL || filedate(f) == SUCCEED) {
		return;
	}
 	if (isonlibrary(f) == SUCCEED)	/* isonlibrary will set time */
		return;
 	if (isanarchive(f) == SUCCEED)	/* isanarchive will set time */
		return;
	else if ((f->fflag & ROOTP) == 0)
	{
		fputs("Can't get date for file '", stderr);
		fputs(f->fname, stderr);
		fputc('\n', stderr);
		f->fdate = endoftime;
	} 
	else f->fdate = bigbang;
	return;
}


#ifdef VAXVMS
/*
 * filedate - return file's creation date (VAX/VMS only.)
 * Returns -1 if file cannot be found, 0 if succesful.
 */
filedate(fnd)
FILENODE *fnd;
{
	unsigned *datetime;
	DATE adate();
	struct FAB *fptr;
	struct XABDAT *dptr;

	fptr = malloc(sizeof(struct FAB));	/* allocate FAB and XABDAT */
	dptr = malloc(sizeof(struct XABDAT));
	if (fptr == NULL || dptr == NULL) allerr();
	*fptr = cc$rms_fab;			/* initialize FAB and XABDAT */
	*dptr = cc$rms_xabdat;
	fptr->fab$l_xab = (char *) dptr;	/* FAB -> XABDAT */

	fptr->fab$l_fna = fnd->fname;		/* setup filename */
	fptr->fab$b_fns = strlen(fnd->fname);

	if (sys$open(fptr) != RMS$_NORMAL ||	/* open the file */
	   sys$display(fptr) != RMS$_NORMAL)	/* get XABDAT info */
		return -1;

	datetime = &(dptr->xab$q_cdt);		/* record 64-bit date */
	fnd->fdate = adate(datetime[0], datetime[1]);

	sys$close(fptr);			/* close the file */

	free(dptr);				/* clean up and return */
	free(fptr);
	return 0;
}

/*
 * laterdt - compare two dates.
 * Return -1, 0 or 1 if date1 < date2, date1 == date2, or date1 > date2
 */
laterdt(date1, date2)
DATE date1, date2;
{
	if (date1->ds_high > date2->ds_high ||
	   (date1->ds_high >= date2->ds_high &&
	    date1->ds_low > date2->ds_low)) return 1;
	else if (date1->ds_high == date2->ds_high &&
	   date1->ds_low == date2->ds_low) return 0;
	else return -1;
}


/*
 * adate - allocate a date with the given time
 */
DATE adate(time1, time2)
unsigned time1, time2;
{
	DATE d;

	if ((d = (DATE)malloc(sizeof(struct date_str))) == NULL) allerr();
	d->ds_low = time1;
	d->ds_high = time2;
	return d;

}

initrootdates()
{
	bigbang = adate(0, 0);		/* init root dates */
	endoftime = adate(~0, ~0);
}

printdate(fdate)
DATE fdate;
{
	printf("( %u, %u )\n",
		(f->fdate != NULL) ? (f->fdate)->ds_high : 0,
		(f->fdate != NULL) ? (f->fdate)->ds_low : 0);
}
#endif


#ifdef MSDOS
/*
 * filedate - return file's creation date (MSDOS only.)
 * Returns -1 if file cannot be found, 0 if successful
 */
filedate(fnd)
FILENODE *fnd;
{
	DATE osdate(), newdate;
	int success;

	success = FAILURE;
	newdate = osdate(fnd->fname, &success);
	if (success == FAILURE) return FAILURE;
	fnd->fdate = newdate;
        fnd->fflag |= EXTRACT;		/* don't extract this file again */
	return SUCCEED;
}

/*
 * laterdt - compare two dates.
 * Return -1, 0 or 1 if date1 < date2, date1 == date2, or date1 > date2
 */
laterdt(date1, date2)
DATE date1, date2;
{
	if (date1->ft_year > date2->ft_year) return 1;

	if (date1->ft_year < date2->ft_year) return -1;
/* years are equal */
	if (date1->ft_month > date2->ft_month) return 1;

	if (date1->ft_month < date2->ft_month) return -1;
/* months are equal */
	if (date1->ft_day > date2->ft_day) return 1;

	if (date1->ft_day < date2->ft_day) return -1;
/* days are equal */
	if (date1->ft_hour > date2->ft_hour) return 1;

	if (date1->ft_hour < date2->ft_hour) return -1;
/* hours are equal */
	if (date1->ft_min > date2->ft_min) return 1;

	if (date1->ft_min < date2->ft_min) return -1;
/* minutes are equal */
	if (date1->ft_tsec > date2->ft_tsec) return 1;

	if (date1->ft_tsec < date2->ft_tsec) return -1;
/* everything is equal */
	return 0;
}

/*
 * adate - allocate a date struct to be filled out later.
 */
DATE adate()
{
	DATE d;

	if ((d = (DATE) calloc(1, sizeof(struct ftime))) == (DATE) NULL) allerr();
	return d;
}

initrootdates()		/* init root dates */
{
	bigbang = adate();
	bigbang->ft_tsec = 0;
	bigbang->ft_min = 0;
	bigbang->ft_hour = 0;
	bigbang->ft_day = 1;
	bigbang->ft_month = 1;
	bigbang->ft_year = 0;
	endoftime = adate();
	endoftime->ft_tsec = 29;
	endoftime->ft_min = 59;
	endoftime->ft_hour = 23;
	endoftime->ft_day = 31;
	endoftime->ft_month = 11;
	endoftime->ft_year = 127;
}

printdate(fdate)
DATE fdate;
{
	char buf[10];
	
	fputs("( ", stdout);
	itoa(fdate->ft_hour, buf);
	fputs(buf, stdout);
	fputc(':', stdout);
	itoa(fdate->ft_min, buf);
	fputs(buf, stdout);
	fputc(':', stdout);
	itoa(fdate->ft_tsec, buf);
	fputs(buf, stdout);

	fputs(", ", stdout);
	itoa(fdate->ft_month, buf);
	fputs(buf, stdout);
	fputc('-', stdout);
	itoa(fdate->ft_day, buf);
	fputs(buf, stdout);
	fputc('-', stdout);
	itoa(fdate->ft_year + 80, buf);
	fputs(buf, stdout);
	puts(" )");
}
#endif
//E*O*F parsedir.c//

echo x - token.c
cat > "token.c" << '//E*O*F token.c//'
#include <stdio.h>
#include <ctype.h>
#include "make.h"

/*
 * Get next token from the string.  Return a pointer to it, or NULL.
 * Adjust pointer to point to next part of string.
 * The string is modified.
 * A token consists of any number of non-white characters.
 */
char *token(strpp)
char **strpp;
{
	char *s, *beg, *stripwh();

	(void) stripwh(strpp);
	if(!**strpp) return NULL;

	beg = s = *strpp;
	while(*s && !isspace(*s)) ++s;
	if(*s) *s++ = '\0';
	*strpp = s;
	return beg;
}


/*
 * Parse character escape-sequences in a line of text.
 *	<EscChar><EscChar> = <EscChar>
 *	<EscChar>n = newline, and so on
 *	<EscChar><char> = <char>
 * The string is truncated at the first non-escaped occurance of 'comchar'.
 */
escape(str, comchar)
char *str, comchar;
{
	char *d, c;

	for(d = str; *str && *str != comchar; ++str)
	    if(*str == ESCCHAR && *(str + 1)) switch((c = *++str))
		{
		   case ESCCHAR:
			*d++ = ESCCHAR;
			break;

		   case 'n':
			*d++ = '\n';
			break;

		   case 'r':
			*d++ = '\r';
			break;

		   case 't':
			*d++ = '\t';
			break;

		   case 'b':
			*d++ = '\b';
			break;

		   case 'f':
			*d++ = '\f';
			break;

		   default:
			*d++ = c;
			break;
		} else *d++ = *str;

	*d++ = 0;
}

static 
char *stripwh(strpp)
char **strpp;
{
	char *s;

	s = *strpp;
	while(isspace(*s)) ++s;
	return (*strpp = s);
}
//E*O*F token.c//

exit 0
