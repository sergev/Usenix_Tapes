# include       "sps.h"
# include       "flags.h"
# ifdef BSD42
# include       <sys/file.h>
# endif
# include       <nlist.h>
# include       <stdio.h>

/* INITSYMBOLS - Reads kmem values into the Info structure */
/*
** THIS CODE COPIES KMEM VALUES INTO THE INFO STRUCTURE ASSUMING THAT
** VALUES READ FROM THE KERNEL HAVE TYPE CADDR_T. THEREFORE, WE ARE
** MAKING THE DUBIOUS ASSUMPTION THAT INTS, POINTERS AND CADDR_T's
** HAVE IDENTICAL SIZES.
*/
initsymbols ()
{
	register struct nlist   *np ;
	register struct symbol  *s ;
	register struct nlist   *np0 ;
	char                    *filesymbol ;
	extern int              Flkmem ;
	extern struct flags     Flg ;
	extern struct symbol    Symbollist[] ;
	extern struct info      Info ;
	char                    *getcore() ;
	char                    *strncpy() ;

	filesymbol = Flg.flg_s ? Flg.flg_s : FILE_SYMBOL ;
	/* Find the length of the symbol table */
	for ( s = Symbollist ; s->s_kname ; s++ )
		;
	/* Construct an nlist structure by copying names from the symbol table*/
	np0 = (struct nlist*)getcore( (s-Symbollist+1)*sizeof( struct nlist ) );
	for ( s = Symbollist, np = np0 ; s->s_kname ; s++, np++ )
	{                                       
		np->n_name = s->s_kname ;       
		np[1].n_name = (char*)0 ;       
		np->n_value = 0 ;
	}
# ifdef BSD42
	if ( access( filesymbol, R_OK ) < 0 )
# else
	if ( access( filesymbol, 4 ) < 0 )
# endif
	{
		fprintf( stderr, "sps - Can't open symbol file %s", filesymbol);
		sysperror() ;
	}
	/* Get kernel addresses */
	nlist( filesymbol, np0 ) ;              
	if ( np0[0].n_value == -1 )
	{
		fprintf( stderr, "sps - Can't read symbol file %s", filesymbol);
		sysperror() ;
	}
	for ( s = Symbollist, np = np0 ; s->s_kname ; s++, np++ )
	{                                       
		if ( !np->n_value )             
		{
			fprintf( stderr, "sps - Can't find symbol %s in %s",
				np->n_name, filesymbol ) ;
			/* Assume this error to be unimportant if the address
			   is only associated with a process wait state.
			   This may happen if the system has been configured
			   without a particular device. */
			fprintf( stderr, &Info.i_waitstate[ 0 ] <= s->s_info
				&& s->s_info < &Info.i_waitstate[ NWAITSTATE ]
				? " (error is not serious)\n"
				: " (ERROR MAY BE SERIOUS)\n" ) ;
			*s->s_info = (caddr_t)0 ;
			continue ;
		}
		/* If no indirection is required, just copy the obtained value
		   into the `Info' structure. */
		if ( !s->s_indirect )           
		{                               
		/* DUBIOUS ASSUMPTION THAT KMEM VALUE HAS SIZE OF A CADDR_T */
			*s->s_info = (caddr_t)np->n_value ;
			continue ;              
		}                               
		/* Otherwise one level of indirection is required. Using the
		   obtained address, look again in the kernel for the value */
		memseek( Flkmem, (long)np->n_value ) ;
		/* DUBIOUS ASSUMPTION THAT KMEM VALUE HAS SIZE OF A CADDR_T */
		(void)read( Flkmem, (char*)s->s_info, sizeof(caddr_t) ) ;
	}
	free( (char*)np0 ) ;
}
