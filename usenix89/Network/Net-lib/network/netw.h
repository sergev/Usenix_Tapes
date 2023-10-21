/*
 * $Header: netw.h,v 1.1 87/04/15 21:29:45 josh Exp $
 *
 * $Log:	netw.h,v $
 * Revision 1.1  87/04/15  21:29:45  josh
 * Initial revision
 * 
 */


#include <sys/ioctl.h>

typedef struct {
        struct sgttyb   sgttyb;
        struct tchars   tchars;
        struct ltchars  ltchars;
        int             l, lb;
	} TCOND;

#define loadtty(x) _loadtty(&x)

#define stty(x,y,z) _stty(&x,y,z)

#define COOKED 0
#define BAUD 1

#ifndef FALSE
#define FALSE 0
#endif FALSE

#ifndef TRUE
#define TRUE 1
#endif TRUE
