MODULE pacman_board;

{ Written January, 1985 by Geoffrey Cooper                                  }
{ program for creating a pacman board
  needed improvements:
    - ability to save a board for later re-editing
    - ability to better define size of board - middle button?
    - ability to set SDOTS as well as dots.}
{ Copyright (C) 1985, IMAGEN Corporation                                    }
{ This software may be duplicated in part of in whole so long as [1] this   }
{ notice is preserved in the copy, and [2] no financial gain is derived     }
{ from the copy.  Copies of this software other than as restricted above    }
{ may be made only with the consent of the author.                          }

%include '/sys/ins/base.ins.pas';
%include '/sys/ins/error.ins.pas';
%include '/sys/ins/kbd.ins.pas';
%include '/sys/ins/gpr.ins.pas';
%include '/sys/ins/vfmt.ins.pas';
%include '/sys/ins/pgm.ins.pas';
%include '/sys/ins/pad.ins.pas';

DEFINE 
    board_$init, board_$reinit,
    board_$get_num_dots, board_$draw_board, board_$try_pac_position,
    board_$can_turn, board_$clear_dot, board_$show_score;

%include 'fig.ins.pas';
%include 'board.ins.pas';

CONST
    board_width_x =    31;
    board_width_y =    34;

    halfguage = guage div 2;

    score_x = guage;
    score_y = (board_width_y+1) * guage;

    pac_x = guage;
    pac_y = score_y + 16;

TYPE
    board_$config = array [0..board_width_x-1, 0..board_width_y-1]
                          of board_$elt;

