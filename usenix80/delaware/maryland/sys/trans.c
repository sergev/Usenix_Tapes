#
/*
**
** This is supposedly a PAL-11R to UNIX as assembly
** language translator. It assumes that the
** PAL assembly input is correct. Indeed if
** it is not, then you will get garbage out
** as well. It will read the PAL from a
** file (or standard input) and print the translated code
** on the standard output.
**
** DATA GOODIES
**
** The three string arrays 'oldops', 'newops' and 'olddir'
** are used as keyword change determiners. If an 
** alphanumeric read in matches one of the strings
** in the tables 'oldops' and 'olddir', appropriate
** translation procedures are invoked.
** IF ANY ENTRIES ARE ADDED, THEN CHANGE THE
** "top" PARAMETER SENT TO "lookup".
**
*/
#define	DIRTOP	13

char *olddir[]{ /* DOS Assembler Directives */
   "ascii","asciz","asect","blkb","blkw",
   "byte","csect","dsect","endc","even",
   "globl","if","psect","word",0
};
/*
**
*/
#define	OPSTOP	43

char *oldops[]{ /* operators and others to be changed*/
   "absd",
   "ac0","ac1","ac2","ac3","ac4","ac5",
   "addd","bpt","ccc","clrd","cmpd","divd",
   "iot","ldcdf","ldcfd","ldcid","ldcif","ldcld",
   "ldclf","ldd","ldexp","ldf","modd","muld",
   "negd","nop","r6","r7","rti","rtt","scc","stcdf","stcdi","stcdl",
   "stcfd","stcfi","stcfl","std","stexp","stf",
   "subd","trap","tstd",
   0
};
/*
**
*/
char *newops[]{ /* operators and others to change to */
   "absf",
   "fr0","fr1","fr2","fr3","fr4","fr5",
   "addf","003","257","clrf","cmpf","divf",
   "004","movof","movof","movif","movif","movif",
   "movif","movf","movie","movf","modf","mulf",
   "negf","240","sp","pc","002","006","277","movfo","movfi","movfi",
   "movfo","movfi","movfi","movf","movei","movf",
   "subf","sys","tstf",
   0
};
/*
**
**
** Below are various miscellaneous storages
** used throughout the procedures. The explanations
** beside each should be sufficient.
**
**
*/
char inbuff[512], /* The 512 character read buffer */
     newline[100], /* This will hold the translation */
     token[100], /* A token is read into here */
     last; /* Strangely, this holds the NEXT input character */
/*
**
*/
int tclass, /* This will hold the 'class' of the token */
    cclass, /* Used to hold the class of the current character */
    lclass, /* Used to hold the class of the next character */
    newptr, /* Will point into the translation line */
    bptr,  /* Points into 'inbuff' */
    rsize,  /* The # of characters read in with last 'read' */
    infile;  /* The input file descriptor */
