/*
 *  cavn - compare argv versus name
 */

cavn (argc, argv, foundlist, name)
int     argc;
char  **argv;
int    *foundlist;
char   *name;
{
    int     retval = 0;

    while (argc-- > 0) {
	if (!strcmp (*argv, name)) {
	    (*foundlist)++;
	    retval++;
	}
	argv++;
	foundlist++;
    }
    return (retval);
}
