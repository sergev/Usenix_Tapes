
(*
 *  testpw_p.p
 *
 *  Pascal test program for print_window, the routine which will dump
 *  the current window in the correct format for use by dpr.
 *  Copyright (c) Rich Burridge, Sun Australia 1986.
 *
 *  Version 1.1.
 *
 *  No responsibility is taken for any errors inherent either in the comments
 *  or the code of this program, but if reported to me then an attempt will
 *  be made to fix them.
 *)

program test(input,output) ;

    procedure print_window(
       printer : packed array[lb..ub : integer] of char) ;
    external c ;

begin
  print_window('lp') ;
end .
