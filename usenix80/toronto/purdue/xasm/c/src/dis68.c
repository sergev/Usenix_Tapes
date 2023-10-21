#define NUMSYM 500	/* max number of symbols	*/
	extern	fin,fout;
/*	6800 diss-assembler

	Peter D Hallenbeck

	(c) copyright Sept 1978

*/
	int pc;			/* progam counter	*/
	int bc;			/* byte counter		*/
	int fdi;		/* input file descriptor	*/
	int fdo;		/* output file descriptor	*/
	int ibuff[513];		/* input buffer		*/
	int ibuffc;		/* input buffer counter	*/
	char *ibuffp;		/* input buffer pointer	*/
	int b1,b2;		/* punchout bytes	*/
	int fb2;		/* 2nd byte flag	*/
	int errorf;		/* end of file flag	*/
	int curmod;		/* bad file type flag 0 = bad	*/
	int stable[NUMSYM];	/* symbol table		*/
	char sstable[NUMSYM*9];
	int vtable[NUMSYM];	/* adress table		*/
	int ltab;		/* number of symbols	*/

	char *datas[]{
	"adda",
	"adda",
	"adda",
	"adda",
	"addb",
	"addb",
	"addb",
	"addb",
	"aba ",
	"adca",
	"adca",
	"adca",
	"adca",
	"adcb",
	"adcb",
	"adcb",
	"adcb",
	"anda",
	"anda",
	"anda",
	"anda",
	"andb",
	"andb",
	"andb",
	"andb",
	"nba ",
	"bita",
	"bita",
	"bita",
	"bita",
	"bitb",
	"bitb",
	"bitb",
	"bitb",
	"clr ",
	"clr ",
	"clra",
	"clrb",
	"cmpa",
	"cmpa",
	"cmpa",
	"cmpa",
	"cmpb",
	"cmpb",
	"cmpb",
	"cmpb",
	"cba ",
	"com ",
	"com ",
	"coma",
	"comb",
	"neg ",
	"neg ",
	"nega",
	"negb",
	"daa ",
	"dec ",
	"dec ",
	"deca",
	"decb",
	"eora",
	"eora",
	"eora",
	"eora",
	"eorb",
	"eorb",
	"eorb",
	"eorb",
	"inc ",
	"inc ",
	"inca",
	"inca",
	"ldaa",
	"ldaa",
	"ldaa",
	"ldaa",
	"ldab",
	"ldab",
	"ldab",
	"ldab",
	"oraa",
	"oraa",
	"oraa",
	"oraa",
	"orab",
	"orab",
	"orab",
	"orab",
	"psha",
	"pshb",
	"pula",
	"pulb",
	"rol ",
	"rol ",
	"rola",
	"rolb",
	"ror ",
	"ror ",
	"rora",
	"rorb",
	"asl ",
	"asl ",
	"asla",
	"aslb",
	"asr ",
	"asr ",
	"asra",
	"asrb",
	"lsr ",
	"lsr ",
	"lsra",
	"lsrb",
	"staa",
	"staa",
	"staa",
	"staa",
	"stab",
	"stab",
	"stab",
	"stab",
	"suba",
	"suba",
	"suba",
	"suba",
	"subb",
	"subb",
	"subb",
	"subb",
	"sba ",
	"sbca",
	"sbca",
	"sbca",
	"sbca",
	"sbcb",
	"sbcb",
	"sbcb",
	"sbcb",
	"tab ",
	"tba ",
	"tst ",
	"tst ",
	"tsta",
	"tstb",
	"cpx ",
	"cpx ",
	"cpx ",
	"cpx ",
	"dex ",
	"des ",
	"inx ",
	"inx ",
	"ldx ",
	"ldx ",
	"ldx ",
	"ldx ",
	"lds ",
	"lds ",
	"lds ",
	"lds ",
	"stx ",
	"stx ",
	"stx ",
	"stx ",
	"sts ",
	"sts ",
	"sts ",
	"sts ",
	"txs ",
	"tsx ",
	"bra ",
	"bcc ",
	"bcs ",
	"beq ",
	"bge ",
	"bgt ",
	"bhi ",
	"ble ",
	"bls ",
	"blt ",
	"bmi ",
	"bne ",
	"bvc ",
	"bvs ",
	"bpl ",
	"bsr ",
	"jmp ",
	"jmp ",
	"jsr ",
	"jsr ",
	"nop ",
	"rti ",
	"rts ",
	"swi ",
	"wai ",
	"clc ",
	"cli ",
	"clv ",
	"sec ",
	"sei ",
	"sev ",
	"tap ",
	"tpa ",
	"hcf ",
	"    ",
	-1
	};

	int datav[]{
139,	155,	171,	187,	203,	219,	235,	251,	27,	137,
153,	169,	185,	201,	217,	233,	249,	132,	148,	164,
180,	196,	212,	228,	244,	20,	133,	149,	165,	181,
197,	213,	229,	245,	111,	127,	79,	95,	129,	145,
161,	177,	193,	209,	225,	241,	17,	99,	115,	67,
83,	96,	112,	64,	80,	25,	106,	122,	74,	90,
136,	152,	168,	184,	200,	216,	232,	248,	108,	124,
76,	92,	134,	150,	166,	182,	198,	214,	230,	246,
138,	154,	170,	186,	202,	218,	234,	250,	54,	55,
50,	51,	105,	121,	73,	89,	102,	118,	70,	86,
104,	120,	72,	88,	103,	119,	71,	87,	100,	116,
68,	84,	135,	151,	167,	183,	199,	215,	231,	247,
128,	144,	160,	176,	192,	208,	224,	240,	16,	130,
146,	162,	178,	194,	210,	226,	242,	22,	23,	109,
125,	77,	93,	140,	156,	172,	188,	9,	52,	8,
49,	206,	222,	238,	254,	142,	158,	174,	189,	207,
223,	239,	255,	143,	159,	175,	191,	53,	48,	32,
36,	37,	39,	44,	46,	34,	47,	35,	45,	43,
38,	40,	41,	42,	141,	110,	126,	173,	189,	1,
59,	57,	63,	62,	12,	14,	10,	13,	15,	11,
6,	7,	221	};

	char dataa[]{
'i',	'd',	'x',	'e',	'i',	'd',	'x',	'e',	'n',	'i',
'd',	'x',	'e',	'i',	'd',	'x',	'e',	'i',	'd',	'x',
'e',	'i',	'd',	'x',	'e',	'n',	'i',	'd',	'x',	'e',
'i',	'd',	'x',	'e',	'x',	'e',	'n',	'n',	'i',	'd',
'x',	'e',	'i',	'd',	'x',	'e',	'n',	'x',	'e',	'n',
'n',	'x',	'e',	'n',	'n',	'n',	'x',	'e',	'n',	'n',
'i',	'd',	'x',	'e',	'i',	'd',	'x',	'e',	'x',	'e',
'n',	'n',	'i',	'd',	'x',	'e',	'i',	'd',	'x',	'e',
'i',	'd',	'x',	'e',	'i',	'd',	'x',	'e',	'n',	'n',
'n',	'n',	'x',	'e',	'n',	'n',	'x',	'e',	'n',	'n',
'x',	'e',	'n',	'n',	'x',	'e',	'n',	'n',	'x',	'e',
'n',	'n',	'i',	'd',	'x',	'e',	'i',	'd',	'x',	'e',
'i',	'd',	'x',	'e',	'i',	'd',	'x',	'e',	'n',	'i',
'd',	'x',	'e',	'i',	'd',	'x',	'e',	'n',	'n',	'x',
'e',	'n',	'n',	'i',	'd',	'x',	'e',	'n',	'n',	'n',
'n',	'i',	'd',	'x',	'e',	'i',	'd',	'x',	'e',	'i',
'd',	'x',	'e',	'i',	'd',	'x',	'e',	'n',	'n',	'r',
'r',	'r',	'r',	'r',	'r',	'r',	'r',	'r',	'r',	'r',
'r',	'r',	'r',	'r',	'r',	'x',	'e',	'x',	'e',	'n',
'n',	'n',	'n',	'n',	'n',	'n',	'n',	'n',	'n',	'n',
'n',	'n',	'n',	'?'	};

	int datal[]{
2,	2,	2,	3,	2,	2,	2,	3,	1,	2,
2,	2,	3,	2,	2,	2,	3,	2,	2,	2,
3,	2,	2,	2,	3,	1,	2,	2,	2,	3,
2,	2,	2,	3,	2,	3,	1,	1,	2,	2,
2,	3,	2,	2,	2,	3,	1,	2,	3,	1,
1,	2,	3,	1,	1,	1,	2,	3,	1,	1,
2,	2,	2,	3,	2,	2,	2,	3,	2,	3,
1,	1,	2,	2,	2,	3,	2,	2,	2,	3,
2,	2,	2,	3,	2,	2,	2,	3,	1,	1,
1,	1,	2,	3,	1,	1,	2,	3,	1,	1,
2,	3,	1,	1,	2,	3,	1,	1,	2,	3,
1,	1,	2,	2,	2,	3,	2,	2,	2,	3,
2,	2,	2,	3,	2,	2,	2,	3,	1,	2,
2,	2,	3,	2,	2,	2,	3,	1,	1,	2,
3,	1,	1,	3,	2,	2,	3,	1,	1,	1,
1,	3,	2,	2,	3,	3,	2,	2,	3,	3,
2,	2,	3,	3,	2,	2,	3,	1,	1,	2,
2,	2,	2,	2,	2,	2,	2,	2,	2,	2,
2,	2,	2,	2,	2,	2,	3,	2,	3,	1,
1,	1,	1,	1,	1,	1,	1,	1,	1,	1,
1,	1,	1,	0,	};

