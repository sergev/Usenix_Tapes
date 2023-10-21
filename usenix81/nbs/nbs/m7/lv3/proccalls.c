/*  proccalls.c
*
*      'Proccalls' preprocessess stack and counter calls found before the 
*      pattern and before and after the replacement symbol. 'Maksub' 
*      preprocesses stack and counter calls after the replacement definition.
*
*      Arguments: mac - input macro buffer.
*                 i - current position in mac.
*                 pmac - preprocessed macro buffer.
*                 j - current position in pmac.
*                 code - tells where in the pattern we are preprocessing calls.
*
*      Calls: procstck, proccntr, fatal_error
*
*      UE's: 'fatal_error' is called if the end of string character is found
*             before the delimiting character.
*
*                            Programmer:  G. Skillman
*/
proccalls(mac,i,pmac,j,code)

char   *mac, *pmac;
int    *i, *j, code;
{
       extern char      STACK, CNTR, LINECONT;
       extern char      MACDELIM, MACTERM, REPLACE, SCANOFF, EOS;

       switch (code)
       {
             case 1: /* preprocess before pat and after replacement symbol. */
                   while (mac[*i] != MACDELIM)
                   {
                          if (mac[*i] == MACTERM || mac[*i] == EOS)
                             fatal_error("PROCCALL: error in macro definition");
                          else if (mac[*i] == STACK)
                             procstck(mac,i,pmac,j);
                          else if (mac[*i] == CNTR)
                             proccntr(mac,i,pmac,j);
			  else if (mac[*i] ==  LINECONT)
	   			     while (mac[++(*i)] != LINECONT);
                          ++(*i);
                   }
                   break;

             case 2: /* preprocess after replacement symbol. */
                   while (mac[*i] != REPLACE && mac[*i] != SCANOFF)
                   {
                          if (mac[*i] == MACTERM || mac[*i] == EOS)
                             fatal_error("PROCCALL: error in macro definition");
                          else if (mac[*i] == STACK)
                             procstck(mac,i,pmac,j);
                          else if (mac[*i] == CNTR)
                             proccntr(mac,i,pmac,j);
			  else if (mac[*i] == LINECONT)
				     while (mac[++(*i)] != LINECONT);
                          ++(*i);
                   }
                   break;
       }
}
