/*  Driver  for applying a unary operation to a picture file.  */
/*                 Les Kitchen, 25th July 1979.                */


main( argc, argv )
int  argc;  char  *argv[];
{
  int  c, r;            /* Assorted indices */
  char  *in, *out;      /* I/O buffers */
  struct header  head;  /* For picture-file header */
  char  unop();         /* The unary operation */


  /* Transfer header */
  if( readp( STDIN, &head, sizeof head ) < sizeof head
  ||  head.nbyte != 1 ){
    crash( argv[0], "Input not a byte per pixel file!");
  }
  write( STDOUT, &head, sizeof head );


  /* Get buffer space */
  in = sbrk( head.ncol );  out = sbrk( head.ncol );


  /* Process row by row */
  for( r = 1;  r <= head.nrow;  ++r ){
    if( readp( STDIN, in, head.ncol ) < head.ncol ){
      crash( argv[0], "Premature end of input!");
    }
    for( c = 0;  c < head.ncol;  ++c )  out[c] = unop( in[c] );
    write( STDOUT, out, head.ncol );
  }/*for rows*/


  if( readp( STDIN, in, head.ncol ) > 0 ) {
    crash( argv[0], "Excess input data remaining!" );
  }

  cexit();
}/*main*/
