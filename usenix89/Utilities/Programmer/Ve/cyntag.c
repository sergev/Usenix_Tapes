
#include <stdio.h>

static  int	line ;
static	FILE	*outfp ;
static	FILE	*infp ;
static	char	mesg[254] ;
static	char	filename[100] ;

int	makecyntag(infilep, outfilep)
FILE	*infilep ;
FILE	*outfilep ;
{
	int	find = 0 ;
	char	lbuf[512] ;

	infp = infilep ;
	outfp = outfilep ;
	while (fgets(lbuf, 512, infp) != NULL)
	{
		switch ( CynMatch1(lbuf) )
		{
		   case  1 : 
			find = 1 ;
			break ;
		   case -1 : 
		 	return(0) ;
		   case  0 : 
			switch (CynMatch2(lbuf) )
			{
			   case  1 : 
				find = 1 ;
				break ;
			   case -1 : 
				return(0) ;
			   case  0 : 
				switch (CynMatch3(lbuf) )
				{
				   case  1 : 
					find = 1 ;
					break ;
				   case -1 : 
					return(0) ;
				   case  0 : 
					switch (CynMatch4(lbuf) )
					{
					   case  1 : 
						find = 1 ;
						break ;
					   case -1 : 
						return(0) ;
					}
			        }
		        }
	         }
	} 
	return(find) ;
}


int	CynMatch1(lbuf)
/*
 *	Match ordinary error line with the format
 *	file: line: error
 */
char	*lbuf ;
{
	if ( sscanf(lbuf, "%[^: \t]: %d: %[^\n]", filename, &line, mesg) > 2)  
	{
		fprintf(outfp, "\"%s\", line %d: %s\n", filename, line, mesg ) ;
		return(1) ;
	}
	return(0) ;
}


int	CynMatch2(lbuf)
/*
 *	match line with the format 
 *	variable "multiply declared":
 */
char	*lbuf ;
{
	char	c ;
	char	varname[100] ;
	char	linebuf[512] ;

	if (
		sscanf(lbuf, "%s multiply declared%c", varname, &c) > 1
		&&
		c == ':'
	   )
	{
	    if (
		(getc(infp) == '\t')
		&&
		fgets(linebuf, 512, infp) != NULL
	       )
	    {
		if (
		    sscanf(linebuf, "%[^:]: %d %[^\n]", filename, &line, mesg)  > 2 
		    &&
		    strncmp(mesg, "implicitly as", 13) == 0
		   )
		{
			if (
			    getc(infp) != '\t' 
			    || 
			    fgets(linebuf, 512, infp) == NULL)
			    return(-1) ;
		}
		else 
		{
			if (fscanf(infp, " %[^:]: %d %[^\n]\n", filename, &line, mesg)  <  3)  
			   return(-1) ;
		}
		fprintf(
			outfp,
			"\"%s\", line %d: %s %s : %s", 
			filename, 
			line, 
			varname,
			mesg, 
			linebuf 
		       ) ;
		return(1) ;
	    }
	    else return(-1) ;
	}
	else return(0) ;
}

int	CynMatch3(lbuf)
/*
 *	match line with the format 
 *	variable "multiply defined":
 */
char	*lbuf ;
{
	char	word[512] ;
	char	linebuf[512] ;
	char	c ;

	if (
		sscanf(lbuf, "%*s multiply defined%c",  &c) > 0
		&&
		c == ':'
	   )
	{
	    
		while ((c = getc(infp)) == '\t')
		{
			if (
			    fgets(linebuf, 512, infp) != NULL
			    &&
			    sscanf(linebuf, "%[^:]: %d", filename, &line) > 1
			   )
			{	
				fprintf(
					outfp,
					"\"%s\", line %d: %s", 
					filename, 
					line, 
					lbuf 
				       ) ;
		        }
			else return(-1) ;
		}
		ungetc(c, infp) ;
		return(1) ;
	}
	else return(0) ;

}

int	CynMatch4(lbuf)
/*
 *	match line with the format 
 *	"function" function: library 
 *      or
 *      "function" function: file: line
 */
char	*lbuf ;
{
	char	word[512] ;
	char	linebuf[512] ;
	char	c ;

	if (
		sscanf(lbuf, "function %*[^:]: %[^\n]",  word) > 0
	   )
	{
		lbuf[strlen(lbuf) - 1] = '\0' ;
		while ((c = getc(infp)) == '\t')
		{
			if (
			    fgets(linebuf, 512, infp) != NULL
			    &&
			    sscanf(linebuf, "%[^:]: %d, %[^\n]", filename, &line, mesg) > 2
			   )
			{	
				fprintf(
					outfp,
					"\"%s\", line %d: %s %s\n", 
					filename, 
				        line,
					lbuf,
				        mesg 
				       ) ;
		        }
			else return(-1) ;
		}
		ungetc(c, infp) ;
		return(1) ;
	}
	else return(0) ;
}



