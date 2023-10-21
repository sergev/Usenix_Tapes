/*  Archetype driver for applying a local operation to a byte per pixel     */
/*  picture file, with header in Russ Smith's style.  The user must:-       */
/*                                                                          */
/*      (o)    #include  "/mnt/class/les/defns.t"  (important definitions). */
/*                                                                          */
/*      (i)    #define the following constants:                             */
/*             O_HEIGHT  the vertical height of the local operator          */
/*                       in pixels;                                         */
/*             O_WIDTH   the horizontal width of the local operator         */
/*                       in pixels;                                         */
/*             O_BAKGND  the background filler value for border points      */
/*                       where the operator can't fit (typically zero);     */
/*             O_X_CNTR & O_Y_CNTR  the x & y co-ordinates of the "centre"  */
/*                       of the operator (the output of the operator is     */
/*                       stored at the point in the output picture which    */
/*                       corresponds to the centre).                        */
/*                                                                          */
/* N.B. All co-ordinates are strictly Cartesian: The x co-ordinate          */
/*      increases from zero, left to right across the picture; and the y    */
/*      co-ordinate from zero, bottom to top.  The picture is stored on     */
/*      file by rows, with the first row (y=0) at the bottom of the picture.*/
/*                                                                          */
/*      (ii)   Declare global variables for storing operator parameters.    */
/*                                                                          */
/*      (iii)  Include a procedure:                                         */
/*                      getpars( argc, argv ) int argc; char *argv[];       */
/*             for setting of the above global variables from the command   */
/*             line (using standard conventions for argc & argv).           */
/*                                                                          */
/*      (iv)   Include a function:                                          */
/*                      char localop( nbd )  char *nbd[ O_HEIGHT ];         */
/*             which returns the result of the local operator.  Using       */
/*             zero-origin Cartesian co-ordinates, relative to the          */
/*             neighbourhood, the point (x,y) can be accessed as            */
/*             nbd[y][x] inside localop.                                    */
/*                                                                          */
/*      (v)    #include this file.                                          */
/*                                                                          */
/*                  Les Kitchen, 1st August 1979.                           */
/*                                                                          */

main( argc, argv )
int  argc;  char  *argv[];
{
  char  *in[O_HEIGHT], *out, *p;  /* I/O buffer pointers */
  char  *nbd[O_HEIGHT];           /* Neighbourhood to passed to localop */
  struct header  head;            /* Header for picture files */
  int r, c, k;                    /* Assorted indices */

  /* Process program parameters from call line */
  getpars( argc, argv );

  /* Transfer header */
  if( readp( STDIN, &head, sizeof head ) < sizeof head
  ||  head.nbyte != 1 ) {
    crash( argv[0], "Input not a byte per pixel picture file!" );
  }
  if( head.ncol < O_WIDTH  ||  head.nrow < O_HEIGHT ) {
    crash( argv[0], "Input picture too small for operator!" );
  }
  write( STDOUT, &head, sizeof head );


  /* Get buffer space */
  out = sbrk( head.ncol );
  for( k = 0;  k < O_HEIGHT;  ++k )  in[k] = sbrk( head.ncol );

  /* Write filler rows at bottom of picture */
  for( c = 0;  c < head.ncol;  ++c )  out[c] = O_BAKGND;
  for( k = 0;  k < O_Y_CNTR;  ++k )  write( STDOUT, out, head.ncol );
  /* Note that extreme elements of out buffer will keep background value. */

  /* Prime input buffer queue */
  for( k = 0;  k < O_HEIGHT-1;  ++k ) {
    if( readp( STDIN, in[k], head.ncol ) < head.ncol )  goto runout;
  }


  /* Process row by row */
  for( r = 0;  r <= head.nrow - O_HEIGHT;  ++r ) {
    /* Get next row into top of picture buffer */
    if( readp( STDIN, in[ O_HEIGHT-1 ], head.ncol ) < head.ncol )  goto runout;

    /* Run across columns */
    for( c = 0;  c <= head.ncol - O_WIDTH;  ++c )  {

      /* Fabricate neighbourhood */
      for( k = 0;  k < O_HEIGHT;  ++k )  nbd[k] = &( in[k][c] );
      /* Perform local operation */
      out[ c + O_X_CNTR ] = localop( nbd );
    }
    write( STDOUT, out, head.ncol );  /* Output a picture row */

    /* Slide up buffer by cycling pointers */
    p = in[0];  /* Now free */
    for( k = 1;  k < O_HEIGHT;  ++k )  in[k-1] = in[k];  /* Shift */
    in[ O_HEIGHT-1 ] = p;  /* Available buffer */

  }/* for rows */

  /* Now put out filler rows at top */
  for( c = 0;  c < head.ncol;  ++c )  out[c] = O_BAKGND;
  for( k = 0;  k < O_HEIGHT - (O_Y_CNTR + 1);  ++k ) {
    write( STDOUT, out, head.ncol );
  }

  /* Check that all the picture has been used up */
  if( readp( STDIN, in[ O_HEIGHT-1 ], head.ncol ) > 0 ) {
    crash( argv[0], "Excess input data remaining!" );
  }

  /* ...And we're done */
  cexit();


  /* In case picture runs out prematurely */
  runout:
  crash( argv[0], "Premature end of input!" );

}/*main*/
