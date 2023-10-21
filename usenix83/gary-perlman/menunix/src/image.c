/*Copyright (c) 1981 Gary Perlman  All rights reserved*/
#include <curses.h>
#include <ctype.h>
image ()
	{
	int i, j;
	char c;
	FILE *ioptr;
	char file[100];
	sprintf (file, "image.%d", getpid ());
	ioptr = fopen (file, "w");
	for (i = 0; i < 24; i++)
		{
		for (j = 0; j < 80; j++)
			{
			c = stdscr->_y[i][j];
			if (isascii (c))
			    fputc (stdscr->_y[i][j], ioptr);
			else fputc (' ', ioptr);
			}
		fputc ('\n', ioptr);
		}
	fclose (ioptr);
	printf ("");
	}
