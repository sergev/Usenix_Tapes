/*		binsearch - do a binary search of a structure.
		84/04/07.  A. E. Klietz.
*/

binsearch(match_string, structarray, size_struct, num_structs)
/* Search an array of structures for the "match_string" and return
			-2 if match_string is not unique
			-1 if match_string was not found
			 i if ith structure matches match_string

   Each structure contains a string to be compared.
The structure array must be alphabetized.  No error message is
printed.
	The structure must be aligned at least as well as a pointer. */

char	*match_string;	/* string to match */
register char	*structarray;	/* array of structures to search */
short 	size_struct;	/* size of each structure element in bytes */
short 	num_structs;	/* number of structures in array */
{

register short	pos, diff, lower, upper, indx;

	if (match_string[0] == '\0') /* if null string */
		return(-1);  /* no match */

	lower = 0;
	upper = num_structs - 1;
	do {
		pos = (lower + upper) / 2;
		diff = strcmp(&structarray[indx = pos * size_struct], match_string);
		if (diff <= 0) /* if match_string >= &structarray[pos] */
			lower = pos + 1;
		if (diff >= 0) /* if match_string <= &structarray[pos] */
			upper = pos - 1;
	} while (lower <= upper);

	if (strcmp(&structarray[indx], match_string) == 0) 
		return(pos);

	if (!substring(match_string, &structarray[indx])) {
		++pos;
		indx = pos * size_struct;
		if (pos > num_structs - 1 || !substring(match_string, &structarray[indx]))
			return(-1);
	}
	
	if (pos < num_structs - 1)
		if (substring(match_string, &structarray[(pos + 1) * size_struct]))
			return(-2); /* not unique error. */

	return(pos);
}


substring(part, full)
/* Returns TRUE if "part" is a left anchored substring of "full". */

register char	*part, *full;
{
	register	char	ch;

	while ((ch = *part++) == *full++ && ch != '\0')
		;
	return(ch == '\0');	
}
