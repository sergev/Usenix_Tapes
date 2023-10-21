/*
 * Buffer management for public domain tar.
 *
 * Written by John Gilmore, ihnp4!hoptoad!gnu, on 25 August 1985.
 *
 * @(#) buffer.c 1.14 10/28/86 Public Domain - gnu
 */

#include <stdio.h>
#include <errno.h>
#include <sys/types.h>		/* For non-Berkeley systems */
#include <sys/file.h>
#include <signal.h>

#include "tar.h"
#include "port.h"

#define	STDIN	0		/* Standard input  file descriptor */
#define	STDOUT	1		/* Standard output file descriptor */

#define	PREAD	0		/* Read  file descriptor from pipe() */
#define	PWRITE	1		/* Write file descriptor from pipe() */

extern char	*valloc();

/*
 * V7 doesn't have a #define for this.
 */
#ifndef O_RDONLY
#define	O_RDONLY	0
#endif

#define	MAGIC_STAT	105	/* Magic status returned by child, if
				   it can't exec compress.  We hope compress
				   never returns this status! */
/*
 * The record pointed to by save_rec should not be overlaid
 * when reading in a new tape block.  Copy it to record_save_area first, and
 * change the pointer in *save_rec to point to record_save_area.
 * Saved_recno records the record number at the time of the save.
 * This is used by annofile() to print the record number of a file's
 * header record.
 */
static union record **save_rec;
static union record record_save_area;
static int	    saved_recno;

/*
 * PID of child compress program, if f_compress.
 */
static int	compress_pid;

/*
 * Record number of the start of this block of records
 */
static int	baserec;

/*
 * Error recovery stuff
 */
static int	r_error_count;


/*
 * Return the location of the next available input or output record.
 */
union record *
findrec()
{
	if (ar_record == ar_last) {
		flush_archive();
		if (ar_record == ar_last)
			return (union record *)NULL;	/* EOF */
	}
	return ar_record;
}


/*
 * Indicate that we have used all records up thru the argument.
 * (should the arg have an off-by-1? XXX FIXME)
 */
void
userec(rec)
	union record *rec;
{
	while(rec >= ar_record)
		ar_record++;
	/*
	 * Do NOT flush the archive here.  If we do, the same
	 * argument to userec() could mean the next record (if the
	 * input block is exactly one record long), which is not what
	 * is intended.
	 */
	if (ar_record > ar_last)
		abort();
}


/*
 * Return a pointer to the end of the current records buffer.
 * All the space between findrec() and endofrecs() is available
 * for filling with data, or taking data from.
 */
union record *
endofrecs()
{
	return ar_last;
}


/*
 * Open an archive file.  The argument specifies whether we are
 * reading or writing.
 */
