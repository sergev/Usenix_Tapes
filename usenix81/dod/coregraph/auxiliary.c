#include "inclusions.c"


/****************************************************************************/
/*                                                                          */
/*     FUNCTION: identity                                                   */
/*                                                                          */
/*     PURPOSE: SET A 3X2 ARRAY EQUAL TO THE IDENTITY MATRIX.               */
/*                                                                          */
/****************************************************************************/

identity(arrayptr)
   float *arrayptr;   /*** POINTER TO ARRAY NAME PASSED. ***/
   {
   float *temptr;     /*** TEMPORARY POINTER VARIABLE ***/

   for(temptr = arrayptr;temptr < (arrayptr + 6);temptr++)
      {
      if(temptr == arrayptr || temptr == arrayptr + 3)
	 *temptr = 1.0;     /*** POSITIONS [0][0] & [1][1] ARE SET TO 1; ***/
      else *temptr = 0.0;   /*** ALL OTHERS SET TO 0. ***/
      }
   }





/****************************************************************************/
/*                                                                          */
/*     FUNCTION: identchk                                                   */
/*                                                                          */
/*     PURPOSE: CHECK TO SEE IF A 3X2 ARRAY IS EQUAL TO THE IDENTITY MATRIX.*/
/*                                                                          */
/****************************************************************************/

identchk(arrayptr)
   float *arrayptr;   /*** POINTER TO ARRAY NAME PASSED. ***/
   {
   float *temptr;     /*** TEMPORARY POINTER VARIABLE ***/

   for(temptr = arrayptr;temptr < (arrayptr + 6);temptr++)
      {
      if(temptr == arrayptr || temptr == arrayptr + 3)
	 {
	 if(*temptr != 1.0)
	    {
	    return(1);
	    }
	 }
      else if(*temptr != 0.0)
	 {
	 return(1);
	 }
      }
   return(0);
   }





/****************************************************************************/
/*                                                                          */
/*     FUNCTION: strcmp                                                     */
/*                                                                          */
/*     PURPOSE: COMPARE TWO STRINGS FOR EQUALITY.                           */
/*                                                                          */
/****************************************************************************/

strcmp(string1,string2)
   char *string1,*string2;
   {
   for( ;*string1 == *string2;string1++,string2++)
      {
      if(*string1 == '\0')   /*** CHECK FOR END OF STRING. ***/
	 {
	 return(0);   /*** EQUAL STRINGS ***/
	 }
      }
   return(-1);   /*** UNEQUAL STRINGS ***/
   }




/****************************************************************************/
/*                                                                          */
/*     FUNCTION: length                                                     */
/*                                                                          */
/*     PURPOSE: COMPUTE THE LENGTH ( NUMBER OF CHARACTERS) OF A STRING.     */
/*                                                                          */
/****************************************************************************/

length(string)
   char *string;
   {
   register int counter;

   for(counter = 0;*string != '\0';string++,counter++)
      ;
   return(counter);
   }






/****************************************************************************/
/*                                                                          */
/*     FUNCTION: attrchk                                                    */
/*                                                                          */
/*     PURPOSE: CHECK THE ATTRIBUTE VALUE PASSED TO SEE IF IT FALLS IN      */
/*              THE FOLLOWING RANGE:  -1 <-> +1                             */
/*                                                                          */
/****************************************************************************/

attrchk(value,funcname)
   float value;
   char *funcname;
   {
   int errnum;

   if((value < -1) || (value > 1))
      {
      errnum = 13;
      errhand(funcname,errnum);
      return(3);
      }
   return(0);
   }





/****************************************************************************/
/*                                                                          */
/*     FUNCTION: sattrck                                                    */
/*                                                                          */
/*     PURPOSE: CHECK THE ATTRIBUTE VALUE PASSED TO SEE IF IT EQUALS EITHER */
/*              TRUE OR FALSE.                                              */
/*                                                                          */
/****************************************************************************/

