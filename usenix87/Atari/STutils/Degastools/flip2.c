/*
 *
 * Flip2.c    flips the DEGAS file into  a destination file.
 *             flip without mirror image.
 *      usage : flip infile outfile
 * Modified by K.L. Martin from Jerome M. Lang Copy.c program
 * February 23, 1986
 */

#include <osbind.h>

#define F_READ  0
#define F_WRITE 1
#define ONE     1

#define BUF_LEN 32034
#define HEAD    34
#define MASK    0177

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
                Cconws(" flip2 ");

                if( 0 > (outfile = openwr(*++argv)) ) {
                        erreur(" Can't create outfile ", *argv);
                        fclose(infile);
                        break;
                }


                Cconws(*argv);
                Cconws("\r\n");

                flip2_file(infile, outfile);

                Fclose(infile);
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
                erreur("usage: flip2 infile outfile ","");
}

flip2_file(in, out)
FDESC   in, out;
{
        char    xx, yy;
        int     i, j, k, row, col;
        long    howmany, howmany2;

        while(( howmany =  Fread(in,(long) BUF_LEN, buffer)) > 0 ) {
               for(i=0; i <= howmany; i++)
                    buffer2[i] = 0;
               for(i=0; i < HEAD; i++)
                    buffer2[i] = buffer[i];

               for(row = 0; row < 400; row++) {
                    for(col = 0; col < 80; col++) {
                         xx = buffer[HEAD + (row * 80) + col];
                         for(i = 0, yy = 0; i < 8; i++, xx >>= 1) {
                              yy <<= 1;
                              if(( xx & (char) ONE ) == (char) ONE)
                                   yy |= 01;
                         }
                         buffer2[HEAD + (400 - row - 1) * 80 + 
                              (80 - col - 1)] = yy;
                    }
               }
               if(howmany != Fwrite(out, howmany, buffer2)) {
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