VAR
    wall_bm     : gpr_$bitmap_desc_t;
    dot_bm      : gpr_$bitmap_desc_t;
    sdot_bm     : gpr_$bitmap_desc_t;

    board_numdcor: integer;
    board_numscor: integer;

    score       : integer;
    numpacs     : integer;
    w           : gpr_$window_t;
    status      : status_$t;

    board       : board_$config;
    board_init  : board_$config := [
    [ wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall],
    [ wall, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, wall],
    [ wall, dcor, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, dcor, wall, wall, wall, dcor, wall],
    [ wall, dcor, dcor, dcor, wall, dcor, dcor, dcor, wall, dcor, dcor, dcor, wall, dcor, dcor, dcor, wall, dcor, dcor, dcor, wall, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, wall, wall, dcor, wall],
    [ wall, dcor, wall, dcor, wall, dcor, wall, dcor, wall, dcor, wall, dcor, wall, dcor, wall, dcor, wall, dcor, wall, dcor, wall, dcor, wall, wall, wall, wall, wall, wall, dcor, dcor, wall, wall, dcor, wall],
    [ wall, dcor, wall, dcor, dcor, dcor, wall, dcor, dcor, dcor, wall, dcor, dcor, dcor, wall, dcor, dcor, dcor, wall, dcor, dcor, dcor, wall, dcor, dcor, dcor, dcor, dcor, dcor, dcor, wall, wall, dcor, wall],
    [ wall, dcor, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, dcor, wall, wall, wall, wall, wall, wall, wall, wall, dcor, wall],
    [ wall, dcor, wall, dcor, scor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, wall, wall, dcor, wall],
    [ wall, dcor, wall, dcor, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, dcor, wall, wall, wall, dcor, wall, wall, wall, dcor, wall, wall, wall, dcor, wall, wall, dcor, wall],
    [ wall, dcor, wall, dcor, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, dcor, wall, wall, wall, dcor, wall, wall, wall, wall, wall, wall, wall, dcor, wall, wall, dcor, wall],
    [ wall, dcor, wall, dcor, scor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, wall, wall, dcor, wall],
    [ wall, dcor, wall, wall, wall, wall, wall, wall, wall, wall, dcor, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, dcor, wall, wall, wall, wall, wall, wall, wall, dcor, wall],
    [ wall, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, wall],
    [ wall, dcor, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, dcor, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, dcor, wall],
    [ wall, dcor, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, dcor, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, dcor, wall],
    [ wall, dcor, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, dcor, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, dcor, wall, dcor, wall],
    [ wall, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, scor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, wall],
    [ wall, dcor, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, dcor, wall, wall, wall, dcor, wall, wall, wall, dcor, wall, wall, wall, wall, dcor, wall],
    [ wall, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, wall, dcor, wall, wall, wall, dcor, wall, wall, wall, dcor, wall, wall, wall, wall, dcor, wall],
    [ wall, dcor, wall, wall, wall, wall, wall, wall, wall, wall, wall, dcor, wall, dcor, wall, wall, wall, dcor, wall, dcor, wall, wall, wall, dcor, wall, wall, wall, dcor, wall, wall, wall, wall, dcor, wall],
    [ wall, dcor, wall, wall, wall, wall, wall, wall, wall, wall, wall, dcor, wall, dcor, dcor, wall, wall, dcor, wall, dcor, wall, dcor, dcor, dcor, wall, wall, wall, dcor, wall, wall, wall, wall, dcor, wall],
    [ wall, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, wall, wall, wall, wall, wall, dcor, wall, dcor, wall, dcor, wall, dcor, dcor, dcor, dcor, dcor, wall, wall, wall, wall, dcor, wall],
    [ wall, dcor, wall, wall, wall, wall, wall, wall, dcor, wall, wall, dcor, wall, wall, wall, wall, wall, dcor, wall, dcor, wall, dcor, wall, dcor, wall, wall, wall, dcor, wall, wall, wall, wall, dcor, wall],
    [ wall, dcor, wall, wall, wall, wall, wall, wall, dcor, wall, wall, dcor, dcor, dcor, dcor, dcor, dcor, dcor, wall, dcor, wall, dcor, wall, dcor, wall, wall, wall, dcor, wall, wall, wall, wall, dcor, wall],
    [ wall, dcor, wall, wall, wall, wall, wall, wall, dcor, wall, wall, wall, wall, wall, wall, wall, wall, dcor, wall, dcor, wall, dcor, wall, dcor, wall, wall, wall, dcor, wall, wall, wall, wall, dcor, wall],
    [ wall, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, wall, dcor, wall, dcor, wall, dcor, wall, dcor, dcor, dcor, dcor, dcor, wall, wall, wall, wall, dcor, wall],
    [ wall, dcor, wall, wall, wall, dcor, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, dcor, wall, dcor, wall, dcor, wall, dcor, wall, wall, wall, dcor, wall, wall, wall, wall, dcor, wall],
    [ wall, dcor, wall, wall, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, wall, dcor, wall, dcor, dcor, dcor, wall, wall, wall, dcor, wall, wall, wall, wall, dcor, wall],
    [ wall, dcor, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, dcor, wall, dcor, wall, wall, wall, wall, wall, dcor, wall, wall, wall, wall, dcor, wall],
    [ wall, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, dcor, wall],
    [ wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall]
    ];


PROCEDURE board_$fail(r: integer);
begin
    gpr_$terminate(false, status);
    writeln('board_$fail(', r:0, ')');
    pgm_$exit;
end;

PROCEDURE board_$print_integer(n: integer; IN ctl: string; x, y: integer);
VAR
    text: string;
    nlong: integer32;
    textlen: integer;
    status: status_$t;
    dummy: integer32;
BEGIN
    nlong := n;
    vfmt_$encode2(ctl, text, 80, textlen, nlong, dummy);
    gpr_$move(x, y, status);
    gpr_$text(text, textlen, status);
END;

PROCEDURE board_$draw_board;
VAR
    x, y: INTEGER;
    bm: gpr_$bitmap_desc_t;
    pos: gpr_$position_t;
