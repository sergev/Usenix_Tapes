
/* search.c -*-update-version-*-
** HFVR VERSION=Fri May 23 13:50:16 1986
*/
#include <stdio.h>

#define MIN(a,b)	( (a) < (b) ? (a) : (b) )
#define	TSIZE		128

void initfsindex(s,t)
register char	*s;
register int	t[];
{ register int i;
  register int slength;
  register int *p;

  slength = strlen(s);
  for ( i = 0 ; i < 128 ; i++ ) t[i] = slength;

  for ( i = 0 ; i < slength -1 ; i++ ) {
   p = &t[s[i]];
   *p = MIN( *p, slength-i-1 );
  }
}/*initfsindex*/

/* fsindex: return pointer to where small starts in big, or NULL if
** not found. pos must be initialized by call to initfsindex */
char *fsindex(big,small,pos)
char	*big;
char	*small;
int	pos[];
{ register char *pes;
  register char *peb;
  register char *pb;
  register char *ps;
  int slength;

/* ABCD.     DEFGH.	where . means EOS
** 01234     012345
**  ^ ^       ^   ^
**  | |       |   |
** ps pes     pb  peb
*/

  slength = strlen(small);
  if ( slength == 0 ) return(big);
  pes = &small[slength-1];	/* points to last char of small */
  peb = &big[strlen(big)];	/* points to EOS in big */
  pb =  &big[slength-1];	/* starting point for search */

l1: 
    if ( pb < peb ) {	/* then not past end of big */
    ps = pes;	/* set to end of small */

l2: 
     if ( *pb != *ps ) {	/* then not found so skip */
       pb += ( *pb < (char)TSIZE ? pos[*pb] : slength );
       goto l1;
     }
     if ( ps != small) {	/* then not full small found yet */
       ps--;
       pb--;
       goto l2;			/* try previous char in small */
     } else {
       return(pb);		/* full small found at pb */
     }
    } else {
      return(NULL);	/* not found */
    }
}/*fsindex*/

/* sindex: returns pointer to start of small in big, or NULL if not found
** calls fsindex after initializing */
char *sindex(big,small)
char big[];
char small[];
{ int	table[TSIZE];

  initfsindex(small,table);
  return(fsindex(big,small,table));
}/*sindex*/

int table[TSIZE];
FILE *fp;
char *filename;
char noname[] = "";

#define LENGTH 2048

int workon(s)
{ register char *lp;
  char line[LENGTH];
  register int res;

  res = 1;
  while (  (lp = fgets(line,LENGTH,fp)) != NULL ) {
    if ( fsindex(line,s,table) != NULL ) {
     res = 0;
     if ( filename != noname ) printf("%s:",filename);
     printf("%s",line);
    }
  }
  return(res);
}/*workon*/

int main(argc,argv)
int	argc;
char	*argv[];
{ register int res;	/* result */
  register int i;

  res = 1;
  filename = noname;
  if ( argc <= 1 ) {
    fprintf(stderr,"\007Usage: %s <string> <files>*\n",argv[0]);
    exit(1);
  }

/* set search table */
  initfsindex(argv[1],table);

/* work on stdin if no files given */
  if ( argc == 2 ) {
   fp = stdin;
   exit(workon(argv[1]));
  }

/* else work on each file */
  for ( i = 2 ; i < argc ; i++ ) {
   fp = fopen(argv[i],"r");
   if (NULL == fp ) {
     fprintf(stderr,"%s: cannot open %s\n",argv[0],argv[i]);
   } else {
     if (argc > 3 ) filename = argv[i];
     res = workon(argv[1]);
     fclose(fp);
   }/*fi*/
  }/*for*/
  exit(res);
}/*main*/
