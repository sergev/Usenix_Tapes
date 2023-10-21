squeeze(lold,old,lnew,new)
	int lold,lnew;	float old[],new[];
{
	register j,k,l;	float scale,delta,fk,position,flold,flnew;

	flold = lold;
	flnew = lnew;
	scale = flold / flnew;
	for(j = 0;j != lnew;j++){
		position = scale * j;	k = position;
		fk = k;
		delta = position - fk;
		new[j] = ((old[k+1] - old[k]) * delta) + old[k];
		}
}
