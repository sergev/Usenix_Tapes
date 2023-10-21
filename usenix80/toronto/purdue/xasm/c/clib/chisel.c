chisel(index,table)
	int index,*table;
{
	register t,f;
	register *i;
	t = table[index] - table[0];
	f = table[index + 1] - table[0];
	bumpd(table[0],t,f,lost(table));
	i = table;
	while(*++i != -1)if(i >= (table+ index+ 1)){
				*i=- f-t;
				}
	slide(table,index,index+1,(i- table));
}

lost(a)
	char **a;
{
	register *i;
	register char *p;
	i = a;
	while(*i++ != -1);
	i =- 2;
	p = *i;
	while(*p++);
	return((--p) - *a);
}
