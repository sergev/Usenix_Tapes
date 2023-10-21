	extern fin,fout;
/* uses: scanin islet island basin basout     from the c-library   */
	int addr;	/* address of byte read from 6800 input */
	int gbnb;	/* number of bytes to read from input file */

	int type;	/* type of addressing for instruction */
	int pc;		/* pc for current line */
	int data;	/* instruction byte */
	int operhand;	/* operhand for instruction */
	int index;	/* index for the instructin into tables */

	int opcode[170];
	int inst[170];
	char iinst[610];
	int kind[170];
	int bcount[170];
	char *lukup[]{
		"%32",
		"im",
		"as",
		"zp",
		"ac",
		"ip",
		"ix",
		"iy",
		"zx",
		"ax",
		"ay",
		"rl",
		"id",
		"zy",
		"ii",		/* invalid instruction */
		-1
		};

main(argc,argv)
	int argc;	char **argv;
{

	char opener[20],foo[5];
	register i,j,ii;
	int fd;

	gbnb = 0;
	mixerup(inst,iinst);

	fd = open("/usr2/wa1yyn/6502/dis65.opcode",0);
	if(fd == -1){
		printf("Can't find /v/wa1yyn/6502/dis65.opcode.\n");
		return;
		}
	fin = fd;
	i = 1;
	while(getchar() != '\0'){
		scanin(foo);
		opcode[i] = basin(foo,16);
		scanin(foo);
		cement(foo,inst);
		scanin(foo);
		kind[i] = llu(foo,lukup) -1;
		scanin(foo);
		bcount[i++] = basin(foo,10);
		getchar();			/* kill line-feed */
		}
	flush();	close(fd);	flush();
	if(argc == 1){
		cats(opener,"mt.out");
		}
	    else cats(opener,argv[1]);
	fd = open(opener,0);
	if(fd == -1){
		printf("Can't find '%s'.\n",opener);
		flush();
		return;
		}
	fin = fd;

    while((data = getbyte()) != -1){
	for(i = 1;opcode[i] != -1;i++){
		if(data == opcode[i]){
			index = i;
			type = kind[i];
			goto gotit;
			}
		}
	type = 13;		/* undefinded instruction */
gotit:	pc = addr;
	jnum(pc,16,4,0);	printf("  ");
	jnum(data,16,2,0);	printf(" ");
	operhand = 0;

    switch(type){
	case 0:					/* imediate */
		operhand = getbyte();
		jnum(operhand,16,2,1);	printf("     ");
		printf("%s #",inst[index]);
		jnum(operhand,16,2,1);
		break;
	case 1:					/* absolute */
		operhand = getbyte();
		operhand =+ getbyte() << 8;
		jnum(swapit(operhand),16,4,0);
		printf("   %s ",inst[index]);
		jnum(operhand,16,4,0);
		break;
	case 2:					/* zero page */
		operhand = getbyte();
		jnum(operhand,16,2,1);
		printf("     %s ",inst[index]);
		jnum(operhand,16,2,1);
		break;
	case 3:					/* accumulator */
		printf("       %s   ",inst[index]);
		break;
	case 4:					/* implyed */
		printf("       %s   ",inst[index]);
		break;
	case 5:					/* (ind,x) */
		operhand = getbyte();
		jnum(operhand,16,2,1);
		printf("     %s (",inst[index]);
		jnum(operhand,16,2,0);
		printf(",x)");
		break;
	case 6:					/* (ind),y */
		operhand = getbyte();
		jnum(operhand,16,2,1);
		printf("     %s (",inst[index]);
		jnum(operhand,16,2,0);
		printf("),y");
		break;
	case 7:					/* page zero,x */
		operhand = getbyte();
		jnum(operhand,16,2,1);
		printf("     %s ",inst[index]);
		jnum(operhand,16,2,0);
		printf(",x");
		break;
	case 8:					/* absolute,x */
		operhand = getbyte();
		operhand =+ getbyte() << 8;
		jnum(swapit(operhand),16,4,1);
		printf("   %s ",inst[index]);
		jnum(operhand,16,4,1);
		printf(",x");
		break;
	case 9:					/* absolute,y */
		operhand = getbyte();
		operhand =+ getbyte() << 8;
		jnum(swapit(operhand),16,4,1);
		printf("   %s ",inst[index]);
		jnum(operhand,16,4,1);
		printf(",y");
		break;
	case 10:				/* relative */
		operhand = getbyte();
		jnum(operhand,16,2,1);
		printf("     %s ",inst[index]);
		if(operhand & 0200){
			ii = ~operhand & 0177;
			ii = pc - ii + 1;
			}
		    else {
			ii = pc + operhand + 2;
			}
		jnum(ii,16,4,0);
		break;
	case 11:				/* inderect */
		operhand = getbyte();
		operhand =+ getbyte() << 8;
		jnum(swapit(operhand),16,4,0);
		printf("   %s (",inst[index]);
		jnum(operhand,16,4,0);
		printf(")");
		break;
	case 12:				/* page zero,y */
		operhand = getbyte();
		jnum(operhand,16,2,1);
		printf("     %s ",inst[index]);
		jnum(operhand,16,2,0);
		printf(",y");
		break;
	case 13:				/* undefined */
		printf("       ???    ");
		break;
	}			/* end of switch */

	printf("    \t");
	pall(data & 0177);
	if(operhand & 0177){
		pall(operhand & 0177);
		pall((operhand >> 8) & 0177);
		}

	putchar('\n');
	}		/* end of while */




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
		while((c != '\0') && (c != ';'))c = getchar();
		if(c == '\0')return(-1);
		gbnb = 16 * getit();
		gbnb = (gbnb + getit());
		if(gbnb == 0)return(-1);
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
	return((c - '0') & 017);
}

pall(thing)
	char thing;
{
	if(thing == 0){printf("  ");	return;}
	if(thing < 32){
		putchar('^');
		putchar(thing + 0100);
		}
	    else if(thing == 0177){
		printf("^R");
		}
		else printf(" %c",thing);
}

swapit(num)
	int num;
{
	register i,j;

	i = num & 0377;
	j = (num >> 8) & 0377;
	i = (i << 8) & 0177400;
	return(i + j);
}
