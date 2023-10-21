/*
 * Add a newsgroup onto the end of .newsrc.
 */

#include <stdio.h>
#include "defs.h"
#include "newsrc.h"


addrc(ngp)
	struct ngentry *ngp;
	{
	if (lastng != NULL) {
		lastng->ng_next = ngp;
		lastng = ngp;
	} else {
		lastng = firstng = ngp;
	}
}