/*
**
**
**
** Just Routine Stuff
**
**
** The main routine which follows starts up the
** the translator. It will decide what file to
** use as input('standard input' is the default), open
** it for read-only, do the translation and
** close the file.
**
**
*/
main(argc,argv)
   int argc; /* The # of arguements on the call line */
   char *argv[]; /* The input arguements */
{
   if(argc < 2) infile = 0 ;  /* Default input file is standard input */
   else
      if((infile = open(argv[1],0)) == -1)  {
         write(2,"Error on open of input file.\n",29);
         write(2,"Translation stopped.\n",21);
         return;
      }
   bptr = 511;
   rsize = 511;
   nextchar(); /* Initialize the translator */
   fillit(); /* Go translate the input */
   return; /* Bye */
}
/*
**
**
** The routine below will fill the string 'token'
** with the next token from the input stream. Blanks,
** alphanumerics, numerals and single-character symbols
** are the only ones known. This routine will set up
** the variable 'tclass' with the token's class.
**
**
*/
gettoken() {
   register i; /* A temporary variable */
   i = 0; /* which points into 'token' */
   token[i++] = nextchar(); /* Get the token's first character */
   tclass = cclass; /* And set up the token's class */
   switch(tclass) { /* Now, compile the whole token */
      case 1 :  /* Token will be all blanks */
         while(lclass == 1)  /* Gather all the blanks */
            token[i++] = nextchar();
         break;
      case 2 :  /* Token is ALPHANUMERIC */
         while(lclass == 2 || lclass == 3)  /* Letter or digit */
            token[i++] = nextchar();  /* Gather them in */
         break;
      case 3 :  /* Token is a number */
         while(lclass == 3)  /* Digit */
            token[i++] = nextchar();  /* Pick a #, any #... */
         break;
      case 4 :  /* The catchall for anything else */
         break;  /* Don't do anything */
   }
   token[i] = '\0';  /* End of token mark */
   return;
}
/*
**
**
**
** The routine below will read one character from
** the input file, convert it to lower case if that
** is necessary, decide it's class and return the
** character.
**
**
*/
nextchar() {
   register x;  /* A temporary character store */
   register i;  /* A temporary test variable */
   x = last;  /* The last character read in */
   i = reada(infile,inbuff,1);  /* Get the next character */
   last = inbuff[0];  /* And put it in last */
   if(!i)  /* If i == 0 then EOF */
      last = '\0';  /* So flag it so */
   if(x >= 'A' && x <= 'Z')  /* Convert UPPER to lower */
      x =+ 'a' - 'A';  /* Do it */
   cclass = lclass;  /* Set up the current class */
   lclass = getclass(last);  /* And decide the next class */
   return(x);  /* Return the character */
}
/*
**
**
**
** That below is a buffered input read routine.
** It will read in a large block of characters
** at one time if necessary.
**
**
*/
reada(file,buffer,num)
   int file,  /* The file descriptor */
       num;  /* A dummy parameter */
   char *buffer;  /* The input buffer to use */
{
   while(1)  {  /* In case an actual 'read' has to be done */
      if(bptr < rsize)  {  /* Get one character */
         buffer[0] = buffer[bptr++];
         return(1);  /* Return successful */
      }
      rsize = read(file,buffer,512);  /* Else, try to get some more */
      if(rsize <= 0)  /* Then all done with input */
         return(0);
      bptr = 0;
   }
}
/*
**
**
**
** The next routine will decide what class a passed
** character should have.
**
**
*/
getclass(x)
   char x;  /* The passed character */
{
   if(x == ' ')  /* Blank */
      return(1);  /* The BLANK class (no, that's not an epithet) */
   if(x >= 'a' && x <= 'z')  /* A letter */
      return(2);  /* The letter class */
   if(x >= 'A' && x <= 'Z')  /* A LETTER */
      return(2);  /* The letter class */
   if(x >= '0' && x <= '9')  /* A digit */
      return(3);  /* The DIGIT class */
   if(x == '\t')  /* Treat a "tab" like a blank */
      return(1);
   return(4);  /* The catchall class */
}
/*
**
**
**
** Then there are the miscellaneous routines.
**
** That immediately below will look up a
** string in a table and, if found, will
** return the index. The one below it will
** copy one string to an arbitrary location in 
** another.
**
**
*/
lookup(thing,table,top)
   char thing[],  /* The string to look up */
        **table;  /* The table to search */
   int top;  /* The length of the table (minus 1) */
{
   register i,j,x;   /* Some temporaries */
   int bottom;  /* The bottom of the table */
   bottom = 0;  /* At first, it really is the bottom */
   do
   {  /* Do a binary search */
      i = (bottom + top) / 2 ;  /* That's the way you start off */
      for(j=0; (x = thing[j]) == table[i][j]; j++)
         if(!x) return(i);
      if(table[i][j] < x)  bottom = ++i;
      else  top = --i;
   }  while (bottom <= top);
   return(-1);  /* If here, then it wasn't in the table */
}
/*
**
**
*/
copyit(thing,where,start)
   int start;  /* The location to transfer to */
   char *thing,  /* The string to transfer */
        where[];  /* The string to be changed */
{
   register i;   /* To speed things up */
   i = start;
   while(where[i++] = *thing++);  /* Transfer it */
   return(--i);  /* And return the next position */
}
/*
**
**
**
** Below is the main routine of the translator.
** This routine will scan through the input text,
** changing things as required by calling on
** sub-procedures. It returns when no more text remains.
**
**
*/
fillit() { 
   register i,j;  /* Temporaries */
   while(1) {  /* Loop forever (almost) */
      gettoken();  /* Get the token */
      switch(tclass) {  /* Go do the appropriate thing */
         case 1 :  /* Blanks and numbers are */
         case 3 :  /* directly echoed */
            newptr = copyit(token,newline,newptr);  /* Do it */
            break;
         case 2 :  /* Do something special for alphanumerics */
            i = lookup(token,oldops,OPSTOP);  /* Is the token an old op ? */
            if(i >= 0) {  /* If so, change old to new */
               j = 0;
               while((token[j] = newops[i][j]) != '\0')
                  j++;
            }
            newptr = copyit(token,newline,newptr);  /* Copy into output line */
            break;
         case 4 :  /* Hopefully a single character */
            switch(token[0]) {  /* Do the correct thing */
               case ';' :  /* A comment to echo */
                  skipem();  /* So, skip them */
                  break;
               case '.' :  /* Might be a Directive */
                  dotdoit();  /* So try to change it */
                  break;
               case '"' :  /* Both use the */
               case '\'' :  /* same routine */
                  quoted();  /* Do it */
                  break;
               case '%' : case '/' : case '@' :  /* These must all */
               case '#' : case '<' : case '>' :  /* be changed */
                  changem();  /* SO, go do it */
                  break;
               case '(' : case ')' : case '[' : case ']' : case ':' :
               case '=' : case ',' : case '$' : case '+' : case '-' :
               case '*' : case '&' : case '!' : case '^' :   /* These remain the same */
                  newptr = copyit(token,newline,newptr);  /* Copy them directly */
                  break;
               case '\n' :  /* If NEWLINE, then print the new line... */
                  newptr =  copyit(token,newline,newptr);  /* Copy it */
                  printf("%s",newline);  /* Print it */
                  newptr = 0;  /* Reset the line pointer */
                  break;
               case '\0' :  /* End of the input */
                  return(1);  /* Successful */
               default :  /* Burp */
                  write(2,"Whoops! Something is funny!\n",28);
                  write(2,"Illegal character detected in 'fillit'.\n",40);
                  write(2,"Check the input line following the last one printed.\n",53);
                  return(0);
            }
      }
   }
}
/*
**
**
**
** The routine below will just echo comments
**
**
*/
skipem() {
   register x;  /*  A temporary */
   register i;  /* And another */
   token[0] = '/';  /* Comment character is not semi-colon */
   newptr = copyit(token,newline,newptr);  /* Copy it */
   newline[newptr++] = last;  /* Start the scan */
   while((x = nextchar()) != '\n')  /* Get the comment */
      newline[newptr++] = last;
   newline[newptr] = '\0';  /* The end of the string */
   printf("%s",newline);  /* Print it */
   newptr = 0;  /* Reset the pointer */
   return;
}
/*
**
**
**
** The next routine will change single character symbols
** to their new form (such as '@' --> '*').
**
**
*/
changem() {
   switch(token[0]) {  /* Do the appropriate thing */
      case '%' :  /* A register symbol */
         token[0] = 'r';
         if(last == '6')  {  /* Then change output to 'sp' */
            token[0] = 's';
            token[1] = 'p';
            token[2] = '\0';
            nextchar();
         }
         if(last == '7')  {  /* Then change output to 'pc' */
            token[0] = 'p';
            token[1] = 'c';
            token[2] = '\0';
            nextchar();
         }
         break;
      case '/' :  /* The division symbol */
         token[0] = '\\';
         token[1] = '/';
         token[2] = '\0';
         break;
      case '@' :  /* The indirect flag */
         token[0] = '*';
         break;
      case '#' :  /* The immediate flag */
         token[0] = '$';
         break;
      case '<' :  /* An expression organiser */
         token[0] = '[';
         break;
      case '>' :  /* And another */
         token[0] = ']';
         break;
   }
   newptr = copyit(token,newline,newptr);  /* Copy it */
   return;
}
/*
**
**
**
** The routine below is invoked by a top-level
** ".". Finding this could mean that
** an assembler directive might need changing.
** This routine will determine whether or not
** to change it, and if so, it will invoke the
** proper sub-routine to scan the line.
**
**
*/
dotdoit() {
   register i;  /* A temporary */
   if(lclass == 2) {  /* Then this is a directive */
      gettoken();  /* Get the next token */
      i = lookup(token,olddir,DIRTOP);  /* Is it one to be changed? */
      if(i == -1) {  /* If not there, then treat as a comment */
         newptr = copyit("/",newline,newptr);
	 newptr = copyit(token,newline,newptr);
	 token[1] = 0;
         skipem();  /* Call the comment skipper */
         return;  /* And return */
      }
      switch(i) {  /* If it was there, then go do the proper thing */
         case 0 :  /* ASCII */
         case 1 :  /* ASCIZ */
            ascii(i);  /* Invoke the ASCII scanner */
            return;
         case 2 :  /* ASECT */
         case 6 :  /* CSECT */
         case 12 :  /* PSECT */
         case 7 :  /* DSECT */
            acpdsect(i);  /* Go change them */
            return;
         case 11 :  /* An IF */
            ifscan();  /* Do the correct thing */
            return;
         case 8 :  /* An ENDC */
            newptr = copyit(".endif",newline,newptr);  /* Copy it */
            return;
         case 9 :  /* EVEN */
            newptr = copyit(".even",newline,newptr);  /* Do it */
            return;
         case 5 :  /* BYTE */
            newptr = copyit(".byte",newline,newptr);  /* The first thing to do */
            exprs();  /* Then scan the following expression(s) */
            return;
         case 13 :  /* WORD */
            wexprs();  /* Call the word-expression scanner */
            return;
         case 3 :  /* BLKB */
            newptr = copyit(".=.+[",newline,newptr);  /* The first stuff */
            expr();  /* The second stuff */
            newptr = copyit("]",newline,newptr);  /* The final stuff */
            return;
         case 4 :  /* BLKW */
            newptr = copyit(".=2*[",newline,newptr);  /* The first stuff */
            expr();  /* The second stuff */
            newptr = copyit("]+.",newline,newptr);  /* The final stuff */
            return;
         case 10 :  /* GLOBL */
            newptr = copyit(".globl",newline,newptr);  /* Copy directly */
            return;
      }
   }
   newptr = copyit(token,newline,newptr);  /* If it is just "." */
}
/*
**
**
**
** Now for the sub-routines.
**
** The routine  below will do the proper
** thing for a single or double quote token.
**
**
*/
quoted()  {
   token[1] = last;  /* The character after the quote(s) is echoed */
   token[2] = '\0';  /* For a single quote */
   if(token[0] == '"') {  /* However, if it is a double quote... */
      reada(infile,inbuff,1);  /* Get the next character */
      token[2] = inbuff[0];  /* Put it in the token */
      token[3] = '\0';  /* And move the end of token mark */
   }
   newptr = copyit(token,newline,newptr);  /* Copy the token into the line */
   nextchar();  /* Update the get-a-character routine */
   return;
}
/*
**
**
**
** The routine you see below will change the
** format of an ascii constant. It will do this for 
** both ascii and asciz.
**
**
*/
ascii(i)
   int i;  /* The passed type of ascii (ascii or asciz) */
{
   register x;  /* To temporarily hold a test character */
   register j;  /* Used as an index into token */
   newptr = copyit("<",newline,newptr);  /* The UNIX as ascii-start symbol */
   while(last == ' ' || last == '\t') nextchar(); /* Skip over the blank space */
   x = last;  /* Get the first non-blank character after ASCII or ASCIZ */
   nextchar();  /* And then skip over it */
   j = 1;  /* Set up the index */
   token[0] = last;  /* The first character is that last read in */
   while(last != x)  {  /* Get the rest of the text */
      nextchar();  /* Read in the character */
      token[j++] = last;  /* And store it away */
      if(last == '>')  {  /* Then it must be ESCAPED */
         token[j-1] = '\\';
         token[j++] = '>';
      }
   }
   token[j-1] = '>';  /* In case token was ASCII */
   if(i == 1)  {  /* Then it was an ASCIZ */
      token[j-1] = '\\';
      token[j++] = '0';
      token[j++] = '>';
   }
   token[j] = '\0';  /* End of token mark */
   newptr = copyit(token,newline,newptr);  /* Copy it */
   gettoken();  /* Skip over one token */
   return;
}
/*
**
**
**
** The routine below will handle
** ASECT, CSECT, PSECT and DSECT.
**
**
*/
acpdsect(i)
   int i;  /* The passed type of directive */
{
   switch(i)  {  /* Go do the correct thing */
      case 2 :  /* ASECT */
         newptr = copyit(".data /",newline,newptr);  /* Do it */
         break;
      case 6 :  /* CSECT */
      case 12 :  /* PSECT */
         newptr = copyit(".text /",newline,newptr);  /* Copy it */
         break;
      case 7 :  /* DSECT */
         newptr = copyit(".bss /",newline,newptr);
         break;
   }
   return;
}
/*
**
**
**
**  If you are an IF, you get scanned by IFSCAN.
**
**
*/
ifscan()  { 
   newptr = copyit(".if",newline,newptr); /* Copy the IF */
   gettoken();  /* Get the following blank(s) */
   newptr = copyit(token,newline,newptr);  /* Copy them */
   gettoken();  /* Skip the NE */
   gettoken();  /* And the , */
   expr();  /* And scan the following expression */
   return;
}
/*
**
**
**
** The routine below is called by many others.
** It will scan thru an expression, changing individual
** characters as necessary.
**
**
*/
expr()  {
   while(1)  {  /* Loop for the whole expression */
      while(lclass != 4)  {  /* Echo all but the single character symbols */
         gettoken();  /* Get a token */
         newptr = copyit(token,newline,newptr);  /* And copy it */
      }
      switch(last)  {  /* If here, then something special to do */
         case ';' :  /* Comment */
         case '\n' :  /* New line */
         case '\0' :  /* EOF */
            return(0);  /* Done with translation */
         case ',' :  /* Tell the calling process that there's more to do */
            return(1);
         case '\'':  /* Single quote */
         case '"' :  /* And double */
            gettoken();  /* Get the quote(s) */
            quoted();  /* and do the proper thing */
            break;
         case '%' : case '/' : case '@' :
         case '#' : case '<' : case '>' :
            gettoken();  /* Get the token */
            changem();  /* And change it */
            break;
         case '.' : case '(' : case ')' : case '[' : case ']' :
         case ':' : case '=' : case '$' : case '+' : case '-' :
         case '*' : case '&' : case '!' : case '^' :  
            gettoken();  /* Get the token */
            newptr = copyit(token,newline,newptr); /* And echo it */
            break;
      }
   }
}
/*
**
**
**
** The two short routines below will scan for byte and
** word expressions. They both will repeatedly call 'expr'.
**
**
*/
exprs()  {
   while(expr())  {  /* scan through the expression */
      newptr = copyit(",",newline,newptr);  /* Write out a comma */
      gettoken();  /* And skip the one in the text */
   }  /* Loop through all the expressions on the line */
   return;
}
/*
**
**
*/
wexprs()  {
   while(expr())  {  /* Scan through the expression */
      newptr = copyit(";",newline,newptr);  /* The word expression delimiter */
      gettoken();  /* Skip over a comma */
   }  /* Loop for all the expressions on the line */
   return;
}
/*
**
***************************************************************************************
**
*/
