dumpt(table)
	char **table;
{
	register *p;
	register i;
	p = table;
	p++;
	i = 1;
	while(*p != -1){
		jnum(i++,10,3,0);
		printf("- %s\n",*p++);
	}
}
