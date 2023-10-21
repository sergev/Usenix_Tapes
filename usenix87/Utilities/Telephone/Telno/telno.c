/*
 * telno - permute telephone number letters
 *
 * Author: David M. Ihnat
 *
 * This is a trivial program to accept a typical United States-type,
 * 7-digit telephone number, and provide all possible unique permutations of
 * the letters found on the standard dial positions.  Although this is
 * a quick-and-dirty program, I did try to design it in such a manner
 * that different characters and telephone number lengths should be
 * easy to do.
 *
 * I freely grant anyone the right to do anything they wish with this
 * thing, as long as it's (as usual) not for profit.  I apologize only
 * casually for the code, as I was more than 3 sheets to the wind when
 * I slapped it out...logic and Jack Daniel's are strange glass-fellows...
 */
#include <stdio.h>

#define	VERSION		"1.0"	/* As if there's going to be a 2.0...  */

#define	TEL_LEN		7	/* Length of a telephone number in digits */
#define	MAX_COMB	2188	/* Maximum number of combinations */

#define	NULL_CHAR	'@'	/* Indicates not to do this character */

/*
 * This must equal the length of the entries in telno_def[]. In this
 * manner, you can add arbitrary combinations of letters, digits, etc.
 * to satisfy different character sets; or, for instance, you may wish
 * to allow the actual number in the string, as well.  If you *do*
 * change SUBSTR_LEN, don't forget to change MAX_COMB.
 */
#define	SUBSTR_LEN	3
char *telno_def[10] = {
/*	  0      1      2      3      4      5      6      7      8      9  */
	"0@@", "1@@", "abc", "def", "ghi", "jkl", "mno", "prs", "tuv", "wxy"
	};

/* The payoff; 3 possible/no, plus null string*/
char listary[MAX_COMB][TEL_LEN+1];

list_idx = 0;

extern int errno;

main(argc,argv)
int argc;
char *argv[];
{
	register char *telptr;
	register short index;
	short tel_idx[TEL_LEN];

	argc--,argv++;		/* Most micros don't support argv[0] */
	if(!argc)
	{
		fprintf(stderr,"Usage: xxx[-]yyyy\n",*argv);
		exit(1);
	}

	telptr = *argv;

	/* If there is a dash in the telephone string, cut it out. */
	if(telptr[3] == '-')
	{
		register char *p1,*p2;
		for(p1 = &telptr[3], p2 = &telptr[4];*p1 != '\0';)
			*p1++ = *p2++;
	}

	fprintf(stderr,"Processing for %s....",telptr);

	/*
	 * Now to break the telephone number apart into an array of
	 * indices
	 */
	for(index=0;index < TEL_LEN;index++)
		tel_idx[index] = (*telptr++) - '0';

	/* Now build the permuations */
	proc_num(tel_idx);

	/* Null-terminate the array */
	listary[list_idx][0] = '\0';

	/* Now sort the array */
	sort_num();

	/* Finally, print it out. */
	fprintf(stderr,"done.\n\n");
	dump_num();
}

proc_num(prim_idx)
short prim_idx[];
{
	register int index;
	short sec_idx[TEL_LEN];

	/*
	 * Permute the given TEL_LEN-digit array for all possible
	 * combinations of related key-top digits.  This could
	 * be done either iteratively or recursively; to keep
	 * stack usage down, I've selected an iterative approach.
	 *
	 * If you extend or shrink the length of a telepone number,
	 * change the number of for loops here.
	 */

	for(sec_idx[0] = 0;sec_idx[0] < SUBSTR_LEN;++sec_idx[0])
	 for(sec_idx[1] = 0;sec_idx[1] < SUBSTR_LEN;++sec_idx[1])
	  for(sec_idx[2] = 0;sec_idx[2] < SUBSTR_LEN;++sec_idx[2])
	   for(sec_idx[3] = 0;sec_idx[3] < SUBSTR_LEN;++sec_idx[3])
	    for(sec_idx[4] = 0;sec_idx[4] < SUBSTR_LEN;++sec_idx[4])
	     for(sec_idx[5] = 0;sec_idx[5] < SUBSTR_LEN;++sec_idx[5])
	      for(sec_idx[6] = 0;sec_idx[6] < SUBSTR_LEN;++sec_idx[6])
	      {
		bld_str(prim_idx,sec_idx);
	      }
}

bld_str(prim_idx,sec_idx)
short prim_idx[],sec_idx[];
{
	register int idx;
	register char c;
	for(idx=0; idx < TEL_LEN; idx++)
	{
		c = telno_def[prim_idx[idx]][sec_idx[idx]];

		if(c == NULL_CHAR)
			return;

		listary[list_idx][idx] = c;
	}
	listary[list_idx][8] = '\0';
	list_idx++;
}

sort_num()
{
	int strcmp();

	/*
	 * Cheat--use the library sort.  If your library doesn't have
	 * one, well...have fun.  It's not too hard...
	 */
	qsort(listary,(list_idx-1),8,strcmp);
}

dump_num()
{
	/*
	 * Dump the permutations, 10 per line.  (Just fits on 80-col
	 * printout)
	 */
	register int idx1,idx2;
	for(idx1 = 0;*listary[idx1] != '\0';)
	{
		for(idx2=0;(*listary[idx1] != '\0') && (idx2 < 10); idx1++,idx2++)
		{
			fputs(listary[idx1],stdout);
			fputc(' ',stdout);
		}
		fputc('\n',stdout);
	}

	fputc('\n',stdout);

}
