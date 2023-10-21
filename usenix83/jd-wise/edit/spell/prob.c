double p,s,y,atof(),log(),exp();
long d,k,n,atol();
char line[128];
main()
	{
    loop:
	printf("d= ");
	gets(line);
	d=atol(line);
	printf("n= ");
	gets(line);
	n=atol(line);
	printf("best k= %f\nk= ",log(2.)*n/d);
	gets(line);
	k=atol(line);
	y=d*k;
	y=y/n;
	for(s=p=1.-exp(-y); --k;)
		p *= s;
	printf("p=%f (1-p)**k=%f\n",exp(-y),p);
	goto loop;
	}
