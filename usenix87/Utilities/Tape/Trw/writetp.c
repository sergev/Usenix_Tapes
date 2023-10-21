# include	"dec.h"
# include	<ctype.h>

char	fmtab[] = {'F',		/* recfm based on defines for recode  */
		   'F',
		   'V',
		   'V' };

char	blktab[] = {' ',	/* blocking attribute based on recode */
		    'B',
		    ' ',
		    'B' };

writetp(argv,fstart,fend)
/*
**  Writes user-specified UNIX files to tape
**  according to user-specified format.
**  This function is entered when given 'w' option,
**  required parameters (dev=, vol=, fn=, tf=ansi or tf=stdl),
**  and successful open of tpdr or ntpdr.
*/
char	**argv;			/* command line argument list         */
int	fstart;			/* index in argv of first file name   */
int	fend;			/* index in argv of last file name    */
{
	int	valid;		/* valid input flag                   */
	char	s[256];		/* input string			      */
	int	i, j, nb;
	extern int	itoa();
	extern int	atoibm();
	extern int	err();
	extern int	eread();
	extern int	readrec();
        
	process_wvol();
	for (j=1; fstart<=fend && j<MAXFILES; fstart++, j++)
	{
		if (!complete)
		{
			valid = 0;
			printf("\nEnter input parameters for %s\n",argv[fstart]);
			printf("\nPlease enter recfm:  ");
			while (!valid)
			{
			eread(s);
			if (strncmp(s,"F",2) == 0 || strncmp(s,"F ",2) == 0)
			{
				recode = F;
				valid = 1;
			}
			else if ((strncmp(s,"V",2) == 0 || strncmp(s,"V ",2) == 0) && !ansi)
			{	
				recode = V;
				valid = 1;
			}
			else if (strncmp(s,"FB",2) == 0)
			{
				recode = FB;
				valid = 1;
			}
			else if ((strncmp(s, "VB",2) == 0) && !ansi)
			{
				recode = VB;
				valid = 1;
			}
			else
				printf("\nInvalid recfm - Please reenter:  ");
			}
			valid = 0;
			printf("\nPlease enter reclen:  ");
			while (!valid)
			{
			eread(s);
			reclen = atoi(s);
			if (reclen < MAXBUF)
			 	valid = 1;
			else
				printf("\nInvalid reclen - Please reenter:  ");
			}
			if (recode == F || recode == V)
				blklen = reclen;
			else               
			{
				valid = 0;
				printf("\nPlease enter blklen:  ");
				while (!valid)
				{
				eread(s);
				blklen = atoi(s);
				if (blklen < MAXBUF)
				{	
					if (recode == FB && blklen%reclen == 0)
						valid = 1;
					else if (blklen%reclen == 4)
						valid = 1; 
				}
				if (!valid)
					printf("\nInvalid blklen - Please reenter: ");
				}
			}
			
		}

		printf("\n");
		strcpy(ufname, argv[fstart]);
		if (keep == 0)
		{
			printf("Current File Name:  ");
			printf("%s", argv[fstart]);
			printf("\n\nNew File Name:  ");
			eread(s);
		}
		for (i=0; i<17; i++)
			fname[i] = SPACE;
		if (s[0] == 0 || keep)
			strncpy(fname, argv[fstart], MIN(strlen(argv[fstart]), 17));
		else
			strncpy(fname, s, MIN(strlen(s), 17));
		if (strlen(fname) < 17)
			strncat(fname, "                 ", 
					(17 - strlen(fname)));
		for (i=0; i < 17; ++i)
			fname[i] = islower(fname[i]) ? toupper(fname[i]) : fname[i];
		process_whdr1();
 		process_whdr2();
		wrteof();
		process_wfile();
		wrteof();
		process_weof1();
		process_weof2();
		wrteof();
	}
	wrtfinaleof();
}

