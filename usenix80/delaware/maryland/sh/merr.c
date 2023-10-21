#

#define E2BIG   7
#define ENOEXEC 8

char    *shnam "/bin/sh";
char    *dummy[] { "-", 0};
int     old;
int     new;
int     errno;
char    *command;
char    buffer[256] {'/', 'u', 's', 'r', '/', 'b', 'i', 'n', '/'};


main(argc, argv)
int argc;
char *argv[];
{
	register char *o, *n;
	int j;

	argv[argc] = 0;
	if ((old = dup(2)) < 3 || (old = dup(1)) < 3 || (old = dup(2)) < 3
	    || (old = dup(1)) < 3 || (old = dup(2)) < 3
	    || (old = dup(1)) < 3) {
		dup(0);
		dup(0);
		old = dup(0);
	}
	if (argc < 2) {
		new = 1;
		command = shnam;
		argv = dummy;
	} else {
		if (argv[0][0] == 'm')
			new = 1;
		else {
			argv++;
			if (argv[0][0] == '-')
				new = ((argv[0][1] == 0)? 1:
					creat(&argv[0][1], 0664));
			else {
				if ((new = open(argv[0], 1)) > 0)
					seek(new, 0, 2);
				else
					new = creat(argv[0], 0664);
			}
		}
		argv++;
		command = argv[0];
	}
	if(new != 2) {
		if(old == 2) old = dup(2);
		dup(new);
		close(2);
		if(dup(new) != 2) {
			if(dup(new) != 2) {
				write(old, "Cannot make error file\n", 23);
				exit(1);
			}
		}
	}
	for(j = 3; j < 10; j++) close(j);
	execute(command, argv);
	if(argv[0][0] == '/') {
		write(2, "command not found\n", 18);
		exit(1);
	}
	o = command;
	n = &buffer[9];
	while(*n++ = *o++) if(n >= &buffer[255]) {
		write(2, "Command too long\n", 17);
		exit(1);
	}
	execute(&buffer[4], argv);
	execute(&buffer[0], argv);
	write(2, "Command not found\n", 18);
	return(1);
}


execute(afile, aarg)
char *afile;
char **aarg;
{
	register char *file, **arg;

	arg = aarg;
	file = afile;
	execv(file, arg);
	if (errno==ENOEXEC) {
		arg[0] = file;
		*--arg = shnam;
		execv(*arg, arg);
	}
	if (errno==E2BIG)
		toolong();
}

toolong()
{
	write(2, "Arg list too long\n", 18);
	exit(1);
}

