/* kb.c to parse boolean expressions  -*-update-version-*-
** HFVR VERSION=Wed Jan  7 07:51:33 1987
**
** SYNTAX accepted:
** ------ ---------
** <query> ::= <term> { OR <term> }*
**
** <term> ::= <factor> { AND <factor> }*
**
** <factor> ::=   <key>
** 	        | NOT <factor>
**	        | ( <query> )
**
** <key> ::= <anything except ( ) ! & | SP TAB \n \0 > and or not ~
**
** alternative syntax for some keywords:
** -----------
** NOT		!	~ 	not
** AND		&	&&	and
** OR		|	||	or
** (		[	{	<
** )		]	}	>
*/

#include "kb.h"

/* all types of symbols */
typedef enum { LPARENTsymbol,	/* ( */
	       RPARENTsymbol,   /* ) */
	       ORsymbol,	/* | */
	       ANDsymbol,	/* & */
	       NOTsymbol,	/* ! */
	       EOFsymbol,	/* '\0' */
	       KEYsymbol,	/* rest */
	     } SYMBOL;

/* symbol as parsed */
typedef struct { SYMBOL	symbol;		/* symbol type */
        	 char	key[SIZE];	/* spelling */
	       } ITEM;

ITEM	next;			/* holds last symbol and spelling */
char	*string1;		/* ptr to start of string to parse */
char	*string;		/* ptr to rest of string to be parsed */
char	lastchar;		/* last char read */
int	lastindex;		/* index into string */
int	count = 0;		/* times true interpreter */

/* extern void initfsindex(); */

#define SETCODE(INS,ARG1,ARG2,ARG3,KEY1,CODESYM)	\
	{ CODE	*ptr; 					\
	  ptr = &(instruction[(INS)]);			\
	  ptr->arg1 = setptr((ARG1));			\
	  ptr->arg2 = setptr((ARG2));			\
	  ptr->arg3 = setptr((ARG3));			\
	  ptr->count = 0;				\
	  if ( strcmp(ptr->key,(KEY1)) != 0 ) {		\
	    strcpy(ptr->key,(KEY1));			\
	    ptr->table = (KEYTAB *) malloc(sizeof(KEYTAB));\
	    initfsindex(ptr->key,ptr->table);		\
	  }/*fi*/					\
	  ptr->code = (CODESYM);			\
	}

#define MAXNROFCODES 100
CODE instruction[MAXNROFCODES];
#define NOPC -1			/* the novalue value for PC */
int	PC;			/* Program Counter */

/* return pointer to instruction.value, NULL if PC=NOPC */
BOOL *setptr(pc)
int	pc;
{
 if ( pc != NOPC ) {
   return( &(instruction[pc].value) );
 } else {
   return(NULL);
 }
}/*setpc*/

/* print error message and exit */
void error(s)
char	s[];
{ int i;

  fprintf(stderr,"\007kb: ERROR in query\n");
  fprintf(stderr,"    query=%s\n",string1);
  fprintf(stderr,"           ");
  for (i = 1 ; i < lastindex ; i++) fprintf(stderr," ");
  fprintf(stderr,"^\n");
  fprintf(stderr,"    %s\n",s);
  exit(1);
}/*error*/

char *prcodesym(code)
CODEsym	code;
{ static char	res[10];

  switch(code) {
  case RFIcode : strcpy(res,"RFI"); break;
  case IFFcode : strcpy(res,"IFF"); break;
  case IFTcode : strcpy(res,"IFT"); break;
  case NOTcode : strcpy(res,"NOT"); break;
  case KEYcode : strcpy(res,"KEY"); break;
  case COPcode : strcpy(res,"COP"); break;
  case CITcode : strcpy(res,"CIT"); break;
  case CIFcode : strcpy(res,"CIF"); break;
  default      : strcpy(res,"???"); break;
  }
  return(res);
}/*prcodesym*/

