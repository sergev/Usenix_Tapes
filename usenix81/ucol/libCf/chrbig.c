chrbig_(ix)
/* Fortran-callable routine to set character size on a
   Tektronix 4014 with enhanced graphics.
   The call is:
                call chrbig(n)
   where n is a integer equal to 1,2,3, or 4 (other
   values have no effect). 1 sets the characters to the
   largest size, 4 to the smallest.
   Programmed by D. Agnew Oct 25 1980
*/
int *ix;
{
char buf[2];
if (*ix<1 || *ix>4)
   return;
buf[0] = '\033';
switch (*ix){
   case 1:
      buf[1] = '\070';
      break;
   case 2:
      buf[1] = '\071';
      break;
   case 3:
      buf[1] = '\072';
      break;
   case 4:
      buf[1] = '\073';
      break;
}
raw(1);
write(1,buf,2);
cooked(1);
return;
}
