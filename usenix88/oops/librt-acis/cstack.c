int f1(p1,p2,p3,p4,p5,p6,p7,p8)
int p1,p2,p3,p4,p5,p6,p7,p8;
{
	register r1 = p1+p8;
	register r2 = p2+p8;
	register r3 = p3+p8;
	register r4 = p4+p8;
	p4 = f2(r1,r2,r3,r4);
	return p4;
}

int f2(p1,p2,p3,p4)
int p1,p2,p3,p4;
{
	int x[129];
	float f1 = 1.0, f2 = 2.0, f3 = 3.0, f4 = 4.0;
	return x[128];
}

extern double pow();

float f3(p1,p2,p3,p4,p5,p6,p7,p8)
float p1,p2,p3,p4,p5,p6,p7,p8;
{
	register float r1 = p1*p8;
	register float r2 = p2*p8;
	register float r3 = p3*p8;
	register float r4 = p4*p8;
	p4 = pow(r1,r2) + pow(r3,r4);
	return p4;
}

main()
{
	int x;
	x = f1(1,2,3,4,5,6,7,8);
}
