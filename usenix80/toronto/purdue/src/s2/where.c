main(argc, argv)
	int argc;
	char **argv;
{
	char buffer[16], look[8], chr, name[8];
	int fd, a, eql, found, numarg;
	register i;
	if(argc == 1) {
		printf("Usage is:");
		printf(" where <login id> {login id} {login id}......\n");
		exit(1);
	}
	fd = open("/etc/utmp", 0);
	for(numarg = 1; numarg < argc; numarg++) {
		for(i = 0; i != 8; i++)
			look[i] = ' ';
		for(i = 0; argv[numarg][i] != '\0'; i++) {
			if(i < 8)
				look[i] = argv[numarg][i];
		}
		if(i > 8) {
			printf("invalid user name\n");
			exit(1);
		}
		seek(fd, 0, 0);
		found = 0;
		while(1) {
			a = read(fd, buffer, 16);
			if(a == 0) {
				if(!found) {
					i = 0;
					while(look[i] != ' ' && i != 8)
						putchar(look[i++]);
					printf(" not logged in\n");
				}
				break;
			}
			for(i = 0; i != 8; i++)
				name[i] = ' ';
			for(i = 0; i != 8; i++) {
				chr = buffer[i];
				if(chr == ' ')
					break;
				name[i] = chr;
			}
			eql = 1;
			for(i = 0; i != 8; i++)
				eql = eql&(name[i] == look[i]);
			if(eql == 1) {
				i = 0;
				while(i != 8 && look[i] != ' ')
					putchar(look[i++]);
				printf(" is at tty%c\n", buffer[8]);
				found++;
			}
		}
	}
}
