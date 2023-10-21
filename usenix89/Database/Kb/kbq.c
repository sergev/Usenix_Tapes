
# cut here: kbq.c : just cc -o kbq.o it and link kb.o and kbq.c to
# generate kbq kbu kbc and kbk (all linked)
/* kbq.c to query db-keys for keys -*-update-version-*-
** HFVR VERSION=Wed May 28 12:34:37 1986
*/

#include "kb.h"
#define LENGTH 2048	/* of line from keys file */

FILE *keys;		/* file of keys */

char line[LENGTH];	/* to hold one line from keys file */
char KNOWHOW[512];	/* holds copy of $KNOWHOW */
char *NAME;		/* pointer to pgram name */
BOOL dflag = FALSE;	/* TRUE if must print code */

/* findkey return TRUE if found else FALSE */
BOOL findkey(ptr)
CODE	*ptr;
{ extern char *fsindex();

 if ( fsindex(line,ptr->key,ptr->table) != NULL ) {
  return(TRUE);
 } else {
  return(FALSE);
 }
}/*findkey*/

/* get next line from keys and interpret it */
SYM readkey()
{ extern BOOL interpret();
  register char *lp;

/* return EOFsym if no more stuff */
  lp = fgets(line,LENGTH,keys);
  if ( NULL == lp ) return(EOFsym);

/* now interpret against the line */
  if ( TRUE == interpret() ) {
   return(FOUNDsym);
  } else {
   return(NOTFOUNDsym);
  }
}/*readkey*/

/* open keys file via $KNOWHOW or $HOME/.knowhow */
void openkeys()
{ char KEYS[512];
  extern char *getenv();
  char *ptr;

  ptr = getenv("KNOWHOW");
  if ( ptr != NULL ) {
   strcpy(KNOWHOW,ptr);
  } else {
   ptr = getenv("HOME");
   strcpy(KNOWHOW,ptr);
   strcat(KNOWHOW,"/.knowhow");
  }
  strcpy(KEYS,KNOWHOW);
  strcat(KEYS,".keys");
  keys = fopen(KEYS,"r");
  if (keys == NULL) {
   fprintf(stderr,"\007fb: Cannot open knowlegde base: %s\n",KEYS);
  }
}/*openkeys*/

/* fish id at start of line out of it into itemid */
void getitemid(itemid)
register char *itemid;
{ register char *s;

  s = line;
  while ( isdigit(*s) ) {
   *itemid = *s;
   s++; itemid++;
  }
  *itemid = '\0';
}/*getitemid*/

/* print item with id as on beginning of line */
void printitem()
{ char cmd[128];
  char itemid[SIZE];

  /* dbprint db id */
  strcpy(cmd,"$TOOLS/lib/gmacs/bin/dbprint ");
  strcat(cmd,KNOWHOW);
  strcat(cmd," ");
  getitemid(itemid);
  strcat(cmd,itemid);
  strcat(cmd," ; echo");
  system(cmd);
}/*printitem*/

/* print just key of current item */
printkey()
{ char itemid[SIZE];

  getitemid(itemid);
  printf("%s\n",itemid);
}/*printkey*/

/* do update on result of query */
void update()
{ char *ptr;
  char TMP[80];
  char EDITOR[512];
  char cmd[512];
  char itemid[SIZE];

  /* TMP = /usr/tm/K$$ */
  strcpy(TMP,"/usr/tmp/KXXXXXX");
  mktemp(TMP);
  unlink(TMP);

  /* : ${EDITOR=/bin/ed} */
  ptr = getenv("EDITOR");
  if ( ptr != NULL ) {
   strcpy(EDITOR,ptr);
  } else {
   strcpy(EDITOR,"/bin/ed");
  }

  /* CMD=$TOOLS/lib/gmacs/bin/dbprint db item > $TMP */
  strcpy(cmd,"$TOOLS/lib/gmacs/bin/dbprint ");
  strcat(cmd,KNOWHOW);
  strcat(cmd," ");
  getitemid(itemid);
  strcat(cmd,itemid);
  strcat(cmd," > ");
  strcat(cmd,TMP);
  system(cmd);

  /* $EDITOR $TMP */
  printf("kb: Wait while I start %s ...",EDITOR); fflush(stdout);
  strcpy(cmd,EDITOR);
  strcat(cmd," ");
  strcat(cmd,TMP);
  system(cmd);

  /* cat $TMP | dbadd db item */
  strcpy(cmd,"/bin/cat ");
  strcat(cmd,TMP);
  strcat(cmd," | $TOOLS/lib/gmacs/bin/dbadd ");
  strcat(cmd,KNOWHOW);
  strcat(cmd," ");
  strcat(cmd,itemid);
  system(cmd);
  printf("kb: item %s updated\n",itemid);
}/*update*/

/* set global NAME to start of program name */
void getname(name)
char *name;
{ extern char *strrchr();

  NAME = strrchr(name,'/');
  if ( NULL == NAME ) {
   NAME = name;
  } else {
   NAME++;
  }
}/*getname*/

/* determine whether we should print or update */
void workonid()
{
  switch(NAME[2]) {
   case 'u'     : update();    break;	/* kbu */
   case 'q'     : printitem(); break;	/* kbq */
   case 'c'     : 	       break;   /* kbc */
   case 'k'	: printkey();   break;	/* kbk */
   default	: printitem(); break;
  }
}/*workonid*/

main(argc,argv)
int	argc;
char	*argv[];
{ register SYM result;
  register int count;
  extern void parse();

  count = 0;
  if ( argc < 2) {
   fprintf(stderr,"\007Usage: %s [-d] <query>\n",argv[0]);
   exit(1);
  }
  getname(argv[0]);
  if ( strcmp(argv[1],"-d") == 0 ) {
   dflag = TRUE;
   parse(argv[2]);
  } else {
   parse(argv[1]);
  }
  openkeys();
  result = readkey();
  while (result != EOFsym ) {
   if (result == FOUNDsym) {
     workonid();
     count++;
   }
   result = readkey();
  }
  fprintf(stderr,"kb: %d items found\n",count);
  return(0);
}/*main*/
