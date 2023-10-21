static int dly = 10;

send12_(ix,iy,isw)
/*
   $$$$$ calls cooked, raw, and write $$$$$

     Send12_ writes graphics data to a Tektronix 401x (or equivalent).
     It is fortran callable:

            call send12(ix,iy,isw)

     (ix,iy) is the coordinate of the raster we are moving to (0<=ix<1024,
     0<=iy<780).  If isw is 1 the vector will be bright, if isw is 2 the
     vector will be dark.  If isw is -1 send12's internal buffer will be
     flushed (ix and iy are ignored).  If isw is 0 then ix is taken
     to be a control flag (iy is ignored).  The following flags are
     defined:  -9 blank the screen, -10 delay, -11 set alpha mode, -12
     read the alpha cursor, -13 read the graphics cursor.  Programmed
     on 6 December 1979 by R. Buland.
*/
int *ix,*iy,*isw;
{
static int i = -1;  /* The buffer pointer. */
static char buf[120];  /* The internal buffer. */
int j;

if (*isw > 0) {  /* Encode a vector. */
  if (*isw > 1) buf[++i] = '\035';  /* Flag a dark vector. */
  buf[++i] = ((*iy>>5) & 037) | 040;  /* Encode iy. */
  buf[++i] = (*iy & 037) | 0140;
  buf[++i] = ((*ix>>5) & 037) | 040;  /* Encode ix. */
  buf[++i] = (*ix & 037) | 0100;
  if (i >= 100) goto xmit;  /* Spill the buffer if it is full. */
}
else if (*isw == 0) {  /* Handle a control flag. */
  switch (*ix) {
  case -9:  /* Blank the screen. */
    buf[++i] = '\033';
    buf[++i] = '\014';
    break;
  case -10:  /* Make a delay by sending nulls. */
    for (j = 0; j < dly; ++j) buf[++i] = '\0';
    break;
  case -11:  /* Set the terminal to alpha mode. */
    buf[++i] = '\037';
    break;
  case -12:  /* Read the alpha cursor. */
    buf[++i] = '\033';
    buf[++i] = '\005';
    break;
  case -13:  /* Read the graphics cursor. */
    buf[++i] = '\033';
    buf[++i] = '\032';
    break;
  default:  /* Anything else is an error. */
    break;
  }
  if (i >= 100) goto xmit;  /* Spill the buffer if it is full. */
}
else {  /* Flush the buffer. */
  if(i < 0) return;  /* Ignore this command if the buffer is empty. */
xmit:
  raw(1);  /* Set the standard output to raw mode. */
  write(1,buf,++i);  /* Dump the buffer. */
  cooked(1);  /* Set the standard output to cooked mode. */
  i = -1;  /* Reset the buffer pointer. */
}
return;
}
dly12_(j)
/*
   $$$$$ calls no other routine $$$$$

     Dly12_ resets the delay count for a -10 flag to send12_.  The fortran
     call is:

            call dly12(k)

     where k is the new delay count.  Programmed on 6 December 1979 by
     R. Buland.
*/
int *j;
{
dly = *j;
return;
}
