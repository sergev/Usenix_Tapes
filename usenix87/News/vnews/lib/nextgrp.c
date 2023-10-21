#include <stdio.h>
#include "defs.h"
#include "newsrc.h"


struct ngentry *
nextgrp(ngp)
	struct ngentry *ngp;
	{
	if (ngp == NULL)
		return firstng;
	else
		return ngp->ng_next;
}
