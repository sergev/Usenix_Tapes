/* oopsinit.c -- OOPS initialization

	THIS SOFTWARE FITS THE DESCRIPTION IN THE U.S. COPYRIGHT ACT OF A
	"UNITED STATES GOVERNMENT WORK".  IT WAS WRITTEN AS A PART OF THE
	AUTHOR'S OFFICIAL DUTIES AS A GOVERNMENT EMPLOYEE.  THIS MEANS IT
	CANNOT BE COPYRIGHTED.  THIS SOFTWARE IS FREELY AVAILABLE TO THE
	PUBLIC FOR USE WITHOUT A COPYRIGHT NOTICE, AND THERE ARE NO
	RESTRICTIONS ON ITS USE, NOW OR SUBSEQUENTLY.

Author:
	K. E. Gorlen
	Bg. 12A, Rm. 2017
	Computer Systems Laboratory
	Division of Computer Research and Technology
	National Institutes of Health
	Bethesda, Maryland 20892
	Phone: (301) 496-5363
	uucp: uunet!ncifcrf.gov!nih-csl!keith
	Internet: keith%nih-csl@ncifcrf.gov

Function:
	
Initializes OOPS

$Log:	oopsinit.c,v $
 * Revision 1.2  88/01/16  23:42:07  keith
 * Remove pre-RCS modification history
 * 
*/
#include "Object.h"
#include <errlib.h>

static char rcsid[] = "$Header: oopsinit.c,v 1.2 88/01/16 23:42:07 keith Exp $";

// OOPS Tables

unsigned char	char_bit_mask[sizeof(char)*8];
unsigned short	short_bit_mask[sizeof(short)*8];
unsigned int	int_bit_mask[sizeof(int)*8];
unsigned char bit_count[256];
unsigned char bit_reverse[256];
	
static void InitTables()
{
	register unsigned i,j;
	
	for (i=0, j=1; i<sizeof(char)*8; i++, j <<= 1) char_bit_mask[i] = j;
	for (i=0, j=1; i<sizeof(short)*8; i++, j <<= 1) short_bit_mask[i] = j;
	for (i=0, j=1; i<sizeof(int)*8; i++, j <<= 1) int_bit_mask[i] = j;

	for (i=0; i<256; i++) {
		bit_count[i] = 0;
		j = i;
		while (j != 0) {
			bit_count[i]++;
			j &= j-1;
		}
	}
	
	for (i=0; i<256; i++) {
		bit_reverse[i] = 0;
		j = i;
		register unsigned char m = 0x80;
		while (j != 0) {
			if ((j&1) != 0) bit_reverse[i] |= m;
			j >>= 1;
			m >>= 1;
		}
	}
}

extern void OOPS__errinit();	// error facility initializer for OOPS
extern void OOPS__classInit();	// initialize OOPS classes 

bool OOPS__Initialized;		// YES if OOPS initialization completed

void OOPS_init()		// OOPS initialization, called once from _main 
				// AFTER execution of all static constructors
{
	OOPS__errinit();	// initialize the OOPS error handler 
	seterropt(ERROR,WARNING,NO,3,NULL);
	InitTables();		// initialize OOPS tables
	OOPS__classInit();	// initialize OOPS classes
	OOPS__Initialized = YES;
}