/* generates 3-address code and returns PC of newly generated instruction */
int generate(code,key,PC1,PC2,PC3)
CODEsym	code;
char	key[];
int	PC1;	/* PC at which result for arg1 can be found */
int	PC2;	/* PC at which result for arg2 can be found */
int	PC3;	/* PC at which result for arg3 can be found */
{ int	temp;

  SETCODE(PC,PC1,PC2,PC3,key,code);
  temp = PC;
  PC++;
  return(temp);
}/*generate */

/* read next character from string */
#define NEXTCHAR	{ lastchar = string[0]; \
			 if ( string[0] != '\0' ) { /* no skip past EOS */ \
			   string++; \
			   lastindex++; \
			}}

/* skip all white space characters */
void skipblanks()
{
 while ( (lastchar == '\t') ||
         (lastchar == '\n') ||
	 (lastchar == ' ' )
       ) {
  NEXTCHAR;
 }
}/*skipblanks*/

/* return TRUE if ch is NOT a special character (eg. not part of key) */
BOOL notspecial(ch)
register char ch;
{
  switch ( ch ) {
       case  '(' : return(FALSE); break;
       case  '[' : return(FALSE); break;
       case  '{' : return(FALSE); break;
       case  '<' : return(FALSE); break;
       case  ')' : return(FALSE); break;
       case  ']' : return(FALSE); break;
       case  '}' : return(FALSE); break;
       case  '>' : return(FALSE); break;
       case  '|' : return(FALSE); break;
       case  '&' : return(FALSE); break;
       case  '!' : return(FALSE); break;
       case  '~' : return(FALSE); break;
       case  '\n': return(FALSE); break;
       case  '\t': return(FALSE); break;
       case  ' ' : return(FALSE); break;
       case  '\0': return(FALSE); break;
       default   : return(TRUE);  break;
  }
  return(TRUE);
}/*notspecial*/

/* read in spelling of key into next */
void getkey()
{ register int i;

  i = 0;
  while ( notspecial(lastchar) ) {
   next.key[i] = lastchar;
   if (i < SIZE-1) i++;
   next.key[i] = '\0';
   NEXTCHAR;
  }
}/*getkey*/

/* to debug: print SYMBOL type */
void prsymbol(symbol)
ITEM	symbol;
{ 
  switch(symbol.symbol) {
   case LPARENTsymbol : printf("LEFT   (\n"); break;
   case RPARENTsymbol : printf("RIGHT  )\n"); break;
   case ORsymbol      : printf("OR     |\n"); break;
   case ANDsymbol     : printf("AND    &\n"); break;
   case NOTsymbol     : printf("NOT    !\n"); break;
   case EOFsymbol     : printf("EOF\n"); break;
   case KEYsymbol     : printf("KEY    %s\n",symbol.key); break;
   default            : error("Unknown symbol"); break;
  }
}/*prsymbol*/

/* search for keywords, if not found return KEYsymbol */
/* to search is in next.key */
SYMBOL keyword(key)
register char	key[];
{
  switch (key[0]) {
   case 'a'   : if ( strcmp(key,"and") == 0 ) {
		 return(ANDsymbol);
   		} else {
		 return(KEYsymbol);
		}
		break;
   case 'A'   : if ( strcmp(key,"AND") == 0 ) {
		 return(ANDsymbol);
   		} else {
		 return(KEYsymbol);
		}
		break;
   case 'o'   : if ( strcmp(key,"or") == 0 ) {
		 return(ORsymbol);
   		} else {
		 return(KEYsymbol);
		}
		break;
   case 'O'   : if ( strcmp(key,"OR") == 0 ) {
		 return(ORsymbol);
   		} else {
		 return(KEYsymbol);
		}
		break;
   case 'n'   : if ( strcmp(key,"not") == 0 ) {
		 return(NOTsymbol);
   		} else {
		 return(KEYsymbol);
		}
		break;
   case 'N'   : if ( strcmp(key,"NOT") == 0 ) {
		 return(NOTsymbol);
   		} else {
		 return(KEYsymbol);
		}
		break;
   default    : return(KEYsymbol);
   		break;
  }
  return(KEYsymbol);
}/*keyword*/

