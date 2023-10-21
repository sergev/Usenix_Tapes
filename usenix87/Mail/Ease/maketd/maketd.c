/* maketd - MAKE Transitive Dependencies.
 * (This is a lie - the dependencies are not transitive, but "all"
 * dependencies are correctly made.)
 *
 * Based loosely on a shell script by Stephan Bechtolsheim, svb@purdue
 * Other Makefile related features have been added or merged in from
 * other programs.
 *
 * Written & hacked by Stephen Uitti, PUCC staff, ach@pucc-j, 1985
 * maketd is copyright (C) Purdue University, 1985
 *
 * removed some of Steve's good, but unnecessary, options in favor
 * of more compile time flags & better implicit rules in the makefile
 * dinked: -q -e -E -k
 * Kevin S Braunsdorf, PUCC UNIX Group 1986	(ksb@j.cc.purdue.edu)
 *
 * Permission is hereby given for its free reproduction and
 * modification for non-commercial purposes, provided that this
 * notice and all embedded copyright notices be retained.
 * Commercial organisations may give away copies as part of their
 * systems provided that they do so without charge, and that they
 * acknowledge the source of the software.
 */
#include <sys/types.h>
#include <sys/param.h>
#include <sys/file.h>			/* for access */
#ifdef BSD2_9
#include <ndir.h>			/* for MAXPATHLEN, MAXNAMLEN */
#endif
#ifdef BSD4_2
#include <sys/dir.h>			/* for MAXNAMLEN in 4.2 */
#endif
#include <ctype.h>			/* for isupper */
#include <stdio.h>
extern char *rindex(), *index(), *strcat(), *strcpy();

#include "srtunq.h"
#include "abrv.h"
#include "nshpopen.h"
#include "maketd.h"

#ifndef CPP
#define CPP	"/lib/cpp "
#endif CPP not in Makefile

/* forward functions */
void	msoio();			/* open output file */
void	rdwr();				/* read old Makefile into new */
void	mkdepend();			/* does the real work */

/* globals */
char *prgnm;				/* our program name		*/
FILE *makefd;				/* makefile stream		*/
int	alldep = FALSE;			/* -a all - /usr/include too	*/
char   *targetname = NULL;		/* -t target name for next file */
char   *destsuffix = ".o";		/* -s suffix for targets	*/
int	header = TRUE;			/* print header & trailer	*/
int	usestdout = FALSE;		/* -d use stdout for makefile	*/
int	forcehead = FALSE;		/* -f force header/trailer	*/
int	makenseen = FALSE;		/* output file has been specified */
char   *makename = "makefile";		/* -m default file for edit	*/
int	backedup = FALSE;		/* for interupt recovery	*/
char	backupfn[MAXNAMLEN+1];		/* backup file name		*/
int	ismakeopen = FALSE;		/* if the output file is open	*/
char	objpath[MAXPATHLEN+1];		/* -o prepended to .o's		*/
int	nonlocalo = FALSE;		/* -nonlocalo objects in source dir */
int	replace = FALSE;		/* -r replace depends in Makefile */
char	cppflags[BUFSIZ];		/* -D, -I, -U flags to pass to cpp */
int	shortincl = TRUE;		/* -x do abreviations		*/
int	verbose = FALSE;		/* -v verbage for the debugger	*/
static char   sopts[] = "abdfhrxv";	/* single char opts		*/
static SRTUNQ u;			/* unique include files		*/

char   usage[] =
"Usage: maketd [-a -b -d -f -h -mMAKEFILE -nonlocalo -oDIR -r -sSUFFIX\n\
 -tTARGETNAME -x -v -Iincludedir -Ddefine -Uundefine file...]\n";
