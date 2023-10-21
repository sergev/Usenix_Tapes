mixerup(table,texts)
	int *table;	char *texts;
{
	register *p;
	p = table;
	*p++ = texts;
	*p = -1;
	p = texts;
	*p = '\0';
}
