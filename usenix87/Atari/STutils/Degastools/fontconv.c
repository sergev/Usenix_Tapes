/*
 *
 * FontConv.c    And's two files and places the result in the destination
 *      usage : fontcon infile outfile
 * Modified by K.L. Martin from Jerome M. Lang Copy.c program
 * March 10, 1986
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

        if( argc < 1)
                usage();

        while( --argc > 0 )
        {
                if( 0 > (infile = (FDESC)Fopen(*++argv,F_READ)) ) {
                        erreur(" Can't open infile ",*argv);
                        break;
                }

                Cconws(*argv);
                Cconws(" fontcon ");


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

                fontconv(infile, outfile);

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
                erreur("usage: fontconv infile outfile ","");
}

fontconv(in, out)
FDESC   in, out;
{
        int     i, num;
        long    howmany, howmany2;

        while(( howmany =  Fread(in,(long) BUF_LEN, buffer)) > 0 ) {
               if(howmany > 1024)
                    howmany = 1024;
               num = howmany / 4;

               call_conv( 0, num * 2, num);
               call_conv( num, num * 4, num);
               call_conv( num * 2, 0, num);
               call_conv( num * 3, num * 6, num);


               if((howmany * 2) != Fwrite(out, howmany * 2, buffer2)) {
                        erreur("Error in writing","");
                        break; 
                }
        }
}

call_conv(b1, b2, count)
int  b1, b2, count;
{

     int  i, j;
     for(i = b1; i < b1 + count;) {
          for( j = 0; j < 4; j++)
               buffer2[b2++] = 0;
          for( j = 0; j < 8; j++)
               buffer2[b2++] = buffer[i++];
          for( j = 0; j < 4; j++)
               buffer2[b2++] = 0;
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
