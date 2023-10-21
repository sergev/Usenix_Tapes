#define NUMSYM 500	/* max number of symbols	*/
	extern	fin,fout;
/*	8080 dis-assembler

	Peter D Hallenbeck

	(c) copyright Sept 1978

*/
	int pc;			/* progam counter	*/
	int bc;			/* byte counter		*/
	int fdi;		/* input file descriptor	*/
	int ibuff[513];		/* input buffer		*/
	int ibuffc;		/* input buffer counter	*/
	char *ibuffp;		/* input buffer pointer	*/
	int errorf;		/* error flag	*/
	int curmod;		/* end of file flag	*/
	int stable[NUMSYM];	/* symbol table		*/
	char sstable[NUMSYM*9];
	int vtable[NUMSYM];	/* adress table		*/
	int ltab;		/* number of symbols	*/

	int *datas[] {
	"nop",
	"lxi  b,",
	"stax b",
	"inx  b",
	"inr  b",
	"dcr  b",
	"mvi  b,",
	"rlc",
	"dad  b",
	"ldax b",
	"dcx  b",
	"inr  c",
	"dcr  c",
	"mvi  c,",
	"rrc",
	"lxi  d,",
	"stax d",
	"inx  d",
	"inr  d",
	"dcr  d",
	"mvi  d,",
	"ral",
	"dad  d",
	"ldax d",
	"dcx  d",
	"inr  e",
	"dcr  e",
	"mvi  e,",
	"rar",
	"lxi  h,",
	"shld ",
	"inx  h",
	"inr  h",
	"dcr  h",
	"mvi  h,",
	"daa",
	"dad  h",
	"lhld ",
	"dcx  h",
	"inr  l",
	"dcr  l",
	"mvi  l,",
	"cma",
	"lxi  sp,",
	"sta  ",
	"inx  sp",
	"inr  m",
	"dcr  m",
	"mvi  m,",
	"stc",
	"dad  sp",
	"lda  ",
	"dcx  sp",
	"inr  a",
	"dcr  a",
	"mvi  a,",
	"cmc",
	"mov  b,b",
	"mov  b,c",
	"mov  b,d",
	"mov  b,e",
	"mov  b,h",
	"mov  b,l",
	"mov  b,m",
	"mov  b,a",
	"mov  c,b",
	"mov  c,c",
	"mov  c,d",
	"mov  c,e",
	"mov  c,h",
	"mov  c,l",
	"mov  c,m",
	"mov  c,a",
	"mov  d,b",
	"mov  d,c",
	"mov  d,d",
	"mov  d,e",
	"mov  d,h",
	"mov  d,l",
	"mov  d,m",
	"mov  d,a",
	"mov  e,b",
	"mov  e,c",
	"mov  e,d",
	"mov  e,e",
	"mov  e,h",
	"mov  e,l",
	"mov  e,m",
	"mov  e,a",
	"mov  h,b",
	"mov  h,c",
	"mov  h,d",
	"mov  h,e",
	"mov  h,h",
	"mov  h,l",
	"mov  h,m",
	"mov  h,a",
	"mov  l,b",
	"mov  l,c",
	"mov  l,d",
	"mov  l,e",
	"mov  l,h",
	"mov  l,l",
	"mov  l,m",
	"mov  l,a",
	"mov  m,b",
	"mov  m,c",
	"mov  m,d",
	"mov  m,e",
	"mov  m,h",
	"mov  m,l",
	"hlt",
	"mov  m,a",
	"mov  a,b",
	"mov  a,c",
	"mov  a,d",
	"mov  a,e",
	"mov  a,h",
	"mov  a,l",
	"mov  a,m",
	"mov  a,a",
	"add  b",
	"add  c",
	"add  d",
	"add  e",
	"add  h",
	"add  l",
	"add  m",
	"add  a",
	"adc  b",
	"adc  c",
	"adc  d",
	"adc  e",
	"adc  h",
	"adc  l",
	"adc  m",
	"adc  a",
	"sub  b",
	"sub  c",
	"sub  d",
	"sub  e",
	"sub  h",
	"sub  l",
	"sub  m",
	"sub  a",
	"sbb  b",
	"sbb  c",
	"sbb  d",
	"sbb  e",
	"sbb  h",
	"sbb  l",
	"sbb  m",
	"sbb  a",
	"ana  b",
	"ana  c",
	"ana  d",
	"ana  e",
	"ana  h",
	"ana  l",
	"ana  m",
	"ana  a",
	"xra  b",
	"xra  c",
	"xra  d",
	"xra  e",
	"xra  h",
	"xra  l",
	"xra  m",
	"xra  a",
	"ora  b",
	"ora  c",
	"ora  d",
	"ora  e",
	"ora  h",
	"ora  l",
	"ora  m",
	"ora  a",
	"cmp  b",
	"cmp  c",
	"cmp  d",
	"cmp  e",
	"cmp  h",
	"cmp  l",
	"cmp  m",
	"cmp  a",
	"rnz",
	"pop  b",
	"jnz  ",
	"jmp  ",
	"cnz  ",
	"push b",
	"adi  ",
	"rst  0",
	"rz",
	"ret",
	"jz   ",
	"cz   ",
	"call ",
	"aci  ",
	"rst  1",
	"rnc",
	"pop  d",
	"jnc  ",
	"out  ",
	"cnc  ",
	"push d",
	"sui  ",
	"rst  2",
	"rc",
	"jc   ",
	"in   ",
	"cc   ",
	"sbi  ",
	"rst  3",
	"rpo",
	"pop  h",
	"jpo  ",
	"xthl",
	"cpo  ",
	"push h",
	"ani  ",
	"rst  4",
	"rpe",
	"pchl",
	"jpe  ",
	"xchg",
	"cpe  ",
	"xri  ",
	"rst  5",
	"rp",
	"pop psw",
	"jp   ",
	"di",
	"cp   ",
	"psh psw",
	"ori  ",
	"rst  6",
	"rm",
	"sphl",
	"jm   ",
	"ei",
	"cm   ",
	"cpi  ",
	"rst",
	"rst",
	-1
	};

	int datav[]{
	0,
	1,
	2,
	3,
	4,
	5,
	6,
	7,
	9,
	10,
	11,
	12,
	13,
	14,
	15,
	17,
	18,
	19,
	20,
	21,
	22,
	23,
	25,
	26,
	27,
	28,
	29,
	30,
	31,
	33,
	34,
	35,
	36,
	37,
	38,
	39,
	41,
	42,
	43,
	44,
	45,
	46,
	47,
	49,
	50,
	51,
	52,
	53,
	54,
	55,
	57,
	58,
	59,
	60,
	61,
	62,
	63,
	64,
	65,
	66,
	67,
	68,
	69,
	70,
	71,
	72,
	73,
	74,
	75,
	76,
	77,
	78,
	79,
	80,
	81,
	82,
	83,
	84,
	85,
	86,
	87,
	88,
	89,
	90,
	91,
	92,
	93,
	94,
	95,
	96,
	97,
	98,
	99,
	100,
	101,
	102,
	103,
	104,
	105,
	106,
	107,
	108,
	109,
	110,
	111,
	112,
	113,
	114,
	115,
	116,
	117,
	118,
	119,
	120,
	121,
	122,
	123,
	124,
	125,
	126,
	127,
	128,
	129,
	130,
	131,
	132,
	133,
	134,
	135,
	136,
	137,
	138,
	139,
	140,
	141,
	142,
	143,
	144,
	145,
	146,
	147,
	148,
	149,
	150,
	151,
	152,
	153,
	154,
	155,
	156,
	157,
	158,
	159,
	160,
	161,
	162,
	163,
	164,
	165,
	166,
	167,
	168,
	169,
	170,
	171,
	172,
	173,
	174,
	175,
	176,
	177,
	178,
	179,
	180,
	181,
	182,
	183,
	184,
	185,
	186,
	187,
	188,
	189,
	190,
	191,
	192,
	193,
	194,
	195,
	196,
	197,
	198,
	199,
	200,
	201,
	202,
	204,
	205,
	206,
	207,
	208,
	209,
	210,
	211,
	212,
	213,
	214,
	215,
	216,
	218,
	219,
	220,
	222,
	223,
	224,
	225,
	226,
	227,
	228,
	229,
	230,
	231,
	232,
	233,
	234,
	235,
	236,
	238,
	239,
	240,
	241,
	242,
	243,
	244,
	245,
	246,
	247,
	248,
	249,
	250,
	251,
	252,
	254,
	255,
	255,
	255,
	255
	};

	int datat[]{
	1,
	3,
	1,
	1,
	1,
	1,
	2,
	1,
	1,
	1,
	1,
	1,
	1,
	2,
	1,
	3,
	1,
	1,
	1,
	1,
	2,
	1,
	1,
	1,
	1,
	1,
	1,
	2,
	1,
	3,
	3,
	1,
	1,
	1,
	2,
	1,
	1,
	3,
	1,
	1,
	1,
	2,
	1,
	3,
	3,
	1,
	1,
	1,
	2,
	1,
	1,
	3,
	1,
	1,
	1,
	2,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	3,
	3,
	3,
	1,
	2,
	1,
	1,
	1,
	3,
	3,
	3,
	2,
	1,
	1,
	1,
	3,
	2,
	3,
	1,
	2,
	1,
	1,
	3,
	2,
	3,
	2,
	1,
	1,
	1,
	3,
	1,
	3,
	1,
	2,
	1,
	1,
	1,
	3,
	1,
	3,
	2,
	1,
	1,
	1,
	3,
	1,
	3,
	1,
	2,
	1,
	1,
	1,
	3,
	1,
	3,
	2,
	7,
	1,
	1
	};


