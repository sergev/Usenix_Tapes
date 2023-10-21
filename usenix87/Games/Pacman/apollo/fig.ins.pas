{ ******************************************************** }
{ ******************************************************** }
{ *********                                      ********* }
{ *********    FIG.INS.PAS                       ********* }
{ *********                                      ********* }
{ *********    Insert file for MOBILE_FIGURE     ********* }
{ *********    Module.                           ********* }
{ *********                                      ********* }
{ *********    Written 12/24/84 by Geof Cooper   ********* }
{ *********                                      ********* }
{ ******************************************************** }
{ ******************************************************** }
{ Copyright (C) 1984, 1985, IMAGEN Corporation                              }
{ This software may be duplicated in part of in whole so long as [1] this   }
{ notice is preserved in the copy, and [2] no financial gain is derived     }
{ from the copy.  Copies of this software other than as restricted above    }
{ may be made only with the consent of the author.                          }
                                                  
CONST
    num_orientations = 4;   { number of orientations of figure }
    guage = 16;

    { orientations: set up so orientation*360/num_or. = angle }
    or$right         = 0;
    or$up            = 1;
    or$left          = 2;
    or$down          = 3;

TYPE
    fig_$orientations = array[0..num_orientations-1] of gpr_$bitmap_desc_t;
    fig_$rep = RECORD
                { bitmaps describing figure in all orientations }
                figures     : fig_$orientations;

                { position on screen }
                position    : gpr_$position_t;

                { orientation selects one of above }
                orientation : 0..num_orientations-1;
    
                { velocity in direction of orientation, in pixels/unit time }
                velocity    : PInteger;
              END;
    fig_$t = ^fig_$rep;

PROCEDURE fig_$alloc_fig_bitmaps( OUT f: fig_$orientations ); EXTERN;

PROCEDURE fig_$create( IN figures: fig_$orientations;
                       IN pos_x, pos_y: Integer;
                       OUT r: fig_$t ); EXTERN;

PROCEDURE fig_$refresh( IN r: fig_$t ); EXTERN;

PROCEDURE fig_$move( IN r: fig_$t;
                     IN pos: gpr_$position_t ); EXTERN;
{ ASSUMES that raster op is XOR }


PROCEDURE fig_$elapse_time( IN r: fig_$t;
                            IN t: PInteger;
                            OUT newpos: gpr_$position_t ); EXTERN;

PROCEDURE fig_$turn( IN r: fig_$t; IN orient: PInteger ); EXTERN;

PROCEDURE fig_$set_velocity( IN r: fig_$t;
                             IN velocity: PInteger ); EXTERN;

FUNCTION fig_$coincident( IN r1, r2: fig_$t ): BOOLEAN;
    EXTERN;
