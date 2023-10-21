c Demo code segment to illustrate some PREP facilities.  This is
c just a preprocessor demo and will not compile without adding
c a lot of variable declarations.


#include "premac.h"

c flag to call alternate window filler if window size = array size
: PIXIE_FLAG	(((xpix1-xpix0+1) == nrows) & ((ypix1-ypix0+1) == ncols))) ;

      include 'tencomn'

c open the input data file and initialize the device
      call init

c skip over skip0 data sets
      call skipdat( skip0 )
      if (eoflag) call exodus

c enter the menu
      call menu

c read data tables from the input file and plot until empty
      begin
         
c clear the record numbers
         do j = 1, 10
            record( j ) = 0
         end_do

         do j = 1, 10

            icount = j
            call getdat
            record( icount ) = first_record
            leave_do (eoflag)

c on first dataset of a group reset background
            if ( icount .eq. 1 ) then
               call vsbcol(dev, backcol)
               call vclrwk(dev)
            end if

c weed the data to make it fit in the window
            call compact

c clear a window and label it
            call windower

c Plot the data table , 1st arg is absolute first dim of buffer
            if ( PIXIE_FLAG ) then
               call pixie( HARD_X_DIM, nrows, ncols,
     *                     xpix0, PHYS_HEIGHT - 1 - ypix1,
     *                     buffer )
            else
               call winfill( HARD_X_DIM, nrows, ncols,
     *                       xpix0, xpix1,
     *                       PHYS_HEIGHT - 1 - ypix1,
     *                       PHYS_HEIGHT - 1 - ypix0,
     *                       buffer )
            end if

c see if the user is tired and wants to quit
            status = vsmstr( dev, ten, zero, echoxy, dummy)
            if ( status .gt. 0 ) then
               case [ upper( dummy(1:1) ) ]
                  of ( 'Q' )   call exodus
                  of ( 'R' )   leave_do
                  of ( 'B' )   leave_do
               end_case
            end if

         end_do

c skip over skip data sets
         call skipdat( skip )

c Delay and wait for keystroke.  Quit on Q,q; continue on cr; enlarge
c on keys 1,2,3,...9,0 (0 --> 10); make a dump file on D, d.
c If in movie mode, skip this input section, make a dump, and continue
         if ( movie_mode ) then
            if (eoflag) call exodus
            call dump

         else
c stay in this loop if end of file has been reached.
            begin

               case ( last_key )
               last_key = key(dev)

                  of ( 'D' )   call dump
                               continue_case
                  of ( 'Q' )   call exodus
                  of ( 'R' )   call restart
                  of ( 'B' )   call pop( recn )
                               recn = max0( recn, 1 )
                               eoflag = .false.
                  default      call push( max0( record(1), 1 ) )

                               call enlarger
               end_case

            while ( eoflag )
            again

         end if

      again

c Restore the video mode and turn off the device
      call exodus
      end
