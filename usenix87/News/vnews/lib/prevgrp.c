#include <stdio.h>
#include "defs.h"
#include "newsrc.h"


struct ngentry *
prevgrp(ngp)
	struct ngentry *ngp;
	{
	register struct ngentry *p;

	if (ngp == NULL)
		return NULL;
	for (p = ngtable ; p->ng_next != ngp ; p++)
		if (p >= ngtable + MAXGROUPS - 1)
			return NULL;
	return p;
}
