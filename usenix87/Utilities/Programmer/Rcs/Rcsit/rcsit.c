#ifndef lint
static char *RCSid = "$Header: rcsit.c,v 1.26 86/03/27 14:16:01 mcooper Exp $";
#endif

/*
 *---------------------------------------------------------
 * $Source: /w/mcooper/src/rcsit/RCS/rcsit.c,v $
 * $Revision: 1.26 $
 * $Date: 86/03/27 14:16:01 $
 * $State: Exp $
 * $Author: mcooper $
 * $Locker: mcooper $
 *---------------------------------------------------------
 * Michael Cooper (mcooper@usc-oberon.arpa)
 * University Computing Services,
 * University of Southern California,
 * Los Angeles, California,   90089-0251
 * (213) 743-3462
 *---------------------------------------------------------
 *
 * $Log:	rcsit.c,v $
 * Revision 1.26  86/03/27  14:16:01  mcooper
 * Added usage() message.
 * Moved $Header$ into object for ident.
 * 
 * Revision 1.25  86/02/26  15:03:45  mcooper
 * Added -Fheader_file option.
 * 
 * Revision 1.24  86/02/06  10:58:59  mcooper
 * Fixed default error message.
 * 
 * Revision 1.23  86/02/03  14:17:07  mcooper
 * Added the type ``Pascal''.
 * Also deleted log messages up until 1.17
 * (first mod.sources release).
 * 
 * Revision 1.22  85/12/19  14:24:35  mcooper
 * Added more patches from Joe Chapman (joe@cca-unix.ARPA)
 * for TeX support.  Also used his suggestion for
 * checking to see if only one of the file spec. flags
 * is set.
 * 
 * Revision 1.21  85/12/18  11:05:50  mcooper
 * Added "#ifndef lint" to the default header for .c 
 * files.  This keeps lint quite about the:
 * 
 * 	static char RCSid
 * 
 * Revision 1.20  85/12/18  10:21:24  mcooper
 * Added patches from Joe Chapman (joe@cca-unix.ARPA)
 * to keep from dereferencing NULL on the 68K.
 * 
 * Revision 1.19  85/12/16  17:34:11  mcooper
 * Changed .header.* --> .header.*
 * More appropriate name.
 * 
 * Revision 1.18  85/11/26  17:03:32  mcooper
 * Change message telling of what header was added.
 * 
 */

/*
 * rcsit -- 	Prepare files for RCS.  rcsit puts the correct headings
 *		at the top of files to prepare them for RCS headings
 *		and log tracking.
 *
 * Michael Cooper	(mcooper@usc-oberon.arpa)
 * University Computing Services, USC
 *
 * 9-16-85
 */

#include <stdio.h>
#include <ctype.h>
#include <sys/file.h>

#ifdef NULL
#undef NULL
#endif
#define NULL		'\0'
#define LENGTH		132		/* length of line */
#define TRUE		1
#define FALSE		0
#define USAGE "[ -chfsmMxp ] [ -qad ] [ -I{flags} ] [ -R{flags} ] \
\n\t\t[ -H{directory} ] [ -F header_file ] file [ file1 file2 ... ]\n"

#ifdef DEBUG
 int debugon = TRUE;
#else
 int debugon = FALSE;
#endif

static char 	*progname;		/* program name */
static char 	*rcsdir;

/*
 * Messages to be printed for the user.
 */
static char	*msg_name;		
static char 	*m_stdc = "Standard C",
		*m_include = "C Include",
		*m_fortran = "Fortran",
		*m_pascal = "Pascal",
		*m_make	= "Makefile",
		*m_shell = "Shell Script",
		*m_manual = "Manual",
		*m_tex = "TeX";

/*
 * The default headers to put at the beginning of the file(s).
 * Notice that the words Header and Log do not appear here
 * because RCS will put in the keyword substitutions when rcsit.c
 * is co'ed.
 */
static char	*header;
static char	*h_stdc = 
"#ifndef lint\nstatic char *RCSid = \"$%s$\";\n#endif\n\n/*\n * $%s$\n */\n\n";
static char	*h_include = 
	"/*\n * $%s$\n *\n * $%s$\n */\n\n";
static char	*h_make =
	"#\n# $%s$\n#\n# $%s$\n#\n";
static char 	*h_manual =
	"...\n... $%s$\n... \n... $%s$\n...\n";
static char 	*h_fortran =
	"c\nc $%s$\nc\nc $%s$\nc\n";
static char	*h_tex =
	"%% $%s$\n%%\n%% $%s$\n%%\n";
static char	*h_pascal = 
	"(*\n * $%s$\n *\n * $%s$\n *)\n\n";

/*
 * Header file names
 */
