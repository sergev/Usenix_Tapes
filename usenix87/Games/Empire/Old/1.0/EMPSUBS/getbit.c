getbit(bit, bitmap)
int	bit, bitmap[];
{

	return((1 << (bit&017)) & bitmap[(bit>>4) & 01777]);
}

setbit(bit, bitmap, value)
int	bit, bitmap[], value;
{
	register int	i, b, *bp;

	bp = &bitmap[(bit>>4) & 01777];
	b = bit;
	b &= 017;
	i = value;
	i <<= b;
	if( i != 0 ) {
		*bp |= i;
	} else {
		*bp &= ~(1 << b);
	}
	return;
}