char   helptext[] =
"-a\tdo all dependencies, including /usr/include\n\
-b\tgenerate binary, rather than object related dependencies\n\
-d\tdependencies to stdout, rather than Makefile\n\
-f\tforce header/trailer (use with -d)\n\
-h\thelp (this text)\n\
-m\tspecify MAKEFILE for edit\n\
-nonlocalo Objects live in source directory\n\
-o\tprepend DIR to target: DIR/a.o: foo.h\n\
-r\treplace dependencies for a target\n\
-s\tchange suffix target's SUFFIX: a.SUFFIX: foo.h\n\
-t\tchange target's basename: TARGET.o: foo.h\n\
-x\tdon't abbreviate includes\n\
-v\tprint extra verbose (debug) output to stderr\n\
-I\tspecify include directory, as in /lib/cpp\n\
-D\tspecify defines, as in /lib/cpp\n\
-U\tspecify undefines, as in /lib/cpp\n";

char deplin[] = "# DO NOT DELETE THIS LINE - make depend DEPENDS ON IT\n";
char searchdep[] = "# DO NOT DELETE THIS LINE";
char trailer[] = "\n# *** Do not add anything here - It will go away. ***\n";

/* some init & argv parsing */
main(argc, argv)
register int argc;
register char **argv;
{
    register a;				/* argv subscript		*/
    register i;				/* tmp				*/
    register len;			/* length of current argument	*/
    register files = FALSE;		/* files ever seen		*/
    register char *q;			/* tmp				*/

    /* prgnm = program name, for error printing */
    if ((prgnm = rindex(argv[0], '/')) == NULL)
	prgnm = argv[0];
    else
	prgnm++;
    catchsig();				/* init signal traps */
    srtinit(&abrv);			/* init abbreviations tree */
    for (a = 1; a < argc; a++) {	/* argv prepass: find all -I's */
	if (argv[a][0] == '-' && argv[a][1] == 'I' && strlen(&argv[a][2]) > 2)
	    if ((q = srtin(&abrv, hincl(&argv[a][2]), lngsrt)) != NULL)
		fprintf(stderr, "%s: %s - %s\n", prgnm, q, &argv[a][2]);
    }
    cppflags[0] = '\0';			/* terminate cpp flags string */
    objpath[0] = '\0';			/* init object path */
    srtinit(&u);			/* init sorting database tag */
    for (a = 1; a < argc; a++) {
	len = strlen(argv[a]);
	if (argv[a][0] == '-' && len > 2 && index(sopts, argv[a][1]) != NULL)
	    err("options must be listed seperately - '%s'\n", argv[a]);
	if (len > 1 && argv[a][0] == '-') {
	    switch (argv[a][1]) {
	    case 'D':			/* /lib/cpp flags to pass */
	    case 'I':
	    case 'U':
		if (strlen(cppflags) + strlen(argv[a]) + 2 > BUFSIZ)
		    err("too many cpp flags - buffer overflow");
		strcat(cppflags, argv[a]);
		strcat(cppflags, " ");	/* add a space separator */
		break;
	    case 'a':			/* /usr/include deps too */
		alldep = TRUE;
		break;
	    case 'b':			/* target has no suffix */
		destsuffix = "";
		break;
	    case 'd':			/* don't edit Makefile */
		if (ismakeopen)
		    err("Makefile already open, -d must precede filenames");
		if (makenseen)
		    err("Conflict - check -d and -m options");
		makenseen = TRUE;
		usestdout = TRUE;
		if (!forcehead)
		    header = FALSE;	/* don't do header & trailer */
		break;
	    case 'f':			/* force header/trailer */
		forcehead = TRUE;
		header = TRUE;
		break;
	    case 'h':			/* help */
		fputs(usage, stdout);
		fputs(helptext, stdout);
		exit(0);
		break;
	    case 'm':			/* specify makefile name for edit */
		if (ismakeopen)
		    err("Makefile already open, -m must precede filenames");
		if (makenseen)
		    err("Conflict, check -m and -d options.");
		if (strlen(makename) == 0)
		    err("-m option requires file name.");
		makenseen = TRUE;
		makename = &argv[a][2];
		break;
	    case 'n':			/* objects reside with sources */
		if (strcmp("nonlocalo", &argv[a][1]) != 0)
		    err("bad -n option"); /* what a crock of an option */
		if (objpath[0] != '\0')
		    err("nonlocalo conflict - check -o's");
		nonlocalo = TRUE;
		break;
	    case 'o':
		if (nonlocalo)
		    err("object path conflict - check -o's and -nonlocalo's");
		strcpy(objpath, &argv[a][2]);
		i = strlen(objpath);
		if (i == 0)
		    err("-o requires path string.");
		if (i >= MAXPATHLEN)
		    err("Object path too long: max is %d.", MAXPATHLEN);
		if (objpath[i - 1] != '/')
		    strcat(objpath, "/");
		break;
	    case 'r':			/* replace mode */
		if (ismakeopen)
		    err("Makefile already open, -r must precede filenames");
		replace = TRUE;
		break;
	    case 's':			/* destination suffix */
		destsuffix = &argv[a][2];
		break;
	    case 't':			/* set target's basename */
		targetname = &argv[a][2];
		if (len <= 2)
		    err("target option requires name.");
		break;
	    case 'v':			/* user wants to hear noise */
		verbose = TRUE;
		break;
	    case 'x':			/* don't abbrev. */
		if (files)
		    err("-x option must preceed all files.");
		shortincl = FALSE;
		break;
	    default:
		err("Unknown option %s.", argv[a]);
	    }				/* end switch */
	} else {			/* must be a filename */
	    if (verbose)
		fprintf(stderr, "%s: working on %s.\n", prgnm, argv[a]);
	    files = TRUE;		/* at least one file seen */
	    if (replace && a != argc - 1)
		err("Only one file allowed with -r (edit aborted)");
	    mkdepend(argv[a]);		/* file to process */
	    targetname = NULL;		/* affect only one file */
	}				/* if option */
    }					/* for argv */
    if (ismakeopen && header)
	fputs(trailer, makefd);		/* do not delete... */
#if	DEL_BACKUP
    if (backedup)
	if (unlink(backupfn))
	    err("Can't delete backup file %s on completion", backupfn);
#endif	DEL_BACKUP
    if (!files)
	err("No files to process, use -h for full help.\n%s", usage);
    exit(0);				/* exit status - good */
}

