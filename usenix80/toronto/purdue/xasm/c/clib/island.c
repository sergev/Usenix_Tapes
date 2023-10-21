island(ch)
	char ch;
{
	if(((ch >= 'A')&&(ch <= 'Z'))||((ch >= 'a')&&(ch <= 'z'))) return (1);
	if((ch >= '0')&&(ch <= '9')) return(1);
	if(ch == '_')return(1);
	return(0);
}
