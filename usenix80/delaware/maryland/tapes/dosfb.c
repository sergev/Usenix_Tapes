#
#define SIZE 512

char *ip;
char *lp;
char *op;
char ibuff[SIZE];
char obuff[SIZE];
char chksum 0;

main()
{
	register int len;

	lp = ip = ibuff;
	op = obuff;
	for(;;) {
		while(inchar() != 1); /* header */
		inchar();
		/* data length */
		len = (inchar() & 0377);
		len =+ (inchar() << 8);
		len =- 4;
		/* transfer data */
		if(len) while(len-- != 0) outchar(inchar());
		inchar(); /* checksum */
	}
}

inchar()
{
	register cnt;

	if(ip >= lp) {
		ip = ibuff;
		if((cnt = read(0, ip, sizeof ibuff)) <= 0) {
			if(op != obuff) write(1, obuff, op - obuff);
			exit(0);
		} else lp = &ip[cnt];
	}
	return(*ip++);
}

outchar(c)
char c;
{
	*op++ = c;
	if(op >= &obuff[SIZE]) {
		if(write(1, obuff, SIZE) != SIZE) exit(1);
		op = obuff;
	}
}
