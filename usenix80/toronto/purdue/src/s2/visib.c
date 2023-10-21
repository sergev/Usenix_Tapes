#
/*
#

/*
 *	vconv -- make invisible ascii characters visible
 *
 *	jjb 
 */

#define BS	010
#define ESC	033

#define BSIZE	512
#define THRESH	450
int nflg	0;


main(argc,argp)
char **argp;
{
	extern int fout;
	static char ibuf[BSIZE];
	register char *ip;
	registerint addr,cnt;

	if(argc > 1)
		if(argp[1][0] == '-' && argp[1][1] == 'n')
			nflg++;
	fout = dup(1);
	addr = 0;
	while((cnt = read(0,ibuf,BSIZE)) > 0){
		ip = ibuf;
		while(cnt-- > 0){
			if((addr % 16) == 0 && nflg){
				printf("\n%6o\t",addr);
			}
			vconv(*ip++);
			addr++;
		}
	}
	putchar('\n');
	flush();
}

vconv(c)
char c;
{
	if(c >= ' '  &&  c <= '~'){
		putchar(c);
		return;
	}
	putchar('\\');
	switch(c){
	case '\n':
		putchar('n');
		if(!nflg)
			putchar('\n');
		break;

	case '\r':
		putchar('r');
		break;

	case '\t':
		putchar('t');
		break;

	case BS:
		putchar('b');
		break;

	case ESC:
		putchar('e');
		break;

	default:
		putchar('0');
		if(c & 0370){
			if(c & 0300)
				putchar(((c>>6)&03) + '0');
			putchar(((c>>3)&07) + '0');
		}
		if(c != 0)
			putchar((c&07) + '0');
	}
}

