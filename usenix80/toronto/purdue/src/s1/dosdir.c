/*
 * dosdir - short directory listing of dos magtape
 */
char	*radtab "?abcdefghijklmnopqrstuvwxyz$./01234567890";
char	c1[4], c2[4], c3[4];
main(argc, argv)
char *argv[];
{
	int fd, eof, blocks, buf[512];
	int count;

	if (argc != 2) {
		printf("usage: dosdir tape-file\n");
		exit(1);
	}
	if ((fd=open(argv[1])) < 0)  {
		printf("Can't open %s\n", argv[1]);
	}
	for(;;) {
		if ((count=read(fd,&buf,512)) != 14)  {
			if (count == 0)
				if (eof++)
					exit(0);
			printf("First block not DOS header\n");
			exit(2);
		}
		unpak(buf[0],&c1);
		unpak(buf[1],&c2);
		unpak(buf[2],&c3);
		printf("%s%s.%s	",c1, c2, c3);
		blocks = eof = 0;
		while ((count=read(fd, &buf, 512)) > 0) {
			blocks++;
			eof = 0;
		}
		printf("%d\n", blocks);
		if (count < 0) {
			printf("read error\n");
			exit(3);
		}
		if (count == 0) 
			if (eof++)
				exit(0);
	}
}

unpak(i,ccp)
char	*ccp;
{
	register char	*cp;
	register	d,e;
	int	c;
	extern	ldivr;
	cp = ccp;
	d = ldiv(0,i,40);
	e = ldivr;
	c = ldiv(0,d,40);
	d = ldivr;
	if (c) *cp++ = radtab[c];
		else *cp++ = ' ';
	if (d) *cp++ = radtab[d];
		else *cp++ = ' ';
	if (e) *cp++ = radtab[e];
		else *cp++ = ' ';
	return(cp);
}