BEGIN
    gpr_$set_raster_op(0, 3, status);
    gpr_$clear(0, status);
    for x := 0 to board_width_x - 1 do
      begin
        pos.x_coord := x * guage;
        for y := 0 to board_width_y -1 do
          begin
            if board[x, y] <> ecor then
              begin
                case board[x, y] of
                    wall: bm := wall_bm;
                    dcor: bm := dot_bm;
                    scor: bm := sdot_bm
                end;
                pos.y_coord := y * guage;
                gpr_$bit_blt(bm, w, 0, pos, 0, status);
              end
          end
      end;

    board_$print_integer(score, 'score: %5sd%$', score_x, score_y);
    board_$print_integer(numpacs, 'pacs left: %5sd%$', pac_x, pac_y);

    gpr_$set_raster_op(0, 6, status);
END;

PROCEDURE board_$reinit;
BEGIN
    score := 0;
    board := board_init;
    board_$draw_board;
END;

PROCEDURE board_$get_num_dots(* OUT dots, sdots: Integer *);
BEGIN
    dots := board_numdcor;
    sdots := board_numscor;
END;

PROCEDURE board_$init(* screen: gpr_$bitmap_desc_t;
                        screen_size: gpr_$offset_t,
                        pacs: integer *);
VAR
    attr: gpr_$attribute_desc_t;
    size: gpr_$offset_t;
    point: gpr_$position_t;
    x, y: integer;
BEGIN
    board_numscor := 0;
    board_numdcor := 0;
    for x := 0 to board_width_x-1 do
        for y := 0 to board_width_y-1 do
            case board_init[x, y] of
              scor: board_numscor := board_numscor + 1;
              dcor: board_numdcor := board_numdcor + 1;
              wall:;
              ecor:
            end;
    screen_size.x_size := screen_size.x_size div guage;
    screen_size.y_size := screen_size.y_size div guage;
    if screen_size.x_size < board_width_x then
        {board_$fail(1)};
    if screen_size.y_size < board_width_y then
        {board_$fail(2)};
    gpr_$allocate_attribute_block(attr, status);
    size.x_size := guage;
    size.y_size := guage;
    w.window_base.x_coord := 0;
    w.window_base.y_coord := 0;
    w.window_size := size;
    gpr_$allocate_bitmap(size, 0, attr, wall_bm, status);
    gpr_$set_bitmap(wall_bm, status);
    gpr_$clear(1, status);

    gpr_$allocate_bitmap(size, 0, attr, dot_bm, status);
    gpr_$set_bitmap(dot_bm, status);
    point.x_coord := halfguage;
    point.y_coord := halfguage;
    x := guage div 8;
    if x = 0 then x := 1;
    gpr_$circle_filled(point, x, status);

    gpr_$allocate_bitmap(size, 0, attr, sdot_bm, status);
    gpr_$set_bitmap(sdot_bm, status);
    gpr_$set_raster_op(0, 6, status);
    x := guage div 3;
    if x = 0 then x := 1;
    gpr_$circle_filled(point, x, status);
    gpr_$set_fill_value(0, status);
    x := guage div 5;
    if x = 0 then x := 1;
    gpr_$circle_filled(point, x, status);
    gpr_$set_fill_value(1, status);
    
    gpr_$set_bitmap(screen, status);

    score := 0;
    numpacs := pacs;
    board := board_init
END;

FUNCTION board_$entierGuage(i: integer): integer;
BEGIN
    board_$entierGuage := i - (i mod guage);
END;

{ modifies a position to avoid hitting a wall }
PROCEDURE board_$try_pac_position(* IN OUT pos: gpr_$position_t *);
VAR
    x, y: integer;
    x0, y0: integer;
    test: integer;
    extrem: integer;
