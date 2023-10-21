/* THIS PROGRAM MUST HAVE 04750 PERMISSIONS, AND BE OWNED BY  */
/* USER ROOT AND THAT GROUP WHICH IS TO BE ALLOWED TO USE IT. */

main (argc, argv)
	int	argc;
	char	*argv[];
{
	setgid (5);
	setuid (0);
/*	nice (-4);	de-comment only if you want to be rude/nasty */
	execv ("/bin/csh", argv);
}
