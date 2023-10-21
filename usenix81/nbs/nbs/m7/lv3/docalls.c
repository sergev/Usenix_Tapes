/*  docalls.c
*
*      This routine processes stack and counter calls found before and after
*      the pattern. 'Rewrite' processes the calls found after the replacement
*      definition.
*
*      Parameters: pat - preprocessed pattern buffer.
*                  j - current position in the pattern.
*                  line - the input string buffer.
*                  tagary - array containing locations of tagged text.
*
*       Calls: dostck, docntr
*
*       UE's: see lower level routines.
*
* 
*                                          Programmer: G. Skillman
*/
docalls(pat,patpos,line,tagary)

char     *line, *pat;
int      *patpos, tagary[100][2];
{
         /* eoce and eose will be used to resume processing after a call. */
         int j, eoce, eose; 
         extern char       MACDELIM, STACK, CNTR;
         j = *patpos;

         for (;pat[j] != MACDELIM; ++j)
                if (pat[j] == STACK)
                {
                         dostck(pat,j,line,tagary,&eose);
                         j = eose - 1;
                }
                else if (pat[j] == CNTR)
                {
                         docntr(pat,j,&eoce);
                         j = eoce - 1;
                }
         *patpos = j + 1;
}