/* msoio - Make Sure Output Is Open.
 * Interacts strongly via globals: makefd, backedup, backupfn, makename,
 * header, ismakeopen
 */
void
msoio(targ)
char *targ;				/* if -r, is target name */
{
    FILE *tmpfd;			/* temp file desc for -d */
    char buf[BUFSIZ];			/* for reading the makefile */

    if (ismakeopen)
	return;
    ismakeopen = TRUE;			/* will be: all errs are fatal */
    if (usestdout) {
	makefd = stdout;
	if (header) {
	    fputc('\n', makefd);	/* one blank line */
	    fputs(deplin, makefd);	/* the first line */
	}
	/* scan "makefile" or "Makefile" for include defines */
	if (access(makename, R_OK) != 0) {
	    makename[0] = 'M';		/* try Makefile */
	    if (access(makename, R_OK) != 0)
		return;			/* just punt */
	}
	if ((tmpfd = fopen(makename, "r")) == NULL)
	    return;			/* just punt */
	while (fgets(buf, BUFSIZ, tmpfd) != NULL)
	    srchincl(buf);		/* scan whole file */
	fclose(tmpfd);
	return;
    }					/* ... if standard out */
/* !makenseen means (default) try "makefile" then "Makefile" */
    if (!makenseen && access(makename, F_OK) != 0)
	makename[0] = 'M';		/* try Makefile */
/* side effect: "Makefile" will be created if neither exist */
    if (access(makename, F_OK) == 0) {	/* if makefile exits */
	strcpy(backupfn, makename);	/* get rid of .bak */
	strcat(backupfn, ".bak");
	if (access(backupfn, F_OK) == 0) {
	    if (unlink(backupfn))
		err("Can't remove %s for pre-edit\n", backupfn);
	}
	if (link(makename, backupfn))	/* mv makefile to .bak */
	    err("Can't link %s to %s.", backupfn, makename);
	backedup = TRUE;		/* for interupt status */
	if (unlink(makename))
	    err("Can't unlink %s during rename.", makename);
    } else {
	backupfn[0] = '\0';		/* no copy (no makefile) */
    }
    if ((makefd = fopen(makename, "w")) == NULL)
	err("Can't open output file '%s' for write.", makename);
    if (backupfn[0] != '\0')		/* if no .bak file - done */
	rdwr(targ);			/* read/write Makefile */
    else
	fputs(deplin, makefd);		/* must start with this */
}