open_archive(read)
	int read;
{

	if (ar_file[0] == '-' && ar_file[1] == '\0') {
		if (read)	archive = STDIN;
		else		archive = STDOUT;
	} else if (read) {
		archive = open(ar_file, O_RDONLY);
	} else {
		archive = creat(ar_file, 0666);
	}

	if (archive < 0) {
		perror(ar_file);
		exit(EX_BADARCH);
	}

	/*NOSTRICT*/
	ar_block = (union record *) valloc((unsigned)blocksize);
	if (!ar_block) {
		fprintf(stderr,
		"tar: could not allocate memory for blocking factor %d\n",
			blocking);
		exit(EX_ARGSBAD);
	}

	ar_record = ar_block;
	ar_last   = ar_block + blocking;

	/*
	 * Handle compressed archives.
	 *
	 * FIXME, currently supported for reading only.
	 * FIXME, writing involves forking again for a small process
	 * that will reblock the output of compress to the user's specs.
	 */
	if (f_compress) {
		int pipes[2];
		int err;

		if (!read) {
			fprintf(stderr,
				"tar: cannot write compressed archives yet.\n");
			exit(EX_ARGSBAD);
		}

		/* Create a pipe to get compress's output to us */
		err = pipe(pipes);
		if (err < 0) {
			perror ("tar: cannot create pipe to compress");
			exit(EX_SYSTEM);
		}
		
		/* Fork compress process */
		compress_pid = fork();
		if (compress_pid < 0) {
			perror("tar: cannot fork compress");
			exit(EX_SYSTEM);
		}

		/*
		 * Child process.
		 * 
 		 * Move input to stdin, write side of pipe to stdout,
		 * then exec compress.
		 */
		if (compress_pid == 0) {
			(void) close (pipes[PREAD]);	/* We won't use it */
			if (archive != STDIN) {
				(void) close(STDIN);
				err = dup(archive);
				if (err != 0) {
					perror(
					 "tar: cannot dup input to stdin");
					exit(EX_SYSTEM);
				}
				(void) close(archive);
			}
			if (pipes[PWRITE] != STDOUT) {
				(void) close (STDOUT);
				err = dup (pipes[PWRITE]);
				if (err != STDOUT) {
					perror(
					  "tar: cannot dup pipe output");
					exit(MAGIC_STAT);
				}
				(void) close (pipes[PWRITE]);
			}
			execlp("compress", "compress", "-d", (char *)0);
			perror("tar: cannot exec compress");
			exit(MAGIC_STAT);
		}

		/*
		 * Parent process.  Clean up.
		 * FIXME, note that this may leave standard input closed,
		 * if the compressed archive was on standard input.
		 */
		(void) close (archive);		/* Close compressed archive */
		(void) close (pipes[PWRITE]);	/* Close write side of pipe */
		archive = pipes[PREAD];		/* Read side is our archive */

#ifdef BSD42
		f_reblock++;		/* Pipe will give random # of bytes */
#endif BSD42
	}

	ar_reading = read;
	if (read) {
		ar_last = ar_block;		/* Set up for 1st block = # 0 */
		flush_archive();
	}
}


/*
 * Remember a union record * as pointing to something that we
 * need to keep when reading onward in the file.  Only one such
 * thing can be remembered at once, and it only works when reading
 * an archive.
 */
saverec(pointer)
	union record **pointer;
{

	save_rec = pointer;
	saved_recno = baserec + ar_record - ar_block;
}

/*
 * Perform a write to flush the buffer.
 */
fl_write()
{
	int err;

	err = write(archive, ar_block->charptr, blocksize);
	if (err == blocksize) return;
	/* FIXME, multi volume support on write goes here */
	if (err < 0)
		perror(ar_file);
	else
		fprintf(stderr, "tar: %s: write failed, short %d bytes\n",
			ar_file, blocksize - err);
	exit(EX_BADARCH);
}


/*
 * Handle read errors on the archive.
 *
 * If the read should be retried, readerror() returns to the caller.
 */
void
readerror()
{
#	define	READ_ERROR_MAX	10

	read_error_flag++;		/* Tell callers */

	annorec(stderr, tar);
	fprintf(stderr, "Read error on ");
	perror(ar_file);

	if (baserec == 0) {
		/* First block of tape.  Probably stupidity error */
		exit(EX_BADARCH);
	}	

	/*
	 * Read error in mid archive.  We retry up to READ_ERROR_MAX times
	 * and then give up on reading the archive.  We set read_error_flag
	 * for our callers, so they can cope if they want.
	 */
	if (r_error_count++ > READ_ERROR_MAX) {
		annorec(stderr, tar);
		fprintf(stderr, "Too many errors, quitting.\n");
		exit(EX_BADARCH);
	}
	return;
}


/*
 * Perform a read to flush the buffer.
 */