main(argc,argv)
	int argc;	char **argv;
{
	char opener[50],s[50],ss[50],*p,symfil[50];
	register i;	register char c;
	register *j;
	int ii,index,tmp;

	mixerup(stable,sstable);
	if(argc == 1){
		copystr(opener,"m.out");
		}
	    else copystr(opener,argv[1]);
	fdi = open(opener,0);
	if(fdi == -1){printf("Can't find '%s'.\n",opener);return;}


	ibuffc = 0;	errorf = 1;	bc = 0;	ltab = 0;
	copystr(s,opener);	cats(s,".dis");
	fdo = creat(s,0604);
	if(fdo == -1){printf("Can't make '%s'.\n",s);return;}
	curmod = 0;	pc = 0;		fb2 = 0;

    if(argc == 3){
	ii = open(argv[2],0);
	if(ii == -1){
		printf("Can't find '%s'.\n");
		return;
		}
	fin = ii;
laber:	while((ii = scanin(s)) != -2){
		if(s[0] == '\0')goto laber;
laber2:		ii = scanin(symfil);
		if((symfil[0] == '\0')&&(ii != -2))goto laber2;
		if(ii == -2){
			printf("Abrupt end of symbol table file.\n");
			goto start;
			}
		p = s;
		while(*p){
			if(valid(*p++)){
				printf("Adress field not number -'%s'.\n",s);;
				fout = fdo;
				goto start;
				}
			}
		if(islet(symfil[0]) != 1){
			printf("Invalid lable - '%s'.\n",symfil);
			goto laber;
			}
		cement(symfil,stable);
		vtable[llu(symfil,stable)] = basin(s,16);
		ltab++;
		if(ltab > (NUMSYM - 5)){
			printf("Symbol table overflow- increase NUMSYM and try again.\n");
			goto start;
			}
		}
	}
	flush();	fin = 0;	flush();
	fout = fdo;

start:
    while(errorf){		/* for as long as i have input..  */
	b1 = gnb();
	if(!curmod)goto booboo;
	index = -1;
	j = datav;
	for(i = 0;i != 203;i++){
		if(*j++ == b1)index = i;
		}
	pnum(pc);		putchar('\t');
	jnum(b1,16,2,0);	putchar(' ');
    if(index != -1){
	if(datal[index] == 2){
	    b2 = gnb();
	    jnum(b2,16,2,1);	printf("    ");
	    }
	  else if(datal[index] == 3){
	    b2 = gnw();
	    jnum(b2,16,4,0);	printf("  ");
	    }
	  else printf("      ");
	printf("%s  ",datas[index]);
	switch(dataa[index]){
	    case 'n':				/* inherant	*/
		printf("    ");
		break;
	    case 'i':				/* immediate	*/
		putchar('#');
		if(datal[index] == 2)
			pbyte(b2);
		   else pnum(b2);
		printf(" ");
		break;
	    case 'r':				/* relative	*/
		if(b2 & 0200){
			b2 =& 0177;	b2 =+ 0177600;
			ii = pc + b2 + 1;
			}
		    else {
			ii = (b2 & 0177) + 1;
			ii =+ pc;
			}
		pnum(ii);
		putchar(' ');
		break;
	    case 'd':				/* direct	*/
		pbyte(b2);
		printf("  ");
		break;
	    case 'x':				/* indexed	*/
		jnum(b2,16,2,0);
		printf(",x");
		break;
	    case 'e':				/* extended	*/
		pnum(b2);
		break;
	    }
	printf("     \t");
	pasc(b1);
	if(datal[index] == 2)pasc(b2);
	if(datal[index] == 3){
		pasc(b2 >> 8);
		pasc(b2);
		}
	putchar('\n');
	}			/* end of 'if(index != -1....' */
    else {				/* undefined instruction */
	printf("      ? ?       \t\t");
	pasc(b1);	putchar('\n');
	}

    }					/* end of 'while(errorf.. */




booboo: flush();
	fout = 1;
	if(!curmod)printf("Not binary file - '%s'.\n",opener);
	flush();
}

