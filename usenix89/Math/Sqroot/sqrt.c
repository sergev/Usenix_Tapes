
/* Fast integer square root test routine.
 * Ben Cranston 3/9/87
 */

int isqrt(n)
int n;
{
    int a,b,c;

    a = n;
    b = n;
    if (n>1) {
	while (a>0) {
	    a = a >> 2;
	    b = b >> 1;
	}
	do {
	    a = b;
	    c = n / b;
	    b = (c + a) >> 1;
	} while ( (a-c)<-1 || (a-c)>1 );
    }

    return(b);
}

main(argc,argv)
int argc;
char **argv;
{
    int num,root,last;

    last = 0;
    for ( num=1 ; num<65000 ; num++ ) {
	root = isqrt(num);
	if ( (root*root)>num ) {
	    printf("Too big!\n");
	}
	if ( ((root+1)*(root+1)) <= num ) {
	    printf("Too Small!\n");
	}
	if (last!=root) {
	    last = root;
	    printf("The square root of %4d is %3d\n",num,root);
	}
    }
}
