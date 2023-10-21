
 ...
            if (isupper(field2[0]))
               field2[0] -= 'A' - 'a';
            }
/*
         res = strcmp(field1, field2);
 */
/*
 *	Use a special comparison that ignores diacriticals.
 */
         res = tstrcmp(field1, field2);
         }
      if (neg)
 ...

/*
 *	Compare two strings, ignoring diacritical marks.
 */
int
tstrcmp(str1, str2)
char *str1, *str2;
{
	char hold1[101], hold2[101];
	deldiacr(hold1, str1);
	deldiacr(hold2, str2);
	return strcmp(hold1, hold2);
}

/*
 *	deldiacr - delete diacritical marks and other ditroff escapes.
 */

deldiacr(snew, sold)
char *snew, *sold;
{
	char delim='\0';
	while (*sold) {
		if ('\\'==*sold) {
/*
 *	Transparent newline - abort.
 */
			if (!sold[1]) break;
/*
 *	Matching delimiter on overstrike, but bash-escaped - pass it.
 */
			else if (delim==sold[1]) {
				*snew++ = delim;
				sold+=2;
				continue;
			}
			else switch(*++sold) {
/*
 *	Bash-bash.
 */
			case '\\':
			case 'e':
				*snew++ = '\\';
				sold++;
				break;
			case 'o':
/*
 *	Premature end of line?
 */
				if (!(delim = *++sold)) break;
				sold++;
				break;
/*
 *	Special character.
 */
			case '(':
/*
 *	Within overstrike - ignore it.
 */
				if (delim) {
					if (*++sold && *++sold) sold++;
				}
/*
 *	Otherwise just let it be.
 */
				else *snew++ = *sold++;
				break;
			}
		}
		else if (delim==*sold) {
/*
 *	Matching delimiter on overstrike.
 */
			delim=0;
			sold++;
		}
/*
 *	If within overstrike, accept letters only.
 */
		else {
			if (!delim || isalpha(*sold)) *snew++ = *sold;
			sold++;
		}
	}
	*snew = '\0';
}

/* makecites - make standard citation strings, using citetemplate currently in effect */
   makecites()
 ...
      for (i = 0; field[i] = field[i+n]; i++)
         ;
      }
/*
 *	Use deldiacr to kluge the length. --Col. S., 1987
 */
   else if (isdigit(*ptr)) {
	char hold[101];			/* For checking with deldiacr */
      for (; isdigit(*ptr); ptr++)
         n = 10 * n + (*ptr - '0');
      if (n > len)
         n = len;
/*
      field[n] = 0;
 */
	while (deldiacr(hold,field),strlen(hold)>n) field[--len]='\0';
/*
 *	How was that for a one-liner? --Col. S.
 */
      }

   if (*ptr == 'u') {
 ...
