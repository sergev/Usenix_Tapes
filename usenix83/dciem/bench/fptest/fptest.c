/* floating point benchmark process
 * this program is intended to take up a moderate amount of main memory,
 * and burn cpu cycles. It is one component of an overall UNIX benchmarking
 * scheme.
 * Author: Martin Tuori, DCIEM, Toronto, 1981.
 */

char consume_space[1024*12];

main(){
	float f1,f2;
	int i,j,ival;
	char cval;
	long longval;
	double dblval;

	f1= 12345.6789;
	f2= 98765.432;
	for(i=20000;i--;){
		for(j=20;j--;){
			f1= f1 *f2/f2*f2/f2*f2/f2;
		}
		cval= ival= longval= dblval= f1;
	}
	printf("%d,%d,%ld,%e,%e\n",cval,ival,longval,dblval,f1);
}
