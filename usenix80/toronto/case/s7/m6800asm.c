int codec;
char checks;
char cbuf[42];
int fcode,flist;
char lstr[100];
int ln,nerror;
int pass,labsym,expflg,ltype;
int type,class,state;
int clss[128] {
	3,3,3,3,3,3,3,3,
	3,3,4,3,3,3,3,3,
	3,3,3,3,3,3,3,3,
	3,3,3,3,3,3,3,3,
	3,3,3,2,3,2,3,2,
	3,3,2,2,2,2,3,2,
	1,1,1,1,1,1,1,1,
	1,1,3,3,3,3,3,3,
	3,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,3,0,3,3,3,
	3,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,3,3,3,3,3
	};
struct mat {
	int stat[4];
	int action[4];
	}
	mat[5]
	{
	1,1,0,0,1,1,4,2,
	2,1,2,0,1,1,1,2,
	3,0,0,0,1,4,3,2,
	0,0,0,0,6,4,3,2,
	0,0,0,0,5,4,3,2
	};
struct table {
	char *symbol;
	int format;
	int value;
	} table[200]
	{
	"aba",	16,	0033,
	"adc",	1,	0211,
	"add",	1,	0213,
	"and",	1,	0204,
	"asl",	3,	0110,
	"asr",	3,	0107,
	"bcc",	8,	0044,
	"bcs",	8,	0045,
	"beq",	8,	0047,
	"bge",	8,	0054,
	"bgt",	8,	0056,
	"bhi",	8,	0042,
	"bit",	1,	0205,
	"ble",	8,	0057,
	"bls",	8,	0043,
	"blt",	8,	0055,
	"bmi",	8,	0053,
	"bne",	8,	0046,
	"bpl",	8,	0052,
	"bra",	8,	0040,
	"bsr",	8,	0215,
	"bvc",	8,	0050,
	"bvs",	8,	0051,
	"cra",	16,	0021,
	"clc",	16,	0014,
	"cli",	16,	0016,
	"clr",	3,	0117,
	"clv",	16,	0012,
	"cmp",	1,	0201,
	"com",	3,	0103,
	"cpx",	5,	0214,
	"daa",	16,	0031,
	"dec",	3,	0112,
	"des",	16,	0064,
	"dex",	16,	0011,
	"eor",	1,	0210,
	"inc",	3,	0114,
	"ins",	16,	0061,
	"inx",	16,	0010,
	"jmp",	7,	0116,
	"jsr",	7,	0215,
	"lda",	1,	0206,
	"lds",	5,	0216,
	"ldx",	5,	0316,
	"lsr",	3,	0104,
	"neg",	3,	0100,
	"nop",	16,	0001,
	"ora",	1,	0212,
	"psh",	4,	0066,
	"pul",	4,	0062,
	"rol",	3,	0111,
	"ror",	3,	0106,
	"rti",	16,	0073,
	"rts",	16,	0071,
	"sba",	16,	0020,
	"sbc",	1,	0202,
	"sec",	16,	0015,
	"sei",	16,	0017,
	"sev",	16,	0013,
	"sta",	2,	0207,
	"sts",	6,	0217,
	"stx",	6,	0317,
	"sub",	1,	0200,
	"swi",	16,	0077,
	"tab",	16,	0026,
	"tap",	16,	0006,
	"tba",	16,	0027,
	"tpa",	16,	0007,
	"tst",	3,	0115,
	"tsx",	16,	0060,
	"txs",	16,	0065,
	"wai",	16,	0076,
	"org",	9,	0,
	"end",	10,	0,
	"equ",	11,	0,
	"fcb",	12,	0,
	"fcc",	13,	0,
	"fdb",	14,	0,
	"rmb",	15,	0,
	"#",	20,	0,
	"'",	21,	0,
	",",	22,	0,
	"a",	23,	0,
	"b",	24,	0,
	"x",	25,	0,
	"\n",	0,	0,
	"+",	26,	0,
	"-",	27,	0,
	"*",	28,	0,
	"/",	29,	0,
	"%",	31,	0,
	0,	0,	0
	};
int rlcnt 0;
int lpos;
int rlpos;
char buf[100];
char line[100];
int opsym;
int acc,add,amode;
int pc;
char space,tab;
char *freep,*stptr;
int symptr,sptr,symp,lpos;
char line[100];
char sym[81];
char frespc[1000];


