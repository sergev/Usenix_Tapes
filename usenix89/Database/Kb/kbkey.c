
#include <stdio.h>
extern long time();

main()
{ char buf [30];
 int i;
 sprintf(buf,"%ld",time(0));
 for (i = strlen(buf)-1 ; i > -1 ; i--) putchar(buf[i]);
 putchar('\n');
}
