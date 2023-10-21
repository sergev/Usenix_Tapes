88,89d
75c

	/*
	 * enable all existing M7850 memory parity controllers
	 */

	UISA->r[7] = ka6[1]; 		/* io segment */
	UISD->r[7] = 077406;
	for ( ;  nm7850s < MAX7850S;  nm7850s++ )
		if ( suiword( M7850BASE+2*nm7850s, 1 ) < 0 )
			break;		/* that's all of them */

	printf( "mem = %l, %d pcs\n", maxmem*5/16, nm7850s );
.
38a
 *	enable parity controllers
.
31a
int	nm7850s	0;			/* set up by "main" */

.
11a
#define	M7850BASE	0172100		/* min M7850 CSR address */
#define MAX7850S	16		/* max possible M7850 modules */

.
1a
/*
 * main.c - Unix initialization (kernel mode)
 *
 *	modified 04-Jun-1980 by D A Gwyn:
 *	1) enable M7850 parity controllers, if any.
 */
.
w
q
