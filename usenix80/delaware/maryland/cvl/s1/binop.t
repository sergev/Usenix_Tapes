/* Archetypal driver for applying a binary operator between two picture  */
/* files.  To construct a program:                                       */
/*          #include  "/mnt/class/les/defns.t"                           */
/*          #include  "/mnt/class/les/binop.t"  (this file)              */
/*          char  binop( p, q )  char  p, q;                             */
/*          { ...function for computing pointwise binary operation...}   */
/*                                                                       */
/* The first file is read from standard input, the second from the file  */
/* argument given on the call, EXCEPT that if the file argument is       */
/* prefixed immediately with a hyphen ("-"), the order of the files is   */
/* reversed when applying the binary operator.  The result picture is    */
/* written to standard output.                                           */
/*                                                                       */
/*                      Les Kitchen, 2nd August 1979.                    */

main( argc, argv )
int  argc;  char  *argv[];
{
  int  file1, file2;    /* File descriptor numbers for the two pictures */
  char  *in1, *in2, *out;       /* I/O buffer pointers */
  struct header  head1, head2;  /* Headers for picture files */
  int  r, c;                    /* Row and column indices */
  char  binop();                /* The binary operator function */


  /* Check and process arguments */
  if( argc < 2 )  crash( argv[0], "File argument needed!");

  /* Look for hyphen prefix */
  if( argv[1][0] != '-' ) {
    /* Input-operation-file */
    file1 = STDIN;
    file2 = open( argv[1], READ );
    if( file2 < 0 )  goto cantopen;
  } else {
    /* File-operation-input */
    file1 = open( &argv[1][1], READ );  /* Skip leading hyphen */
    if( file1 < 0 )  goto cantopen;
    file2 = STDIN;
  }


  /* Read, check, and transfer headers */
  if( readp( file1, &head1, sizeof head1 ) < sizeof head1
  ||  head1.nbyte != 1 ) {
    crash( argv[0], "First file not a byte per pixel file!" );
  }

  if( readp( file2, &head2, sizeof head2 ) < sizeof head2
  ||  head2.nbyte != 1 ) {
    crash( argv[0], "Second file not a byte per pixel file!" );
  }

  if( head1.ncol != head2.ncol  ||  head1.nrow != head2.nrow ) {
    crash( argv[0], "Picture files don't match!" );
  }

  write( STDOUT, &head1, sizeof head1 );


  /* Get buffer space */
  in1 = sbrk(head1.ncol);  in2 = sbrk(head2.ncol);  out = sbrk(head1.ncol);


  /* Process row by row */
  for( r = 1;  r <= head1.nrow;  ++r ) {

    if( readp( file1, in1, head1.ncol ) < head1.ncol ) {
      crash( argv[0], "Premature end on first file!" );
    }
    if( readp( file2, in2, head2.ncol ) < head2.ncol ) {
      crash( argv[0], "Premature end on second file!" );
    }

    /* Run across and apply binary operator */
    for( c = 0;  c < head1.ncol;  ++c )  out[c] = binop( in1[c], in2[c] );

    write( STDOUT, out, head1.ncol );

  }/*for r*/

  /* Check for excess data */
  if( readp( file1, in1, head1.ncol ) > 0 ) {
    crash( argv[0], "Excess data remaining on first file!" );
  }
  if( readp( file2, in2, head2.ncol ) > 0 ) {
    crash( argv[0], "Excess data remaining on second file!" );
  }

  cexit();  /* That's all! */


  /* In case file can't be opened ... */
  cantopen:
  crash( argv[0], "Can't open file argument!" );

}/*main*/




