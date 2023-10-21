alsort(stable,vtable)
	char **stable;	int *vtable;
{
	register *p;
	register char *t,*b;
	int nswap,ip,tmp;

	if((stable[0] == -1)||(stable[1] == -1))return;
loop:	p = stable;	ip = 0;		nswap = 0;
iloop:	while(*(p+2) != -1){
		p++;
		t = *p;		b = *(p+1);		ip++;
		while(*t && *b){
			if(*t < *b)goto iloop;
			    else if(*t++ != *b++){
				exchs(stable,ip);
				if(vtable != -1){
					tmp = vtable[ip];
					vtable[ip] = vtable[ip + 1];
					vtable[ip + 1] = tmp;
					}
				nswap++;
				goto iloop;
				}
			}
		}
	if(nswap)goto loop;
}
exchs(stable,i)
	int i;	char **stable;
{
	register char *f,*t;
	register n;
	char tmp[140];

	n = i;
	f = stable[n];	t = tmp;
	while(*t++ = *f++);		/* save 1st string	*/
	t = stable[n++];	f = stable[n];
	for(n = 1;*t++ = *f++;n++);	/* put 2nd in as 1st	*/
	stable[i+1] = stable[i] + n;	/* re-do 2nds' pointer	*/
	f = tmp;
	while(*t++ = *f++);		/* put saved 1st in as 2nd */
}