/* get next symbol from string, EOFsymbol set if no more */
void getsymbol()
{
  skipblanks();
  switch (lastchar) {
   case '('   :
   case '{'   :
   case '['   :
   case '<'   : next.symbol = LPARENTsymbol;
   		strcpy(next.key,"(");
		NEXTCHAR;
   		break;
   case ')'   :
   case '}'   :
   case ']'   :
   case '>'   : next.symbol = RPARENTsymbol;
   		strcpy(next.key,")");
		NEXTCHAR;
		break;
   case '|'   : next.symbol = ORsymbol;
   		strcpy(next.key,"|");
		NEXTCHAR;
		if (lastchar == '|') NEXTCHAR;	/* allow || */
		break;
   case '&'   : next.symbol = ANDsymbol;
   		strcpy(next.key,"&");
		NEXTCHAR;
		if (lastchar == '&' ) NEXTCHAR;	/* allow && */
		break;
   case '~'   :
   case '!'   : next.symbol = NOTsymbol;
   		strcpy(next.key,"!");
		NEXTCHAR;
		break;
   case '\0'  : next.symbol = EOFsymbol;
   		strcpy(next.key,"");
		break;
   default    : getkey();
   		next.symbol = keyword(next.key);
   		break;
  }
/*  prsymbol(next); */
}/* getsymbol*/

/* check if current symbol is correct and then skip it */
void skipsymbol(symbol)
SYMBOL	symbol;
{
  if (next.symbol != symbol) {
    switch (symbol) {
     case LPARENTsymbol : error("'(' expected"); break;
     case RPARENTsymbol : error("')' expected"); break;
     case ORsymbol      : error("'OR' expected"); break;
     case ANDsymbol     : error("'AND' expected"); break;
     case NOTsymbol     : error("'NOT' expected"); break;
     case KEYsymbol     : error("<key> expected"); break;
     case EOFsymbol     : error("End of query expected"); break;
     default            : error("Something is wrong!!"); break;
    }
  }
  getsymbol();
}/*skipsymbol*/

int query();	/* forward definition */

int factor()
{ int temp;

  switch ( next.symbol) {
   case LPARENTsymbol : getsymbol();	/* skip ( */
   			temp = query();
   			skipsymbol(RPARENTsymbol);
			return(temp);
			break;
   case NOTsymbol     : getsymbol();	/* skip ! */
			return(generate(NOTcode,"",factor(),NOPC,NOPC));
   			break;
   case KEYsymbol     : temp = generate(KEYcode,next.key,NOPC,NOPC,NOPC);
   			getsymbol();	/* skip KEYcode */
			return(temp);
      			break;
   default            : error("Factor expected");
   			break;
  }
  return(NOPC);	/* never */
}/*factor*/

/* remember that 'temp1 AND temp2' is equivalent to:
** [temp ] 1. eval temp1
** [pciff] 2. if ![temp] then [temp2]=false ; goto [temp2]+1
** [temp2] 3. eval temp2
**         4.
*/
int term()
{ int temp;
  int pciff;

  temp = factor();
  while ( next.symbol == ANDsymbol ) {
   getsymbol();	/* skip & */
   pciff = generate(IFFcode,"",temp,NOPC,NOPC);
   temp = factor();
   /* backpatch arg2 and arg3 from IFF */
   instruction[pciff].arg2 = &(instruction[temp].value);
   instruction[pciff].arg3 = (BOOL *)&(instruction[temp+1]);
  }
  return(temp);
}/*term*/

/* remember that 'temp1 OR temp2' is equivalent to:
** [temp] 1. eval temp1
** [pcift] 2. if [temp] then [temp2]=true ; goto [temp+1]
** [temp2] 3. eval temp2
**         4.
*/
int query()
{ int temp;
  int pcift;

  temp = term();
  while ( next.symbol == ORsymbol ) {
   getsymbol();	/* skip | */
   pcift = generate(IFTcode,"",temp,NOPC,NOPC);
   temp = term();
   /* backpatch arg2 and arg3 from IFT */
   instruction[pcift].arg2 = &(instruction[temp].value);
   instruction[pcift].arg3 = (BOOL *)&(instruction[temp+1]);
  }
  return(temp);
}/*query*/

