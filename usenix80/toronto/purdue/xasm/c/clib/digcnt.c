digcnt(n,p)
	int n,p;	{
register i;
for( i= 0;n != 0;i++)n = ldiv(0,n,p);
if(i==0)i++;
return(i);
}
