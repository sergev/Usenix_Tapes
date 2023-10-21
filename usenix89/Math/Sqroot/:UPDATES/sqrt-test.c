

/* Faster integer square root test routine.
 * This is actually the same algorithm you learned in grade school and
 * have painfully rediscovered every five years since then.  Thanks to
 * Charles Mesztenyi for suggesting the binary analog.
 * Ben Cranston 4/23/87
 */

int isqrt(n)
int n;
{
    int a,b,c,as,bs;

    a = 1;
    b = 1;
    while (a<=n) {
	a = a << 2;
	b = b << 1;
    }
    as = 0;
    bs = 0;
    while ( (b>1) && (n>0) ) {
	a = a>>2;
	b = b>>1;
	c = n - (as|a);
	if ( c>=0 ) {
	    n = c;
	    as |= (a<<1);
	    bs |= b;
	}
	as = as>>1;
    }

    return(bs);
}

main(argc,argv)
int argc;
char **argv;
{
    int num,root,last;

    last = -1;
    for ( num=0 ; num<65000 ; num++ ) {
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
    printf("Yow!  Are we having fun yet?\n");
}