/* create beginging of new Makefile by reading old one */
void
rdwr(targ)
char *targ;
{
    register FILE *oldfd;		/* file pointer for .old */
    char rwbuf[BUFSIZ];			/* temp for read/write */
    register tlen;			/* targ length */
    register puntln = FALSE;		/* punt current line? */
    register contln = FALSE;		/* previous line ended with '\'? */
    register blankln = 0;		/* number of blank lines seen */
    register srchsn = FALSE;		/* search line seen? */

    if ((oldfd = fopen(backupfn, "r")) == NULL)
	err("Can't open backup copy of %s\n", makename);
    tlen = strlen(targ);
    while (fgets(rwbuf, BUFSIZ, oldfd) != NULL) { /* until EOF */
	if (!srchsn) {
	    if (strncmp(searchdep, rwbuf, (sizeof searchdep) - 1) == 0) {
		srchsn = TRUE;
		fputs(deplin, makefd);	/* re-write this line */
		if (!replace)
		    break;
		continue;		/* don't print this line */
	    }
	} else {
	    if (strcmp("\n", rwbuf) == 0) {
		if (!puntln)
		    blankln++;
		contln = FALSE;
		puntln = FALSE;
		continue;		/* don't output this blank line */
	    }
	    if (!contln) {
		if (strncmp(targ, rwbuf, tlen) == 0)
		    puntln = TRUE;
		else if (strcmp(&trailer[1], rwbuf) == 0)
		    puntln = TRUE;
	    }
	    if (lastlnch(rwbuf) == '\\')
		contln = TRUE;
	    else
		contln = FALSE;
	}				/* if srchsn */
	if (!puntln) {
	    srchincl(&rwbuf[0]);	/* search this line for defines */
	    if (blankln != 0) {		/* compress mult blank lines to one */
		putc('\n', makefd);
		blankln = 0;
	    }
	    fputs(rwbuf, makefd);	/* non targ lines */
	}
    }					/* while fgets */
    if (!srchsn)			/* deplin never found */
	fputs(deplin, makefd);		/* so write one */
    (void) fclose(oldfd);		/* close the .old file for gigles */
}

#define MAXCOL 78			/* output width max for makefile */

