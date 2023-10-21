/* This subroutine is used mainly to zero an array  */

presto(lx,x,preset)
	int lx;	float *x,preset;
{
	register i;
	i = 0;
	while(i < lx)x[i++] = preset;
}