get()
{
	if(ibuffc == 0){
		ibuffc = read(fdi,&ibuff,512);
		if(ibuffc == 0)return('\0');
		if(ibuffc == -1)return('\0');
		ibuffp = ibuff;
		}
	ibuffc--;
	return((*ibuffp++) & 0177);
}

getbyte()
{
	register i;	register char c;

	c = get();
	if((c < '0') || ((c > '9') && (c < 'A'))){
		return(-1);
		}
	if(c > 'F')return(-1);
	c =- 060;
	if(c > 9)c =- ('A' - ':');
	i = (c << 4) & 0360;
	c = get();
	if((c < '0') || ((c > '9') && (c < 'A')))return(-1);
	if(c > 'F')return(-1);
	c =- 060;
	if(c > 9)c =- ('A' - ':');
	return(i + c);
}

getword()
{
	register i;
	i = getbyte() << 8;
	return(i + getbyte());
}

gnb(){
	register char c;
	register barfo;

	if(bc > 0)goto gnbaaa;
	while(c = get()){
	    if(c == 'S'){
	      if(get() == '1'){
		curmod++;
		bc = getbyte() -3;
		if(bc == -4){
			curmod = 0;	return;
			}
		pc = getword() - 1;
		if(pc == -2){
			curmod = 0;	return;
			}
gnbaaa:		pc =+ 1;
		--bc;
		barfo = getbyte();
		if(barfo == -1)curmod = 0;
		return(barfo);
		}
	      }
	    }
	errorf = 0;
}