defsym(){
	if(table[symptr].format!=17) perror("multiply defined symbol");
	else
	{
	table[symptr].format=18;
	table[symptr].value=table[pc].value;
	}
	}

hex(num)
	int num;
	{
	if(num<=9) return(num+060);
	else return(num-10+0101);
	}

cdout(bin)
	int bin;
	{
	if(codec==0) stblk();
	cbuf[codec]=hex((bin>>4)&017);
	checks=checks+cbuf[codec];
	codec=codec+1;
	cbuf[codec]=hex(bin&017);
	checks=checks+cbuf[codec];
	codec=codec+1;
	if(codec==40) endblk();
	}

stblk()
	{
	int tmp;
	cbuf[0]='S';
	cbuf[1]='1';
	tmp=table[pc].value;
	cbuf[4]=hex((tmp>>12)&017);
	cbuf[5]=hex((tmp>>8)&017);
	cbuf[6]=hex((tmp>>4)&017);
	cbuf[7]=hex(tmp&017);
	codec=8;
	checks=cbuf[4]+cbuf[5]+cbuf[6]+cbuf[7];
	}

endblk() {
	cbuf[2]=hex(((codec-2)>>5)&017);
	cbuf[3]=hex(((codec-2)>>1)&017);
	checks=checks+cbuf[2]+cbuf[3];
	cbuf[codec]=hex((-(checks+1)>>4)&017);
	cbuf[codec+1]=hex(-(checks+1)&017);
	write(fcode,cbuf,codec+2);
	codec=0;
	leader();
	}
code(bin)
	int bin;
	{
	if(pass==2)cdout(bin);
	table[pc].value=table[pc].value+1;
	}
ac() {
	acc=0;
	if(table[symptr].format==24) acc=1;
	else
	if(table[symptr].format!=23) perror("acculmulator syntax error");
	getsym();
	}

outadd() {
	if((amode!=4)&&(add>255)&&(pass==2)) perror("truncation error");
	if(amode==4) code((add>>8) & 0377);
	code(add & 0377);
	if(amode==4) list(add);
	else {
		slist(add & 0377);
		blank(3); }
	}

exp() {
	int tmp,val,val2,op;

	expflg=0;
	op=26; val=0;
	if(table[symptr].format==26) getsym();
	else if(table[symptr].format==27) {
		op=27;
		getsym(); }
loop:	tmp=table[symptr].format;
	val2=table[symptr].value;
	getsym();
	if((tmp!=18)&&(tmp!=19)&&(tmp!=28)) {
		if(pass==1) expflg=1;
		else perror("expression syntax error");
		return(val); }
	switch(op) {
		case 26: val=val+val2;
			break;
		case 27: val=val-val2;
			 break;
		case 28: val=val*val2;
			 break;
		case 29: val=val/val2;
			 break;
		}
	op=table[symptr].format;
	if((op==26)||(op==27)||(op==28)||(op==29)) {
		getsym();
		goto loop; }
	return(val);
	}

address() {
	add=0;
	amode=0;
	if(table[symptr].format==20) {
		getsym();
		amode=1;
		if(table[symptr].format==21) {
			add= *table[symptr].symbol & 0177;
			getsym(); }
		else add=exp();
		}
	else
		if(table[symptr].format==25) {
			getsym();
			amode=3;
			}
	else
		if(table[symptr].format==22) {
			getsym();
			if(table[symptr].format==25) {
			getsym();
			amode=3;
			}
			else
			perror("address syntax error");
			}
	else
	if(table[symptr].format==31) {
		getsym();
		add=exp();
		amode=2; }
	else
		{
		add=exp();
		if(table[symptr].format==22) {
			getsym();
			if(table[symptr].format==25) {
			amode=3;
			getsym();
			}
		else perror("address syntax error");
		}
	else amode=4; }
	}

perror(s)
	char s[];
	{
	nerror=nerror+1;
	printf("%s\n",s);
	printf("%d %s",ln,line);
	}
list(val)
	int val;
	{
	if(pass==2) {
		printf(-1,lstr," %06o",val);
		write(flist,lstr,7);
		}
	return(val);
	}

