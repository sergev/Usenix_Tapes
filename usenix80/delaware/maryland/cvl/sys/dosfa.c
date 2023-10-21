char    buff[512];
int flag;

main()
{
	register i, j, c;

	flag = 0;
	while((i = read(0, buff, 512)) > 0) for(j = 0; j < i; j++) {
		c = buff[j] & 0177;
		if(flag && c != '\n' && c != 0)
			putchar('\r');
		if(c == '\r') {
			flag++;
			continue;
		}
		if(c != 0) {
			putchar(c);
			flag = 0;
		}
	}
	flush();
}
