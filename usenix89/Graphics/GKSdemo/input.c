#include <fcntl.h>
#include <tam.h>
#include <ctype.h>
#include "mongo.h"


	/* open the given file, setting 'fp' to its file pointer */

get_file(cmd_buf)
char *cmd_buf;
{
	char filename[20], dummy[10];

	sscanf(cmd_buf, "%s %s", dummy, filename);
	if ((fp = fopen(filename, "r")) == NULL) {
		wprintf(text_w, "cannot open file %s\n", filename);
	}
}
	

	/* grab the x data from the given column */

get_xc(cmd_buf)
char *cmd_buf;
{
	int i, j, c1, c2;
	char c[10];
	char buf[BUFSIZ];
	char dummy[MAX_COLUMN][10];

	if (fp == NULL) {
		wprintf(text_w, "no file open\n");
		return;
	}
	if (sscanf(cmd_buf, "%s %d", c, &c1) < 2) {
		wprintf(text_w, "%d\n", xc);
		return;
	}
	else if ((c1 < 1) || (c1 > MAX_COLUMN)) {
		wprintf(text_w, "%d: invalid column number\n", c1);
		return;
	}
	xc = c1;

	fseek(fp, (long) 0, 0);
	i = 0;

	while ((fgets(buf, BUFSIZ, fp) != NULL) && (i < AR_SIZE)) {
		for (j = 0; j < MAX_COLUMN; j++)
			dummy[j][0] = '\0';
		if (sscanf(buf, "%s %s %s %s %s %s %s %s %s %s", dummy[0], dummy[1],
			dummy[2], dummy[3], dummy[4], dummy[5], dummy[6], dummy[7],
			dummy[8], dummy[9]) < xc)
			wprintf(text_w, "inconsistent data on line %d\n", i + 1);
		if (sscanf(dummy[xc - 1], "%lf", &file_x[i]) < 1)
			wprintf(text_w, "inconsistent data on line %d\n", i + 1);
		if  (++i >= AR_SIZE)
			break;
	}
 	num_read = i;
}


	/* grab the y data from the given column */

get_yc(cmd_buf)
char *cmd_buf;
{
	int i, j, c1, c2;
	char c[10];
	char buf[BUFSIZ];
	char dummy[MAX_COLUMN][10];

	if (fp == NULL) {
		wprintf(text_w, "no file open\n");
		return;
	}
	if (sscanf(cmd_buf, "%s %d", c, &c1) < 2) {
		wprintf(text_w, "%d\n", yc);
		return;
	}
	else if ((c1 < 1) || (c1 > MAX_COLUMN)) {
		wprintf(text_w, "%d: invalid column number\n", c1);
		return;
	}
	yc = c1;

	fseek(fp, (long) 0, 0);
	i = 0;

	while ((fgets(buf, BUFSIZ, fp) != NULL) && (i < AR_SIZE)) {
		for (j = 0; j < MAX_COLUMN; j++)
			dummy[j][0] = '\0';
		if (sscanf(buf, "%s %s %s %s %s %s %s %s %s %s", dummy[0], dummy[1],
			dummy[2], dummy[3], dummy[4], dummy[5], dummy[6], dummy[7],
			dummy[8], dummy[9]) < yc)
			wprintf(text_w, "inconsistent data on line %d\n", i + 1);
		if (sscanf(dummy[yc - 1], "%lf", &file_y[i]) < 1)
			wprintf(text_w, "inconsistent data on line %d\n", i + 1);
		if  (++i >= AR_SIZE)
			break;
	} 	
	num_read = i;
}


	/* get the limits for the plot - if called without arguments,
	   set limits by the extremes of the data points. put the limits
	   into the variables:
				minx, maxx, miny, maxy
	   in that order. */

get_limits(cmd_buf)
char *cmd_buf;
{
	char dummy[10];
	int i;
	double diff;
	
	
	if (sscanf(cmd_buf, "%s %lf %lf %lf %lf", 
				dummy, &minx, &maxx, &miny, &maxy) < 5) {
		maxx = minx = file_x[0];
		maxy = miny = file_y[0];
	
		for (i = low_line - 1; (i < num_read) && (i < high_line - 1); i++) {
			if (file_x[i] > maxx)
				maxx = file_x[i];
			if (file_x[i] < minx)
				minx = file_x[i];
			if (file_y[i] > maxy)
				maxy = file_y[i];
			if (file_y[i] < miny)
				miny = file_y[i];
		}

		diff = (maxx - minx) / 20;
		maxx += diff; 
		minx -= diff; 
		diff = (maxy - miny) / 20;
		maxy += diff; 
		miny -= diff; 

	}
}


	/* use only data from between the given two lines in plotting
	   and so forth */

lines(cmd_buf)
char *cmd_buf;
{
	char dummy[10];
	int num, l, h;

	if ((num = sscanf(cmd_buf, "%s %d %d", dummy, &l, &h)) == 1) {
		wprintf(text_w, "%d %d\n", low_line, high_line);
	}
	else if (num < 3) {
		wprintf(text_w, "usage: lines low high\n");
	}
	else {
		if ((l >= 1) && (l < AR_SIZE))
			low_line = l;
		if ((h >= 1) && (h < AR_SIZE))
			high_line = h;
	}
}


	/* get a string from a text window up to, but not including,
	   a newline */

wgets(wn, str)
short wn;
char *str;
{
	char *s = str;
	int c;

	while (((c = *s++ = wgetc(wn)) != LF) && (c != CR))
		wputc(wn, c);
	wputc(wn, LF);
	*(s-1) = '\0';
	fix_text(str);	
}


	/* convert all the backspace characters in the given string
	   into actual deletions in the text, so that
			limirts
	   becomes the proper
			limits
	*/

fix_text(str)
char *str;
{
	int i;
	static char new[100], *p, *q;

	for (q = new, p = str; (*q = *p) != '\0'; p++, q++)
		if (*p == BACKSPACE)
			q -= 2;
	strcpy(str, new);
}


	/* if given two arguments, move the text position to the
	   given place in file (not NDC) coordinates. if given no
	   arguments, print the current text position. if given one,
	   print an error message. */

position(cmd_buf)
char *cmd_buf;
{
	int num;
	char dummy[10];
	double px, py;

	if ((num = sscanf(cmd_buf, "%s %lf %lf", dummy, &px, &py)) == 1) {
		wprintf(text_w, "%lf %lf\n", x_text_pos, y_text_pos);
	}
	else if (num >= 3) {
		x_text_pos = px;
		y_text_pos = py;
	}
	else {
		wprintf(text_w, "usage: position x_pos y_pos\n");
	}
}
