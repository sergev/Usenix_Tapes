/*
**
**
**
** This group of routines will read in
** Unix formatted absolute code, strip
** off everything  but  the header and
** the symbol table,  change a  few of
** the entries,  then write the result
** out to a given file.
**
**
*/
/*
**
**
** DATA
**
**
*/
char	*emsg0	"Input and Output files needed.\n",
	*emsg1	"Symbol table conversion stopped.\n",
	*emsg2	"Error on open of input file.\n",
	*emsg3	"Error on 'creat' of output file.\n",
	*emsg4	"Error in header of input file.\n",
	*emsg5	"Error on output of header.\n",
	*emsg6	"Error while skipping text and data segments.\n",
	*emsg7	"Error detected in symbol table.\n",
	*emsg8	"Error on overwrite of header.\n";
/*
**
*/
int	infile,  /* The input file descriptor */
	outfile,  /* The output file descriptor */
	stabsize,  /* The # of bytes to skip (the text and data segments) */
	header[8],  /* A place holder for the header */
	symtab[6];  /* A place holder for the symbol table entry */
/*
**
*/
/*
**
**
**
** The 'main' routine will call the other various routines
** and, if an error is detected, print an error message to
** the diagnostic file and return.
**
**
*/
main(argc,argv)
	char	*argv[];
{
	char	*ifn,   /* The input file name */
		*ofn;   /* The output file name */
	if(argc < 3)  {  /* Then not enough info sent to this routine */
		write(2,emsg0,31);
		write(2,emsg1,33);
		return;
	}
	ifn = argv[1];   /* Get the file names */
	ofn = argv[2];
	if((infile = open(ifn,0)) == -1)  {  /* Bad news */
		write(2,emsg2,29);
		write(2,emsg1,33);
		return;
	}
	if((outfile = creat(ofn,0666)) == -1)  {  /* Bad news again */
		write(2,emsg3,33);
		write(2,emsg1,33);
		close(infile);
		return;
	}
	if(gethead())  {  /* Get the header and hope for no errors */
		write(2,emsg4,31);
		write(2,emsg1,33);
		close(infile);
		close(outfile);
		return;
	}
	if(skipem())  {  /* Skip over the text and data segments */
		write(2,emsg6,44);
		write(2,emsg1,33);
		close(infile);
		close(outfile);
		return;
	}
	if(puthead()) {  /* Copy the header to the output file (to be changed later) */
		write(2,emsg5,27);
		write(2,emsg1,33);
		close(infile);
		close(outfile);
		return;
	}
	if(convert())  {  /* Convert the symbol table entries and write them out */
		write(2,emsg7,32);
		write(2,emsg1,33);
		close(infile);
		close(outfile);
		return;
	}
	if(fixhead())  {  /* Now fix the header portion of the output file */
		write(2,emsg8,30);
		write(2,emsg1,33);
	}
	close(infile);
	close(outfile);
	return;
}
/*
**
**
**
** The routine below will read in the header,
** do a little (very) error checking, compute
** the # of bytes to skip and return.
**
**
*/
gethead()
{
	if(read(infile,header,16) <= 0) return(1);
	if(header[0] < 0407 || header[0] > 0411) return(1);
	return(0);
}
/*
**
**
**
** The small routine below will skip over the unused text and data
** segments by doing a seek on the file.
**
**
*/
skipem()  
{
	if(seek(infile,header[1],1) == -1) return(1);
	if(seek(infile,header[2],1) == -1) return(1);
	if(!header[7]) {
		if(seek(infile,header[1],1) == -1) return(1);
		if(seek(infile,header[2],1) == -1) return(1);
	}
	return(0);
}
/*
**
**
**
** This routine will change the header to a semi-correct
** form and write it out to the output file.
**
**
*/
puthead()
{
	header[0] = 0407;  /*  Read-Write code */
	stabsize = header[4]; /* The # of symbol table entries times 12 */
	header[1] = header[2] = header[3] = header[4] =
		header[5] = header[6] = header[7] = 0;
	if(write(outfile,header,16) < 0) return(1);  /* Uh oh if < 0 */
	return(0);
}
/*
**
**
**
** This next routine will scan thru the symbol table,  pull those
** entries out which are externals, change their external code to
** 041 and write them out to the output file.
**
**
*/
convert()  
{
	while(stabsize > 0)  {  /* Get the entry, check it and maybe write it out */
		stabsize =- 12;
		if(read(infile,symtab,12) <= 0)    /* Then bad news */
			return(1);
		if(symtab[4] < 045 && symtab[4] > 037)  {  /* Then copy it */
			symtab[4] = 041;  /* Change the magic # */
			header[4] =+ 12;  /* Add this entry to total length */
			if(write(outfile,symtab,12) < 0) return(1);  /* And write it to the temp file */
		}
	}
	return(0);
}
/*
**
**
**
** The routine below will take the now correct header
** and write it out to the output file (in the correct
** place).
**
**
*/
fixhead()
{
	if(seek(outfile,0,0) < 0) return(1);  /* Go back to the start of the file */
	if(write(outfile,header,16) < 0) return(1);  /* And write out the good header */
	return(0);
}
/*
**
**
*******************************************************************************************
**
**
*/
