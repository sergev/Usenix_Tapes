#
/* tail command 
 *
 *	tail where [file]
 *	where is +_n[type]
 *	- means n lines before end
 *	+ means nth line from beginning
 *	type 'b' means tail n blocks, not lines
 *	type 'c' means tail n characters
*/
#define LBIN 4097
#define ESPIPE 29
struct { long longint; };
char bin[LBIN];
int *buf bin;
int errno;

main(argc,argv)
char **argv;
{
	long n,di;
	int fromend;
	register i,j,k;
	char *p;
	int partial,piped,bylines;
	char *arg;
	seek(0,0,1);
	piped = errno==ESPIPE;
	arg = argv[1];
	if(argc<=1 || *arg!='-'&&*arg!='+') {
		arg = "-10l";
		argc++;
		argv--;
	}
	fromend = *arg=='-';
	arg++;
	if(!digit(*arg))
		goto errcom;
	n = 0;
	while(digit(*arg))
		n = n*10 + *arg++ - '0';
	if(!fromend&&n>0)
		n--;
	if(argc>2) {
		close(0);
		if(open(argv[2],0)!=0) {
			write(2,"tail: can't open input\n",23);
			exit(1);
		}
	}
	bylines = 0;
	switch(*arg) {
	case 'b':
		n =<< 9;
		break;
	case 'c':
		break;
	case '\0':
	case 'l':
		bylines = 1;
		break;
	default:
		goto errcom;
	}
	if(fromend)
		goto keep;

			/*seek from beginning */

	if(bylines) {
		j = 0;
		while(n-->0) {
			do {
				if(j--<=0) {
					p = bin;
					j = read(0,p,512);
					if(j--<=0) exit(0);
				}
			} while(*p++ != '\n');
		}
		write(1,p,j);
	} else  if(n>0) {
		if(!piped)
			fstat(0,buf);
		if(piped||(buf[2]&060000)==020000)
			while(n>0) {
				i = n>512?512:n;
				i = read(0,bin,i);
				if(i<=0) exit(0);
				n =- i;
			}
		else
			lseek(0,n,0);
	}
	while((i=read(0,bin,512))>0)
		write(1,bin,i);
	exit(0);

			/*seek from end*/

keep:
	if(n<=0) exit(0);
	if(!piped) {
		fstat(0,bin);
		bin[8] = 0;
		di = !bylines&&n<LBIN?n:LBIN-1;
		if(bin[8].longint>di)
			lseek(0,-di,2);
	}
	partial = 1;
	for(;;) {
		i = 0;
		do {
			j = read(0,&bin[i],LBIN-i);
			if(j<=0)
				goto brka;
			i =+ j;
		} while(i<LBIN);
		partial = 0;
	}
brka:
	if(!bylines) {
		k =
		    n<=i ? i-n:
		    partial ? 0:
		    n>=LBIN ? i+1:
		    i-n+LBIN;
		k--;
	} else {
		k = i;
		j = 0;
		do {
			do {
				if(--k<0) {
					if(partial)
						goto brkb;
					k = LBIN -1;
				}
			} while(bin[k]!='\n'&&k!=i);
		} while(j++<n&&k!=i);
brkb:
		if(k==i) do {
			if(++k>=LBIN)
				k = 0;
		} while(bin[k]!='\n'&&k!=i);
	}
	if(k<i)
		write(1,&bin[k+1],i-k-1);
	else {
		write(1,&bin[k+1],LBIN-k-1);
		write(1,bin,i);
	}
	exit(0);
errcom:
	write(2,"usage: tail +_n[lbc] [file]\n",29);
	exit(1);
}

digit(c)
{
	return(c>='0'&&c<='9');
}
