#include "dca2troff.h"

do_multi()
{
    char c;

    mb_class = get1ch();
    mb_count = get1ch();
    mb_type = get1ch();

    mb_count &= 0377;		/* count is a 1 byte value */
    mb_count = mb_count - 2;		/* count includes count and type */

    switch (mb_class)
	{
	case 0xd1:
		do_mbd1();
		return;
	case 0xd2:
		do_mbd2();
		return;
	case 0xd4:
		do_mbd4();
		return;
	case 0xd8:
		do_mbd8();
		return;
	case 0xd9:
		do_mbd9();
		return;
	default:
		fprintf(stderr, "unknown mb_class ( %02x )\n", mb_class);
		do_flush(mb_count);
		return;
	}
}
