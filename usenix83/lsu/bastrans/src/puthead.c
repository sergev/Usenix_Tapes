puthead(fd,title)
int fd;
char *title;
{
	register i;
	int tbuf[2];
	char *c;
	time(tbuf);
	c=ctime(tbuf);
	squeeze(c,'\n',0,0);
	for (i=0;(title[i]=upper(title[i]))!='\0';i++);
	title[i]='\0';
	fprintf(fd,"\n#\n#include <stdio.h>\n#include <math.h>\n\n/*\n *\n");
	fprintf(fd," *\t\t\t%s\n *\tTranslated by BASTRANS %s\n */\n\n",title,c);
	fprintf(fd,"main()\n{\n");
}
