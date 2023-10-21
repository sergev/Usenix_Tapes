#include "errors.h"
#include "errlib.h"
#include "testerrs.h"

main() {
	OOPS__errinit();

	seterror(OOPS__FIRSTERROR,WARNING);
	seterror(OOPS__LAST_ERROR,FATAL,0,1,0);
}