/* findcode return NOPC if not found else place of KEYcode found
** with key=code.key , will always find at least itself */
int find1code(key)
char key[];
{ int i;
  
  for ( i=0 ;; i < MAXNROFCODES ) {
   switch(instruction[i].code) {
   case KEYcode : if ( strcmp(instruction[i].key,key) == 0 ) {
		    return(i);
		  } else {
   		  i++;	/* skip to next */
		  }
		  break;
   case RFIcode : return(NOPC);	/* not found */
   		  break;
   default      : i++; /* skip to next */ 
		  break;
  }/*switch*/
 }/*for*/
}/*find1code*/

/* findcode return NOPC if not found else place of KEYcode found
** with key is substring of code.key  will always find itself */
int find2code(key)
char key[];
{ extern char *sindex();	/* return NULL if not found */
  int i;
  
  for ( i=0 ;; i < MAXNROFCODES ) {
   switch(instruction[i].code) {
   case KEYcode : if ( sindex(instruction[i].key,key) != NULL ) {
		    return(i);
		  } else {
   		  i++;	/* skip to next */
		  }
		  break;
   case RFIcode : return(NOPC);	/* not found */
   		  break;
   default      : i++; /* skip to next */ 
		  break;
  }/*switch*/
 }/*for*/
}/*find2code*/

/* findcode return NOPC if not found else place of KEYcode found
** with code.key is substring of key will always find itself */
int find3code(key)
char key[];
{ extern char *sindex();	/* return NULL if not found */
  int i;
  
  for ( i=0 ;; i < MAXNROFCODES ) {
   switch(instruction[i].code) {
   case KEYcode : if ( sindex(key,instruction[i].key) != NULL ) {
		    return(i);
		  } else {
   		  i++;	/* skip to next */
		  }
		  break;
   case RFIcode : return(NOPC);	/* not found */
   		  break;
   default      : i++; /* skip to next */ 
		  break;
  }/*switch*/
 }/*for*/
}/*find3code*/

/* optimize instructions by looking to add COPcode instructions */
void optkeys1()
{ int i;
  int j;

  for ( i=0 ;; i < MAXNROFCODES ) {
   switch(instruction[i].code) {
    case KEYcode : j = find1code(instruction[i].key);
    		   if ( j != i) {	/* then used before */
		    SETCODE(i,j,NOPC,NOPC,instruction[i].key,COPcode);
		    instruction[i].arg2 = &(instruction[j].count);
		   }
		   i++; 	/* skip to next */
		   break;
    case RFIcode : return; 
		   break;
    default      : i++;  /* skip to next */
    		   break;
   }
  }
}/*optkeys1*/

/* optimize instructions by looking to add CITcode instructions */
/* must call AFTER optkeys1(), otherwise gets a mess */
void optkeys2()
{ int i;
  int j;

  for ( i=0 ;; i < MAXNROFCODES ) {
   switch(instruction[i].code) {
    case KEYcode : j = find2code(instruction[i].key);
    		   if ( j != i) {	/* then used before */
		     SETCODE(i,j,NOPC,NOPC,instruction[i].key,CITcode);
		     instruction[i].arg2 = &(instruction[j].count);
		   }
		   i++; 	/* skip to next */
		   break;
    case RFIcode : return; 
		   break;
    default      : i++;  /* skip to next */
    		   break;
   }
  }
}/*optkeys2*/

/* optimize instructions by looking to add CIFcode instructions */
/* must call AFTER optkeys2(), otherwise gets a mess */
void optkeys3()
{ int i;
  int j;

  for ( i=0 ;; i < MAXNROFCODES ) {
   switch(instruction[i].code) {
    case KEYcode : j = find3code(instruction[i].key);
    		   if ( j != i) {	/* then used before */
			SETCODE(i,j,NOPC,NOPC,instruction[i].key,CIFcode);
			instruction[i].arg2 = &(instruction[j].count);
		   }
		   i++; 	/* skip to next */
		   break;
    case RFIcode : return; 
		   break;
    default      : i++;  /* skip to next */
    		   break;
   }
  }
}/*optkeys3*/

