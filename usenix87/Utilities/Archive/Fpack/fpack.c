/*MANUAL.TH FPACK 1WI "December 15, 1985" "Wang Institute" "Unix User's Manual"
*/

/*MANUAL.SH NAME
fpack \- pack and unpack ascii files with simple archiving scheme
*/

/*MANUAL.SH USAGE
.B fpack
[files]
*/

/*MANUAL.SH DESCRIPTION
.I fpack
is a simple plain text file archiving scheme to either
reduce the number of files or to package them together.
It is designed to be portable to systems between which
files may be transferred, such as between UNIX and MSDOS.
It can save space on systems that use disk blocks for files that occupy
a small part of a block.
One of the program's requirements is that it does not alter the format
of its input, so files like documents or human readable data files
are not converted to a special format.
.PP
Files are delimited by a special string at the start of a line:
.br
	fpack:!@#$%^&*():	<filename>
.br
*/

/*MANUAL.SH NOTES
.PP
Text outside file delimiters in an archive will be ignored.
.PP
If a file does not end with a newline character,
one will be silently added.
.PP
If a file to be unpacked exists,
then it will not be overwritten.
Instead, the contents of the file(s) being unpacked will be discarded.
*/

/*MANUAL.SH EXAMPLES
.nf
Pack up some C source files.
	fpack a.c b.c c.c > archive
Unpack all files.
	fpack < archive
.fi
*/

/*MANUAL.SH "SEE ALSO"
shar(1), sh(1) make a more flexible archiving scheme for UNIX.
*/

/*MANUAL.SH AUTHOR
Gary Perlman
*/


#include <stdio.h>

#ifndef lint
static	char	sccsid[] = "@(#)fpack.c 1.1 (WangInst) 12/15/85";
#endif

#define	MAGIC	"fpack:!@#$%^&*(): " /* default file delimiter */

#ifdef	VERBOSE
#define	blabber(fun,file) fprintf (stderr, "fpack: %s '%s'\n", fun, file)
#else
#define	blabber(fun,file)
#endif

typedef	int       Status;      /* return/exit status of functions */
#define	SUCCESS   ((Status) 0)
#define	FAILURE   ((Status) 1)

/*FUNCTION main */
main (argc, argv)
int 	argc;     /* argument count */
char	**argv;   /* argument vector */
	{
	Status	result = SUCCESS;

	if (argc == 1) /* no files, unpack stdin */
		result = funpack ();
	else
		{
		while (--argc)
			if (fpack (*++argv) == FAILURE)
				result = FAILURE;
		printf ("%s\n", MAGIC); /* end of files */
		}

	exit (result);
	}

/*FUNCTION fpack:	pack files for later extraction by funpack */
Status
fpack (file)
char	*file;
	{
	FILE	*ioptr;
	char	line[BUFSIZ];
	char	*ptr;
	
	if (ioptr = fopen (file, "r"))
		{
		blabber ("packing", file);
		printf ("%s%s%s\n", MAGIC, file);
		while (fgets (line, BUFSIZ, ioptr))
			fputs (line, stdout);
		for (ptr = line; *ptr; ptr++);
		if (ptr > line && *(ptr-1) != '\n') /* incomplete last line */
			putc ('\n', stdout);
		fclose (ioptr);
		return (SUCCESS);
		}
	fprintf (stderr, "fpack: Can't open '%s' for reading\n", file);
	return (FAILURE);
	}

/*FUNCTION funpack:	unpack and create files packed by fpack */
Status
funpack ()
	{
	FILE	*ioptr = NULL;
	char	line[BUFSIZ];
	int 	maglen = strlen (MAGIC);
	char	*ptr;

	while (gets (line))
		{
		if (!strncmp (MAGIC, line, maglen))
			{
			if (ioptr)
				{
				fclose (ioptr);
				ioptr = NULL;
				}
			ptr = line + maglen;
			if (*ptr == '\0') /* done */
				ioptr = NULL;
			else if (access (ptr, 4) == 0) /* file exists */
				{
				fprintf (stderr, "fpack: '%s' exists (not unpacked)\n", ptr);
				ioptr = NULL;
				}
			else if ((ioptr = fopen (ptr, "w")) == NULL)
				{
				fprintf (stderr, "fpack: Can't create '%s'\n", ptr);
				return (FAILURE);
				}
			if (ioptr != NULL)
				blabber ("unpacking", ptr);
			}
		else if (ioptr != NULL)
			{
			fputs (line, ioptr);
			putc ('\n', ioptr);
			}
		}
	return (SUCCESS);
	}

/*FUNCTION access:	determine if a file exists */
#ifdef MSDOS /* fake a version of the access(2) function */
access (file, mode)
char	*file;
int 	mode;     /* this is ignored in MSDOS version */
	{
	FILE	*ioptr;
	if (ioptr = fopen (file, "r"))
		fclose (ioptr);
	return (ioptr == NULL);
	}
#endif