fl_read()
{
	int err;		/* Result from system call */
	int left;		/* Bytes left */
	char *more;		/* Pointer to next byte to read */

	/*
	 * Clear the count of errors.  This only applies to a single
	 * call to fl_read.  We leave read_error_flag alone; it is
	 * only turned off by higher level software.
	 */
	r_error_count = 0;	/* Clear error count */

	/*
	 * If we are about to wipe out a record that
	 * somebody needs to keep, copy it out to a holding
	 * area and adjust somebody's pointer to it.
	 */
	if (save_rec &&
	    *save_rec >= ar_record &&
	    *save_rec < ar_last) {
		record_save_area = **save_rec;
		*save_rec = &record_save_area;
	}
error_loop:
	err = read(archive, ar_block->charptr, blocksize);
	if (err == blocksize) return;
	if (err < 0) {
		readerror();
		goto error_loop;	/* Try again */
	}

	more = ar_block->charptr + err;
	left = blocksize - err;

again:
	if (0 == (((unsigned)left) % RECORDSIZE)) {
		/* FIXME, for size=0, multi vol support */
		/* On the first block, warn about the problem */
		if (!f_reblock && baserec == 0 && f_verbose) {
			annorec(stderr, tar);
			fprintf(stderr, "Blocksize = %d records\n",
				err / RECORDSIZE);
		}
		ar_last = ar_block + ((unsigned)(blocksize - left))/RECORDSIZE;
		return;
	}
	if (f_reblock) {
		/*
		 * User warned us about this.  Fix up.
		 */
		if (left > 0) {
error_loop_2:
			err = read(archive, more, left);
			if (err < 0) {
				readerror();
				goto error_loop_2;	/* Try again */
			}
			if (err == 0) {
				annorec(stderr, tar);
				fprintf(stderr,
		"%s: eof not on block boundary, strange...\n",
					ar_file);
				exit(EX_BADARCH);
			}
			left -= err;
			more += err;
			goto again;
		}
	} else {
		annorec(stderr, tar);
		fprintf(stderr, "%s: read %d bytes, strange...\n",
			ar_file, err);
		exit(EX_BADARCH);
	}
}


/*
 * Flush the current buffer to/from the archive.
 */
flush_archive()
{
	baserec += ar_last - ar_block;/* Keep track of block #s */
	ar_record = ar_block;		/* Restore pointer to start */
	ar_last = ar_block + blocking;	/* Restore pointer to end */

	if (!ar_reading) 
		fl_write();
	else
		fl_read();
}

/*
 * Close the archive file.
 */
close_archive()
{
	int child;
	int status;

	if (!ar_reading) flush_archive();
	(void) close(archive);

	if (f_compress) {
		/*
		 * Loop waiting for the right child to die, or for
		 * no more kids.
		 */
		while (((child = wait(&status)) != compress_pid) && child != -1)
			;

		if (child != -1) {
			switch (TERM_SIGNAL(status)) {
			case 0:		/* Terminated by itself */
				if (TERM_VALUE(status) == MAGIC_STAT) {
					exit(EX_SYSTEM);/* Child had trouble */
				}
				if (TERM_VALUE(status))
					fprintf(stderr,
				  "tar: compress child returned status %d\n",
						TERM_VALUE(status));
			case SIGPIPE:
				break;		/* This is OK. */

			default:
				fprintf(stderr,
				 "tar: compress child died with signal %d%s\n",
				 TERM_SIGNAL(status),
				 TERM_COREDUMP(status)? " (core dumped)": "");
			}
		}
	}
}


/*
 * Message management.
 *
 * anno writes a message prefix on stream (eg stdout, stderr).
 *
 * The specified prefix is normally output followed by a colon and a space.
 * However, if other command line options are set, more output can come
 * out, such as the record # within the archive.
 *
 * If the specified prefix is NULL, no output is produced unless the
 * command line option(s) are set.
 *
 * If the third argument is 1, the "saved" record # is used; if 0, the
 * "current" record # is used.
 */
void
anno(stream, prefix, savedp)
	FILE	*stream;
	char	*prefix;
	int	savedp;
{
#	define	MAXANNO	50
	char	buffer[MAXANNO];	/* Holds annorecment */
#	define	ANNOWIDTH 13
	int	space;

	if (f_sayblock) {
		if (prefix) {
			fputs(prefix, stream);
			putc(' ', stream);
		}
		sprintf(buffer, "rec %d: ",
			savedp?	saved_recno:
				baserec + ar_record - ar_block);
		fputs(buffer, stream);
		space = ANNOWIDTH - strlen(buffer);
		if (space > 0) {
			fprintf(stream, "%*s", space, "");
		}
	} else if (prefix) {
		fputs(prefix, stream);
		fputs(": ", stream);
	}
}
