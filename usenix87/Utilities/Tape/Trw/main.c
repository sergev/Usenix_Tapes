#include	"dec.h"

main(argc, argv)
int	argc;
char	**argv;
{
	         int	c, i;   
	         char   **str;
	         char   option;		        /* read, write, or dir      */
		 char	sv[80];			/* numeric char string      */
		 int	n1;			/* M of files=M-N or M      */
		 int 	n2;			/* N of files=M-N           */
		 int	fstart;			/* [] of 1st file to write  */
		 int	fend;			/* [] of last file to write */
		 int    setflag();
		 int    match();
	extern int	readtp();
	extern int	dsrep();
	extern int	err();
	extern int	writetp();

	fstart = fend = 0;
	keep = 1;
	if (argc < 2)
		err(CMDERR);
	option = *argv[1];
	if (option != 'r' && option != 'w' && option != 'd')
		err(CMDERR);
	for (c = 2; c < argc; ++c)
	{
		str = argv + c;
		if (match("blklen=", str))
		{
			setflag(&wrtflag, 2);
			blklen = atoi(*str);
			if (blklen > MAXBUF)
				err(MAXERR);
			if (fend < fstart)
				fend = c;	
			continue;
		}
		if (match("dev=", str))
		{
			setflag(&flags, 0);
			tpdr = *str;
			if (match(DEVRMT, str))
			{		
				devflag = 1; 
				strcat(strcat(ntpdr, DEVNRMT), *str);
			}
			if (fend < fstart)
				fend = c;
			continue;
		}
		if (match("files=", str))
		{
			while (**str)
			{
				if (**str == ',')
					(*str)++;
				else if (**str == '-')
				{
					(*str)++;
					for (sv[0]='\0';**str != ',' && 
						        **str != '\0'; (*str)++)
						strncat(sv, *str, 1);
					sscanf(sv, "%d", &n2);  
					for (i = ++n1; i <= n2; i++)
						setflag(&files, i-1);
				}
				else      
				{
					for (sv[0]='\0';**str != ',' &&
						        **str != '-' &&
					                **str != '\0';(*str)++)
					       strncat(sv, *str, 1);
					sscanf(sv, "%d", &n1);
					setflag(&files, n1-1);
				}
			}
			if (fend < fstart) 
				fend = c;
			continue;
		}
		if (match("fn=", str)) 
		{
			setflag(&wrtflag, 3);
			*(argv+c) = *str;  
			fstart = c;	
			if (**str == '\0')
				fstart++;      	
			continue;
		}
		if (match("keep=", str))
		{
			if (match("n", str) || match("N", str))
			   	keep = 0;
			if (fend < fstart)
			 	fend = c;
			continue;
		}
		if (match("pad=", str))
		{
			if (match("y", str) || match("Y", str))
				p = 1; 
			if (fend < fstart)
				fend = c;
			continue;
		}
		if (match("recfm=",str))
		{
			if (fend < fstart) 
				fend = c;
			if (strcmp(*str, "F") == 0)
			{ 
				setflag(&wrtflag, 0);
				recode = F;
			}
			else if (strcmp(*str, "FB") == 0)
			{	
				setflag(&wrtflag, 0);	
				recode = FB;
			}	
			else if (strcmp(*str, "V") == 0)
			{
				setflag(&wrtflag, 0);	
				recode = V;
			}	
			else if (strcmp(*str, "VB") == 0)
			{
				setflag(&wrtflag, 0);
				recode = VB;
			}
			else
				err(FMTERR);
			continue;
		}
		if (match("reclen=", str))
		{
			setflag(&wrtflag, 1);
			reclen = atoi(*str);
		 	if (fend < fstart)
				fend = c;		
			continue;
		}
		if (match("tf=ansi", str))
		{
			setflag(&flags, 1);
			ansi = 1;
			if (fend < fstart)
				fend = c;
			continue;
		}
		if (match("tf=stdl", str))
		{
			setflag(&flags, 1);
			stdl = 1;
			if (fend < fstart)
				fend = c;
			continue;
		}
		if (match("vol=", str))
		{
			setflag(&flags, 2);
			strncpy(vid, *str, MIN(strlen(*str), 6));
			if (strlen(vid) < 6)           
				strncat(vid, "      ", 6-strlen(vid));
			if (fend < fstart)
				fend = c;
			continue;
		}	
	}
	
	if (fend)
		fend--;        
	else
		fend = argc - 1;

	switch (wrtflag)
	{
		case 15:
			complete = 1;
			if (recode == F && reclen != blklen)
				err(BLKERR);
			else if (recode == FB && blklen % reclen != 0)
				err(BLKERR);
			else if (recode == V && blklen != reclen)
				err(BLKERR);
			else if (recode == VB && blklen % reclen != 4)          
				err(BLKERR);
			else if (ansi && (recode == V || recode == VB))
				err(RFMERR);
			break;

		case  9:
			if (recode == F || recode == FB)
			{
				complete = 1;
				recode = F;
				reclen = 80;
				blklen = 80;
			}
			else
				err(RECERR);
			break;

	}

	switch (option)
	{
		case 'r':     
			if (flags == 7)
			{
				if ((tfd = open(tpdr, 0)) >= 0)
				{
					if (!files)
						files = ~files;
					readtp();	
				}
				else
					err(OPNERR);
			}
			else
				err(FLGERR);
			break;

		case 'w':
			if (flags == 7)
			{
				if ((wrtflag & 8) == 8)
					if (devflag)
						if ((tfd = open(ntpdr, 1)) >= 0)
							writetp(argv,fstart,fend);	
						else
							err(OPNERR);
					else
						if ((tfd = open(tpdr, 1)) >= 0)
							writetp(argv,fstart,fend);
			 			else
							err(OPNERR);
				else
					err(FNMERR);
			}
			else
				err(FLGERR);
			break;

		case 'd':
			if (flags == 7)
			{
				if ((tfd = open(tpdr, 0)) >= 0)   
					dsrep();
				else
					err(OPNERR);	
			}
			else
				err(FLGERR);
			break;
	}
	if (close(tfd) != 0)
		err(CLOERR);
}



match(s, cs)
/*
**  match returns a 1 if the strings, pointed to by s and cs,
**  are identical and advances the cs pointer the strlen(s),
**  otherwise it returns a 0.
*/

register char	*s;
register char	**cs;
{
	register char	*ls;

	ls = *cs;
	for (; **cs == *s; ++(*cs))
	{
		if (*++s == '\0')
		{
			++*cs;
			return(1);
		}
	}
	*cs = ls;
	return(0);
}



setflag(flg, n)
/*
** Sets the (n + 1)st bit in flg to 1.
*/
register int	*flg;
register int	n;
{
	register int	f;

	if (n < MAXFILES)
	{
		f = 1;
		f <<= n;
		*flg |= f;
	}
	    
}