static char	*header_c 	= ".header.c";		/* .c header */
static char 	*header_h 	= ".header.h";		/* .h header */
static char 	*header_f 	= ".header.f";		/* .f header */
static char 	*header_p 	= ".header.p";		/* .p header */
static char 	*header_man 	= ".header.man";	/* man header */
static char	*header_make	= ".header.make";	/* make header */
static char	*header_sh	= ".header.sh";		/* sh script header */
static char	*header_tex	= ".header.tex";	/* TeX header */
static char	*tpath;					/* path to header */
static char	tfile[BUFSIZ];				/* header file */
static char	tbuf[BUFSIZ];				/* current tfile */

/*
 * Command line flags
 */
int	Iflag	= FALSE;			/* run ci(1) */
int	rcsflag = FALSE;			/* run rcs(1) */
int	aflag	= TRUE;				/* do auto guess */
int	dflag	= TRUE;				/* creat RCS dir. */
int	qflag	= FALSE;			/* be quiet! */
int 	cflag	= FALSE;			/* std c file */
int	fflag	= FALSE;			/* fortran file */
int	pflag	= FALSE;			/* pascal file */
int	hflag	= FALSE;			/* include file */
int	sflag	= FALSE;			/* shell script */
int 	mflag	= FALSE;			/* Makefile */
int	Mflag	= FALSE;			/* manual */
int	Hflag	= FALSE;			/* header flag */
int	Fflag	= FALSE;			/* header file flag */
int	xflag	= FALSE;			/* TeX flag */

main(argc, argv)
int	argc;
char 	*argv[];
{
	int x;
	char	tmp[LENGTH];
	char	*file;
	char	*flags;
	char 	*tmpfile = "/tmp/rcsitXXXXXX";
	char 	*mktemp();
	char	*gettmp();
	char	*getenv();
	FILE 	*fd, 
		*fdtmp,
		*fopen();

	progname = *argv;
	for (x = 1; x < argc; x++) {
		if (argv[x][0] != '-')
			break;
		switch (argv[x][1]) {
			case 'a':
				aflag = FALSE;
				break;
			case 'q':
				qflag = TRUE;
				break;
			case 'd':
				dflag = FALSE;
				break;
			case 'f':
				fflag = TRUE;
				break;
			case 'h':
				hflag = TRUE;
				break;
			case 's':
				sflag = TRUE;	
				break;
			case 'm':
				mflag = TRUE;
				break;
			case 'M':
				Mflag = TRUE;
				break;
			case 'i':
			case 'I':
				Iflag = TRUE;
				flags = &argv[x][2];
				break;
			case 'r':
			case 'R':
				rcsflag = TRUE;
				flags = &argv[x][2];
				break;
			case 'F':
				Fflag = TRUE;
				strcpy(tfile, &argv[x][2]);
				msg_name = &argv[x][2];
				break;
			case 'H':
				Hflag = TRUE;
				tpath = &argv[x][2];
				break;
			case 'c':
				cflag = TRUE;
				break;
			case 'x':
				xflag = TRUE;
				break;
			case 'p':
				pflag = TRUE;
				break;
			default:
				usage();
		}
	}
	argc -= (x - 1);
	argv += (x - 1);

	if(hflag + mflag + Mflag + cflag + sflag + fflag + xflag + pflag > 1)
		fatal("Only ONE of -c,-f,-m,-M,-h,-s,-p,-x may be specified.");
	if(Iflag && rcsflag) 
		fatal("Only ONE of ``-i'' and ``-r'' may be specified.");

	if(cflag || hflag || mflag || Mflag || fflag || sflag || xflag || pflag)
		aflag = FALSE;

	if((rcsdir = getenv("RCSDIR")) == NULL)
		rcsdir = "RCS";
	if(Iflag && dflag)
		checkdir();	/* Make RCS directory for ci */

	if((tpath == NULL || *tpath == NULL) && 
	    ((tpath = getenv("TEMPLATE")) == NULL))
		if((tpath = getenv("HOME")) == NULL)
			fatal(
			  "Cannot find environment variable HOME or TEMPLATE");

	/*
	 * make tmp file once.
	 */
	mktemp(tmpfile);

	while (--argc) {	/* Main loop */
		file = *++argv;
		debug(sprintf(tmp, "...file (*++argv) = %s...", file));

		if(access(file, 4) != 0)
			fatal(
		"Cannot access %s.  No read permission OR file does not exist.",
				file);
		if((fdtmp = fopen(tmpfile, "w")) == NULL) {
			fatal("Cannot open tmpfile (%s).", tmpfile);
		}

		if(!Fflag)
			if(aflag)
				auto_guess(file); /* try and guess file type */
			else
				set_flags();	  /* check and set flags */

		if(Hflag && !Fflag) {
			/*
			 * first get names of headers, then create
			 * path name to it.
			 */
			get_temp();
			sprintf(tfile, "%s/%s", tpath, tbuf);
		}
		if(access(tfile, 0) == 0 && (Hflag || Fflag)) {
			if(!qflag || debugon)
				printf("Adding %s header file to %s...",
					msg_name, file);
			copy(tfile, tmpfile, "w");
			copy(file, tmpfile, "a");
		} else {
			if(!qflag || debugon)
				printf(
				"Adding default header (%s format) to %s...",
					msg_name, file);
			/*
			 * put the Keywords into header string
			 */
			sprintf(tmp, header, "Header", "Log");
			fputs(tmp, fdtmp);
			/*
			 * fclose'em, just in case.
			 */
			fclose(fdtmp);
			copy(file, tmpfile, "a");
		}
		unlink(file);
		copy(tmpfile, file, "w");
		unlink(tmpfile);

		if(!qflag || debugon)
			printf("done.\n");

		if(Iflag){
			rcs("ci", file, flags);
			if(Mflag){	/* kludge to tell rcs about manuals */
				rcs("rcs", file, "c'... '");
				/*
				 * kludge part 2 - if the user tried a ci
				 * with a -l option, then the header is
				 * messed up in the currently checked out
				 * man file.  So we have to co the file to 
				 * clean up the header.  Plus we use the
				 * -l option of co to insure file locking.
				 */
				if(checkfor("l", flags)){
					unlink(file);
					rcs("co", file, "l");
				}
			}
		}
		if(rcsflag)
			rcs("rcs", file, flags);
	}
}

