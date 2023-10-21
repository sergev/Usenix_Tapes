# define copy(source, sink)	while(*sink++ = *source++)

char *repeat(result, str, repfac)
register char *result;
char *str;
int repfac;
{
register i;
register char *pstr;
char *presult;

	presult = result;
	pstr = str;
	copy(pstr,result);
	for(i = 0; i < repfac-1; i++){
		pstr = str;
		result--;
		copy(pstr, result);
	}
	return(presult);
}
