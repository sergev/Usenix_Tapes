
#include <stdio.h>

int	makecctag(infp, outfp)
FILE	*infp ;
FILE	*outfp ;
{
	int	find = 0 ;
	char	lbuf[512] ;

	while (fgets(lbuf, 512, infp) != NULL)
	{
	         if (*lbuf == '"' )
		 {
			fputs(lbuf, outfp) ;
			find = 1 ;
		 }
	}
	return(find) ;
}
