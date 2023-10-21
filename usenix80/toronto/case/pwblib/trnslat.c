char *trnslat(str, old, new, result)
register char *str;
char old, new, *result;
{
register char *presult;

	for(presult = result; *str; str++)
		*presult++ = (*str == old ? new: *str);
	return(result);
}