sattrck(value,funcname)
   int value;
   char *funcname;
   {
   int errnum;

   if((value != TRUE) && (value != FALSE))
      {
      errnum = 13;
      errhand(funcname,errnum);
      return(3);
      }
   return(0);
   }





/****************************************************************************/
/*                                                                          */
/*     FUNCTION: fldefck                                                    */
/*                                                                          */
/*     PURPOSE: CHECK THE FLOATING POINT DEFAULT PARAMETER TO SEE IF IT     */
/*              FALLS WITHIN IT'S OWN RANGE: MINIMUM < VALUE > MAXIMUM.     */
/*                                                                          */
/****************************************************************************/

fldefck(value,minval,maxval,funcname)
   float value,minval,maxval;
   char *funcname;
   {
   int errnum;

   if((value < minval) || (value > maxval))
      {
      errnum = 13;
      errhand(funcname,errnum);
      return(3);
      }
   return(0);
   }





/****************************************************************************/
/*                                                                          */
/*     FUNCTION: indefck                                                    */
/*                                                                          */
/*     PURPOSE: CHECK THE INTEGER DEFAULT PARAMETER TO SEE IF IT            */
/*              FALLS WITHIN IT'S OWN RANGE: MINIMUM < VALUE > MAXIMUM.     */
/*                                                                          */
/****************************************************************************/

indefck(value,minval,maxval,funcname)
   int value,minval,maxval;
   char *funcname;
   {
   int errnum;

   if((value < minval) || (value > maxval))
      {
      errnum = 13;
      errhand(funcname,errnum);
      return(3);
      }
   return(0);
   }





/****************************************************************************/
/*                                                                          */
/*     FUNCTION: strchk                                                     */
/*                                                                          */
/*     PURPOSE: CHECK A STRING FOR AN ILLEGAL CHARACTER.                    */
/*                                                                          */
/****************************************************************************/

strchk(string)
   char *string;
   {

   for( ;*string != '\0';string++)
      {
      if((*string < MINASCII) || (*string > MAXASCII))
	 {
	 *string = QUESTION;
	 return(2);
	 }
      }

   return(0);
   }


/****************************************************************************/
/*                                                                          */
/*     FUNCTION: pdfwrite                                                   */
/*                                                                          */
/*     PURPOSE: MAKE AN ENTRY IN THE PSEUDO DISPLAY FILE                    */
/*                                                                          */
/****************************************************************************/

pdfwrite(bytecnt,buffer)
   int bytecnt;
   char *buffer;
   {

   if(write(pdfd,buffer,bytecnt) != bytecnt)
      {
      printf("System message:Pdf write error.\n");
      printf("Dump of buffer:%s\n",buffer);
      }
   else
      {
      pdfnext =+ bytecnt;
      }

   return(0);
   }



/****************************************************************************/
/*                                                                          */
/*     FUNCTION: pdfread                                                    */
/*                                                                          */
/*     PURPOSE: READ AN ENTRY FROM THE PSEUDO DISPLAY FILE                  */
/*                                                                          */
/****************************************************************************/

pdfread(bytecnt,buffer)
   int bytecnt;
   char *buffer;
   {
   int n_read;

   if((n_read = read(pdfd,buffer,bytecnt)) == 0)
      {

      /**************************************************************/
      /***                                                        ***/
      /*** END-OF-FILE MARK REACHED, ASSUME UNCLOSED OPEN SEGMENT ***/
      /*** SEND PDFENDSEGMENT TO READING ROUTINE                  ***/
      /***                                                        ***/
      /**************************************************************/

      *buffer = PDFENDSEGMENT;
      }

   else if(n_read != bytecnt)
      {
      printf("System message:PDF boundaries are misalligned, see Core system manager.\n");
      exit(1);
      }

   else if(n_read == -1)
      {
      printf("System message:An error of some sort has occured during a PDF read.\n");
      exit(1);
      }

   return(0);
   }



