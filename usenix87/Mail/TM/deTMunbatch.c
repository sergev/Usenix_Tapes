/*
 * unbatchnews: extract news in batched format and process it one article
 * at a time.  The format looks like
 *	#! rnews 1234
 *	article containing 1234 characters
 *	#! rnews 4321
 *	article containing 4321 characters
 *
 *	Special purpose processing: Create new files in
 *		~news/uucp/TM* using the article ID as the filename.
 */

#ifndef lint
static char	*SccsId = "@(#)unbatch.c	1.8	9/3/84";
#endif !lint

# include <stdio.h>

char buf[BUFSIZ];
char hdr[BUFSIZ];

static char RNEWS_STR[] = "#! rnews ";
static char MSG_STR[] = "Message-ID: <";
static char TMPNAME[(sizeof "/usr/spool/news/uucp/") + (sizeof "TM.xxxxx.xxx")] =
	"/usr/spool/news/uucp/";

main(argc, argv)
int argc;
char **argv;
{
	register int length, x;
	register FILE *ndx_pfn, *pfn = NULL;
	register long size;
	register char *angle_brack, *idptr;
	char filename[BUFSIZ];
	char *index(), *gets();
	long atol();

	if ((argc != 2) ||
	    (strlen (argv[1]) != 12) ||
	    (0 != strncmp ("TM.", argv[1], 3)))
	{
		fprintf (stderr, "Argument error, argv[1]=%s.\n", argv[1]);
		exit (1);
	}

	strcat (TMPNAME, argv[1]);
	sprintf (filename, "%s/index", TMPNAME);
	if (0 == (ndx_pfn = fopen (filename, "w")))
		perror();
	gets (buf);
	while (0 == (x = strncmp (buf, RNEWS_STR, (sizeof RNEWS_STR) - 1)))
	{
		size = atol (buf - 1 + (sizeof RNEWS_STR));
		if(size <= 0)
		{
			fprintf (stderr, "Bad size=%d.\n", size);
			exit (3);
		}
		*hdr = NULL;
		pfn = NULL;
		while ((pfn == NULL)  &&  (size > 0)  &&  !feof(stdin))
		{
			fgets (buf, BUFSIZ, stdin);
			/* Save header info until article ID arrives. */
			strcat (hdr, buf);
			if (0 == strncmp (MSG_STR, buf, (sizeof MSG_STR) - 1))
			{
				idptr = buf + (sizeof MSG_STR) - 1;
				if (angle_brack = index (idptr, '>'))
				{
					*angle_brack = NULL;
					sprintf(filename,"%s/%s",TMPNAME,idptr);
					pfn = fopen(filename, "w");
					fprintf (ndx_pfn, "%s\n", idptr);
					fwrite (hdr,length = strlen(hdr),1,pfn);
					if ( 0 >= (size -= length))
					{
						fprintf (stderr,
						"Oops, header size too big.\n");
						exit (4);
					}
				}
				else
				{
					fprintf (stderr,
						"Oops, bad Message-ID.\n");
					exit (5);
				}
			}
		}
		if (pfn == NULL)	/* Article ID was not found... */
			exit();
		while (fgets (buf, BUFSIZ, stdin)	&&	(0 < size))
		{
			fwrite (buf, length=strlen(buf), 1, pfn);
			size -= length;
		}
		if (size != 0)
		{
			fprintf (stderr, "Article size != %d, filename=%s.\n",
					size, filename);
			exit (6);
		}
		fclose(pfn);
	}
	if (x)	/* x != 0   implies that the RNEWS strncmp test failed above. */
	{
		fprintf(stderr, "out of sync, skipping %s\n", buf);
		exit (2);
	}
}
