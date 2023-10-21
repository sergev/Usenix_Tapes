#include	"dec.h"
#include	<ctype.h>
 
int	nb;				/* number of bytes read or -1 */
int	vrlen;
char	recfm;
char	blkatr;
FILE	*ofp;

readtp()
/*
**  Reads user-specified files from tape to the
**  current directory.
**  This function is entered when given 'r' option,
**  required parameters (dev=, vol=, tf=ansi or tf=stdl),
**  and successful open of tpdr.
*/
{
	extern int	ibmtoa();      
	extern int	check_id();
	extern int	eread();
	extern int	err();
	extern int	user_exit();
	extern int	wrtrec();

	firstrec = 1;
	blkct = 0;
	while (files && ((nb = read(tfd, inptbuf, MAXBUF)) >= 0 &&
	       		(nb || !(eov && (++eov == 3)))))
	{
		if (nb == 0)
			continue;
		if (stdl)
			ibmtoa(inptbuf, nb);
		if (!ok_vol_label && !firstrec)
			err(VOLERR);
		if (firstrec)
			firstrec = 0;
		switch (check_id(inptbuf))
		{
			case L_VOL1:
				process_rvol();
				continue;

			case L_HDR1:
				if (files & 1)
					process_rhdr1();
				continue;

			case L_HDR2:
				if (files & 1)
					process_rhdr2();
				blkct = 0;
				continue;

			case L_EOF1:
				process_reof1();
				continue;
		
			case L_EOF2:
				continue;

			case L_EOV1:
				err(EOVERR);
				continue;

			case L_UTL:
				user_exit();
				continue;

			case L_UHL:
				user_exit();	
				continue;

			default:
				if (files & 1)
				{
					if (stdl)
						atoibm(inptbuf, nb);
					process_rfile();
				}
		}
	}
	if (nb == -1)
		err(REDERR);
	else
		printf("\nProcessing complete\n");
}



process_rvol()
/*
** check volid (volume serial number)
** check accessibility if ansi
*/
{

	ok_vol_label = 1;
	if (strncmp(inptbuf+4, vid, 6))
		err(VIDERR);
	if (ansi && (*(inptbuf+10) != SPACE))
			err(SECERR);
}



process_rhdr1()
/*
** check security status and establish
** the UNIX file name to be used
*/
{
	int	i, j;

	if (ansi && (*(inptbuf+53) != SPACE))
		err(FPRERR);
	if (stdl && (*(inptbuf+53) != '0'))
		err(FPRERR);

	if (keep == 0)
	{
		printf("Current File Name:  ");
		putline(inptbuf+4, 17);
		printf("\n\nNew File Name:  ");
		eread(ufname);
	}
	if ((fname[0] == 0) || keep)
		for (i=4, j=0; j<17; ++i, ++j)
			fname[j] = isupper(inptbuf[i]) ? tolower(inptbuf[i]) : inptbuf[i];
	if ((ofp = fopen(ufname, "w")) == NULL)
		err(FOPERR);
	eov = 0;
}




process_rhdr2()
/*
** check tape recording technique
*/
{
	if (strncmp(inptbuf+34, SPACES, 2))
		err(TECERR);
	printf("recfm is :   ");
	putchar(*(inptbuf+4));
	recfm = inptbuf[4];   
	printf("\nblkatr is :  ");
	putchar(*(inptbuf+38));
	printf("\n");
	blkatr = *(inptbuf+38);
	if (blkatr == 'B')
		blklen = convert(inptbuf+5, 5);
	reclen = convert(inptbuf+10, 5);
}



process_reof1()
{
	eov++;
	if (blkct != convert(inptbuf+54, 6))
		printf("Block count discrepancy - Processing continues\n");
	if ((files & 1) && (fclose(ofp) == EOF))
		err(FCLERR);
	files >>= 1;
}



process_rfile()
/*
** A data record has been read and converted if stdl 
*/
{
	int	i;

	do
	{
		printf("nb is %d\n", nb);
		if (stdl)
			ibmtoa(inptbuf, nb);
		switch (blkatr)
		{
			case ' ':		/* unblocked records */
				switch (recfm)
				{
					case 'F':
						wrtrec(inptbuf, ofp, reclen, reclen);
						blkct++;
						break;

					case 'V':
						vrlen = convert(inptbuf, 4);
						wrtrec(inptbuf+4, ofp, vrlen-4, reclen-4);
						blkct++;
						break;
				}
				break;

			case 'B':		/* blocked records */
				switch (recfm)
				{
					case 'F':
						for (i=0; i<blklen; i += reclen)
						{
							printf("i is %d\n", i);
							printf("blklen is %d\n", blklen);
							printf("reclen is %d\n", reclen);
							wrtrec(inptbuf+i, ofp, reclen, reclen);
						}
						blkct++;
						break;

					case 'V':
						vblen = convert(inptbuf, 4);
						for (i=4; i<vblen; i += vrlen)
						{
							vrlen = convert(inptbuf+i, 4);
							wrtrec(inptbuf+(i+4), ofp, vrlen-4, reclen-4);
						}
						blkct++;
						break;
				}
				break;

			default:		/* copy as is */
				wrtrec(inptbuf, ofp, nb, nb);
		}
	} while ((nb = read(tfd, inptbuf, MAXBUF)) > 0);
	if (nb == -1)
		err(REDERR);
}




wrtrec(ibuf, ofp, irs, ors)
/*
**  Writes ors bytes of ibuf to
**  pointed to by ofp.
*/
char	*ibuf;
FILE	*ofp;
int	irs,ors;
{
	int	i;

	if (p)
	{
		for (i=0; i<irs; i++, ibuf++)
			putc(*ibuf, ofp);
		if (irs < ors)
			for (i=ors-irs; i--; )
				putc(SPACE, ofp);
		putc('\n', ofp);
	}
	else
	{
		/* strip blanks */
		for (i=irs-1; *(ibuf+i) == SPACE && i>=0; irs--, i--)   
			;
		for (; irs>0; ibuf++, irs--)
			putc(*ibuf, ofp);
		putc('\n', ofp);
	}
}

 

putline(cp, n)
/*
**  Prints n character of cp to stdout.
*/
register char	*cp;
register int	n;
{
	while(n--)
		putchar(*cp++);
	putchar('\n');
}



convert(s, n)
/*
**  Converts n characters of s to an integer
*/
char	*s;
int	n;
{
	char	t[10];
	int	l;

	strncpy(t, s, n);
	l = atoi(t);
	return(l);
}
