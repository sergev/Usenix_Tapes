/*
 *	splitf -- split fortran file into seperate subs.
 *
 *	splitf <files>
 *
 *	jjb, jan '77
 */

char line[130];	/* current line buffer */
int llen;	/* current line length */
int input, output; /* I/O file descriptors */
char name[20];	/* current output file name */
int data_present 1; /* end of data flag */

main(argc, argp)
char **argp;
{
	while(argc-- > 1){
		++argp;
		if((input = open(*argp, 0)) < 0){
			printf("can't read %s\n", *argp);
			continue;
		}
		while(data_present)
			copysub();
		close(input);
	}
}

/*
 *	copysub -- copy the next module from input to
 *			output. Try to select an appropriate
 *			file name.
 */
copysub()
{
	getname();
	while(getline() && !endcard())
		putline();
	putline();
	close(output);
}

/*
 *	getline -- read 1 line from input;
 *			signal eof by clearing "data present".
 */

char *bnext;
char buf[512];
int bcnt 0;

getline()
{
	register char c, *p1, *p2;

	p1 = bnext;
	p2 = line;
	llen = 0;
	do {
		if(bcnt-- > 0){
			c = *p2++ = *p1++;
			llen++;
			if(llen > 131)
				llen--,p2--;
		} else {
			if((bcnt = read(input, buf, 512)) <= 0){
				data_present = 0;
				return(0);
			}
			p1 = buf;
		}
	} while(c != '\n');
	*p2++ = 0;
	bnext = p1;
	return(1);
}

/*
 *	putline -- output current line
 */

putline()
{
	write(output, line, llen);
}

/*
 * endcard -- check to see if this card is an end card
 */

endcard()
{
	char c, *p1, *p2;

	p1 = line;
	p2 = "end";
	while((c = *p1++) != '\n'){
		if(c == ' ' || c == '\t')
			continue;
		if(c != *p2++)
			return(0);
	}
	return(*p2 == 0);
}

/*
 * getname -- determine output file name and open file.
 *
 *		tries to find name of current module from
 *		the "sub" "prog" "funct", etc. cards, if present.
 */

getname()
{
	register char *p1, *p2;
	register int i;

	if(!getline())
		return(0);
	p2 = name;
	if((p1 = keyword())){
		for(i=0; i<6; i++){
			*p2++ = *p1++;
			if(any(p2[-1], " \t"))
				i--,p2--;
			if(any(p2[-1], "\n(") || p2[-1] == 0){
				p2--;
				break;
			}
		}
		*p2++ = '.';
		*p2++ = 'f';
		*p2++ = 0;
	} else {
		gensym(name);
	}
	while((output = creat(name, 0644)) < 0){
		printf("can't create %s\n", name);
			gensym(name);
	}
	putline();
	printf("%s:	%s", name, line);
}

/*
 *	gensym -- generate a unique name
 */

gensym(s)
char *s;
{
	static int proto;
	register char *p1, *p2;

	p1 = s;
	p2 = "sub";
	while(*p1++ = *p2++){}
	p1[-1] = (proto/100)%10 + '0';
	*p1++  = (proto/10 )%10 + '0';
	*p1++  = proto%10 + '0';
	*p1++  = '.';
	*p1++  = 'f';
	*p1++  = 0;
	proto++;
}

/*
 * keyword -- recognize keyword
 */
keyword()
{
	register char *p;

	if(*line != ' ' && *line != '\t')
		return(0);	/* not there */
	if(p = find("program"))
		return(p);
	if(p = find("subroutine"))
		return(p);
	if(p = find("function"))
		return(p);
	if(p = find("integerfunction"))
		return(p);
	if(p = find("logicalfunction"))
		return(p);
	if(p = find("complexfunction"))
		return(p);
	if(p = find("realfunction"))
		return(p);
	if(p = find("blockdata"))
		return("bdata");
	return(0);		/* not found */
}

/*
 *	find -- locate keyword
 */

find(s)
char *s;
{
	register char c, *p1, *p2;

	p1 = line;
	p2 = s;
	while(c = *p1++){
		if(c == ' ' || c == '\t')
			continue;
		if(*p2 == 0)
			return(p1-1);
		if(c != *p2++)
			return(0);
	}
	return(0);
}

/*
 *	any -- true if c in s
 */

any(c,s)
char c,*s;
{
	register cc;

	while(cc = *s++)
		if(cc == c)
			return(1);
	return(0);
}
