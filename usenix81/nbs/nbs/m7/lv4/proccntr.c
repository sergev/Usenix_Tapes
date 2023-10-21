/*  proccntr.c
*
*	This routine is extremely similar to "procstck". It is also called
*	by makpat and maksub and is used to preprocess counter calls.
*	Information stored onto buf includes the counter call character,
*	CNTR, the type of counter call (see user documentation), whether
*	the counter is to be incremented before or after the call or not
*	at all. Also, the counter's name is stored in all three types of
*	calls. If the counter or its increment is assigned a value, the
*	name of the counter/increment and the value must be stored. 
*	Finally, the 'end of counter call' character, "CNTREND" is stored.
*
*	Arguments: 
*	   arg - the input macro string; argpos - where the counter call was.
*	   buf - the preprocessed macro buffer; bufpos - where we are writing.
*
*	Calls: fatal_error, adset, atoi
*
*	UE'S - The only real error is if an unrecognizeable character occurs
*	       in the counter call pattern or if the counter call has two of
* 	       the following: "," or "=".  Note there is no error detection
*	       for two occurences of a "+" or "-" or extraneous letter or
*	       digits.
*
*				Programmers: Tony Marriott & G. Skillman
*/
/* The following will define the maximum size of the pattern buffer: */
# include	"globdefin.h"

proccntr(arg,argpos,buf,bufpos)
char	*buf,*arg;
int	*bufpos,*argpos;
{
	int	type,i,typepos,j,before,stckid,intnum,sizestack;
	char	space, charnum[2];
	extern int	YES,NO,CNTR,CNTREND;

	space = ' ';
	type = 1;
	i = *argpos;
	j = *bufpos;

	buf[j] = CNTR;
	/* typepos will tell where to store the type code when we find out */
	/* what type of counter call this is, 1, 2 or 3.     */
	typepos = j + 1;
	sizestack = j + 2;
	j = j + 3;
	i = i + 2;
	/* "before" flags whether we increment/decrement before or after. */
	before = YES;

	while (arg[i] != CNTREND)
	{
		if (arg[i] >= 'a' && arg[i] <= 'z')
		{
			before = NO;
			addset(arg[i],buf,&j,MAXBUF);
			++i;
		}
                     /* If a digit is encountered, it is supposed to be either
                        an assignment to the counter or its increment. Here, 
			"atoi" gets the STRING (!) of digits and converts them
			to a double byte integer which is put on buf. */
		else if (arg[i] >= '0' && arg[i] <= '9')
		{
			intnum = atoi(&arg[i]);
			itoc(intnum,charnum);
			addset(charnum[0],buf,&j,MAXBUF);
			addset(charnum[1],buf,&j,MAXBUF);
			++i;
			/* "atoi" got the string but didn't move the pointer
			   past that string so we must do it ourselves.  */
			while (arg[i] >= '0' && arg[i] <= '9')
				++i;
		}
		else if (arg[i] == '=') /* set counter to n. */
		{
			++i;
			if (type == 1)
			   type = 2;
			else
			   fatal_error("PROCCNTR: illegal use of '='");
		}
		else if (arg[i] == ',')  /* set increment to n. */
		{
			++i;
			if (type == 1)
			   type = 3;
			else
			   fatal_error("PROCCNTR: illegal use of ','");
		}
		else if (arg[i] == '+' || arg[i] == '-')
		{
			addset(arg[i],buf,&j,MAXBUF);
			/* -2 is the code for before, -3 after. */
			if (before == YES)
				addset(-2,buf,&j,MAXBUF);
			else
				addset(-3,buf,&j,MAXBUF);
			++i;
		}
		else
			if (arg[i] == space)
				++i;
		else
		{
			printf("ERRONEOUS CHAR WAS %c\n",arg[i]);
			fatal_error("PROCCNTR: UNRECOGNIZEABLE CHAR");
		}
	}
	buf[typepos] = type;
	buf[sizestack] = j - *bufpos;
	addset(CNTREND,buf,&j,MAXBUF);
	*argpos = i;
	*bufpos = j;
}
