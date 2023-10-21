# include	"dec.h"

/*
** Header Label Table
*/

struct labid
{
	char	*name;
	int	code;
};

struct labid	Labelids[] =
{
	"VOL",		L_VOL1,
	"HDR",		L_HDR1,
	"EOF",		L_EOF1,
	"EOV",		L_EOV1,
	"UTL",		L_UTL,
	"UHL",		L_UHL,
	0                
};

check_id(s)
/*
**  Searches Labelids for name s and
**  returns numeric code or 0 if not
**  found.
*/
register char	*s;
{
	register struct labid	*cw;

	for (cw = Labelids; cw->name; cw++)
	{
		if (!strncmp(s, cw->name, 3))
		{
			if (cw->code == L_HDR1)
				if (*(s+3) == '2')
					return(L_HDR2);
			if (cw->code == L_EOF1)
				if (*(s+3) == '2')
					return(L_EOF2);
			return(cw->code);
		}
	}

	return(0);
}



user_exit()
{
}



itoa(i, a, n)
/*
**  ITOA -- integer to ascii conversion
**	    converts the integer i to  
**	    an ascii character string 
**	    n digits in length. a is the
**	    position of the rightmost digit.    
*/
register int	i;
register char	*a;
register int	n;
{
	register int	k;
	register char  *j;

	j = a;
	for (k=0; k<n; k++)	/* zeros out the field */
		*j-- = '0';
	j = a;
	k = 0;
	do
	{
		*j-- = i%10 + '0';
		i /= 10;
		k++;
	} while (i > 0 && k < n);
}



eread(s)
char	*s;
/*
** The sizeof(string to be read) < length(s).
** s will be a null-terminated string.
*/
{
	int	c;
	
	while ((c=getchar()) != EOF)
	{
		if (c == '\n')
		{
			*s = 0;
			return(0);
		}
		*s++ = c;
	}
	return(1);
}
