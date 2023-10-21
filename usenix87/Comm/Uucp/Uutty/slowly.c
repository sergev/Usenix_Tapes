#include "uutty.h"
/* 
** This routine handles requests to do things slowly.
** Note the two ways of delaying:  sleeping for "slow"
** seconds, or counting down from "count" to 1.
*/
slowly()
{	uint u;

	D8("slowly() slow=%d count=%d",slow,count);
	if (slow > 0) {
		D8("slowly: slow=%d",slow);
		sleep(slow);
	}
	for (u=count; u; u--) ;
	D8("slowly() done");
}