BEGIN
    if pos.x_coord < 0 then pos.x_coord := 0;
    if pos.y_coord < 0 then pos.y_coord := 0;
    if pos.x_coord > ((board_width_x-1)*guage) then
        pos.x_coord := ((board_width_x-1)*guage);
    if pos.y_coord > ((board_width_y-1)*guage) then
        pos.y_coord := ((board_width_y-1)*guage);

    x := pos.x_coord div guage;
    y := pos.y_coord div guage;

    { find constraints in each direction }
    if board[x, y] = wall then
      begin
        if x < (board_width_x-1) and then board[x+1, y] <> wall then
            begin
                x := x + 1;
                pos.x_coord := x*guage;
            end
        else if y < (board_width_y-1) and then board[x, y+1] <> wall then
            begin
                y := y + 1;
                pos.y_coord := y*guage;
            end
      end;

    extrem := (pos.x_coord + (guage-1)) div guage;
    if extrem >= board_width_x then extrem := board_width_x-1;
    if extrem > x AND THEN board[extrem, y] = wall then
            pos.x_coord := x*guage;

    extrem := (pos.y_coord + (guage-1)) div guage;
    if extrem >= board_width_y then extrem := board_width_y-1;
    if extrem > y AND THEN board[x, extrem] = wall then
            pos.y_coord := y*guage;
END;

{ called to make a figure execute a saved turn }
PROCEDURE board_$can_turn(* IN OUT pos: gpr_$position_t;
                            IN     new_dir: board_$direction;
                            OUT    turn: boolean *);
VAR
    elt: board_$elt;
    x_inc, y_inc: integer;
    x, y: integer;
    spos: gpr_$position_t;
BEGIN
    spos.x_coord := (pos.x_coord+halfguage) div guage;
    spos.y_coord := (pos.y_coord+halfguage) div guage;
    x_inc := 0;
    y_inc := 0;
    case new_dir of
      or$up     : y_inc := -1;
      or$down   : y_inc :=  1;
      or$right  : x_inc :=  1;
      or$left   : x_inc := -1;
    end;
    x := spos.x_coord + x_inc;
    y := spos.y_coord + y_inc;
    if x < 0 OR ELSE x > board_width_x-1 or else
       y < 0 OR ELSE y > board_width_y-1 then turn := false
    else begin
        elt := board[x, y];
        if elt = wall
          then turn := false
          else begin
            pos.x_coord := spos.x_coord * guage;
            pos.y_coord := spos.y_coord * guage;
            turn := true
          end
    end
END;

{ called after above to clear a dot in a square of the board }
PROCEDURE board_$clear_dot(* IN pos: gpr_$position_t; 
                             OUT wasdot, special: boolean *);
VAR
    x, y: integer;
    bm: gpr_$bitmap_desc_t;
    draw: boolean;
    drawpos: gpr_$position_t;
BEGIN
    x := (pos.x_coord+(halfguage)) div guage;
    y := (pos.y_coord+(halfguage)) div guage;
    draw := true;
    case board[x, y] of
        wall: {board_$fail(100);} draw := false;
        ecor: begin
                draw := false;
                wasdot := false;
                special := false;
              end;
        dcor: begin
                bm := dot_bm;
                wasdot := true;
                special := false;
              end;
        scor: begin
                bm := sdot_bm;
                wasdot := true;
                special := true;
              end
    end;
    board[x, y] := ecor; { erase the dot if there was one}
    if draw then 
      begin
        drawpos.x_coord := x * guage;
        drawpos.y_coord := y * guage;
        gpr_$bit_blt(bm, w, 0, drawpos, 0, status);
      end
END;

PROCEDURE board_$show_score(* newscore: integer, newnumpacs: integer *);
BEGIN
    if score <> newscore then begin
        board_$print_integer(score, 'score: %5sd%$', score_x, score_y);
        board_$print_integer(newscore, 'score: %5sd%$', score_x, score_y);
        score := newscore
        end;
    if numpacs <> newnumpacs then begin
        board_$print_integer(numpacs, 'pacs left: %5sd%$', pac_x, pac_y);
        board_$print_integer(newnumpacs, 'pacs left: %5sd%$', pac_x, pac_y);
        numpacs := newnumpacs
        end
END;

