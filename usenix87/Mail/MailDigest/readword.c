/**			readword.c			**/

/* Stupid little program to read a line from stdin and write it to the
   specified filename.  This SHOULD be equivalent to

	read var ; echo $var > filename

   but that doesn't work from within Makefiles *sigh*

*/

#include <stdio.h>

main(argc, argv)
int argc;
char *argv[];
{
	FILE *file_device;	/* file descriptor... */
	char  buffer[100];	/* and input buffer   */

	/** first lets check the starting arguments... **/

	if (argc != 2) {
	  fprintf(stderr,"Usage: readword <filename>\n");
	  exit(1);
	}

	/** can we open the file for writing??? **/

	if ((file_device = fopen(argv[1], "w")) == NULL) {
	  fprintf(stderr,"Cannot open file %s for writing!\n", argv[1]);
	  exit(2);
	}

	/** okay...read and write the input **/

	gets(buffer);

	fputs(buffer, file_device);

	/** close the file... **/

	fclose(file_device);

	/** and we're done! **/

	exit(0);
}