slist(val)
	int val;
	{
	if(pass==2) {
		printf(-1,lstr," %03o",val);
		write(flist,lstr,4);
		}
	return(val);
	}

blank(cnt)
	int cnt;
	{
	if(pass==2)
	while(cnt>0) {
		cnt=cnt-1;
		write(flist," ",1);
		}
	}

count(str)
	char *str;
	{
	int i;
	for(i==0;str[i]!=0;i++) ;
	return(i);
	}
leader() {
	write(fcode,"\r\n",3);
	}
comment(){
	}
form0(){
	code(slist(table[opsym].value));
	blank(7);
	}
form1()
	{
	ac();
	address();
	code(slist(table[opsym].value | ((amode-1)<<4) | (acc<<6)));
	outadd();
	}
form2(){
	ac();
	address();
	if(amode==1) perror("illegal addressing mode");
	code(slist(table[opsym].value | ((amode-1)<<4) | (acc<<6)));
	outadd();
	}
form3(){
	if(table[symptr].format==23) {amode=1; getsym();}
	else
	if(table[symptr].format==24) {amode=2; getsym();}
	else
	{
	address();
	if((amode==1)||(amode==2))
		perror("illegal addressing mode");
	}
	code(slist(table[opsym].value | ((amode-1)<<4)));
	if((amode==3)||(amode==4)) outadd();
	}
form4(){
	ac();
	code(slist(table[opsym].value | acc ));
	blank(7);
	}
form5(){
	address();
	code(slist(table[opsym].value|((amode-1)<<4)));
	if(amode==1) amode=4;
	outadd();
	}
form6(){
	address();
	if(amode==1) perror("illegal addressing mode");
	code(slist(table[opsym].value |((amode-1)<<4)));
	outadd();
	}
form7(){
	address();
	if((amode==1)||(amode==2)) perror("illegal addressing mode");
	code(slist(table[opsym].value|((amode-1)<<4)));
	outadd();
	}
form8(){
	int tmp;
	address();
	if((amode>0)&&(amode!=4)) perror("illegal addressing mode");
	tmp=add-table[pc].value-2;
	code(slist(table[opsym].value));
	if(pass==2)if((tmp>255)||(tmp< -256))
			perror("branch out of range");
	code(slist(tmp&0377));
	blank(3);
	}
org(){
	int tmp;
	tmp=exp();
	if(expflg==1) perror("syntax error");
	else table[pc].value=tmp;
	blank(4); list(tmp);
	if(pass==2) {
	if(codec!=0) endblk();
	stblk();
	}
	}
end(){
	int tmp,flg;
	if(pass==2) {
		if(codec!=0) endblk();
		write(fcode,"S9030000FC",10);
		blank(11);
		return;
		}
	flg=0;
	for(tmp=0;table[tmp].symbol!=0;tmp++) {
		if(table[tmp].format==17) {
			printf("undefined symbol %s\n",table[tmp].symbol);
			flg=flg+1; }
		}
	if((flg!=0)||(nerror>0)) exit();
	seek(0,0,0);
	table[pc].value=0;
	ln=0;
	}
equ(){
	int tmp;
	tmp=exp();
	if(expflg==1) table[labsym].format=30;
	else table[labsym].value=tmp;
	blank(4); list(tmp);
	}
fcb(){
	int tmp;
loop:	tmp=exp();
	if(pass==2)if((tmp>255)||(tmp< -256)) perror("truncation error");
	code(tmp&0377);
	if(table[symptr].format==22) {
		getsym();
		goto loop;
		}
	blank(11);
	}
fcc(){
	char *ptr;
	char del;
	table[symptr].format=18;
	ptr=table[symptr].symbol;
	del= *ptr; ptr=ptr+1;
	while((*ptr!=0)&&(*ptr!=del)) {
		code(*ptr);
		ptr=ptr+1;
		}
	blank(11);
	}
fdb(){
	int tmp;
loop:	tmp=exp();
	code((tmp>>8)&0377);
	code(tmp&0377);
	if(table[symptr].format==22) {
		getsym();
		goto loop;}
	blank(11);
	}
