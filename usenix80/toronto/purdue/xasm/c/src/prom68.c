/* uses: scanin islet island basin basout     from the c-library   */
	extern	fin,fout;
	int stadub;	/* sating address upper bits*/
	int startad;	/* starting address of file */
	int addr;	/* address of byte read from 6800 input */
	int gbnb;	/* number of bytes to read from input file */
	char rmem[1026];/* real memory (read from input file)	*/
	char mmem[1026];/* mapped memory (prom data)*/
	char mminit;	/* mapped memory initalize value	*/
	int kbank;	/*which k of 6800 file to grab.		*/
	int uplow;	/* upper/lower half of prom to program.	*/
	int mask;	/* 01777 	*/
	int umask;	/* addresss complement mask		*/

	/* user- set variables :  		*/
	int addcom;	/* address complement word		*/
	int datcom;	/* data complement mask			*/
	int addper[]{
		1,
		2,
		4,
		8,
		16,
		32,
		64,
		128,
		256,
		512,
		1024,
		2048,
		4096,
		};
	int datper[]{
		1,
		2,
		4,
		8,
		16,
		32,
		64,
		128,
		256,
		512,
		1024,
		2048,
		4096,
		};

main(argc,argv)
	int argc;	char **argv;
{
	int s[21];	char c;
	register i,j,k;	int ii,jj,ap,mfile;
	int	preserve,nocomp,*p;

/*	USER should set  addcom  and  datcom as needed for
	a complment mask.  Note that the complementaion occurs AFTER
	the bit-permutating.					*/
	addcom = 0;
	datcom = 0;

	preserve = 1;
	nocomp = 1;
	mminit = 0377;

	ap = 1;		mfile = 1;
	while((ap <= (argc - 1)) && (argv[ap][0] == '-'))ap++;
	if((ap <= (argc -1)) && (argv[ap][0] != '-'))mfile = 0;
	ii = 1;
	while((ii <= (argc - 1)) && (argv[ii][0] != '-')){
		ii++;
		mfile = 0; 
		}
	jj = 1;
	while((argv[ii][jj++] != '\0') && (argv[ii][0] == '-')){
	    switch(argv[ii][jj - 1]){
		case 'd':
			datcom = 0377;
			mminit = 0;
			break;

		case 'a':
			addcom = 01777;
			break;

		case 'p':
			preserve = 0;
			break;

		case 'n':
			nocomp = 0;
			preserve = 0;
			break;

		case 'w':
			i = 64;
			p = addper;
			for(j = 0; j != 7; j++){
				*p++ = i;
				i = i >> 1;
				}
			i = 128;
			p = datper;
			for(j = 0; j != 6; j++){
				*p++ = i;
				i = i >> 1;
				}
			datper[6] = 1;
			datper[7] = 2;
			break;

		default:
			printf("Warning- unknown option- '%c'\n",argv[ii][jj - 1]);
			break;
		}
	    }

	mask = 01777;	umask = 0176000;
	gbnb = 0;
	printf("K-bank? ");
	scanin(s);
	kbank = basin(s,10);
	printf("Upper or lower ? ");
	c = getchar();	uplow = 0;
	if((c == 'u')||(c == 'U'))uplow = 1;
	if(mfile != 1){
		i = open(argv[ap],0);
		if(i == -1){printf("%s not found\n",argv[ap]);return;}
		}
	   else {
		i = open("m.out",0);
		if(i == -1){printf("m.out not found\n");return;}
		}
	fin = i;
	i = creat("prom.obj.tmpq",0600);
	if(i == -1){printf("Can't create output file");return;}
	fout = i;
	i = getbyte();
	startad = addr;
	stadub = addr & umask;
	for(k = 0; k != 1025; k++){rmem[k] = mminit;mmem[k] = 0;}
	while(kbank && (i != -1)){
		i = getbyte();
		if(i != -1){
			if(stadub != (addr & umask)){
				k = addr & umask;
				kbank=- (k-stadub)>>10;
				stadub = k;
				}
			}
		}
	startad = addr;
	if(i == -1){
		fout = 1;	flush();
		printf("No data in that address range\n");
		return;
		}
/*  Now are in correct bank of memory	*/

	printf("\torg\t3c20\n");
loop:	rmem[addr & mask] = i;
	i = getbyte();
	if(i == -1)goto map;
	if(stadub != (addr & umask))goto map;
	goto loop;

/*	Note that data and address compelmentation occur after
	bit permuting.  This can be accounted for either when
	initalizing datcom and addcom, or the code directly below
	can be modifyed to use 'bitper' and get a bit-permuted
	mask.  The choice is a matter of user conveniance and
	whether that complementation occurs before or after the
	bit permutation.					*/
map:
	for(i = 0; i != 1024; i++){
		ii = rmem[i] & 0377;
		k = (bitper(ii,datper)) ^ datcom;
		j = (bitper(i,addper)) ^ addcom;
		mmem[j] = k & 0377;
		}
	k = 0;
	if(uplow)k = 512;
	for(i = 0; i != 64; i++){
		printf("\tdb\t");
		for(j = 0; j != 8; j++){
			ii = mmem[k++] & 0377;
			basout(ii,10);
			if(j != 7)putchar(',');
			}
		putchar('\n');
		}
	printf("\tend\n");
	fin = 0;	flush(fout);	fout = 1;	flush();
	printf("K boundry started at ");
	basout(startad & umask,16);	putchar('\n');
	printf("Data started at ");
	basout(startad,16);
	putchar('\n');
	if(nocomp)shell("mas80 prom.obj.tmpq");
	if(preserve)shell("rm prom.obj.tmpq");
	if(nocomp){
		shell("mv i.out prom.out");
		shell("rm i.lst");
		printf("i.out moved to prom.out\n");
		printf("Data in prompt-80 from 3c20 to 3e1f.\n");
		if(uplow){
			printf("Half-K boundry offset is 200 hex\n");
			}
		   else printf("Use offset of zero.\n");
		}
}

bitper(data,bittab)
	int data;
	int *bittab;
{
	register mask,*p,res;

	mask = 1;
	p =  bittab;
	res = 0;
	while(mask & 01777){
		if(data & mask)res =+ *p;
		mask = mask << 1;
		p++;
		}
	return(res);
}

/*	need globals:
		addr	address of current byte read
		gbnb	number of bytes left in record
		  Note:  gbnb must be set to zero initally
								*/
getbyte()
{
	register char c;	register i,j;
	int sum;

	if(gbnb <= 0){
		c = 1;
gbfoo:
		while((c != '\0') && (c != 'S'))c = getchar();
		if(c == '\0')return(-1);
		if((c = getchar()) != '1')goto gbfoo;
		gbnb = 16 * getit();
		gbnb = (gbnb + getit()) -3;
		sum = 0;
		j = 4096;
		for(i = 0; i != 4; i++){
			sum =+ j*getit();
			j = j>>4;
			}
		addr = sum - 1;
		}
	if(gbnb <= 0)goto gbfoo;
	i = 16 * getit();
	i =+ getit();
	gbnb =- 1;
	addr =+ 1;
	return(i & 0377);
}

getit()
{
	register char c;

	c = getchar();
	if(c == '\0')return(0);
	if(c > '9')c =- 'A' - ':';
	return(c-'0');
}
