#include "stdplt.h"
main()
{
	frame("junk");
	move(0., .8);
	cwidth(.04, "/");
	textf("abcdefghijklmnopqrstuvwxyz\n");
	textf("ABCDEFGHIJKLMNOPQRSTUVWXYZ\n");
	textf("1234567890!\"#$%&'()=-~^|\\}]{[`@_+;*:<,>.?/");
}
