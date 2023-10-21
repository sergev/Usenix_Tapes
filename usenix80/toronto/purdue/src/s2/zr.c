main(argc, argv)
char **argv;
{
	int fildes;

	while (--argc) {
		if ((fildes = creat(*++argv, 0604)) < 0) {
			write(2, "Can't create ", 13);
			while (**argv)
				write(2, (*argv)++, 1);
			write(2, "\n", 1);
		}
		else
			if (fildes >= 0) {
				close(fildes);
				fildes = 0;
			}
	}
}
