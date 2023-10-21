/*
 *
 * Or.c    Or's two files and places the result in the destination
 *      usage : or infile in2file outfile
 * Modified by K.L. Martin from Jerome M. Lang Copy.c program
 * February 23, 1986
 */

#include <osbind.h>

#define F_READ  0
#define F_WRITE 1

#define BUF_LEN 32034

typedef int     FDESC;

char    buffer[ BUF_LEN ];
char    buffer2[ BUF_LEN ];

main(argc, argv)
int     argc;
char    *argv[];
{

        FDESC   infile, outfile, in2file;
        FDESC   openwr();

        if( argc < 2)
                usage();

        while( --argc > 0 )
        {
                if( 0 > (infile = (FDESC)Fopen(*++argv,F_READ)) ) {
                        erreur(" Can't open infile ",*argv);
                        break;
                }
                if( 0 == --argc) {
                        usage();
                        break;
                }

                Cconws(*argv);
                Cconws("|");
                if( 0 > (in2file = (FDESC)Fopen(*++argv,F_READ)) ) {
                        erreur(" Can't open in2file ",*argv);
                        break;
                }
                Cconws(*argv);
                Cconws("->");

                if( 0 == --argc) {
                        usage();
                        break;
                }

                if( 0 > (outfile = openwr(*++argv)) ) {
                        erreur(" Can't create outfile ", *argv);
                        fclose(infile);
                        break;
                }


                Cconws(*argv);
                Cconws("\r\n");

                orfile(infile, in2file, outfile);

                Fclose(infile);
                Fclose(in2file);
                Fclose(outfile);
        }
}

erreur(a,b)
char *a, *b;
{
        Cconws(a);
        Cconws(b);
        Cconws("\r\n");
        Cconout('\007');
        Cconin();
}

usage()
{
                erreur("usage: or infile in2file outfile ","");
}

orfile(in, in2, out)
FDESC   in, in2, out;
{
        int     i;
        long    howmany, howmany2;

        while((( howmany =  Fread(in,(long) BUF_LEN, buffer)) > 0 ) &
               (( howmany2 = Fread(in2,(long) BUF_LEN, buffer2)) >0)) {
               for(i=0; i <= howmany; i++)
                    buffer[i] |= buffer2[i];

               if(howmany != Fwrite(out, howmany, buffer)) {
                        erreur("Error in writing","");
                        break; 
                }
        }
}

FDESC
openwr(file)
char    *file;
{
        FDESC   handle;

        if( (handle = (FDESC)Fcreate(file, 0)) < 0)
                return( (FDESC)Fopen(file, F_WRITE) );
        else
                return( handle );
}
