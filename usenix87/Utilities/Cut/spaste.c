/*
 * Serially paste a file together
 *
 * John Weald
 */
#include <stdio.h>

extern void exit();


spaste(files, c, n)
char *files[];		/* Null terminate list of input files		*/
char c[];		/* The concatintaion characters			*/
int n;			/* The number of above				*/
{
	int i;
	FILE *fp;

	if (files[0] == (char *)NULL)
	{
		spfile(stdin, c, n);
		return;
	}

	for (i = 0; files[i] != (char *)NULL; i++)
	{
		if (*files[i] == '-')
			fp = stdin;
		else if ((fp = fopen(files[i], "r")) == (FILE *)NULL)
		{
			fprintf(stderr, "Failed to open file %s\n", files[i]);
			exit(1);
		}
		spfile(fp, c, n);
		(void)fclose(fp);
	}
}
	
/* 
 * Do the actual paste of a stream.
 *
 * The method here is to read in the stream and replace all newline
 * characters with concatintaion characters. 
 * Output occurs after each chuck is parsed, or if the concatination character
 * is the null seperator (otherwise puts() would screw up on whole chunk).
 *
 * The stream is read in BUFSIZ chunks using fread. The input buffer is one
 * larger than read, so that it can be null terminated. 
 *
 * When we read in each chunk we must check if it needs to be joined to the
 * previous one i.e. the last character on the last chunk was a newline.
 */
static
spfile(fp, con, ncons)
FILE *fp;		/* serially paste this stream			*/
char con[];		/* The concatintaion characters			*/
int ncons;		/* The number of above				*/
{
	char *pstart;	/* The start of the string			*/
	char *ptr;	/* Walks down the stream			*/
	char buf[BUFSIZ + 1]; /* To ensure null termination		*/
	int n;		/* Number of bytes read with fread()		*/
	int k = 0;	/* Index into concatination character array	*/
	int join;	/* Join this chunk to the next?			*/
	char last;	/* The very last character looked at.		*/


	join = 0;
	while ((n = fread(buf, sizeof(char), sizeof(buf) - 1, fp)) != 0)
	{
		if (join)
		{
			/* Join with last chunk */
			putchar(con[k]);
			k = (k + 1) % ncons;
			join = 0;
		}
		/* Join to next chunk? */
		if (buf[n-1] == '\n')
		{
			join++;
			/* Ignore the newline */
			n--;
		}

		/* ensure null terminated buffer */
		buf[n] = '\0';

		
		/* 
	 	 * walk thru this chunk 
		 * replace all newlines with the next concat. char.
		 */
		for (pstart = ptr = buf; *ptr != '\0'; ptr++)
		{
			if (*ptr == '\n')
			{
				*ptr = con[k];
				if (con[k] == '\0')
				{
					fputs(pstart, stdout);
					pstart = ptr + 1;
				}
				k = (k + 1) % ncons;
			}
		}
		fputs(pstart, stdout);

		last = *(ptr - 1);
	}

	/*
	 * Maybe they asked for the newline as the
	 * concatination char. We would hate to give them two newlines
	 * in a row.
	 */
	if (last != '\n')
		putchar('\n');
}
