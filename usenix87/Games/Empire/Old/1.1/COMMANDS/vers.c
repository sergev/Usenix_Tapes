#include        "empdef.h"

char    Version[] = "~@(#)Version 1.1 Old Empire\nOct 1985\n (decompiled 1983 from PDP 11/70 object)";

vers()
{

        printf("%s\n(Probably out of date by now.)", &Version[5]);
        return(NORM_RETURN);
}
