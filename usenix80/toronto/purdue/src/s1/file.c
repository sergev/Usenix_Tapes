int in;
int i 0;
char buf[512];
int mbuf[20];
int *wd {
	&buf[0]};
char *fort[]{
	"function","subroutine","common","dimension","block","integer",
	"real","data","double",0};
char *asc[]{
	"sys","mov","tst","clr","jmp",0};
char *c[]{
	"int","char","float","double","struct","extern",0};
char *as[]{
	"globl","byte","even","text","data","bss","comm",0};
struct apltab{
	char h;
	char l;
} apl[] {
	'O\\', 'O|', 'O*', 'BN', 'CJ', 'A|',  '\'L',
	'V|', '~&', '~V', '~A', '\'.', '<=', '>=',
	'=/', '=|', 'O-', '/-', '<-', '->', 'PL',
	'-:',
	0,0};

char *aplqfn[]{
	"width", "run", "fuzz", "fork", "wait", "exec", "cr",
	"fx", "exit", "pipe", "chdir", "open", "close", "read",
	"write", "creat", "seek", "kill", "rd", "rm", "dup", "ap",
	"rline", "nc", 
	0};
int ibuf[260];
main(argc, argv)
char **argv;
{

	while(argc > 1) {
		printf("%-16s ", argv[1]);
		type(argv[1]);
		argc--;
		argv++;
	}
}

