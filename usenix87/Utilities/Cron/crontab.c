#if !defined(lint) && !defined(LINT)
static char rcsid[] = "$Header: crontab.c,v 1.5 87/05/02 17:33:22 paul Exp $";
#endif

/* $Source: /usr/src/local/vix/cron/crontab.c,v $
 * $Revision: 1.5 $
 * $Log:	crontab.c,v $
 * Revision 1.5  87/05/02  17:33:22  paul
 * baseline for mod.sources release
 * 
 * Revision 1.4  87/03/31  13:11:48  paul
 * I won't say that rs@mirror gave me this idea
 *   but crontab uses getopt() now
 * 
 * Revision 1.3  87/03/30  23:43:48  paul
 * another suggestion from rs@mirror:
 *   use getpwuid(getuid)->pw_name instead of getenv("USER")
 *   this is a boost to security...
 * 
 * Revision 1.2  87/02/11  17:40:12  paul
 * changed command syntax to allow append and replace instead of append as
 * default and no replace at all.
 * 
 * Revision 1.1  87/01/26  23:49:06  paul
 * Initial revision
 */

/* Copyright 1987 by Vixie Enterprises
 * All rights reserved
 *
 * Distribute freely, except: don't sell it, don't remove my name from the
 * source or documentation (don't take credit for my work), mark your changes
 * (don't get me blamed for your possible bugs), don't alter or remove this
 * notice.  Commercial redistribution is negotiable; contact me for details.
 *
 * Send bug reports, bug fixes, enhancements, requests, flames, etc., and
 * I'll try to keep a version up to date.  I can be reached as follows:
 * Paul Vixie, Vixie Enterprises, 329 Noe Street, San Francisco, CA, 94114,
 * (415) 864-7013, {ucbvax!dual,ames,ucsfmis,lll-crg,sun}!ptsfa!vixie!paul.
 */


#define	MAIN_PROGRAM


#include "cron.h"
#include <pwd.h>
#include <errno.h>
#include <sys/file.h>


extern	char	*sprintf();


static char	USER[MAX_UNAME];
static char	FILENAME[MAX_FNAME];
static int	CHECK_ERROR_COUNT;
static enum	{opt_unknown, opt_list, opt_delete, opt_replace, opt_append}
		OPTION;

#if DEBUGGING
static char	*OPTIONS[] = {"???", "list", "delete", "replace", "append"};
#endif

static void
usage()
{
	fprintf(stderr, "usage:  %s [-u user] ...\n", PROGNAME);
	fprintf(stderr, " ... -l         (list user's crontab)\n");
	fprintf(stderr, " ... -d         (delete user's crontab)\n");
	fprintf(stderr, " ... -r file    (replace user's crontab)\n");
	fprintf(stderr, " ... -a file    (append file to user's crontab)\n");
	exit(ERROR_EXIT);
}


void
main(argc, argv)
	int	argc;
	char	*argv[];
{
	void	parse_args(), set_cron_uid(),
		append_cmd(), list_cmd(), delete_cmd(), replace_cmd();

	PROGNAME = argv[0];

	setlinebuf(stderr);
	parse_args(argc, argv);
	set_cron_uid();
	switch (OPTION)
	{
	case opt_list:		list_cmd();
				break;
	case opt_delete:	delete_cmd();
				break;
	case opt_replace:	replace_cmd();
				append_cmd();
				break;
	case opt_append:	append_cmd();
				break;
	}
}


static void
list_cmd()
{
	extern	errno;
	char	n[MAX_FNAME];
	FILE	*f;
	int	ch;

	(void) sprintf(n, SPOOLDIR, USER);
	if (!(f = fopen(n, "r")))
	{
		if (errno == ENOENT)
			fprintf(stderr, "no crontab for %s\n", USER);
		else
			perror(n);
		exit(ERROR_EXIT);
	}

	/* file it open. copy to stdout, close.
	 */
	Set_LineNum(1)
	while (EOF != (ch = get_char(f)))
		putchar(ch);
	fclose(f);
}


static void
delete_cmd()
{
	extern	errno;
	int	unlink();
	void	poke_daemon();

	char	n[MAX_FNAME];

	(void) sprintf(n, SPOOLDIR, USER);

	if (unlink(n))
	{
		if (errno == ENOENT)
			fprintf(stderr, "no crontab for %s\n", USER);
		else
			perror(n);
		exit(ERROR_EXIT);
	}
	poke_daemon();
}


static void
replace_cmd()
{
	extern	errno;
	int	unlink();
	void	poke_daemon();

	char	n[MAX_FNAME];

	(void) sprintf(n, SPOOLDIR, USER);

	if (unlink(n))
	{
		if (errno != ENOENT)
		{
			perror(n);
			exit(ERROR_EXIT);
		}
	}
}