/*
 * debug - print (useless) debugging info.
 */
 
debug(msg)
char *msg;
{
#ifdef DEBUG
	fprintf(stderr, msg);
	putchar ('\n');
#endif
}

/*
 * auto_guess - try and be intelligent and guess type of file
 *		by looking at the suffix or the whole name
 *		in the case of a makefile.
 */

auto_guess(file)
char	*file;
{
	char *suffix;
	char *rindex();

	suffix = rindex(file, '.')+1;
	if((strcmp(file, "makefile") == 0) || (strcmp(file, "Makefile") == 0) ||
	    (strcmp(suffix, "mk") == 0)) {	/* sys V std suffix */
		mflag = TRUE;
		sflag = FALSE;
		cflag = FALSE;
		hflag = FALSE;
		Mflag = FALSE;
		fflag = FALSE;
		xflag = FALSE;
		pflag = FALSE;
	}
	if((strcmp(suffix, "sh") == 0) || (strcmp(suffix, "csh") == 0)) {
		sflag = TRUE;
		cflag = FALSE;
		hflag = FALSE;
		mflag = FALSE;
		Mflag = FALSE;
		fflag = FALSE;
		xflag = FALSE;
		pflag = FALSE;
	}
	if(strcmp(suffix, "tex") == 0) {
		xflag = TRUE;
		sflag = FALSE;
		cflag = FALSE;
		hflag = FALSE;
		mflag = FALSE;
		Mflag = FALSE;
		fflag = FALSE;
		pflag = FALSE;
	}
	if(strcmp(suffix, "c") == 0){
		cflag = TRUE;
		hflag = FALSE;
		mflag = FALSE;
		Mflag = FALSE;
		sflag = FALSE;
		fflag = FALSE;
		xflag = FALSE;
		pflag = FALSE;
	}
	if(strcmp(suffix, "h") == 0){
		hflag = TRUE;
		cflag = FALSE;
		mflag = FALSE;
		Mflag = FALSE;
		sflag = FALSE;
		fflag = FALSE;
		xflag = FALSE;
		pflag = FALSE;
	}
	if(strcmp(suffix, "f") == 0){
		fflag = TRUE;
		hflag = FALSE;
		cflag = FALSE;
		mflag = FALSE;
		Mflag = FALSE;
		sflag = FALSE;
		xflag = FALSE;
		pflag = FALSE;
	}
	if(strcmp(suffix, "p") == 0){
		pflag = TRUE;
		hflag = FALSE;
		cflag = FALSE;
		mflag = FALSE;
		Mflag = FALSE;
		sflag = FALSE;
		xflag = FALSE;
		fflag = FALSE;
	}
	if(isdigit(*suffix) != 0) {
		Mflag = TRUE;
		hflag = FALSE;
		cflag = FALSE;
		mflag = FALSE;
		sflag = FALSE;
		fflag = FALSE;
		xflag = FALSE;
		pflag = FALSE;
	}
	set_flags();
	if(!qflag || debugon)
		printf("Hmm.  This file looks like a %s file.\n", msg_name);
}

/*
 * set_flags - set & check flags
 */
 
