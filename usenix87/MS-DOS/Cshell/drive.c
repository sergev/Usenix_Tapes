#define NULL (void *)0
drive(argc,argv)
	char *argv[];
{
	char *dir,*getcwd();

	bdos(0xe,**argv - 'a');	/* select drive 0 */
	if (NULL == (dir = getcwd(NULL,64)))
		return -1;
	free(dir);
	return 0;
}
