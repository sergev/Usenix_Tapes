	extern	fin,fout;
main(argc,argv)
	int argc;	char **argv;
{
	register char c;
	register i,j;
	int	fd;
	char *p,*k;
	char	card[100];
	char	line[100];
	char    temp;
	char    ts[100];
	int	whopay;
	int	npaper,ntrans;
	int	found;	/* if found account in acct_data */

loop:
	flush();
	printf("\nI am in loop.\n");
	found = 1;
	if(((c = getchar()) != 'R') && (c != 'V'))goto loop;

	p = card;
	while((*p++ = getchar()) != '\n');
	*p = '\0';
	i = 85;
	temp = card[i + 3];
	card[i + 3] = '\0';
	ntrans = basin(card + i,10);
	card[i + 3] = temp;
	temp = card[i];
	card[i] = '\0';
	npaper = basin(card + (i - 3),10);
	card[i] = temp;
	whopay = (card[i - 4]) - '0';

	printf("np, nt = %d,%d\n",npaper,ntrans);       flush();

	if(c == 'R')goto RECORD;
/* verification routine	*/

	if((fd = open("ding_list",0)) == -1){
		printf("Can't open 'ding_list'\n");
		goto loop;
		}
	card[10] = '\0';
	fin = fd;
	while(line[0] = getchar()){
	   k = line;
	   k++;
	   for(i = 1;i != 10;i++)*k++ = getchar(); /* get name */
	   while(getchar() != '\n');
	   *k = '\0';
	     if(comstr(line,card)){      /* if have correct account */
		putchar('N');
		found = 0;
		}
	   }
	if(found)putchar('Y');
	close(fd);
	flush();
	fin = 0;
	goto loop;


RECORD:
	if((fd = open("acct_data",0)) == -1){
		printf("Can't open 'acct_data'\n");
		goto loop;
		}
	k = ts;	p = card;
	for(i = 0;i != 10;i++)*k++ = *p++;
	*k = 0;
	if((i = creat("acct_tmp",0600)) == -1){
		close(fd);
		printf("Can't create 'acct_tmp'\n");
		goto loop;
		}
	fin = fd;       fout = i;
	while((line[0] = getchar()) != '\0'){
	    putchar(line[0]);
	    k = line;	k++;
	    for(i = 1;i != 10;i++)*k++ = putchar(getchar());
	    cpline();
	    *k = '\0';
	    if(comstr(ts,line)){		/* if found account */
		cpline();
		i = whopay;
		while(--i)cpline();
		p = line;
		while(((c = putchar(getchar())) != ' ') && (c != '\t'));
		scanin(ts);
		basout((basin(ts,10) + npaper),10);
		putchar('\t');
		scanin(ts);
		basout((basin(ts,10) + ntrans),10);
		putchar('\n');
		i = 4-whopay;
		while(i--)cpline();
		found = 0;
		}
	      else {
		for(i = 0;i != 5;i++)cpline();
		}
	    }
	while(getchar());
	close(fd);
	flush();
	fin = 0;
	if(fout)close(fout);
	flush();
	fout = 1;

	if(found == 0){
		shell("mv acct_tmp acct_data");
		}
	    else shell("rm acct_tmp");

	if(found){
		if((i = creat("acct_tmp",0600)) == -1){
			printf("Can't create 'acct_tmp'.\n");
			goto loop;
			}
		if((fd = open("acct_lost",0)) == -1){
			if((fd = creat("acct_lost",0600)) == -1){
				printf("Can't open or make 'acct_lost'\n");
				goto loop;
				}
				fout = i;
				goto mumble;
			}
		fin = fd;       fout = i;
		while(c = getchar()){
			putchar(c);
			       }
mumble:         printf("%s",card);
		putchar('\0');
		flush();        close(fout);      flush();
		if(fd)close(fd);
		fin = 0;        fout = 1;
		shell("mv acct_tmp acct_lost");
		}
	putchar('Y');
	flush();
	goto loop;

}
cpline()
	
{
	register char c;
	
	while(c = getchar()){
		putchar(c);
		if(c == '\n')return;
		}
}
