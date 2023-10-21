/* readmsg.c :=: 9/25/85   By SLB */

#include "chat.h"

#define	STARTTXT	'='
#define LOGMSG		'*'
#define ENDTXT		'<'
#define	READLEN	INLEN + 50	/* Max number of chars input */

int	CO;

readmsg()
{
	char	linebuf[READLEN];

	if (stat(myfile, &sbuf)	== ERR)
		return;
	if (sbuf.st_size == 0)
		return;

	if ((wfd=fopen(myfile, "r")) ==	FERROR) {
		error("readmsg(): can't	open myfile", ON);
		quit();
	}

	while (fgets(linebuf, READLEN, wfd) != NULL)
		wordwrap(linebuf);

	if (fclose(wfd)	== ERR)	{
		error("readmsg(): closing error.", ON);
		quit();
	}
	close(creat(myfile, 0644));	/* Wipe out file */
}

wordwrap(s)	/* Wordwrap s to output */
char	*s;
{
	int	buflen, tmplen, cmplen;
	int	indent;

	char	buffer[800],
		temp[40],
		*ptr;

	cmplen = CO - 1;
					/* Check possible text types */
#ifdef INDEX
	if ((s[0] == LOGMSG) || ((ptr=index(s, STARTTXT)) == NULL)) {
#else
	if ((s[0] == LOGMSG) || ((ptr=strchr(s, STARTTXT)) == NULL)) {
#endif INDEX
		fputs(s, stdout);
		fputs("\r", stdout);
		return;
	}

	indent = strlen(s) - strlen(ptr) + 2; /* Get message indent */
	
	strcpy(buffer, "");

	while(strip(s, temp) != EOF) {
		buflen = strlen(buffer);
		tmplen = strlen(temp);
		if ((buflen + tmplen) < cmplen) {
			strcat(buffer, temp);
		} else {
			buflen = strlen(buffer);
			cmplen = buflen + CO + 2;
			strcat(buffer, "\r\n");
			strncat(buffer, "                    ", indent);
			strcat(buffer, temp);
		}
	}
	fputs(buffer, stdout);
	puts("\r");
}

strip(str, outstr)	/* Break string into words 20 chars or less */
char *str, *outstr;
{
	int	i, j;

	for (i=0,j=0; j < 20 && (str[i] != ' ' && str[i] != '\n'); i++, j++) {
			outstr[j] = str[i];
			if (str[i] == '\0')
				return(EOF);
	}
	if (str[i] != '\n') 
		outstr[j++] = str[i];
	outstr[j] = '\0';
	strcpy(str, &str[i+1]);
	return;
}