main(argc,argv)
	int argc;	char **argv;
{
	char opener[50],s[50],ss[50],*p,symfil[50];
	register i,j;	register char c;
	int ii,index,fdo;

	mixerup(stable,sstable);
	if(argc == 1)copystr(opener,"i.out");
	    else copystr(opener,argv[1]);
	fdi = open(opener,0);
	if(fdi == -1){printf("Can't find '%s'.\n",opener);return;}

	errorf = 1;
	ibuffc = 0;
	copystr(s,opener);	cats(s,".dis");
	fdo = creat(s,0604);
	if(fdo == -1){printf("Can't make '%s'.\n",s);return;}
	curmod = 1;	pc = 0;		bc = 0;	ltab = 0;

    if(argc == 3){
	ii = open(argv[2],0);
	if(ii == -1){
		printf("Can't find '%s'.\n",argv[2]);
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
				printf("Adress field not number - '%s'.\n",s);
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

	while(errorf){			/* for as long as is data  */
	    i = gnb();
	    if(!curmod)goto booboo;
	    index = -1;
	    for(j = 0;j != 243;j++){
		if(datav[j] == i)index = j;
		}
	    j = 0;
	    pnum(pc);		printf("\t");
	    jnum(i,16,2,1);		printf(" ");
	    if(index != -1){
		if(datat[index] == 1){
		    printf("      ");
		    }
		  else if(datat[index] == 2){
		    j = gnb();
		    jnum(j,16,2,1);	printf("    ");
		    }
		  else if(datat[index] == 3){
		    j = gnw();
		    jnum(j,16,4,0);	printf("  ");
		    }
		printf("%s",datas[index]);
		if(datat[index] == 2)
		    pbyte(j);
		  else if(datat[index] == 3)
		    pnum(swpbyte(j));
		  else printf("       ");
		printf("     \t");
		pasc(i);
		if(datat[index] == 2)pasc(j);
		if(datat[index] == 3){
			pasc((j >> 8) & 0377);
			pasc(j & 0377);
			}
		putchar('\n');
		}
	    else {				/* undefined	*/
		printf("      ? ? ?     \t");
		pasc(i);	putchar('\n');
		}
	    }


booboo:	flush();
	if(!curmod)printf("Not binary file -'%s'.\n",opener);
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
	i = (getbyte() << 8) & 0177400;
	return(i + getbyte());
}


gnb(){
	register char c;
	register barfo;

	if(bc > 0)goto gnbaaa;
	while(c = get()){
	    if(c == ':'){
		curmod++;
		bc = getbyte();
		if(bc == -1){
			curmod = 0;	return;
			}
		pc = getword() - 1;
		if(getbyte() == -1){curmod = 0;	return;}
gnbaaa:		pc =+ 1;
		--bc;
		barfo = getbyte();
		if(barfo == -1)curmod = 0;
		return(barfo);
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

swpbyte(dat)
	int dat;
{
	register i,k;

	i = (dat << 8)& 0177400;
	k = (dat >> 8)& 0377;
	return(i + k);
}

pasc(thing)
	char thing;
{
	if(islet(thing) != 3)putchar(thing);
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
	char *p;

	l = vtable + 1;
	tt = t;
	xy = 0;
	while(xy++ != ltab){
		if(*l++ == tt){
			goto gotit;
			}
		}
	jnum(tt,16,4,0);
	printf("   ");
	return;
gotit:	p = stable[xy];
	tt = 0;
	while(*p){
		putchar(*p++);
		tt++;
		}
	while(tt++ <5)putchar(' ');
}

pbyte(t)
	char t;
{
	register *l,xy;
	register tt;
	char *p;

	l = vtable + 1;
	tt = t;
	tt =& 0377;
	xy = 0;
	while(xy++ != ltab){
		if(*l++ == tt)goto gotit;
			}
	jnum(tt,16,2,1);
	printf("  ");
	return;
gotit:	p = stable[xy];
	tt = 0;
	while(*p){
		putchar(*p++);
		tt++;
		}
	while(tt++ < 5)putchar(' ');
}
