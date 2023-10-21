sjnum(n,t,base,nchar,jus)
  int n,base,nchar,jus,t;
{
register i;
  i=nchar-(digcnt(n,base));
  if(jus==1){
	basesp(n,t,base);
	for(;i> 0;i--)putchar(' ');
	}
  else {
	for(;i> 0;i--)putchar(' ');
	basesp(n,t,base);
	}
}
