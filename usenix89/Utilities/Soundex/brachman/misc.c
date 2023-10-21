/* misc.c */

/* vi: set tabstop=4 : */

#include <ctype.h>
#include <stdio.h>

#include "sp.h"

/*
 * Special character map that determines what the second character of a word
 * can be; see sp.h
 * May be expanded to contain up to 12 entries plus the terminating entry
 * Must end with an entry of two null bytes
 */
struct spchar_map spchar_map[] = {
	'\'',	QUOTE_CHAR,
	'&',	AMPER_CHAR,
	'.',	PERIOD_CHAR,
	' ',	SPACE_CHAR,
	'\0',	'\0'
};

mk_key(key, soundex, count)
key_t *key;
int soundex;
int count;
{

	key[0] = soundex & 0377;
	key[1] = ((soundex & 037400) >> 8) | ((count & 03) << 6);
	key[2] = (count & 01774) >> 2;
#ifdef DEBUG
	if (ex_soundex(key) != soundex)
		fprintf(stderr, "mk_key: soundex failed\n");
	if (ex_count(key) != count)
		fprintf(stderr, "mk_key: count failed\n");
#endif DEBUG
}

ex_soundex(key)
key_t *key;
{
	register int soundex;

	soundex = key[0] & 0377;
	soundex |= (key[1] & 077) << 8;
	return(soundex);
}

ex_count(key)
key_t *key;
{
	register int count;

	count = (key[1] & 0300) >> 6;
	count |= ((key[2] & 0377) << 2);
	return(count);
}

/*
ex_char(key)
key_t *key;
{
	int ch;

	ch = (key[1] & 076) >> 1;
	return(ch + 'a');
}
*/

/*
 * Unpack a word given the retrieved word of length len and its soundex
 * Extract the first letter from the soundex code
 * If the length is 1 and if it is marked as a single character word
 * then the marked character will be overlaid with a null
 * otherwise a null will be appended to the string
 * Adjust for upper case leading character if necessary
 * Return address of the copy
 */
char *
mk_word(p, len, s)
char *p;
int len, s;
{
	register char *q, ch;
	static char word[MAXWORDLEN + 2];

	q = word;
	if (len == 1 && (*p & SINGLE_CHAR)) {
		*(q + 1) = '\0';
		len = 0;
	}
	else
		*(q + len + 1) = '\0';

	/*
	 * Extract the first character from the soundex and
	 * adjust case
	 */
	if (*p & UPPER_CHAR)
		ch = (s & 037) + 'A';
	else
		ch = (s & 037) + 'a';
	*q++ = ch;

	if (len != 0) {				/* if more than one char adjust second char */
		ch = *p & MASK_CHAR;
		if (ch < 26)
			ch += 'a';
		else if (ch < 52)
			ch = ch - 26 + 'A';
		else if ((ch = fromspchar(ch)) == '\0') {
			fprintf(stderr, "Bogus second char in mk_word\n");
			exit(1);
		}
		*q++ = ch;
		p++;
		len--;
	}

	while (len-- > 0)
		*q++ = *p++;
	return(word);
}

/*
 * Convert the second character of a word to a special character code
 * Return null if there is no mapping
 */
tospchar(ch)
char ch;
{
	register struct spchar_map *m;

	for (m = spchar_map; m->spchar != '\0'; m++)
		if (ch == m->spchar)
			break;
	return(m->code);
}

/*
 * Convert from the special character code to the ASCII code
 * Return null if there is no mapping
 */
fromspchar(ch)
char ch;
{
	register struct spchar_map *m;

	for (m = spchar_map; m->spchar != '\0'; m++)
		if (ch == m->code)
			break;
	return(m->spchar);
}

/*
 * Compare two strings, independent of case, given their lengths
 */
/*
strnmatch(str1, len1, str2, len2)
char *str1, *str2;
int len1, len2;
{
	register char ch1, ch2;

	if (len1 != len2)
		return(0);
	while (len1-- > 0) {
		ch1 = *str1++;
		ch2 = *str2++;
		if (ch1 != ch2) {
			if (isupper(ch1))
				ch1 = tolower(ch1);
			if (isupper(ch2))
				ch2 = tolower(ch2);
			if (ch1 != ch2)
				return(0);
		}
	}
	return(1);
}
*/

/*
 * Compare two strings, independent of case
 */
strmatch(p, q)
char *p, *q;
{
	register char ch1, ch2;

	while (1) {
		ch1 = *p++;
		ch2 = *q++;
		if (ch1 == '\0' || ch2 == '\0')
			break;
		if (ch1 != ch2) {
			if (isupper(ch1))
				ch1 = tolower(ch1);
			if (isupper(ch2))
				ch2 = tolower(ch2);
			if (ch1 != ch2)
				break;
		}
	}
	return(ch1 - ch2);
}

