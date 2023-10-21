itoa(nm)
{
	register char *p;
	register num;
	static char buf[8];

	if(nm == -32768)
		return("-32768");
	else
		p = &buf[7];
	*p = 0;
	num = abs(nm);
	do {
		*--p = num % 10 + '0';
		num =/ 10;
	} while(num);
	if(nm < 0)
		*--p = '-';
	return(p);
}
