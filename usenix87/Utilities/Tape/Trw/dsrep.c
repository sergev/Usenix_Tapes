
# include	"dec.h"
 
dsrep() 
/*
**  Prints to stdout a list of all tape filenames,
**  file numbers, and record formats.
**  This function is entered when given 'd' option,
**  required parameters (dev=, vol=, tf=ansi or tf=stdl),
**  and successful open of tpdr.
*/
{
	extern int	ibmtoa();
	extern int	check_id();
	extern int	user_exit();
	extern int 	err();
	int	nb;			/* number of bytes read or -1 */
	 
	firstrec = eov = 1;
	while (((nb = read(tfd, inptbuf, MAXBUF)) >= 0) &&
		(nb || !(eov && (++eov == 3))))
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
				process_dvol();
				continue;

			case L_HDR1:
				process_dhdr1();
				continue;

			case L_HDR2:
				process_dhdr2();
				continue;

			case L_EOF1:
				++eov;
				continue;

			case L_EOF2:
				continue;

			case L_EOV1:
				err(EOVERR);

			case L_UTL:
				user_exit();
				continue;

			case L_UHL:
				user_exit();
				continue;

		}
	}
	if (nb == -1)
		err(REDERR);
	else
		printf("\nProcessing Complete\n");		
}
  
process_dvol() 
{
	ok_vol_label = 1;
	if (strncmp(inptbuf+4, vid, 6)) 
		err(VIDERR);
	if (ansi && (*(inptbuf+10) != SPACE))
		err(SECERR);
}
		
	
process_dhdr1()
{
        strncpy(fname, inptbuf+4, 17);
}
 
process_dhdr2() 
{
	if (!filectr)
		printf("\t\tFILE NO\t\tFILE NAME\t\tFILE FORMAT\n\n");
	printf("\t\t%d\t\t%s\t\t%c%c\n", ++filectr, fname, 
			*(inptbuf+4), *(inptbuf+38));
	eov = 0;	
}
