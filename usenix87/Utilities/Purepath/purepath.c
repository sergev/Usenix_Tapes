/* purepath.c: -*-update-version-*-
** to make PATH contain every entry only once
** HFVR VERSION=Fri Oct 11 13:57:26 1985
*/

#include <stdio.h>

char	NAME[] = "purepath";
char	*PATH;
typedef	enum{FALSE,TRUE} BOOL;
typedef	struct part
		{ char	      *string;
		  struct part *next;
		} PART;

PART	*root = NULL;

/*newpart: to create and init new part of path record */
PART *newpart()
{ extern PART *malloc();
  PART	*ptr;
  static char EMPTY[] = "";

  ptr = malloc(sizeof(PART));
  if (ptr == NULL) {
    fprintf(stderr,"\007%s: Out of memory. Aborting.\n",NAME);
    exit(1);
  }

  ptr->string = EMPTY;
  ptr->next = NULL;

  return(ptr);
}/*newpart*/

void init()
{ char	*charptr;
  PART	*ptr;

  charptr = PATH;
  while (charptr[0] != '\0') {
    ptr = root;
    root = newpart();
    root->next = ptr;
    root->string = charptr;

/* skip till ':' or EOS, then replace ':' by EOS */
    while ( (charptr[0] != ':') && (charptr[0] != '\0') ) charptr++;
    if (charptr[0] != '\0') {
     charptr[0] = '\0';
     charptr++; 
    }
  }/*while*/
 
}/*init*/

/* return TRUE if string pointed to is unique */
BOOL unique(ptr)
PART	*ptr;
{ BOOL  found;
  PART	*ptr1;
  
  if (ptr == NULL) return(TRUE);
  found = FALSE;
  ptr1 = ptr->next;
  while ( (!found) && (ptr1 != NULL) ){
   if (strcmp(ptr->string,ptr1->string) == 0) {
    found = TRUE;
   } else {
    found = FALSE;
   }
   ptr1 = ptr1->next;
  }/*while*/

  if (found) return(FALSE);  
  return(TRUE);
}/*unique*/

/* print string in PART, first one first */
void printpart(ptr)
PART	*ptr;
{
  if (ptr == NULL) return;
  printpart(ptr->next);
  if (unique(ptr)) {
    printf("%s:",ptr->string);
  }
}/*printpart*/

/* print all parts from root down */
void finit()
{
 printpart(root);
}/*finit*/

void usage()
{
  fprintf(stderr,"Usage: %s [-V] [-H] [path]\n",NAME);
  fprintf(stderr,"Where  -V displays version\n");
  fprintf(stderr,"       -H displays this message\n");
}/*usage*/

/* parse each - option */
void parse(ch)
{
 switch (ch) {
  case 'v'    :
  case 'V'    : printf("%s: version 0.99\n",NAME);
  		exit(1);
		break;
  case 'H'    : usage();
		exit(1);
  		break;
  default     : fprintf(stderr,"\007%s: illegal option -%c\n",NAME,ch);
		usage();
		exit(1);
  		break;
 }/*switch*/
}/*parse*/

/* parse and act on all argv */
void parseoptions(argc,argv)
int	argc;
char	*argv[];
{ extern char *getenv();
  int i;
  
  PATH = getenv("PATH");	/* default */

  for ( i = 1 ; i < argc ; i++) {
   switch(argv[i][0]) {
    case '-'    : parse(argv[i][1]);
    	          break;
    default     : PATH = argv[i];
                  break;
   }
  }/*for*/
}/*parseoptions*/

main(argc,argv)
int	argc;
char	*argv[];
{
 parseoptions(argc,argv);
 init();
 finit();
}/*main*/