/* just call all optimizations in right order */
void optimize()
{
  optkeys1();	/* add COP codes */
  optkeys2();	/* add CIT codes */
  optkeys3();	/* add CIF codes */
}/*optimize*/

/* return index of instruction where ptr=&instruction[i]{.value} */
int getpc(p)
BOOL	*p;
{ CODE	*ptr;
  int 	i;

 for (i = 0; ; i++) {
   ptr = &instruction[i];	/* for speed */
   if ( ((int *)ptr == p) || (&(ptr->value) == p) ) {
    return(i);
   }
   if (ptr->code == RFIcode) return(NOPC);
 }/*for*/
}/*getpc*/

printcode(i,ptr)
int i;
CODE *ptr;
{
 fprintf(stderr,"[%3d] %s %20s  ARG1=(%3d) ARG2=(%3d) ARG3=(%3d)\n",
    			i,
    			prcodesym(ptr->code),
			ptr->key,
			getpc(ptr->arg1),
			getpc(ptr->arg2),
			getpc(ptr->arg3) );
}/*printcode*/

void dumpcode()
{ int i;
  CODE *ptr;

  for ( i=0 ;; i < MAXNROFCODES ) {
    ptr = &instruction[i];
    printcode(i,ptr);
    if (ptr->code == RFIcode) return;
    i++;
  }
}/*dumpcode*/

/* call parse the first time */
void parse(s)
char s[];
{ int temp;
  extern BOOL dflag;

  string = s;
  string1 = s;
  lastindex = 0;
  PC = 0 ;
  NEXTCHAR;	/* get first char */
  getsymbol();	/* get first symbol */
  temp = query();
  skipsymbol(EOFsymbol);	/* must be at end here */
  (void)generate(RFIcode, "", temp, NOPC, NOPC);
  optimize();
  if (dflag) dumpcode();
}/*parse*/

#define NOT(b1)		( ( b1 )           ? FALSE : TRUE )

/* return result TRUE or FALSE from interpreting */
/* this is were we spend most of our time */
BOOL interpret()
{ register CODE *ptr;
  extern BOOL findkey();	/* and here we spend a lot of time */

 ptr = instruction;	/* for speed reasons */
 count++;
 for (;;) {	/*EVER*/
  switch(ptr->code) {
   case KEYcode :    ptr->value = findkey(ptr);
   		     ptr->count = count;	/* set it to done */
		     ptr++;	/* skip to next instruction */
                     break;
   case COPcode :    if ( *(ptr->arg2) == count ) { /* see if done */
   			ptr->value = *(ptr->arg1);
		     } else {
			ptr->value = findkey(ptr);
		     }
		     ptr++;	/* skip to next instruction */
   		     break;
   case CITcode :    if ( (*(ptr->arg2) == count) && (*(ptr->arg1)) ) {
			ptr->value = TRUE;
		     } else {
		        ptr->value = findkey(ptr);
		     }
		     ptr++;	/* skip to next instruction */
		     break;
   case CIFcode :    if ( (*(ptr->arg2) == count) && (!*(ptr->arg1)) ) {
   			ptr->value = FALSE;
		     } else {
			ptr->value = findkey(ptr);
		     }
		     ptr++;	/* skip to next instruction */
		     break;
   case IFFcode :    if ( *(ptr->arg1) ) {
			ptr++;	/* skip to next instruction */
   		     } else {
			*(ptr->arg2) = FALSE;
			ptr = (CODE *)ptr->arg3;	/* goto (arg3) */
		     }
		     break;
   case IFTcode :    if ( *(ptr->arg1) ) {
			*(ptr->arg2) = TRUE;
			ptr = (CODE *)ptr->arg3;	/* goto (arg3) */
   		     } else {
		        ptr++;	/* skip to next instruction */
		     }
		     break;
   case NOTcode :    ptr->value = NOT(*(ptr->arg1));
		     ptr++;	/* skip to next instruction */
                     break;
   case RFIcode :    return(*(ptr->arg1));
                     break;
   default      :    error("Unknown instruction, kb in error");   
                     break;
  }
 }
}/*interpret*/
