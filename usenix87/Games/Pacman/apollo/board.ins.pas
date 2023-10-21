
{ **** INSERT FILE FOR PACMAN_BOARD MODULE **** }
{ Written January, 1985 by Geoffrey Cooper                                  }
{ Copyright (C) 1985, IMAGEN Corporation                                    }
{ This software may be duplicated in part of in whole so long as [1] this   }
{ notice is preserved in the copy, and [2] no financial gain is derived     }
{ from the copy.  Copies of this software other than as restricted above    }
{ may be made only with the consent of the author.                          }

TYPE
    board_$direction = 0..num_orientations-1;
    board_$elt = (wall, ecor, dcor, scor);

PROCEDURE board_$init(screen: gpr_$bitmap_desc_t;
                      screen_size: gpr_$offset_t;
                      pacs: integer); EXTERN;

PROCEDURE board_$reinit; EXTERN;

PROCEDURE board_$get_num_dots(OUT dots, sdots: Integer); EXTERN;

PROCEDURE board_$draw_board; EXTERN;

PROCEDURE board_$try_pac_position(IN OUT pos: gpr_$position_t); EXTERN;

PROCEDURE board_$can_turn(IN OUT pos: gpr_$position_t;
                          IN     new_dir: board_$direction;
                          OUT    turn: boolean); EXTERN;

PROCEDURE board_$clear_dot(pos: gpr_$position_t; 
                           OUT wasdot, special: boolean); EXTERN;


PROCEDURE board_$show_score(newscore, newnumpacs: integer); extern;

