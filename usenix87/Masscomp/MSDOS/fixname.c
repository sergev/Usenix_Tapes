/*
 * Convert a Unix filename to a legal MSDOS name.  Returns a pointer to
 * the 'fixed' name.  Will truncate file and extension names, will
 * substitute the letter 'X' for any illegal character in the name.
 */

#include <stdio.h>
#include <ctype.h>

char *
fixname(name)
char *name;
{
	static char *dev[8] = {"CON", "AUX", "COM1", "LPT1", "PRN", "LPT2",
	"LPT3", "NUL"};
	char *s, *temp, *ext, *malloc(), *strcpy(), *strpbrk(), *strrchr();
	int i, dot, modified;
	static char ans[12];

	temp = malloc(strlen(name)+1);
	strcpy(temp, name);
					/* zap the leading path */
	if (s = strrchr(temp, '/'))
		temp = s+1;
	if (s = strrchr(temp, '\\'))
		temp = s+1;

	ext = NULL;
	dot = 0;
	for (s = temp; *s; ++s) {
		if (*s == '.' && !dot) {
			dot = 1;
			*s = NULL;
			ext = s + 1;
		}
		if (islower(*s))
			*s = toupper(*s);
	}
	if (*temp == NULL) {
		temp = "X";
		printf("'%s' Null name component, using '%s.%s'\n", name, temp, ext);
	}
	for (i=0; i<8; i++) {
		if (!strcmp(temp, dev[i])) {
			*temp = 'X';
			printf("'%s' Is a device name, using '%s.%s'\n", name, temp, ext);
		}
	}
	if (strlen(temp) > 8) {
		*(temp+8) = NULL;
		printf("'%s' Name too long, using, '%s.%s'\n", name, temp, ext);
	}
	if (strlen(ext) > 3) {
		*(ext+3) = NULL;
		printf("'%s' Extension too long, using '%s.%s'\n", name, temp, ext);
	}
	modified = 0;
	while (s = strpbrk(temp, "^+=/[]:',?*\\<>|\". ")) {
		modified++;
		*s = 'X';
	}
	while (s = strpbrk(ext, "^+=/[]:',?*\\<>|\". ")) {
		modified++;
		*s = 'X';
	}
	if (modified)
		printf("'%s' Contains illegal character\(s\), using '%s.%s'\n", name, temp, ext);
	sprintf(ans, "%-8.8s%-3.3s", temp, ext);
	return(ans);
}
