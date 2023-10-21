rcur12_(ix,iy,nchar,len)
/*
   $$$$$ calls cooked, raw, and read $$$$$

     Rcur12_ reads graphics input from a Tektronix 401x (or equivalent)
     and recomposes the x and y coordinates of the cursor.  This routine
     is intended to be called from a fortran program:

            character*1 nchar
            call rcur12(ix,iy,nchar)

     Ix and iy are the integer*2 coordinates of the cursor.  Nchar is
     character*1.  Nchar contains the character (or status byte).
     Programmed on 6 December 1979 by R. Buland.
*/
int *ix,*iy;
char *nchar;
long len;
{
int j = 0;
char rbuf[5];

raw(0);  /* Set the standard input mode to raw. */
while (j < 5) j = read(0,&rbuf[j],5-j)+j;  /* Read five bytes. */
cooked(0);  /* Set the standard input mode to cooked. */
/* Reconstitute the x and y coordinates. */
*ix = (((unsigned) rbuf[1] & 037)<<5) | ((unsigned) rbuf[2] & 037);
*iy = (((unsigned) rbuf[3] & 037)<<5) | ((unsigned) rbuf[4] & 037);
*nchar = rbuf[0] & 0177;  /* Return the input character. */
return;
}
