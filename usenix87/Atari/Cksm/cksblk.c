
int cksblk(block, numbyt, initck, nlines) /*  Checksum a block of bytes  */
char *block;
long int numbyt;
int initck, *nlines;

/*  Arguments:  block:  Pointer to first byte of block to be checksummed.  */
/*              numbyt: Number of bytes in the block.			   */
/*              initck: Initial value for checksum (i.e., partial sum	   */
/*			carried forward from previous blocks).		   */

{
	register char *curbyt, *endptr;
	register int accsm;
	
	endptr = block + numbyt;	/*  Pointer to end+1 of block	*/
	accsm = initck;			/*  Checksum accumulator.	*/


 	for ( curbyt = block;  curbyt < endptr;  curbyt++ )  {
#ifdef vax
		if(*curbyt == 0x0a)
			(*nlines)++;
#endif
		accsm = (accsm + accsm + accsm) + (*curbyt & 0377);
		accsm = (accsm & 07777) + ((accsm >> 12) & 7);
	}

	return(accsm);
}
