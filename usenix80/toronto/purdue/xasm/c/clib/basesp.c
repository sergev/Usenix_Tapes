basesp(n,t,base)
	int n,base;
	char *t;
{
int p[10];
register i;
register char *j;
j = t;
p[0]= 0;
for(i=0;n != 0;i++){
	p[i] = lrem(0,n,base);
	n = ldiv(0,n,base);
	}
if(i!=0)--i;	/* remove this line if allways want leading 0 */
for(;i >= 0;i--)*j++ = (((p[i] > 9)?(p[i]+87):(p[i]+'0')));
*j = '\0';
}