process_wvol()
{
        int i;
 
        strncpy(inptbuf+0, VOL1, 4);
        strncpy(inptbuf+4, vid, 6);
        if (stdl) 
           *(inptbuf+10) = ZERO;
        else
           *(inptbuf+10) = SPACE;
        for (i=11; i<79; i++)
           *(inptbuf+i) = SPACE;
        if (stdl) 
           *(inptbuf+79) = SPACE;
        else
           *(inptbuf+79) = '1';
        if (stdl)
           atoibm(inptbuf, 80);
        if (write(tfd, inptbuf, 80) < 0)
           err(WRTERR);
}

process_whdr1()
{
    	int i;
 
	strncpy(inptbuf+0, HDR1, 4);
     	strncpy(inptbuf+4, fname, 17);
	strncpy(inptbuf+21, vid, 6);
 	strncpy(inptbuf+27, "0001", 4);
	strncpy(inptbuf+31, "0001",4);
	for (i=35; i<42; i++)
		*(inptbuf+i) = SPACE;
	for (i=42; i<47; i++)
		*(inptbuf+i) = ZERO;
        *(inptbuf+47) = SPACE;
 	for (i=48; i<53; i++)
		*(inptbuf+i) = ZERO;
	if (stdl)
		*(inptbuf+53) = ZERO;
        else
		*(inptbuf+53) = SPACE;
	for (i=54; i<60; i++)
		*(inptbuf+i) = ZERO;
	strncpy(inptbuf+60, SYSCD, 13);
	for (i=73; i<80; i++)
		*(inptbuf+i) = SPACE;
	if (stdl)
		atoibm(inptbuf, 80);
	if (write(tfd, inptbuf, 80) < 0)
	{
		err(WRTERR);
	}
	
}
 
process_whdr2()
{
	int	i;
 
	strncpy(inptbuf+0, HDR2, 4);
	*(inptbuf+4) = fmtab[recode];
	itoa(blklen, inptbuf+9, 5);
	itoa(reclen, inptbuf+14, 5);
	*(inptbuf+15) = '3';
	*(inptbuf+16) = ZERO;
	for (i=17; i<38; i++)
		*(inptbuf+i) = SPACE;
	*(inptbuf+38) = blktab[recode];
	for (i=39; i<80; i++)
		*(inptbuf+i) = SPACE;
	if (stdl)
		atoibm(inptbuf, 80);
	if (write(tfd, inptbuf, 80) < 0)
		err(WRTERR);
}

process_wfile()
{
	FILE	*fp;			/* UNIX file to be written to tape  */
	int	more;			/* flag for reading blocked records */
	int	nb;			/* number of bytes read or -1       */
	int	i;
	extern int	atoibm();

	more = 1;
	blkct = 0;
	if ((fp = fopen(ufname, "r")) == NULL)
		err(FOPERR);
	for (i=MAXBUF; --i >= 0;)
		*(inptbuf+i) = SPACE;

	switch (recode)
	{
		case F:
			while ((nb = readrec(fp,inptbuf)) >= 0)
			{
				if (stdl)
					atoibm(inptbuf, nb);
				write(tfd,inptbuf,reclen);
				blkct += 1;
				for (i=nb; --i >= 0;)
					*(inptbuf+i) = SPACE;
			}
			break;

		case FB:
			while (more && (nb = readrec(fp,inptbuf)) >= 0)
			{
				for (i=reclen; i < blklen; i += reclen)
					if ((nb = readrec(fp,inptbuf+i)) < 0)
					{
						more = 0;
						break;
					}
				if (stdl)
					atoibm(inptbuf, blklen);
				write(tfd,inptbuf,blklen);
				blkct += 1;
				for (i=blklen; --i >= 0;)
					*(inptbuf+i) = SPACE;
			}

		case V:
			while ((nb = readrec(fp, inptbuf+4)) >= 0)
			{
				itoa(nb+4, inptbuf+3, 4);
				if (stdl)
					atoibm(inptbuf, nb+4);
				write(tfd, inptbuf, nb+4);
				blkct += 1;
				for (i=nb+4; --i >= 0;)
					*(inptbuf+i) = SPACE;
			}
			break;

		case VB:
			nb = readrec(fp, inptbuf+8);
			while (more && nb >= 0)
			{
				itoa(nb+4, inptbuf+7, 4);
				vblen = nb + 8;
				for (i=nb+8;			
			             i+(nb = readrec(fp,inptbuf+i+4)) < blklen;
		                     i += nb+4)
					if (nb < 0)
					{
						more = 0;
						break;
					}
					else
					{
						itoa(nb+4, inptbuf+i+3, 4);
						vblen += nb + 4;
					}
				itoa(vblen, inptbuf+3, 4);
				if (stdl)
					atoibm(inptbuf, vblen);
				write(tfd, inptbuf, vblen);
				blkct++;
				if (nb > 0)
					bufcpy(inptbuf+8, inptbuf+i+4, nb);
				for (i=vblen-(nb+8); --i >= 0;)
					*(inptbuf+i+nb+8) = SPACE;
			}
			break;


	}
	if (fclose(fp) == EOF)
		err(FCLERR);
}