gnw()
{
	register i;

	i = gnb();
	return(((i << 8)& 0177400) + gnb());
}

pasc(thing)
	char thing;
{
	thing =& 0177;
	if(islet(thing) != 3)putchar(thing);
	if(thing == '\n')printf("\\n");
	if(thing == '\r')printf("\\r");
	if(thing == '\t')printf("\\t");
}

valid(thing)
	char thing;
{
	if((thing >= '0') && (thing <= '9'))return(0);
	if((thing >= 'A')&& (thing <= 'F'))return(0);
	if((thing >= 'a')&& (thing <= 'f'))return(0);
	return(1);
}

pnum(t)
	int t;
{
	register *l,tt,xy;

	l = vtable + 1;
	tt = t;
	xy = 0;
	while(xy++ != ltab){
		if(*l++ == tt){
			goto gotit;
			}
		}
	jnum(tt,16,4,0);
	return;
gotit:	printf("%s",stable[xy]);
}

pbyte(t)
	char t;
{
	register *l,xy;
	register tt;

	l = vtable + 1;
	tt = t;
	tt =& 0377;
	xy = 0;
	while(xy++ != ltab){
		if(*l++ == tt)goto gotit;
			}
	jnum(tt,16,2,1);
	return;
gotit:	printf("%s",stable[xy]);
}
