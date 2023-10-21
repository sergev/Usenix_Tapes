168a
	/*
	 * The code here is a half-hearted
	 * attempt to do something with all
	 * of the 11/70 parity registers.
	 * In fact, there is little that
	 * can be done.
	 */
	case 10:
	case 10+USER:
		printf("parity\n");
		if(cputype == 70) {
			for(i=0; i<4; i++)
				printf("%o ", MEMORY->r[i]);
			MEMORY->r[2] = -1;
		}
		for ( i = 0;  i < nm7850s;  i++ )
			printf( "%o ", M7850BASE->r[i] );
		printf( "\n" );
		if ( dev & USER )  {
			i = SIGBUS;
			break;
		}
		panic("parity");

	/*
	 * Locations 0-2 specify this style trap, since
	 * DEC hardware often generates spurious
	 * traps through location 0.  This is a
	 * symptom of hardware problems and may
	 * represent a real interrupt that got
	 * sent to the wrong place.  Watch out
	 * for hangs on disk completion if this message appears.
	 */
	case 15:
	case 15+USER:
		printf("Random interrupt ignored\n");
		return;
	}
	psignal(u.u_procp, i);

.
166,167d
45a
	extern int nm7850s;
.
33c
 * Called from l.s when a processor trap occurs.
.
13a
#define	MEMORY	0177740		/* 11/70 "memory" subsystem */
#define	M7850BASE	0172100	/* first M7850 controller CSR */
.
1a
/*
 * trap.c - trap handler
 *
 *	modified 04-Jun-1980 by D A Gwyn:
 *	1) installed Ken's mods (trap via 0 & 11/70 parity);
 *	2) added MOS parity controller display.
 */
.
w
q
