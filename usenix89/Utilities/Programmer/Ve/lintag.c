#include 	<stdio.h>

char		lbuf[512] ;
char		rbuf[512] ;
char		mbuf[512] ;
char		*varname[100] ;
char		filename[100] ;
int		linenum1 ;
int		linenum2 ;
int		linenum3 ;
int		lineref = 0 ;
int		j ;
FILE		*infp ;
extern  char		*strchr() ;
extern  char		*strrchr() ;


char	*getline(linebuf)
char	*linebuf ;
{
	if (fgets(linebuf, 512, infp) != NULL)
	{
	  lineref++ ;
/*
	  fprintf(stderr, "line %d : %s\n", lineref, linebuf) ;
*/
	  linebuf[strlen(linebuf) - 1] = '\0' ;
	  return(linebuf) ;
	}
	else return(NULL) ;
}

int makelintag(infilep, outfp)
FILE	*infilep ;
FILE	*outfp ;
{

	int	lintfilepresent = 0 ;

	while (fgets(lbuf, 512, infilep) != NULL)
	{
	         if  (*lbuf == '=' )
		 {
			if ( ! strncmp(lbuf + 1, "=============", 13))
			{
				lintfilepresent = 1 ;
				rewind(infilep) ;
				break ;
			}
		 }
	}
	if (lintfilepresent == 0)
	{
		/*
		fprintf(stderr, "Lint file not present\n") ;
		/**/
		return(0) ;
	}
	else infp = infilep ;
	/*
	fprintf(stderr, "Lint file present\n") ;
	/**/
	while (getline(lbuf) != NULL)
	{
		if (*lbuf == '\0')
		   continue ;
		if (*lbuf == '=')
		{
		       if  (getline(lbuf) == NULL)
			  goto end ;
			while (*lbuf != ' ')
			{
				strcpy(rbuf, lbuf) ;
				if (getline(lbuf) == NULL)
			  		goto end ;
		
				if (match3(lbuf, mbuf, varname,  filename, &linenum1)) 
				{
				    do
				    {
					strcat(mbuf, " - ");
					strcat(mbuf, varname) ;
					fprintf(outfp, "\"%s\", line %d: %s: %s\n", filename, linenum1, rbuf, mbuf) ;
					if (getline(lbuf) == NULL)
					    goto end ;
				    }
				    while (match3(lbuf, mbuf, varname, filename, &linenum1) ) ;
				
				}
				else if (match4( lbuf, mbuf, filename, &linenum1))
				{
				    do
				    {
					fprintf(outfp, "\"%s\", line %d: %s: %s\n", filename, linenum1, rbuf, mbuf) ;
					if (getline(lbuf) == NULL)
					   goto end ;
				    }
				    while (match4( lbuf, mbuf, filename, &linenum1)) ;
				}
		    }
		   goto end ;
		}

	 loop1: strcpy(rbuf, lbuf) ;
	        if (getline(lbuf) == NULL)
		  goto end ;
		switch (*lbuf)
		{
			case '=' :  sscanf(rbuf, "%s", filename) ;
				    /*
				    fprintf(stderr, "File %s found\n", filename) ;
				    /**/
				    break ;
			case '\0' : continue ;
			default   : 
				    goto loop1 ;
		}
		if (getline(lbuf) == NULL)
		  goto end ;
                if (! match1(lbuf,& linenum1, mbuf)) 
		{
        loop2 :	   strcpy(rbuf, lbuf) ;
	           if (getline(lbuf) == NULL)
			goto end ;
		        if (*lbuf == '\0')
			     continue ;
			if ((j = match5(lbuf, &linenum1, &linenum2, &linenum3)) > 1)
			{
			    do
			    {
		   	        fprintf(outfp, "\"%s\", line %d: %s %s\n", filename, linenum1, rbuf, mbuf) ; 
		   		fprintf(outfp, "\"%s\", line %d: %s %s\n", filename, linenum2, rbuf, mbuf) ; 
			      if (j > 2) 
		   		fprintf(outfp, "\"%s\", line %d: %s %s\n", filename, linenum3, rbuf, mbuf) ; 
				if (getline(lbuf) == NULL)
				  goto end ;
			    }
			    while ((j = match5(lbuf, &linenum1, &linenum2, &linenum3)) > 1 ) ;
		        }
			else if (match2(lbuf, &linenum1, mbuf) )
			{
			    do
			    {
		   		fprintf(outfp, "\"%s\", line %d: %s %s\n", filename, linenum1, rbuf, mbuf) ; 
				if (getline(lbuf) == NULL)
				    goto end ;
			    }
			    while (match2(lbuf, &linenum1, mbuf) ) ;
			}
		        goto loop2 ;
		
		}
		else
		do
		{
			
		   fprintf(outfp, "\"%s\", line %d: %s\n", filename, linenum1, mbuf) ; 
		   if (getline(lbuf) == NULL)
			goto end ;
		}
		while (match1( lbuf, &linenum1, mbuf) ) ;
		goto loop2 ;
	}
end :   return(1) ;
}
		   
		   
		
