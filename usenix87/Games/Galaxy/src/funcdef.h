/*
 * %W% (mrdch&amnnon) %G%
 */

/*
 * global macros.
 */

#define skipwhite(s)    doskipwhite(&s)
#define skipblack(s)    doskipblack(&s)
#define skipword(s)     doskipword(&s)
#define skipnum(s)      doskipnum(&s)
#define getpl(s)        dogetpl(&s)

#define ishex(a)      ((a >= '0' && a <= '9')|| (a >= 'a' && a <= 'f'))
#define isnum(a)      ((a >= '0' && a <= '9'))

#define TWOZERO { 0, 0 }
