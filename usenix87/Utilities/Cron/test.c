/* test.c - a test jig for bitstring
 * vix 25mar87 [all test programs are messy]
 */

#include <stdio.h>
#include "bitstring.h"

#define SIZE 50

main()
{
	char t[10];
	bit_decl(string, SIZE)

	while (test(string))
		;
}

static test(s)
	bit_ref(s)
{
	int i;
	char t[10], cmd;

	for (i = 0;  i < SIZE;  i++)
		putchar(bit_test(s, i) ? '1' : '0');
	putchar('\n');

	printf("set, clear, Setall, Clearall: "); fflush(stdout);
	gets(t); if (!t[0]) return 0; else cmd=t[0];
	if (cmd=='s' || cmd=='c')
		{ printf("\t#"); fflush(stdout); gets(t); i=atoi(t); }

	switch (cmd)
	{
	case 's':	bit_set(s, i); break;
	case 'c':	bit_clear(s, i); break;
	case 'S':	bit_setall(s, SIZE); break;
	case 'C':	bit_clearall(s, SIZE); break;
	default:	return 0;
	}
	return 1;
}
