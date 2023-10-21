#include	"tpscript.h"
#include	"hash.h"

/*
 * the non-acsii character names are now hashed and the elements of the
 * hash structure also contain information about whether this particular
 * character can be directly printed where troff thinks it is going
 * or whether it needs special massaging or even contruction from other
 * characters.
 */


HASH_ELEMENT	*hash_free = NOHASH,	/* points to next available entry */
		*hash_tab[HASH_SIZE];	/* starting entries */

hash_init()
{
	register	SPECIAL_NAME	*snp;
	register	HASH_ELEMENT	*hep, **hpp;
	register	int		i;
	char				*s;

	hash_free = (HASH_ELEMENT *)emalloc( ncharname * sizeof(HASH_ELEMENT));

	for( i = 0 ; i < ncharname ; i++ ) {
		s = &charname[ chartab[ i ] ];
		hpp = & hash_tab[ hash_it(s) ];
			/*
			 * skip through the entries already with this hash value
			 */
		while( *hpp != NOHASH )
			hpp = &( (*hpp)->hash_next );
		*hpp = hep = hash_free++;
		hep->hash_next = NOHASH;
		hep->hash_index = i;
		hep->hash_special = NOSPECIAL;	/* assume nothing special */
			/*
			 * now find if it is one of the special names
			 */
		for ( snp = &specnames[0] ; snp->troff_name != NULL ; snp++ ) {
			if ( strcmp( s, snp->troff_name ) == 0 ) {
				hep->hash_special = snp;
				break;
			}
		}
	}

}

dumphash( n )
	int n;
{
	register	int i;

	if ( n >= 0 && n < HASH_SIZE )
		dhsh( n );
	else
		for ( i = 0 ; i < HASH_SIZE ; i++ )
			dhsh( i );
}

dhsh( i )
	int	i;
{
	register	HASH_ELEMENT *hep;

	hep = hash_tab[i];
	fprintf( stderr, "hashed at %d:\n\t", i );
	while ( hep != NOHASH )
	{
		fprintf( stderr, "%s ",
			&( charname[ chartab[ hep->hash_index ] ] ));
		hep = hep->hash_next;
	}
	fprintf( stderr, "\n" );
}

	/*
	 * hashing function
	 */
hash_it( s )
	register	char	*s;
{
	register	int	i;

	i = 0;
	while( *s )
		i += *s++;
	return( i % HASH_SIZE );
}


putspec(specstr)
char	*specstr;
{
	register HASH_ELEMENT		*hep;
	register int			i = 0,
					n;
	register struct fontdesc	*fd;

	fd = tfp.fp_font;
	for ( hep = hash_tab[ hash_it( specstr ) ] ; hep != NOHASH ; hep = hep->hash_next )
	{
		if ( strcmp ( specstr, &charname[ chartab[ hep->hash_index ] ] ) == 0 )
			break;
	}
	if ( hep == NOHASH )
	{
		sprintf(errbuf, "special character '%s' unknown", specstr);
		error(ERR_WARN, errbuf);
		return;
	}

	i = hep->hash_index;
	i += NASCPRINT;			/* skip ordinary chars */
			/* check std font */
	if((n = fd->f_fitab[i] & BMASK) != 0)
		; 		/* ok - must be in current font */
				/* otherwise check special font */
	else if( (spcfnt1 != NOFONTDESC) && (n = spcfnt1->f_fitab[i] & BMASK) != 0)
	{
		fd = spcfnt1;
	}
				/* try the locally defined special font */
	else if( (spcfnt2 != NOFONTDESC) && (n = spcfnt2->f_fitab[i] & BMASK) != 0)
	{
		fd = spcfnt2;
	}
	else
	{
		sprintf(errbuf,
			"Special char '%s' not in current (%d) or special fonts\n",
			specstr, currtfont);
		error(ERR_WARN, errbuf);
		return;
	}
	width_pending += GETWIDTH( fd, n );

	if ( hep->hash_special != NOSPECIAL )
	{
		register	SPECIAL_NAME	*snp;

		setfont( FALSE );	/* ensure size is setup */
		CLOSEWORD();		/* may or may not have been done
					** in setfont()
					*/

		snp = hep->hash_special;
		if ( ( snp->sn_flags & SN_DEFINED ) == 0 )
		{
			snp->sn_flags |= SN_DEFINED;
			fprintf( postr, "\n/C%s { %s } def\n",
				snp->troff_name, snp->definition );
			if ( snp->sn_flags & SN_ANY_MULTIPLE )
				putmultdef( snp );
#ifdef notdef
			if ( snp->sn_flags & SN_FRACTION )
				putfractdef( snp );
			else if ( snp->sn_flags & SN_BRK_ROUNDING )
				putbrackdef( snp );
			else if ( snp->sn_flags & SN_BRACKET )
				putbrackdef( snp );
#endif
		}
		fprintf( postr, "\nC%s", snp->troff_name );
		return;
	}

	if( fd != tfp.fp_font )
	{
		struct fontdesc	*font;

		font = tfp.fp_font;
		tfp.fp_font = fd;
		putch(fd->f_codetab[n] & BMASK);
		tfp.fp_font = font;
	}
	else
	{
		putch(fd->f_codetab[n] & BMASK );
	}
}

	/*
	 * setup definition used in more than one special char
	 * - e.g. Cfrac used for \(12, \(14 etc
	 * first time through we scan through the special names list for
	 * a definition of the pseudo-name "fraction" which is used for the
	 * others
	 */
putmultdef( snp )
	register	SPECIAL_NAME *snp;
{
	register	SPECIAL_NAME *mnp;
	register	int	type;


	type = snp->sn_flags & SN_ANY_MULTIPLE;

	for ( mnp = &multdefs[0] ; mnp->troff_name != (char *)0 ; mnp++ ) {
		if ( type == (mnp->sn_flags & SN_ANY_MULTIPLE ) )
		{
			if ( ( mnp->sn_flags & SN_DEFINED ) == 0 )
			{
				fprintf( postr, "\n/C%s{%s}def",
						mnp->troff_name, mnp->definition);
				mnp->sn_flags |= SN_DEFINED;
			}
			break;
		}
	}
}


resetspcl()
{
	register	SPECIAL_NAME	*snp;

	for ( snp = &specnames[0] ; snp->troff_name != (char *)0 ; snp++ )
		snp->sn_flags &= ~SN_DEFINED;
	for ( snp = &multdefs[0] ; snp->troff_name != (char *)0 ; snp++ )
		snp->sn_flags &= ~SN_DEFINED;
}
