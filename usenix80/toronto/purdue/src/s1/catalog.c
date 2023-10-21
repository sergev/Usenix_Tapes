#
/*
 *	catalog -- catalog a mag tape
 *
 *	syntax:  catalog <tape>  <options>
 *
 *	<tape>: "0" or "1"... or "/dev/rmt1", etc.
 */

#define bufsize 26000

char	buf[bufsize];
char	*tape	"/dev/rmt1";	/* device to catalog */
int	seven	0;		/* read7 mode selected */
int	maxnum	20;		/* max files to catalog */
int	skip	0;		/* skip count */

main(argc,argp)
char **argp;
{
	int fd,count,size,lastsize,filenum;
	float xxx;

	while(--argc){
		if(**++argp == '-')
			switch(*++*argp){
			case '7':
				seven++; break;
			case 'r':
			/*	rewind(fd); break;	*/
			case 'n':
				maxnum = atoi(++*argp); break;
			case 's':
				skip = atoi(++*argp); break;
			}
		else
			if(**argp >= '0' && **argp < '7')
				tape[8] = **argp;
			else 
				tape = *argp;
	}
	if((fd = open(tape,0)) < 0){
		printf("can't open %s\n",tape);
		exit(1);
	}

	printf("\ncatalog of %s%s\n  size   count\n",tape,
		(seven?"  (read7 mode)":""));
	filenum = 1;
	count = 0;
	lastsize = -2;	/* hopefully an impossible value */
	while(1) {
		while((size=read(fd,buf,bufsize))>0 && size==lastsize)
			count++;
		if(count)
			printf("  %6d\n",count);
		switch(size){
		case -1:
			printf(" read error terminates file # %d\n\n",filenum);
			exit(10);
		case 0:
			printf("   end of file # %d\n\n",filenum++);
			if(filenum>maxnum && maxnum > 0)
				exit(0);
			count = 0;
			lastsize = -2;
			break;
		default:
			lastsize = size;
			count = 1;
			xxx = size;
			if(seven)
				size = xxx * 0.750;
			printf("%6d",size);
		}
	}
}