rmb(){
	int tmp;
	tmp=exp();
	if(expflg==1) perror("syntax error");
	else table[pc].value=table[pc].value+tmp;
	blank(4); list(tmp);
	if(pass==2) {
	if(codec!=0) endblk();
	stblk();
	}
	}

getsym(){
	extern lookup();
	int taction;
	int tmp;
	type=0;
	tmp=sptr;
	symp=0;
loop:	class=clss[line[sptr]];
	taction=mat[class].action[state];
	state=mat[class].stat[state];
	switch(taction) {
		case 1: sym[symp]=line[sptr];
			if(symp<80) symp=symp+1;
			break;
		case 4: type=type+1;
			if(tmp!=0) type=type+1;
		case 3: type=type+1;
		case 2: sym[symp]=0;
			lookup();
			return;
		case 5: type=3;
			sym[symp]='\n';
			symp=symp+1;
			sym[symp]=0;
			lookup();
			return;
		case 6: tmp=sptr+1;
			break;
			}
		sptr=sptr+1;
		goto loop;
	}

lookup(){
	symptr=0;
loop:	symp=0;
	stptr=table[symptr].symbol;
	if(stptr==0) goto fail2;
loop0:	if(*stptr!=sym[symp]) goto fail;
	if((*stptr==0)&&(sym[symp]==0)) goto succ;
	stptr=stptr+1;
	symp=symp+1;
	goto loop0;
fail:	symptr=symptr+1;
	goto loop;
fail2:	table[symptr].symbol=freep;
	table[symptr+1].symbol=0;
	if(type==1) {
		table[symptr].format=19;
		table[symptr].value=atoi(sym);
		}
	else
	if((type==2)||(type==3)) table[symptr].format=17;
	symp=0;
loop1:	*freep=sym[symp];
	freep=freep+1;
	symp=symp+1;
	if(sym[symp-1]!=0) goto loop1;
succ:	;
	}

rline(){
	lpos=0;
	sptr=0;
loop:	if(rlcnt==0) {
		if((rlcnt=read(0,buf,100))==0) return(0);
		rlpos=0;
		}
	line[lpos]=buf[rlpos];
	rlcnt=rlcnt-1;
	rlpos=rlpos+1;
	lpos=lpos+1;
	if(line[lpos-1]!='\n') goto loop;
	line[lpos]=0;
	ln=ln+1;
	if(pass==2) {
		printf(-1,lstr,"%3d %06o",ln,table[pc].value);
		write(flist,lstr,10);
		}
	return(lpos);
	}
main(){

	extern int rline();
	extern skip(),form0(),form1(),form2(),form3();
	extern form4(),form5(),form6(),form7(),form8();
	extern org(),end(),equ(),fcb(),fcc(),fdb(),rmb();
	space=' ';
	tab='	';
	nerror=0;
	ln=0;
	freep= &frespc[0];
	sym[0]='*'; sym[1]=0;
	lookup();
	pc=symptr;
	table[pc].value=0;
	pass=1;
	flist=creat("a.list",0644);
	fcode=creat("a.out",0644);
	leader();
	codec=0;
	while(rline()!=0) {
		getsym();
		ltype=type;
		if(type==2) {
			if(pass==1) defsym();
			labsym=symptr;
			getsym();
			}
			opsym=symptr;
		if((ltype==2)&&(pass==2)&&(table[opsym].format!=11))
			{
			if(table[labsym].value!=table[pc].value)
				perror("phase error");
			}
			getsym();
			switch(table[opsym].format){
				case 16:form0();
					break;
				case 1:	form1();
					break;
				case 2:	form2();
					break;
				case 3:	form3();
					break;
				case 4:	form4();
					break;
				case 5:	form5();
					break;
				case 6:	form6();
					break;
				case 7:	form7();
					break;
				case 8:	form8();
					break;
				case 9:	org();
					break;
				case 10:end();
					break;
				case 11:equ();
					break;
				case 12:fcb();
					break;
				case 13:fcc();
					break;
				case 14:fdb();
					break;
				case 15:rmb();
					break;
				default:perror("unrecognized keyword");
				}
		if(pass==2) {
			printf(-1,lstr,"   %s",line);
			write(flist,lstr,count(lstr));
			}
		if(table[opsym].format==10) pass=pass+1;
		comment();
		if(pass==3) exit();
		}
	}
