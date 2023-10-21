#include <stdio.h>
#include "match.h"
#include "Extern.h"
extern char * malloc();
/* makes a pattern descriptor */
struct PattDesc *MakeDesc(Pattern)
char *Pattern;
{
	struct PattDesc *Desc;
	Desc = (struct PattDesc *) malloc(sizeof(struct PattDesc));
	Desc->Pattern=Pattern;
	Desc->PatLen = strlen(Desc->Pattern);
	return(Desc);
} /* main */