/* mkdepend - name is historical, but does the "real work" */
void
mkdepend(infile)
char *infile;
{
    register char *p;			/* temp pointer */
    register char *q;			/* temp pointer */
    register char *r;			/* temp pointer */
    register FILE *cppfd;		/* file desc for /lib/cpp */
    char buf[BUFSIZ];			/* temp buff */
    char basename[MAXNAMLEN+1];		/* just the file name */
    register oplen;			/* length target & path */
    register le;			/* length of current output line */
    register char *targ;		/* target's name */
    register i;				/* tmp for index */
    register firstln;			/* first line of a list */

    if ((p = rindex(infile, '/')) == NULL) /* past path */
	p = infile;
    else
	p++;
    if (nonlocalo && p != infile) {	/* objpath = source path */
	for (q = objpath, r = infile; r < p;)
	    *q++ = *r++;
	*q = '\0';			/* null terminate */
    }
    strcpy(basename, p);
    if ((p = rindex(basename, '.')) != NULL)
	*p = '\0';			/* remove trailing ".*" */
    if (targetname != NULL)		/* set up target's name */
	targ = targetname;
    else
	targ = basename;
    if (makename == NULL) {
	makename = "Makefile";
	if (access(makename, F_OK) != 0)
	    makename[0] = 'm';		/* not a real check */
    }
    msoio(targ);			/* Make Sure Output Is Open */
    abrvsetup();			/* create abrev table, write defs. */
    if (access(infile, R_OK) != 0) {
	fprintf(stderr, "%s: Can't open input file '%s', skipped.\n",
	    prgnm, infile);
	return;
    }
    (void)strcpy(buf, CPP);	/* build cpp cmd line */
#if CPP_M
	strcat(buf, "-M ");		/* -M flag - does dependencies */
#endif
    if (strlen(buf) + strlen(cppflags) + strlen(infile) + 1 > BUFSIZ)
	err("cpp command line buffer overflow");
    (void)strcat(buf, cppflags);	/* add command flags */
    (void)strcat(buf, infile);		/* add file name */
    srtfree(&u);			/* init insertion sorter */
    if (verbose)
	fprintf(stderr, "%s: cpp line is '%s'\n", prgnm, buf);
    if ((cppfd = nshpopen(buf, "r")) == NULL)
	err("Can't open pipe for %s", buf);
#if CPP_M
     while (fgets(buf, BUFSIZ, cppfd) != NULL) {
	 if ((p = index(buf, ':')) == NULL)
	     err("cpp -M format error - colon");
	 p++;			/* pass colon */
	 if (*p++ != SPC)
	     err("cpp -M format error - space");
	 p = hincl(p);		/* skip any uglies in include path */
	 if (!alldep && strncmp("/usr/include", p, 12) == 0)
	     continue;		/* ignore /usr/include... stuff */
	 if (index(p, '\n') != NULL) /* replace newline with EOS */
	     *index(p, '\n') = '\0';
	 if ((q = srtin(&u, p, (int (*)())0)) != NULL) /* insert into list */
	     fprintf(stderr, "%s: %s - %s\n", prgnm, q, p); /* warning */
     }
#else
     while (fgets(buf, BUFSIZ, cppfd) != NULL) {
	 if (buf[0] != '#')		/* must start with '#' */
	     continue;
	 if ((p = index(buf, '"')) == NULL) /* find first double quote */
	     continue;
	 p++;
	 p = hincl(p);		/* skip any uglies in include path */
	 if (index(p, '"') != NULL)	/* terminate the file name */
	     *index(p, '"') = '\0';
	 if (!alldep && strncmp("/usr/include", p, 12) == 0)
	     continue;		/* ignore /usr/include... stuff */
	 if ((q = srtin(&u, p, (int (*)())0)) != NULL) /* insert into list */
	     fprintf(stderr, "%s: %s - %s\n", prgnm, q, p); /* warning */
     }
#endif
    srtgti(&u);				/* init for srtgets */
    /* 2 for colon space */
    oplen = strlen(objpath) + strlen(targ) + strlen(destsuffix) + 2;
    le = MAXCOL;			/* force new line output */
    firstln = TRUE;			/* first line of a file entry */
    while ((p = srtgets(&u)) != NULL) { /* write out the entries */
	if (shortincl)
	    if ((i = findabr(p)) != MXABR) /* i = found index or MXABR */
		p += abrvlen[i] - 2;
	if (le + strlen(p) >= MAXCOL) {
	    if (firstln) {
		le = oplen;
		fprintf(makefd, "\n%s%s%s: ", objpath, targ, destsuffix);
		firstln = FALSE;
	    } else {
		le = 8;
		fprintf(makefd, " \\\n\t");
	    }
	} else {
	    fputc(SPC, makefd);
	}
	if (shortincl && i != MXABR) {
	    fprintf(makefd, "$%c", 'A' + i);
	    p += 2;			/* right place */
	    le += 2;
	}
	fputs(p, makefd);
	le += 1 + strlen(p);
    }
    fputc('\n', makefd);		/* end with newline */
    nshpclose(cppfd);			/* end of that file */
}
