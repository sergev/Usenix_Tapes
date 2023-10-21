/*		Copyright (c) 1982 Gary Perlman
This software may be copied freely provided:  (1) it is not used for
personal or material gain, and (2) this notice accompanies each copy.

Disclaimer:  No gauarantees of performance accompany this software,
nor is any responsbility assumed on the part of the author.  All the
software has been tested extensively and every effort has been made to
insure its reliability.*/

/* returns 1 for an int, 2 for a real, 0 for non-numbers */
#include <stdio.h>
#include <ctype.h>
number (string) char *string;
	{
	char	answer = 1;
	while (isspace (*string)) string++;
	if (!*string) return (0);
	if (*string == '-') string++; /* optional plus not allowed by atof */
	while (isdigit (*string)) string++;
	if (*string == '.')
		{
		answer = 2;
		string++;
		}
	while (isdigit (*string)) string++;
	if (*string == 'E' || *string == 'e')
		{
		answer = 2;
		string++;
		if (*string == '+' || *string == '-') string++;
		while (isdigit (*string)) string++;
		}
	while (isspace (*string)) string++;
	return (*string == NULL ? answer : 0);
	}
