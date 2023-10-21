/*
 *  Fast, sleazy, and ugly zero function.
 *
 *  Note that this will only work on a VAX, but it is real easy to write a
 *  similar function for whatever machine you may need.  If nothing else,
 *  just a simple loop in C will suffice.
 *
 *  Dave Johnson, Rice University.
 *
 *  Enhanced by William LeFebvre of Rice University to handle zeroing more
 *  than 64K.
 */

# define   K	1024

/*
 *  bzero(memory, amount) - set "amount" bytes starting at "memory" to the
 *			    value 0.
 */

bzero(memory, amount)

char *memory;
int  amount;

{
    while (amount >= 64*K)
    {
	_bzero64(memory, 64*K-1);
	memory += 64*K-1;
	amount -= 64*K-1;
    }
    _bzero64(memory, amount);
}

_bzero64(memory, amount)

char *memory;
int  amount;

{
    asm("	movc5	$0, (sp), $0, 8(ap), *4(ap)");
}
