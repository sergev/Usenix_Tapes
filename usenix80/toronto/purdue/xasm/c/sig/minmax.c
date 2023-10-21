minmax(ldata,data,min,max)
	int ldata;	float *data,*min,*max;
{
	register i;
	float mmin,mmax;
	mmin = 10000.0;	mmax = -10000.0;
	for(i = 0;i != ldata;i++){
		if(data[i] < mmin)mmin = data[i];
		if(data[i] > mmax)mmax = data[i];
		}
	*min = mmin;	*max = mmax;
}