int	match1(linebuf, linenum, msg)
/*
 *	match line with the format
 *		(line)  message
 */
char	*linebuf ;
int	*linenum ;
char	*msg ;
{
	return(sscanf(linebuf, "(%d) %[^$]", linenum, msg) > 1) ;
}
		
				   
int	match2(linebuf, linenum, msg)
/*
 *	match line with the format
 *		4 spaces (line)  message
 */
char	*linebuf ;
int	*linenum ;
char	*msg ;
{
	int 	i ;

	for (i = 0 ; linebuf[i] != '\0' && linebuf[i]  == ' ' ; i++);

	if (i != 4)
	   return(0) ;
	else
	{
	   *msg = '\0' ;
	   return(sscanf(linebuf, " (%d) %[^$]", linenum, msg) > 0) ;
	}
}
		
				   
		
			
int   match3(linebuf, msg, vname, fname, linenum)
/*
 *	match line with the format
 *		message	 file1(line1) :: file2(line2)
 */
char	*linebuf ;
char	*msg ;
char	*fname ;
char	*vname ;
int	*linenum ;
{
	int 	i ;

	for (i = 0 ; linebuf[i] != '\0' && linebuf[i]  == ' ' ; i++);

	if (i != 4)
	   return(0) ;
	if (sscanf(linebuf, " %[^\t]\t%s :: %[^\(](%d)", msg, vname, fname, linenum) > 3) 
	{
	
		if (fname[strlen(fname) - 2] != '.')
		{
			char	filenamebuf[100]  ;
			int	line ;

			sscanf(vname, "%[^\(](%d)", filenamebuf, &line) ;
			sprintf(vname, "%s(%d)", fname, *linenum) ;
			strcpy(fname, filenamebuf) ;
			*linenum = line ;
		}
		return(1) ;
	}
	else
		return(0) ;
			
			
}


int   match4(linebuf, msg, fname, linenum)
/*
 *	match line with the format: 
 *		message 	file(line)
 */
char	*linebuf ;
char	*msg ;
char	*fname ;
int	*linenum ;
{
	int 	i ;

	for (i = 0 ; linebuf[i] != '\0' && linebuf[i]  == ' ' ; i++);

	if (i != 4)
	   return(0) ;
	
	if (sscanf(linebuf, " %s %[^\(](%d)", msg, fname, linenum) > 2)
	{
		i = strlen(fname) - 1  ;
		if (fname[i] == '?')
			fname[i] = '\0' ; 
		return(1) ;
	}
	else return(0) ;
}


int	match5(linebuf, lnum1, lnum2, lnum3)
/*
 *	match line which contains only numbers 
 */
char	*linebuf ;
int	*lnum1 ;
int	*lnum2 ;
int	*lnum3 ;
{
	int 	i ;

	for (i = 0 ; linebuf[i] != '\0' && linebuf[i]  == ' ' ; i++);

	if (i != 4)
	   return(0) ;
	
	return( sscanf(linebuf, " (%d)\t(%d)\t(%d)", lnum1, lnum2, lnum3)) ;
}