process_weof1()
{
	int i;
	 
	strncpy(inptbuf+0, EOF1, 4);
	strncpy(inptbuf+4, fname, 17);
	strncpy(inptbuf+21, vid, 6);
	strncpy(inptbuf+27, "0001", 4);
	strncpy(inptbuf+31, "0001", 4);
	for (i=35; i<42; i++)
		*(inptbuf+i) = SPACE;
	for (i=42; i<47; i++)
		*(inptbuf+i) = ZERO;
	*(inptbuf+47) = SPACE;
	for (i=48; i<53; i++)
		*(inptbuf+i) = ZERO;
	if (stdl)
		*(inptbuf+53) = ZERO;
	else
		*(inptbuf+53) = SPACE;
	itoa(blkct, inptbuf+59, 6);
	strncpy(inptbuf+60, SYSCD, 13);
	for (i=73; i<80; i++)
		*(inptbuf+i) = SPACE;
	if (stdl)
		atoibm(inptbuf, 80);
	if (write(tfd, inptbuf, 80) < 0)
		err(WRTERR);
}
 
process_weof2()
{
	int	i;
	 
	strncpy(inptbuf+0, EOF2, 4);
	*(inptbuf+4) = fmtab[recode];
	itoa(blklen, inptbuf+9, 5);
	itoa(reclen, inptbuf+14, 5);
	*(inptbuf+15) = '3';
	*(inptbuf+16) = ZERO;
	for (i=17; i<38; i++)
		*(inptbuf+i) = SPACE;
	*(inptbuf+38) = blktab[recode];
	for (i=39; i<80; i++)
		*(inptbuf+i) = SPACE;
	if (stdl) 
		atoibm(inptbuf, 80);
	if (write(tfd, inptbuf, 80) < 0)
		err(WRTERR);
}
 
wrteof()
{
	if (close(tfd) != 0)
		err(CLOERR);
	if (devflag)
	{
		if ((tfd = open(ntpdr, 1)) < 0)
			err(OPNERR);
	}
	else
		if ((tfd = open(tpdr, 1)) < 0)
			err(OPNERR);
}

wrtfinaleof()
{
	if (devflag)
	{
		close(tfd);
		if ((tfd = open(tpdr, 1)) < 0)
			err(OPNERR);
	}
}


readrec(fp,buf)
/*
**  	Readrec reads a record from the designated file
**	and returns the number of bytes read.
*/
FILE	*fp;
char	*buf;
{
	int	i;
	int	c;

	for (i=0; (c = getc(fp)) != '\n'; i++)
	{
		if (c == EOF)
		{
			if (i == 0)
				return(--i);
			else 
				return(i);
		}
		else 
			*(buf+i) = c;
	}
	return(i);
}
		

 
bufcpy(b, i, n)  
/*
**  Copies i[0] ... i[n-1] to b[0] ... b[n-1].
*/
register char	*b;
register char	*i;
register int	n;
{
	while(n--) 
	{
		*b++ = *i++;
	}
}
