#include <osfcn.h>
#include "DateDict.h"
#include "Date.h"
#include "String.h"

main()
{
	DateDict d;
	d.addName(*new Date(11,"Nov",48), *new String("Smith, Mary"));
	d.addName(*new Date(11,"Nov",48), *new String("Doe, John"));
	d.addName(*new Date(10,"May",47), *new String("Jones, Bill"));
	ostream out(creat("datedict.dat",0664));
	d.storeOn(out);
}
