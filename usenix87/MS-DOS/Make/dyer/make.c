#include <stdio.h>
#include <ctype.h>
#include "make.h"

/*
 *	MAKE - Maintain seperate source files
 *
 *	SYNOPSIS
 *		MK [-f file] [-a] [-n] [-d] [name] ...
 *		   f: use 'file' instead of default makefile
 *		   a: assume all modules are obsolete (recompile everything)
 *		   n: don't recompile, just list steps to recompile
 *		   d: debugging (print tree, file info)
 *		   name: module name to recompile
 *
 *		'secret' options (not to be used by humans):
 *		   -ofile	'file' is the script file to write to
 *
 *	AUTHOR
 *		Landon M. Dyer, Atari Inc.
 *
 */

#define SCRIPTFILE "make$$$$.bat"	/* (default) script-listing file */
#define	INIT	"~INIT"			/* initialization macro */
#define	DEINIT	"~DEINIT"		/* de-init macro */
#define	BEFORE	"~BEFORE"		/* the per-root 'startup' method */
#define	AFTER	"~AFTER"		/* the per-root 'wrapup' method */


char *mfiles[] = {			/* default makefiles */
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


MACRO *mroot = NULL;		/* root of macro-list */
FILENODE *froot = NULL;		/* root of filenode-list */
FILENODE *firstf = NULL;	/* the very first filenode */
FILE *mkfp = NULL;		/* script file */
char *modnames[MAXMODS];	/* module-names mentioned in commandline */
int modcount = 0;		/* #of module-names */
int debug = 0;			/* nonzero: turn on debugging */
int obsolete = 0;		/* nonzero: every file should be recompiled */
int noscript = 0;		/* nonzero: print methods on standard output */
char *scriptf = SCRIPTFILE;	/* default script file */
DATE bigbang;			/* a date, the very earliest possible */
DATE endoftime;			/* a date, the very last possible */


main(argc, argv)
int argc;
char **argv;
{
	int arg, i;
	char *mfile = NULL;
	DATE adate();

	bigbang = adate(0, 0);		/* init root dates */
	endoftime = adate(~0, ~0);

	for(arg = 1; arg < argc; ++arg)
		if(*argv[arg] == '-') switch(tolower(argv[arg][1]))
		{
		   case 'f':
			if(++arg >= argc)
			{
				fprintf(stderr, "-f needs filename argument.\n")
;
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

		   case 'o':
		   	scriptf = argv[arg] + 2;
			break;

		   default:
			fprintf(stderr, "Unknown switch: %c\n", argv[arg][1]);
			break;
		} else if(modcount < MAXMODS)
			modnames[modcount++] = argv[arg];
		else
		{
			fprintf(stderr, "Too many module names.\n");
			return;
		}

	if(mfile != NULL)
	{
		if(fmake(mfile) == -1)
			fprintf(stderr, "Cannot open makefile '%s'.\n", mfile);
	} else {
		for(i = 0; *mfiles[i]; ++i)
			if(fmake(mfiles[i]) != -1) break;
		if(!*mfiles[i])
			fprintf(stderr, "Cannot open makefile.\n");
	}

	if(debug) prtree();
}


/*
 * Construct dependency tree from the makefile 'fn'.
 * Figure out what has to be recompiled, and write a script file to do that.
 */
fmake(fn)
char *fn;
{
	FILE *fp;

	if((fp = fopen(fn, "r")) == NULL) return -1;

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
	char *strp, *tok1, *tok2, *s;
	FILENODE *lastf = NULL;
	FILENODE *sf;

	for(;;)
	{
		if(fgets(ibuf, STRSIZ, fp) == NULL) break;
		mexpand(ibuf, ebuf, STRSIZ, MACCHAR);
		escape(ebuf, COMCHAR);

			/* clobber last newline in string */
		s = ebuf + strlen(ebuf) - 1;
		if(s >= ebuf && *s == '\n') *s = '\0';

		if(*ebuf == '\t')
		{
			addmeth(lastf, ebuf+1);
			continue;
		}

		strp = ebuf;
		if((tok1 = token(&strp)) == NULL) continue;
		if((tok2 = token(&strp)) != NULL)
			if(!strcmp(tok2, DEFMAC))
			{
				if(*strp) defmac(tok1, strp);
				else if(undefmac(tok1) < 0)
				    fprintf(stderr,
					  "Can't undefine macro '%s'\n", tok1);
				continue;
			}
			else if(!strcmp(tok2, DEPEND))
			{
				addmeth(lastf, gmacro(AFTER));

				lastf = filenode(tok1);
				if(firstf == NULL) firstf = lastf;
				lastf->fmake = NULL;

				addmeth(lastf, gmacro(BEFORE));

				lastf->fflag |= ROOTP;
				while((tok1 = token(&strp)) != NULL)
					addfile(lastf, tok1);
				continue;
			}
			else addfile(lastf, tok2);

		do {
			addfile(lastf, tok1);
		} while((tok1 = token(&strp)) != NULL);
	}

	addmeth(lastf, gmacro(AFTER));
}


/*
 * Determine sequence of recompiles from the creation dates.
 * If there is anything to recompile, then create a script file full of commands
.
 */
determ()
{
	FILENODE *f;
	int i;
	char *m;

	if(firstf == NULL)			/* empty tree */
	{
		printf("No changes.\n");
		return;
	}

	if(modcount == 0) examine(firstf, endoftime);
	else for(i = 0; i < modcount; ++i)
	{
		if((f = gfile(modnames[i])) == NULL)
		{
			fprintf(stderr, "Can't find root '%s'.\n", modnames[i]);
			continue;
		}

		if(f->fflag & ROOTP == 0)
		{
			fprintf(stderr, "'%s' is not a root!\n", f->fname);
			continue;
		}
		examine(f, endoftime);
	}

	if(mkfp != NULL)
	{
		if((m = gmacro(DEINIT)) != NULL)
		{
			fputs(m, mkfp);
			fputc('\n', mkfp);
		}
		fclose(mkfp);
	} else printf("No changes.\n");
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
	int rebuildp = 0;
	NODE *n;

	getdate(fnd);
	if(fnd->fnode == NULL && fnd->fflag & ROOTP)
		rebuildp = 1;
	else for(n = fnd->fnode; n != NULL; n = n->nnext)
		if(examine(n->nfile, fnd->fdate)) rebuildp = 1;

	if(rebuildp) recomp(fnd);
	if(obsolete || laterdt(fnd->fdate, date) >= 0)
		rebuildp = 1;
	return rebuildp;
}


/*
 * Make sure a filenode gets recompiled.
 */
recomp(f)
FILENODE *f;
{
	FILENODE *sf;
	char *m;

	if(mkfp == NULL)
	{
		if(noscript) mkfp = stdout;
		else if((mkfp = fopen(scriptf, "w")) == NULL)
			fprintf(stderr, "Cannot create: '%s'\n", scriptf);

	if((m = gmacro(INIT)) != NULL)
		{
			fputs(m, mkfp);
			fputc('\n', mkfp);
		}
	}

	if(f->fflag & REBUILT) return;
	if(f->fmake != NULL) fputs(f->fmake, mkfp);
	f->fflag |= REBUILT;
}


/*
 * Complain about being out of memory, and then die.
 */
allerr() {
	fprintf(stderr, "Can't alloc -- no space left (I give up!)\n");
	exit(1);
}
