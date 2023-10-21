/*
 * "timer.h", zie timer(3).
 */

#ifndef		INCL_TIMER
#define		INCL_TIMER

typedef double	TIMER;		/* een timer */

/*
 * Waarde van time_left (timer (NO_TIME)).
 * Dit is in de buurt van max double (implementatie afhankelijk).
 */
#define		NO_TIME		((double) 1.7e+38)

extern TIMER timer (/*double*/);
extern double time_left (/*TIMER*/);

extern set_tvec (/*TIMER [], unsigned, double*/);
extern unsigned tvec_timeout (/*TIMER [], unsigned, unsigned*/);
extern unsigned tvec_oldest (/*TIMER [], unsigned*/);

#endif
