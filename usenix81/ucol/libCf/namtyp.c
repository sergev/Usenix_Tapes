#define NL 70
namtyp(nin)
char nin[];
/* Checks legality of name in nin, returning functional value of */
/* 0 if illegal, -1 for real, 1 for integer. Names will be */
/* truncated to 12 characters */
/* Note that name is part of nin following last slash (if any)*/
{
  int is= -1,i,file=0;
/*scan through string to first blank, tab, or null, setting is to position*/
/*of last slash (if any)*/
   for(i=0; ctype(nin[i])!=5; i++){
      if(ctype(nin[i])==6)
         is=i;
   }
   if(is==i || i>NL)
      return(file);
   is++;
   i = is;
/*is now points at first character after slash. Starting here, look*/
/*for first character not a letter or number or _ */
   while((ctype(nin[i])<=4) && (i<NL))
     ++i;
   if((ctype(nin[i])>5) || (ctype(nin[is])>2))
     return(file);
   file = 1;
   if(ctype(nin[is])==1)
     file = -1;
   nin[i+1] = '\0';
   if(i<NL)
     nin[i] = '\0';
   return(file);
}
