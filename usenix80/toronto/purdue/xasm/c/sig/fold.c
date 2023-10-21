/*	This is the convolution subroutine. It does summing
	knowing the length of the fcns and the sample rate.  */

fold(la,a,lb,b,lc,c,sample)
	float *a,*b,*c,sample;	int la,lb,lc;
{
	register i,j,k;
	lc = la + lb - 1;
	presto(lc,c,0.);
	for(i = 0;i != la;i++)
	    for(j = 0;j != lb;j++){
		k = i + j;
		c[k] =+ (a[i] * b[j])/sample;

		}
}
