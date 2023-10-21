#include <stdio.h>
/*
 * C [-line width] [+blank count] [filename]
 */

/*
 * Much of the complications in this program
 * comes from trying to optimize the I/O.
 */

char proto[] { "/tmp/lf??" };

char iobuf[512];

main( argc,argv )
	char *argv[];
	{
	extern interrupt();
	extern char _sibuf[],_sobuf[];
	register char *bp, *be;
	register lw;
	int line_width, blank_count, biggest, n_a_line;
	int input, output, cp, lc, buffer_size;
	char *arg;

	setbuf(stdin, _sibuf);
	setbuf(stdout, _sobuf);
	line_width = 80;
	blank_count = 2;
	while( argc-- > 1 ){
		arg = argv[argc];
		switch( *arg ){
		case '-':
			line_width = number( &arg[1] );
			continue;
		case '+':
			blank_count = number( &arg[1] );
			continue;
			}

		close( 0 );
		if( open(arg,0) < 0 )
			error("Can't open '%s'\n",arg);
		}

	if( (output=falloc( proto )) < 0 )
		error("Can't get temp file\n");
	signal(2,&interrupt);
	lw = biggest = 0;

	while( buffer_size = read(0,iobuf,512) ){
		be = &iobuf[buffer_size];
		bp = &iobuf[0];
		while( bp<be )
			if( *bp++ == '\n' ){
				biggest = max(biggest,lw);
				lw = 0;
				}
			else
				lw++;
		write(output,iobuf,buffer_size);
		}

	n_a_line = ( line_width-1 ) / ( biggest=+blank_count );
	close( 0 );
	close( output );
	input = open(proto,0);
	lw = cp = 0;

	while( buffer_size = read(input,iobuf,512) ){
		be = &iobuf[buffer_size];
		bp = &iobuf[0];
		while( bp<be ){
			if( *bp=='\n' ){
				if( ++cp >= n_a_line ){
					putchar('\n');
					cp = 0;
					}
				else 	{
					while(biggest>lw) {
						putchar(' ');
						lw++;
					}
				}
				lw = 0;
				bp++;
				}
			else	 {
				putchar(*bp++);
				lw++;
				}
			}
		}
	if( cp ) putchar('\n');
	unlink(proto);
	}

max( a,b )
	{
	if( b<a ) return( a );
	return( b );
	}

falloc( file )
	char *file;
	{
	int i, nq, pow, p, u;
	char working[30];
	for(i=nq=0; file[i] ; ++i ) nq =+ file[i]=='?';
	for(pow=1; nq; --nq ) pow =* 10;
	for(i=0; i<pow; ++i ){
		p = 0;
		copy(file,working);
		u = i;
		while( u ){
			while( working[p]!='?' ) p++;
			working[p] = u%10 + '0';
			u =/ 10;
			}
		while( working[p] )
			if( working[p++] == '?' )
				working[p-1] = '0';
		if( (u=creat(working,0444)) >= 0){
			copy(working,file);
			return(u);
			}
		}
	return( -1 );
	}

copy( in,out )
	char *in, *out;
	{
	register char *i, *o;
	i = in;
	o = out;
	while( *o++ = *i++ );
	return( out );
	}

interrupt(){
	unlink(proto);
	exit();
	}

number( s )
	char *s;
	{
	register c;
	register char *sp;
	c = 0;
	sp = s;
	while( *sp>='0' && *sp<='9' ) c = c*10 + *sp++ - '0';
	if( *sp!='\0' )
		error("%s: illegal numeric\n",s);
	return( c );
	}

error( format,a1,a2,a3 ){
	printf( format,a1,a2,a3);
	exit();
	}
