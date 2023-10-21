jnum(n,base,nchar,jus)
  int n,base,nchar,jus;
{
register i;
  i=nchar-(digcnt(n,base));
  if(jus==1){
	basout(n,base);
	for(;i> 0;i--)putchar(' ');
	}
  else {
	for(;i> 0;i--)putchar(' ');
	basout(n,base);
	}
}
