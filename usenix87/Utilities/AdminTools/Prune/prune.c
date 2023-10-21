/*
 *	prune - clips off the tops of a list of files described in the
 *		file /etc/prune_list. Designed to be run periodically
 *		from cron, the idea is to automatically shorten log
 *		files that grow forever. This file contains a list of
 *		files (full pathname), and the number of blocks that
 *		should be retained.  To retain some sanity, prune will
 *		clip the file after the next newline. The file
 *		/etc/prune_list should look something like:
 *
 *			/usr/adm/aculog		10
 *			/usr/adm/leo.log	5
 *			/usr/adm/messages	200
 *			/usr/adm/sup_log	5
 *
 *		The crontab entry on infoswx for prune looks like:
 *
 *			0 5 * * * /etc/prune >/usr/adm/prune.log 2>&1
 *
 *		Compile with:  cc -O -o prune prune.c
 *
 *		The following defines may be adjusted to suit your taste
 *		and your system block size.
 *
 *	Ray Davis  infoswx!bees  09/25/85
 */

#define	PRUNELIST	"/etc/prune_list"
#define	BLOCKSIZE	1024

#ifdef USG	/* for System III, System V, or Xenix */
#define	index		strchr
#endif

#define	FALSE		(0)
#define	TRUE		(~0)
#define	REWIND(F)	(lseek(F,0,0))

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/file.h>
#include	<stdio.h>

extern	int	errno;
extern	char	*index();
char	tempfile[] = "/usr/tmp/prune:XXXXXX";

main(argc, argv)
int	argc;
char	**argv;
{
	int	pfd,
		tfd,
		cfd,
		nblocks,
		new_line_found,
		bytes_read,
		nlbytes;
	char	file[128],
		block[BLOCKSIZE+1],
		*nl;
	long	nbytes;
	FILE	*pstream;
	struct	stat	cstat;

	block[BLOCKSIZE] = '\0';

	/* open prune list */
	if ((pfd = open(PRUNELIST, O_RDONLY)) < 0)
		errabort(*argv, "open", PRUNELIST, TRUE);
	pstream = fdopen(pfd, "r");

	/* create unique tempfile */
	mktemp(tempfile);

	/* while read each filename & #blocks */
	while ((fscanf(pstream, "%s%d", file, &nblocks)) != EOF) {
		fprintf(stderr, "pruning %s to %d blocks\n", file, nblocks);
		/* open filename */
		if ((cfd = open(file, O_RDWR)) < 0) {
			errabort(*argv, "open", file, FALSE);
			continue;
		}
		/* seek #blocks from the end of file */
		nbytes = - (nblocks * BLOCKSIZE);
		if ((fstat(cfd, &cstat)) < 0) {
			errabort(*argv, "fstat", file, FALSE);
		} else {
			if (cstat.st_size <= -(nbytes)) {
				fprintf(stderr, "%s: %s: not large enough to prune\n", *argv, file);
			} else {
				if ((lseek(cfd, nbytes, 2)) < 0) {
					fprintf(stderr, "%s: %s: error on seek\n", *argv, file);
				} else {
					/* open tempfile */
					if ((tfd = open(tempfile, O_CREAT|O_RDWR|O_TRUNC, 0600)) < 0)
						errabort(*argv, "open", tempfile, TRUE);
					/* copy to tempfile */
					new_line_found = FALSE;
					while ((bytes_read = read(cfd, block, BLOCKSIZE)) > 0) {
						if (new_line_found) {
							if ((write(tfd, block, bytes_read)) != bytes_read)
								errabort(*argv, "write", tempfile, FALSE);
						} else {
							if ((nl = index(block, '\n')) == NULL) {
								nl = block;
							} else {
								nl++;
								new_line_found = TRUE;
							}
							nlbytes = bytes_read - (nl - block);
							if ((write(tfd, nl, nlbytes)) != nlbytes)
								errabort(*argv, "write", tempfile, FALSE);
						}
					}
					if (bytes_read < 0) {
						errabort(*argv, "read", file, FALSE);
					} else {
						/* re-open filename to truncate */
						close(cfd);
						if ((cfd = open(file, O_RDWR|O_TRUNC)) < 0) {
							errabort(*argv, "re-open", file, FALSE);
							continue;
						}
						/* overwrite file with tempfile */
						REWIND(tfd);
						while ((bytes_read = read(tfd, block, BLOCKSIZE)) > 0) {
							if ((write(cfd, block, bytes_read)) != bytes_read)
								errabort(*argv, "write", file, FALSE);
						}
						if (bytes_read < 0)
							errabort(*argv, "read", tempfile, FALSE);
					}
				}
			}
		}
		close(cfd);
		close(tfd);
	}
	/* remove tempfile */
	unlink(tempfile);
	exit(0);
}

errabort(program, function, filename, should_exit)
char	*program,
	*function,
	*filename;
int	should_exit;
{
	char	errbuff[128];

	sprintf(errbuff, "%s: %s(%s)", program, function, filename);
	perror(errbuff);
	if (should_exit)
		exit(errno);
}
/* end of prune.c */
