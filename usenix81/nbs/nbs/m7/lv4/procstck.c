/*  procstck.c
*
*	"Makpat" or "maksub" has encountered the stack character in the input
*	macro and have called "procstck" to preprocess the call and return the
*	next position on the input and buffer strings just beyond the stack
*	call.  This routine records on the buffer the stack call character, 
*	the type of stack call, whether the stack's pointer is to be incre-
*	mented before or after the stack call, or not at all.  The name of the
*	stack and any references to the pointers as to the type of the stack 
*	call is also stored on the buffer.
*
*	Parameters:
*	     arg - the input macro;  argpos - position of stack call on arg.
*	     buf - buffer for preprocessed macro;  bufpos - current pos. on buf.
*
*	Calls: addset, fatal_error, atoi, esc
*
*	UE's:  Errors include two occurences of "=", "," or ":" and
*	       unrecognizeable characters. Note that no error checking
*	       is done for extraneous lower case letters, digits
*	       or an extraneous "+" or "-".
*
*				Programmer: Tony Marriott
*/
/* The following defines the maximum size of the pattern buffer and the 
   maximum # of macros with the "<" feature. */
# include	"globdefin.h"
procstck(arg,argpos,buf,bufpos)
char	*buf,*arg;
int	*bufpos,*argpos;
{
	int	type,i,typepos,j,before,stckid,sizepos,strpos,sizestack;
	char	space;
	extern int	YES,NO,STACK,STCKEND,ESCAPE;

	space = ' ';
	type = 1;
	i = *argpos;
	j = *bufpos;

	buf[j] = STACK;
	/* typepos marks where on buf we will store the type of stack call. */
	typepos = j + 1;
	sizestack = j + 2;
	j = j + 3;
	i = i + 2;
	/* before - flag which says if incrementation done before or after. */
	before = YES;
	while (arg[i] != STCKEND)  /* while arg[i] != ')' */
	{
		if (arg[i] >= 'a' && arg[i] <= 'z')
		{
			before = NO;
			addset(arg[i],buf,&j,MAXBUF);
			++i;
		}
                     /* else if we've found an integer, then it's either a stack
			pointers value in a type 3 stack call, or it's a tag #
			in a type 2 stack call. Convert it to a single byte and
			store it on buf.  Then move i past the number.  */

		else if (arg[i] >= '0' && arg[i] <= '9')
		{
			/* Remember! atoi gets a string of digits starting at 
			  arg[i] and converts them into a single byte integer */
			addset(atoi(&arg[i]),buf,&j,MAXBUF);
			++i;
			while (arg[i] >= '0' && arg[i] <= '9')
				++i;
		}
		else if (arg[i] == ',')
		{
			++i;
			if (type == 1)
			   type = 2;
			else
                           fatal_error("PROCSTCK: ILLEGAL USE OF ','");
		}
		else if (arg[i] == '=')
		{
			++i;
			if (type == 1)
			   type = 3;
			else
			   fatal_error("PROCSTCK: ILLEGAL USE OF '='");
		}
		else if (arg[i] == ':')
		{
			if (type == 1)
				type = 4;
			else
				fatal_error("PROCSTCK: ILLEGAL USE OF ':'");
			addset(arg[i],buf,&j,MAXBUF);
			sizepos = j;
			++j;
			strpos = i;
			++i;
			while (arg[i] != ':' || arg[i - 1] == ESCAPE)
			{
				addset(esc(arg,&i),buf,&j,MAXBUF);
				++i;
			}
			addset(arg[i],buf,&j,MAXBUF);
			buf[sizepos] = i - strpos - 1;
			++i;
		}
		else if (arg[i] == '+' || arg[i] == '-')
		{
			addset(arg[i],buf,&j,MAXBUF);
			/* Now store whether incrementing is to be done 
			   before (-2) or afterwards (-3).  */
			if (before == YES)
				addset(-2,buf,&j,MAXBUF);
			else
				addset(-3,buf,&j,MAXBUF);
			++i;
		}
		else   /* Skip over spaces.   */
			if (arg[i] == space)
			   ++i;
			else
			{
			   printf("ERRONEOUS CHAR WAS %c\n",arg[i]);
			   fatal_error("PROCSTCK: ILLEGAL CHARACTER. ");
			}
	}
	buf[typepos] = type;
	buf[sizestack] = j - *bufpos;
	addset(STCKEND,buf,&j,MAXBUF);
	*argpos = i;
	*bufpos = j;
}
