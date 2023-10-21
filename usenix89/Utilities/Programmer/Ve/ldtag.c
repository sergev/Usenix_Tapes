#include <stdio.h>

int	makeldtag(infp, outfp)
FILE	*infp ;
FILE	*outfp ;
{
	int	find = 0 ;
	char	lbuf[512] ;
	char	varname[100] ;
	char	filename[100] ;
	char	filename2[100] ;

	while (fgets(lbuf, 512, infp) != NULL)
	{
	         if (
			*lbuf == '_'  
			&& 
			sscanf(lbuf+1, "%s %s",  varname, filename) > 1
		    )  
		{
			filename[strlen(filename) - 1 ] = 'c' ;
			fprintf(outfp, "'%s', %s : undefined symbol %s\n", filename, varname, varname ) ;
			find = 1 ;
		}
	        else if ( sscanf(
				 lbuf, 
				 "ld : Symbol _%s in %s is multiply defined. First defined in %s",
				 varname,
				 filename,
				 filename2
			        ) > 2
			 )
		{
			filename[strlen(filename) - 1 ] = 'c' ;
			filename2[strlen(filename2) - 1 ] = 'c' ;
			fprintf(
				outfp, 
				"'%s', %s : multiply defined symbol %s. First defined in %s\n",
				filename,
				varname,
				varname,
				filename2
			       ) ;
			find = 1 ;
		}
				
	}
	return(find) ;
}