set_flags()
{
	if(cflag || hflag || mflag || Mflag || sflag || fflag || xflag) {
		if(cflag) {
			msg_name = m_stdc;
			header = h_stdc;
		}
		if(hflag) {
			msg_name = m_include;
			header = h_include;
		}
		if(mflag) {
			msg_name = m_make;
			header = h_make;
		}
		if(Mflag) {
			msg_name = m_manual;
			header = h_manual;
		}
		if(sflag) {
			msg_name = m_shell;
			header = h_make;
		}
		if(fflag) {
			msg_name = m_fortran;
			header = h_fortran;
		}
		if(xflag) {
			msg_name = m_tex;
			header = h_tex;
		}
		if(pflag) {
			msg_name = m_pascal;
			header = h_pascal;
		}
	} else {
		cflag = TRUE;
		set_flags();
	}
}

/*
 * copy from -> to
 */

copy(from, to, mode)
char *from;
char *to;
char *mode;
{
	FILE *fdfrom, *fdto, *fopen();
	char tmp[LENGTH];
	char s[LENGTH];

	if((fdfrom = fopen(from, "r")) == NULL) {
		fatal("Cannot open %s for reading.",from);
	}
	if((fdto = fopen(to, mode)) == NULL) {
		fatal("Cannot open %s for \"%s\".",to,mode);
	}
	while(fgets(s, sizeof(s), fdfrom) != NULL)
		fputs(s, fdto);
	fclose(fdfrom);
	fclose(fdto);
}

/*
 * Run RCS's rcsprog on file with flags.
 */

rcs(rcsprog, file, flags)
char *rcsprog;
char *file;
char *flags;
{
	char buf[LENGTH];
	char tmp[LENGTH];

	if(!checkfor("q", flags) && qflag)
		flags = "q";
	if(!flags || !*flags)
		sprintf(buf, "%s %s", rcsprog, file);
	else
		sprintf(buf, "%s -%s %s", rcsprog, flags, file);
	debug(sprintf(tmp,"Running ``%s''...\n", buf));
	if(!qflag)
		lineprint(sprintf(tmp, "Start of ``%s''", buf));
	system(buf);
	if(!qflag)
		lineprint(sprintf(tmp, "End of ``%s''", buf));
}

/*
 * checkdir - make RCS directory if not present.
 */

checkdir()
{
	if(access("RCS", 0) != 0){
		if(!qflag || debugon)
			printf("Cannot find \"RCS\" directory.  Creating...\n");
		if(strcmp(rcsdir, "RCS") != 0) { 
			if(symlink(rcsdir, "RCS") != 0)
				fatal("Symbolic link of %s to RCS failed.", 
					rcsdir);
		} else {
			if(mkdir(rcsdir, 0755) != 0)
				fatal("Cannot create \"%s\" directory.", 
					rcsdir);
		}
	}
}

/*
 * checkfor(x, str) -- check for x in str.  Return 1 (TRUE) if exists.
 *			Otherwise 0 (FALSE).
 */

checkfor(x, str)
char 	*x;
char 	*str;
{
	if (!str)
		return(FALSE);
	while(*str) {
		if(strcmp(str, x) == 0)
			return(TRUE);
		*str++;
	}
	return(FALSE);
}

/*
 * lineprint - print msg in a nice line
 */

lineprint(msg)
char *msg;
{
	int len, left, right, x;

	len = strlen(msg);
	right = (75-len)/2;
	left = right;
	for(x = 0; x < right; ++x)
		putchar('-');
	printf("[ %s ]", msg);
	for(x = 0; x < left; ++x)
		putchar('-');
	putchar('\n');
}

/*
 * fatal - print error and then exit(1).
 */
fatal(format, str)
char *format;
{
	static char namefmt[100];

	sprintf(namefmt, "%s: %s\n", progname, format);
	_doprnt(namefmt, &str, stderr);
	exit(1);
}

/*
 * zap str with NULL's
 */

zap(str)
char str[];
{
	int i, x;

	i = strlen(str);
	for(x = 0; x <= i; )
		str[x++] = NULL;
}

/*
 * get header names
 */

get_temp()
{
	zap(tbuf);
	if(mflag)
		strcpy(tbuf, header_make);
	if(Mflag)
		strcpy(tbuf, header_man);
	if(hflag)
		strcpy(tbuf, header_h);
	if(cflag)
		strcpy(tbuf, header_c);
	if(sflag)
		strcpy(tbuf, header_sh);
	if(fflag)
		strcpy(tbuf, header_f);
	if(xflag)
		strcpy(tbuf, header_tex);
	if(pflag)
		strcpy(tbuf, header_p);
}

usage()
{
	fprintf(stderr, "usage: %s ", progname);
	fprintf(stderr, USAGE);
}
