ctype(c)
char c;
/* finds type of character c, returning values of*/
/*  1 for a-h,o-z,A-H,O-Z  */
/*  2 for i-n,I-N         */
/*  3 for 0-9             */
/*  4 for _         */
/*  5 for blank, end of string, newline, or tab */
/*  6 for slash*/
/*  7 for everything else*/
{
  int i;
  if((c>='a' && c<='h') || (c>= 'o' && c<= 'z'))
    i = 1;
  else if ((c>='A' && c<='H') || (c>='O' && c<= 'Z'))
    i = 1;
  else if ((c>='i' && c<= 'n') || (c>='I' && c<='N'))
    i = 2;
  else if (c>= '0' && c <= '9')
    i = 3;
  else if (c == '_')
    i = 4;
  else if (c==' ' || c=='\n' || c=='\t' || c =='\0')
    i = 5;
  else if (c=='/')
    i = 6;
  else
    i = 7;
  return(i);
}
