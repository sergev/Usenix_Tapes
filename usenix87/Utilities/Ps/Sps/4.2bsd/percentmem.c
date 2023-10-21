# include       "sps.h"
# include       <h/text.h>
# ifdef BSD42
# include	<machine/pte.h>
# else
# include       <h/pte.h>
# include       <h/vmparam.h>
# endif
# include       <h/vmmac.h>

/* PERCENTMEM - Returns the percentage of real memory used by this process */
double  percentmem ( p )

register struct process         *p ;

{
	register struct text    *tp ;
	int                     szptudot ;
	double                  fracmem ;
	extern struct info      Info ;

	tp = p->pr_p.p_textp ;
	if ( !(p->pr_p.p_flag & SLOAD) || !tp )
		return ( 0.0 ) ;
	szptudot = UPAGES + clrnd( ctopt( p->pr_p.p_dsize + p->pr_p.p_ssize ) );
	fracmem = ( (double)p->pr_p.p_rssize + szptudot ) / CLSIZE ;
	if ( tp->x_ccount )
		fracmem += ((double)tp->x_rssize)/CLSIZE/tp->x_ccount ;
	return ( 100.0 * fracmem / (double)Info.i_ecmx ) ;
}
