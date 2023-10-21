#include <stdio.h>
main () {
    char    buff[255];
    int     count;

    count = 1;

    while (gets (buff) != NULL)
	printf ("#define %s LONG%d\n", buff, count++);
}