type(file)
char *file;
{
	int j,nl;
	char ch;

	if(stat(file, mbuf) < 0) {
		printf("cannot stat\n");
		return;
	}
	switch(mbuf[2]&060000) {

	case 020000:
		printf("character");
		goto spcl;

	case 040000:
		printf("directory\n");
		return;

	case 060000:
		printf("block");

spcl:
		printf(" special file (%d/%d)\n",
		(mbuf[6]>>8)&0377,
		mbuf[6]&0377);
		return;
	}

	ibuf[0] = open(file, 0);
	if(ibuf[0] < 0) {
		printf("cannot open\n");
		return;
	}
	in = read(ibuf[0], buf, 512);
	switch(*wd) {

	case 0407:
		printf("executable");
		goto a_out;

	case 0410:
		printf("shared executable");
		goto a_out;

	case 0411:
		printf("shared & separated executable");
a_out:
		if(wd[4] == 0)
			printf(",  stripped");
		if(wd[7] == 0)
			printf(",  relocatable");
		putchar('\n');
		goto out;

	case 0177545:
		printf("archive (version 7) library\n");
		goto out;

	case 0177555:
		printf("archive (version 6) library\n");
		goto out;

	case 0100554:
		printf("APL workspace\n");
		goto out;

	case 0100555:
		printf("APL2 workspace\n");
		goto out;

	case 052525:
		printf("PDS file\n");
		goto out;

	case 017437:
		printf("Packed file\n");
		goto out;
	}

	i = 0;
	if(ccom() == 0)goto notc;
	while(buf[i] == '#'){
		j = i;
		while(buf[i++] != '\n'){
			if(i - j > 255){
				printf("data\n"); 
				goto out;
			}
			if(i >= in)goto notc;
		}
		if(ccom() == 0)goto notc;
	}
check:
	if(lookup(c) == 1){
		while((ch = buf[i++]) != ';' && ch != '{')if(i >= in)goto notc;
		printf("c program");
		goto outa;
	}
	nl = 0;
	while(buf[i] != '('){
		if(buf[i] <= 0){
			printf("data\n"); 
			goto out; 
		}
		if(buf[i] == ';'){
			i++; 
			goto check; 
		}
		if(buf[i++] == '\n')
			if(nl++ > 6)goto notc;
		if(i >= in)goto notc;
	}
	while(buf[i] != ')'){
		if(buf[i++] == '\n')
			if(nl++ > 6)goto notc;
		if(i >= in)goto notc;
	}
	while(buf[i] != '{'){
		if(buf[i++] == '\n')
			if(nl++ > 6)goto notc;
		if(i >= in)goto notc;
	}
	printf("c program");
	goto outa;
notc:
	i = 0;
	while(buf[i] == 'c' || buf[i] == '#'){
		while(buf[i++] != '\n')if(i >= in)goto notfort;
	}
	if(lookup(fort) == 1){
		printf("fortran");
		goto outa;
	}
notfort:
	i=0;
	if(ascom() == 0)goto notas;
	j = i-1;
	if(buf[i] == '.'){
		i++;
		if(lookup(as) == 1){
			printf("assembler program"); 
			goto outa;
		}
		else if(buf[j] == '\n'){
			printf("roff, nroff, or eqn input");
			goto outa;
		}
	}
	while(lookup(asc) == 0){
		if(ascom() == 0)goto notas;
		while(buf[i] != '\n' && buf[i++] != ':')
			if(i >= in)goto notas;
		while(buf[i] == '\n' || buf[i] == ' ' || buf[i] == '\t')if(i++ >= in)goto notas;
		j = i-1;
		if(buf[i] == '.'){
			i++;
			if(lookup(as) == 1){
				printf("assembler program"); 
				goto outa; 
			}
			else if(buf[j] == '\n'){
				printf("roff, nroff, or eqn input");
				goto outa;
			}
		}
	}
	printf("assembler program");
	goto outa;
notas:

	/* see if apl file */
	for(i=0; i<in; i++)
		if(buf[i] < 0)
			goto notapl;
	j = 0;
	for(i=1; i<in-1; i++)
		switch(buf[i]){
		case '\010':
			if(aplbs(buf[i-1], buf[i+1]))
				goto isapl;
			else
				goto notapl;
		case ';':
			if(buf[i+1] == ';' || buf[i+1] == ']')
				j++;
			break;
		case '[':
			if(buf[i+1] == ';')
				j++;
			break;
		case 'L':
			if(qdfn(&buf[i+1]))
				goto isapl;
			break;
		}
	if(j)
		goto probapl;
	else
		goto notapl;
probapl:
	printf("probably an ");
isapl:
	printf("apl function\n");
	goto out;

notapl:
	for(i=0; i < in; i++)if(buf[i] <= 0){
		printf("data\n"); 
		goto out; 
	}
	if((mbuf[2] & 00111) != 0)
		printf("commands");
	else printf("probably text");
outa:
	while(i < in)
		if(buf[i++] <= 0){
			printf(" with garbage\n");
			goto out;
		}
	while((in = read(ibuf[0],buf,512)) > 0)
		for(i = 0; i < in; i++)
			if(buf[i] <= 0){
				printf(" with garbage\n");
				goto out;
			}
	printf("\n");
out:
	close(ibuf[0]);
}
lookup(tab)
char *tab[];
{
	char r;
	int k,j,l;
	while(buf[i] == ' ' || buf[i] == '\t' || buf[i] == '\n')i++;
	for(j=0; tab[j] != 0; j++){
		l=0;
		for(k=i; ((r=tab[j][l++]) == buf[k] && r != '\0');k++);
		if(r == '\0')
			if(buf[k] == ' ' || buf[k] == '\n' || buf[k] == '\t'
			    || buf[k] == '{' || buf[k] == '/'){
				i=k;
				return(1);
			}
	}
	return(0);
}
ccom(){
	char cc;
	while((cc = buf[i]) == ' ' || cc == '\t' || cc == '\n')if(i++ >= in)return(0);
	if(buf[i] == '/' && buf[i+1] == '*'){
		i =+ 2;
		while(buf[i] != '*' || buf[i+1] != '/'){
			if(buf[i] == '\\')i =+ 2;
			else i++;
			if(i >= in)return(0);
		}
		if((i =+ 2) >= in)return(0);
	}
	if(buf[i] == '\n')if(ccom() == 0)return(0);
	return(1);
}
ascom(){
	while(buf[i] == '/'){
		i++;
		while(buf[i++] != '\n')if(i >= in)return(0);
		while(buf[i] == '\n')if(i++ >= in)return(0);
	}
	return(1);
}

aplbs(c1, c2)
char c1, c2;
{
	struct apltab *p;

	p = apl;
	while(p->h){
		if((p->h == c1 && p->l == c2)
		|| (p->h == c2 && p->l == c1))
			return(1);
		p++;
	}
	return(0);
}

qdfn(s)
char *s;
{
	char *p, *q, **t;

	t = aplqfn;
	while(p = *t++){
		q = s;
		while(*q++ == *p)
			p++;
		if(*p == 0)
			return(1);
	}
	return(0);
}