static void
check_error(msg)
	char	*msg;
{
	CHECK_ERROR_COUNT += 1;
	fprintf(stderr, "\"%s\", line %d: %s\n", FILENAME, LINE_NUMBER, msg);
}


static void
append_cmd()
{
	char	*sprintf();
	entry	*load_entry();
	int	load_env();
	void	free_entry();
	void	check_error();
	void	poke_daemon();

	char	n[MAX_FNAME], envstr[MAX_TEMPSTR];
	FILE	*old, *new;
	int	ch;
	entry	*e;
	int	status;

	/* this routine is called for both 'replace' and 'append' commands
	 */

	/* open the new file being installed.
	 */
	if (!(new = fopen(FILENAME, "r")))
	{
		perror(FILENAME);
		exit(ERROR_EXIT);
	}

	/* check the syntax of the file being installed.
	 */

	/* BUG: was reporting errors after the EOF if there were any errors
	 * in the file proper -- kludged it by stopping after first error.
	 *		vix 31mar87
	 */
	CHECK_ERROR_COUNT = 0;
	while (!CHECK_ERROR_COUNT && ERR != (status = load_env(envstr, new)))
	{
		if (status == FALSE)
		{
			if (NULL != (e = load_entry(new, check_error)))
				free((char *) e);
		}
	}
	if (CHECK_ERROR_COUNT != 0)
	{
		fprintf(stderr, "errors in crontab file, can't install.\n");
		fclose(new);
		exit(ERROR_EXIT);
	}
	rewind(new);

	/* open the old file for append-access.  this depends on the file
	 * being created if it does not exist.
	 */
	(void) sprintf(n, SPOOLDIR, USER);
	if (!(old = fopen(n, "a")))
	{
		perror(n);
		fclose(new);
		exit(ERROR_EXIT);
	}

	/* append the new to the old
	 */
	Set_LineNum(1)
	while (EOF != (ch = get_char(new)))
		putc(ch, old);

	/* close them and we're done
	 */
	fclose(old);
	fclose(new);

	/* set the file to be owned by root
	 */
	if (ERR == chown(n, ROOT_UID, -1))
	{
		perror("chown");
		exit(ERROR_EXIT);
	}

	poke_daemon();
}


static void
poke_daemon()
{
	int	fd;

	if (ERR == (fd = open(POKECRON, O_WRONLY|O_CREAT, 0)))
	{
		perror(POKECRON);
		return;
	}
	close(fd);
}
	

static void
parse_args(argc, argv)
	int	argc;
	char	*argv[];
{
	void		usage();
	char		*getenv(), *strcpy();
	int		getuid();
	struct passwd	*getpwnam();
	extern int	getopt(), optind;
	extern char	*optarg;

	struct passwd	*pw;
	int		argch;

	if (!(pw = getpwuid(getuid())))
	{
		fprintf(stderr, "Your UID isn't in the passwd file.\n");
		perror("getpwuid");
	}
	strcpy(USER, pw->pw_name);
	FILENAME[0] = '\0';
	OPTION = opt_unknown;
	while (EOF != (argch = getopt(argc, argv, "u:ldr:a:x:")))
	{
		switch (argch)
		{
		case 'x':
			if (!set_debug_flags(optarg))
				usage();
			break;
		case 'u':
			if (getuid() != ROOT_UID)
			{
				fprintf(stderr,
				"must be privileged to use -u\n"
				);
				exit(ERROR_EXIT);
			}
			if ((struct passwd *)NULL == getpwnam(optarg))
			{
				fprintf(stderr, "%s:  user '%s' unknown\n",
					PROGNAME, optarg);
				exit(ERROR_EXIT);
			}
			(void) strcpy(USER, optarg);
			break;
		case 'l':
			if (OPTION != opt_unknown)
				usage();
			OPTION = opt_list;
			break;
		case 'd':
			if (OPTION != opt_unknown)
				usage();
			OPTION = opt_delete;
			break;
		case 'a':
			if (OPTION != opt_unknown)
				usage();
			OPTION = opt_append;
			(void) strcpy(FILENAME, optarg);
			break;
		case 'r':
			if (OPTION != opt_unknown)
				usage();
			OPTION = opt_replace;
			(void) strcpy(FILENAME, optarg);
			break;
		default:
			usage();
		}
	}

	if (OPTION == opt_unknown || argv[optind] != NULL)
		usage();

	endpwent();

	Debug(DMISC, ("user=%s, file=%s, option=%s\n",
					USER, FILENAME, OPTIONS[(int)OPTION]))
}
