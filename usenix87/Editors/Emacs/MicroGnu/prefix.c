/*
 * This code is a kludge for the two prefix sequences from GNU (well,
 * the two I want) that uemacs doesn't have. Rather than quadruple the table
 * space for keys, plus have to test for them everywhere, I'll just kludge
 * it with functions that are bound to those keys. Later, maybe I'll do
 * prefixes right.
 */

#include "def.h"

/*ARGSUSED*/
ctlx4hack(f, n, k) {
	register KEY	c;

	if ((c = getkey(KPROMPT)) == 'b' || c == 'B')
		return poptobuffer(f, n, KRANDOM);
	if (c == 'f' || c == (KCTRL|'F') || c == 'F')
		return poptofile(f, n, KRANDOM);
	
	if (c == (KCTRL|'G') || c == (KCTLX|KCTRL|'G')
	||  c == (KMETA|KCTRL|'G')) {
		(VOID) ctrlg(FALSE, 1, KRANDOM);
		return ABORT;
	}
	return FALSE;
}

/*ARGSUSED*/
help(f, n, k) {
	register KEY	c;

	c = getkey(KPROMPT);
	while (c == (KCTRL|'H')) {
		ewprintf("B C: ");
		c = getkey(0);
	}
	if (c == 'b' || c == 'B') return wallchart(f, n, KRANDOM);
	if (c == 'c' || c == 'C') return desckey(f, n, KRANDOM);

	if (c == (KCTRL|'G') || c == (KCTLX|KCTRL|'G')
	||  c == (KMETA|KCTRL|'G')) {
		(VOID) ctrlg(FALSE, 1, KRANDOM);
		return ABORT;
	}
	return FALSE;
}
