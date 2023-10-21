#include <stdio.h>

char copr[] "Copyright (c) 1976 Thomas. S. Duff";
int blankcount;
struct header{
	int magic;
	int size;
	int maxx;
	int maxy;
	int xtnd;
	int line_sp;
}header;
struct dispatch{
	char *addr;
	char extend;
	char width;
}dispatch[128];
char *fname "banner.v";
#define BITSPERBYTE 8
int bytesperline;
getfont(){
	register font, bits;
	register struct dispatch *p;
	if((font=open(fname,0))==  -1){
		chdir("/usr/font");
		if((font=open(fname,0))== -1)
			err("Font open error");
	}
	read(font,&header,sizeof header);
	if(header.magic!=0435)
		err("Bad font file");
	read(font,&dispatch,sizeof dispatch);
	if((bits=malloc(header.size))== NULL)
		err("Can't alloc");
	if(read(font,bits,header.size)!=header.size)
		err("Bad font header");
	close(font);
	bytesperline=(header.maxx+BITSPERBYTE-1)/BITSPERBYTE;
	for(p=dispatch;p!=dispatch+128;p++)
		if(p->addr)
			p->addr=+bits;
}
main(argc, argv)
char **argv;
{
	register i;
	register char *k;
	char buf[BUFSIZ];

	setbuf(stdout,buf);
	--argc;
	argv++;
	if(argc>1 &&  **argv=='-'){
		fname = *argv+1;
		--argc;
		argv++;
	}
	getfont();
	while(argc){
		for(i=0;i!=header.maxy+header.xtnd;i++){
			for(k = *argv;*k;k++)
				scanchar(*k, i);
			blankcount=0;
			putchar('\n');
		}
		--argc;
		argv++;
		if(argc)
			for(i=0;i!=header.line_sp;i++)
				putchar('\n');
	}
	exit(0);
}
scanchar(c, line){
	register char *bitp;
	register i, j;
	struct dispatch *p;
	p=dispatch+c;
	line =- p->extend;
	if(line<0 || line>=header.maxy || p->addr==0){
		blankcount=+p->width;
		return;
	}
	bitp=p->addr + bytesperline*line;
	j=0200;
	for(i=0;i!=p->width;i++){
		if(*bitp&j){
			while(blankcount){
				putchar(' ');
				--blankcount;
			}
			putchar('x');
		}
		else
			blankcount++;
		j=>>1;
		if(j==0){
			bitp++;
			j=0200;
		}
	}
}
err(m)
	char *m;
	{
	fprintf(stderr, "%s.\n", m);
	exit(-1);
}
