/*
**		    Copyright (c) 1985	Ken Wellsch
**
**     Permission is hereby granted to all users to possess, use, copy,
**     distribute, and modify the programs and files in this package
**     provided it is not for direct commercial benefit and secondly,
**     that this notice and all modification information be kept and
**     maintained in the package.
**
*/

#include "mdefs.h"

#define MEMSIZ		1100

static struct symstr mem[MEMSIZ] ;
static int pmem = MEMSIZ ;

#define CMEMSIZ		10000

static char cmem[CMEMSIZ] ;
static int pcmem = 0 ;

#define TABSIZ		256

static struct symstr *symtab [TABSIZ] =
{
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
} ;


struct symstr *define (nam,val,mod)
  char *nam ;
  int val, mod ;
{
	register struct symstr *p ;
	int h ;
	extern char *strsav () ;
	extern struct symstr *lookup () ;

	if ( lookup(nam) != NIL )
		return (NIL) ;
	
	if ( pmem <= 0 )
		error ("Symbol define","symbol table full on `%s'\n",nam) ;
	p = &(mem[--pmem]) ;

	p->s_nam = strsav (nam) ;
	p->s_val = val ;
	p->s_aux = 0 ;
	p->s_mod = mod ;

	h = hash (nam) ;
	p->s_nxt = symtab[h] ;
	symtab[h] = p ;

	return (p) ;
}

struct symstr *lookup (name)
  char *name ;
{
	register struct symstr *p ;

	for ( p = symtab[hash(name)] ; p != NIL ; p = p->s_nxt )
	{
		if ( strcmp (name,p->s_nam) == 0 )
			return (p) ;
	}
	return (NIL) ;
}

int hash (s)
  register char *s ;
{
	register int h ;

	for ( h = 0 ; *s != EOS ; )
		h += *s++ ;
	
	return ( ( h % TABSIZ ) ) ;
}

char *strsav (s)
  char *s ;
{
	register char *p ;
	register int n ;

	n = strlen(s) + 1 ;
	if ( (pcmem+n) >= CMEMSIZ )
		error ("Strsave","Out of symbol space for `%-10.10s'\n",s) ;

	p = &(cmem[pcmem]) ;
	pcmem += n ;
	(void) strcpy (p,s) ;
	return (p) ;
}

#ifdef PSTAB

int pstab ()
{
	register int i ;
	register struct symstr *p ;
	int noverb = 0 ;
	int nov = 0 ;
	int noplac = 0 ;
	int nop = 0 ;
	int noobjs = 0 ;
	int noo = 0 ;

	for ( i = 0 ; i < TABSIZ ; i++ )
	{
		for ( p = symtab[i] ; p != NIL ; p = p->s_nxt )
		{
			switch (type(p->s_val))
			{
			  case VERB:
				  printf ("%-12.12s",p->s_nam) ;
				  printf ("\t%5d",p->s_val) ;
				  printf ("\t(verb %d)\n",p->s_aux) ;
				  noverb += p->s_aux ;
				  nov++ ;
				  break ;
			
			  case PLACE:
				  printf ("%-12.12s",p->s_nam) ;
				  printf ("\t%5d",p->s_val) ;
				  if ( p->s_aux > 0 )
				  {
				  	printf ("\t(place %d)\n",(p->s_aux)-1) ;
				  	noplac += (p->s_aux)-1 ;
				  }
				  else
				  	printf ("\t(place %d)\n",(p->s_aux)) ;
				  nop++ ;
				  break ;

			  case OBJECT:
				  printf ("%-12.12s",p->s_nam) ;
				  printf ("\t%5d",p->s_val) ;
				  if ( p->s_aux >= MAXOTEXT )
				  	printf ("\t(code %d)\n",(p->s_aux)-MAXOTEXT) ;
				  else
				  {
				  	printf ("\t(text %d)\n",p->s_aux) ;
					noobjs += p->s_aux ;
				  }
				  noo++ ;
				  break ;
			
			  default:
				  if ( p->s_mod == KEEP )
				  {
					  printf ("%-12.12s",p->s_nam) ;
					  printf ("\t%5d\n",p->s_val) ;
				  }
				  break ;
			}
		}
	}

	printf ("\n\n") ;

	if ( nov )
	{
		printf ("Number of verbs           -- %d\n",nov) ;
		printf ("Total number of code keys -- %d\n",noverb) ;
		printf ("Ave. number of code keys  -- %f\n",(float)noverb/nov) ;
		printf ("\n") ;
	}

	if ( nop )
	{
		printf ("Number of places          -- %d\n",nop) ;
		printf ("Total number of code keys -- %d\n",noplac) ;
		printf ("Ave. number of code keys  -- %f\n",(float)noplac/nop) ;
		printf ("\n") ;
	}

	if ( noo )
	{
		printf ("Number of objects         -- %d\n",noo) ;
		printf ("Total number of text keys -- %d\n",noobjs) ;
		printf ("Ave. number of text keys  -- %f\n",(float)noobjs/noo) ;
		printf ("\n") ;
	}
	return ;
}

#endif PSTAB

int wstab ()
{
	register int i ;
	register struct symstr *p ;
	register char *s ;
	int v, cnt, nxtsym ;

	(void) define ("<NREP>",nrep,KEEP) ;
	(void) define ("<NINIT>",ninit,KEEP) ;
	(void) define ("<NVARS>",nvars,KEEP) ;
	(void) define ("<NOBJ>",nobj,KEEP) ;
	(void) define ("<NPLACE>",nplace,KEEP) ;

	cnt = 0 ;
	clrcode () ;
	nxtsym = SYMTABREC +1 ;

	for ( i = 0 ; i < TABSIZ ; i++ )
	{
		for ( p = symtab[i] ; p != NIL ; p = p->s_nxt )
		{
			if ( p->s_mod == KEEP )
			{
				cnt++ ;
				v = p->s_val ;
				for ( s = p->s_nam ; *s ; )
					appcode (*s++) ;
				appcode (EOS) ;
				appcode (v) ;
				if ( ( cnt % SYMPERREC ) == 0 )
				{
					outcode (nxtsym) ;
					nxtsym++ ;
					clrcode () ;
				}
			}
		}
	}

	if ( ( cnt % SYMPERREC ) != 0 )
		outcode (nxtsym) ;
	
	clrcode () ;
	appcode (cnt) ;
	outcode (SYMTABREC) ;
	return ;
}
