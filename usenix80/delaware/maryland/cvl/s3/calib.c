int     scanner;  /* File descriptor for the scanner */

char	b1[258];
char	b2[258];
char	b3[258];
char	b4[258];

char	tbl[16] {0,17,34,51,68,85,102,119,136,153,170,187,204,221,238,255};
char	tbl1[8] {0,34,68,102,153,187,221,255};

main()
{
	register  i, j, k;

	for(i=0; i<254; i++) {
		b1[i] = 0374&i;
		b2[i] = 07&i? 0 : 0374;
	}

	for(i=0; i<16; i++) {
		for(j=0; j<16; j++) {
			k = i * 16 + j;
			b3[k] = tbl[i];
			b4[k] = tbl1[i/2];
		}
	}

	if((scanner = open("/dev/rdr0",1)) < 0) {
		write(2,"\n\n\07\tCan not open the scanout device.\n\tProgram stopped.\n\n",56);
		return;
	}

	pb(b1,47);
	pb(b2, 8);
	pb(b3,40);
	pb(b2, 8);
	pb(b4,48);
	pb(b2, 8);
	pb(b3,40);
	pb(b2, 8);
	pb(b1,47);

	close(scanner);

	return;
}

pb(buff, count)
char *buff;
int   count;
{
	register  i;

	for(i=0; i<count; i++)
		write(scanner, buff, 254);
}
