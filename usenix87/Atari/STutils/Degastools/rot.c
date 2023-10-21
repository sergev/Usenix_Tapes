/*
 *
 * Rot.c    rotate the DEGAS file into  a destination file.
 *             only operates on the leftmost 400 X 400 pixels.
 *      usage : rot infile outfile
 * Modified by K.L. Martin from Jerome M. Lang Copy.c program
 * February 23, 1986
 */

#include <osbind.h>

#define F_READ  0
#define F_WRITE 1

#define BUF_LEN 32034
#define HEAD    34

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
                Cconws(" rot ");

                if( 0 > (outfile = openwr(*++argv)) ) {
                        erreur(" Can't create outfile ", *argv);
                        fclose(infile);
                        break;
                }


                Cconws(*argv);
                Cconws("\r\n");

                rot_file(infile, outfile);

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
                erreur("usage: rot infile outfile ","");
}

rot_file(in, out)
FDESC   in, out;
{
        char    xx;
        int     i, j, k, l;
        long    howmany, howmany2;

        while(( howmany =  Fread(in,(long) BUF_LEN, buffer)) > 0 ) {
               for(i=0; i <= howmany; i++)
                    buffer2[i] = 0;
               for(i=0; i < HEAD; i++)
                    buffer2[i] = buffer[i];

               for(l = 0; l < 400; l++) {
                    for(j = 0; j < 50; j++) {
                         for(i = 0; i < 8; i++) {
                              xx = buffer[HEAD + j + (l * 80)] & (1<<(8-i-1));
                              if( xx != 0 )
                                   xx = 1 << (8-(l%8)-1) ;
                              /*buffer2[HEAD + l/8 + (j * 8 +i) * 80] |= xx;*/
                              buffer2[HEAD + l/8 + ((50-j-1)*8 + (8-i-1))*80] |= xx;
                         }
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
