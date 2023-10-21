PROGRAM pacm;

{ APOLLO PAC - a pacman like game                                           }
{                                                                           }
{ Written January, 1985 by Geoffrey Cooper                                  }
{                                                                           }
{ Copyright (C) 1985, IMAGEN Corporation                                    }
{ This software may be duplicated in part of in whole so long as [1] this   }
{ notice is preserved in the copy, and [2] no financial gain is derived     }
{ from the copy.  Copies of this software other than as restricted above    }
{ may be made only with the consent of the author.                          }

%include '/sys/ins/base.ins.pas';
%include '/sys/ins/error.ins.pas';
%include '/sys/ins/kbd.ins.pas';
%include '/sys/ins/gpr.ins.pas';
%include '/sys/ins/pgm.ins.pas';
%include '/sys/ins/pad.ins.pas';
%include '/sys/ins/time.ins.pas';
%include '/sys/ins/tone.ins.pas';

%include 'fig.ins.pas';   {mobile_figure module}
%include 'board.ins.pas'; {pacman_board module}

PROCEDURE pacm_$refresh_all; EXTERN;
PROCEDURE pacm_$noop; EXTERN;
PROCEDURE pacm_$refresh_part(IN unobscured, pos_change: boolean); EXTERN;

TYPE
    ndesc = record
                tip_x, tip_y: integer;
                base_x, base_y: integer;
                inc_x, inc_y: integer
            end;

CONST
    pac_init_x = guage;
    pac_init_y = guage;

    nasty_init_x = guage*29;
    nasty_init_y = guage*32;

    max_nasties = 15;

VAR
    play_forever: boolean;

    ndh: array[0..3] of ndesc := [
        [  (guage div 2),  0, -(guage div 2), -(guage div 2),  0,  1 ],
        [  0, -(guage div 2), -(guage div 2),  (guage div 2),  1,  0 ],
        [ -(guage div 2),  0,  (guage div 2), -(guage div 2),  0,  1 ],
        [  0,  (guage div 2), -(guage div 2), -(guage div 2),  1,  0 ]
    ];

    screen      : gpr_$bitmap_desc_t;
    screen_size : gpr_$offset_t;

    pac         : DEFINE fig_$t;
    nasty       : fig_$t;
    nasties     : DEFINE array [1..max_nasties] of fig_$t;
    num_nasties : DEFINE integer;
    pac_time    : integer32;
    num_pacs    : integer;
    screen_rfs  : integer;

    score_dots  : integer;
    score_bigdots: integer;
    score        : integer;

    clock_tick  : integer32 := 20000; { 12.5 ticks per second }

    incs: array [1..3] of integer := [ 1, 3, 2 ];
    nasty_rand   : integer;
    last_tick   : DEFINE time_$clock_t;

{ Initialize the display, using GPR routines }
PROCEDURE pacm_$init_gpr;
CONST
    bitmap_max_size = 1024;
    black_and_white = 0;
    keyset = [ kbd_$up_arrow, kbd_$down_arrow, 
               kbd_$left_arrow, kbd_$right_arrow,
               kbd_$hold2,
               kbd_$l_box_arrow, kbd_$r_box_arrow, 
               kbd_$down_box_arrow2, kbd_$up_box_arrow2,
               kbd_$next_win,
               'f', 's',
               'l',
               'q' ];
    buttonset = ['a', 'A', 'b', 'c']; 
    locatorset  = [];
    raster_op_XOR = 6;
VAR
    attr:           gpr_$attribute_desc_t;
    status:         status_$t;
    unobscured:     BOOLEAN;
    font_width, 
    font_height,
    font_length,
    font_id:        INTEGER;
    font_name:      STRING;
    plane:          integer;

BEGIN
    { Initialize the a displayed bitmap filling the frame }
    screen_size.x_size := bitmap_max_size;
    screen_size.y_size := bitmap_max_size;
    gpr_$init( gpr_$direct, stream_$stdout, screen_size, black_and_white,
                    screen, status );

    gpr_$inq_bitmap_dimensions(screen, screen_size, plane, status);

    gpr_$set_obscured_opt(gpr_$block_if_obs, status);

    { Set up bitmap to use the default font }
    pad_$inq_font( stream_$stdout, font_width, font_height, 
                      font_name, sizeof(String), font_length, status );
    gpr_$load_font_file( font_name, font_length, font_id, status );
    gpr_$set_text_font( font_id, status );

    { enable input from mouse }
    gpr_$enable_input( gpr_$keystroke, keyset, status);

    gpr_$set_raster_op(0, raster_op_XOR, status);

    gpr_$set_cursor_active( true, status );
END;

PROCEDURE add_time( IN OUT t: time_$clock_t; ticktime: linteger );
VAR
    i: linteger;
BEGIN
    i := t.low32 + ticktime;
    if i < t.low32 then t.high16 := t.high16 + 1;
    t.low32 := i
END;

PROCEDURE pregnant_pause;
CONST
    ticktime = 156250;
VAR
    t           :time_$clock_t;
    status      : status_$t;
BEGIN
    t.high16 := 0;
    t.low32  := ticktime;
    time_$wait( time_$relative, t, status );
    add_time(last_tick, ticktime)
END;

PROCEDURE pacm_$init_pac;
VAR
    pac_bitmaps : fig_$orientations;
    pac_size    : gpr_$offset_t;
    status      : status_$t;
    point       : gpr_$position_t;
    unobsc      : boolean;
    i,j         : integer;
    attr        : gpr_$attribute_desc_t;
CONST
    wedge_begin = (guage div 2) - (guage div 8) - 1;
    wedge_end   = (guage div 2) + (guage div 8) + 1;
BEGIN
    gpr_$allocate_attribute_block(attr, status);
    pac_size.x_size := guage;
    pac_size.y_size := guage;
    point.x_coord := guage div 2;
    point.y_coord := guage div 2;
    unobsc := gpr_$acquire_display(status);
    for i := 0 to 3 do
    begin
        gpr_$allocate_bitmap(pac_size, 0, attr,
                             pac_bitmaps[i], status);
        gpr_$set_bitmap(pac_bitmaps[i], status);
        gpr_$circle_filled(point, (guage div 2) - 1, status);
        gpr_$set_draw_value(0, status);

        for j := wedge_begin to wedge_end do
          begin
            gpr_$move((guage div 2), (guage div 2), status);
            CASE i OF
                0: gpr_$line( guage,  j    , status);
                1: gpr_$line( j    ,  0    , status);
                2: gpr_$line( 0    ,  j    , status);
                3: gpr_$line( j    ,  guage, status);
            END
          end;
        gpr_$set_draw_value(1, status)
    END;
    fig_$create(pac_bitmaps, pac_init_x, pac_init_y, pac);
    fig_$set_velocity(pac, (guage div 2) + (guage div 8));
    gpr_$release_display(status);
END;

PROCEDURE pacm_$init_nasty;
CONST
    pi = 3.14159;
    right_angle = pi/2;
    mag = guage div 2;
VAR
    nasty_bitmaps : fig_$orientations;
    size          : gpr_$offset_t;
    status        : status_$t;
    unobsc        : boolean;
    i,j           : integer;
    attr          : gpr_$attribute_desc_t;
    org           : gpr_$position_t;
    org0          : gpr_$position_t;
    angle         : real;
    x, y          : integer;
    x1, y1        : integer;
BEGIN
    org.x_coord := guage div 2;
    org.y_coord := guage div 2;
    org0.x_coord := 0;
    org0.y_coord := 0;
    gpr_$allocate_attribute_block(attr, status);
    size.x_size := guage;
    size.y_size := guage;
    unobsc := gpr_$acquire_display(status);
    for i := 0 to 3 do
        begin
            gpr_$allocate_bitmap(size, 0, attr, nasty_bitmaps[i], status);
            gpr_$set_bitmap(nasty_bitmaps[i], status);
            gpr_$set_coordinate_origin(org, status);

            gpr_$move(ndh[i].tip_x, ndh[i].tip_y, status);
            for j := 0 to guage-1 do begin
                gpr_$line(ndh[i].base_x, ndh[i].base_y, status);
                gpr_$move(ndh[i].tip_x, ndh[i].tip_y, status);
                ndh[i].base_x := ndh[i].base_x + ndh[i].inc_x;
                ndh[i].base_y := ndh[i].base_y + ndh[i].inc_y
                end;
            gpr_$set_coordinate_origin(org0, status);
        end;
    fig_$create(nasty_bitmaps, nasty_init_x, nasty_init_y, nasty);
    fig_$set_velocity(nasty, (guage div 2));
    gpr_$release_display(status);
    nasty_rand := 0;
    nasties[1] := nasty;
    num_nasties := 1
END;

PROCEDURE pacm_$add_nasty(OUT n: fig_$t);
BEGIN
    fig_$create(nasty^.figures, nasty_init_x, nasty_init_y, n);
    fig_$refresh(n);
    fig_$set_velocity(n, (guage div 2));
END;

PROCEDURE pacm_$tick_nasty(nasty: fig_$t);
{
    Algorithm for controlling the nasty:
        using absolute difference between nasty x and y
        positions and pac's, prefer the correct direction
        in each axis, with the axis with the largest distance
        having priority.

        The other two possible turns are random, except that
        the `about face' direction has low priority.
    Then:
        Only try all four possibilities when you have hit
        a wall.

        Only allow yourself to about face every
        ALLOW_REVERSE moves unless you have hit a wall.
}
CONST
    allow_reverse = 50;
VAR
    pos: gpr_$position_t;
    turnpos: gpr_$position_t;
    i: integer;
    orient: integer;
    can_turn: boolean;
    no_change: boolean;
    bound: integer;
    t           :time_$clock_t;
    turns: array[0..3] of integer;
    diff_x, diff_y: integer;
    about_face: integer;
BEGIN
    fig_$elapse_time(nasty, 1, pos);
    board_$try_pac_position(pos);

    nasty_rand := nasty_rand + 1;
    no_change := pos = nasty^.position;
    bound := 1;
    if no_change then bound := 3;

    { find priorities for directions }
    diff_x := pos.x_coord - pac^.position.x_coord;
    diff_y := pos.y_coord - pac^.position.y_coord;
    if abs(diff_x) > abs(diff_y) then begin
        if diff_x > 0 then begin
            turns[0] := or$left;
            turns[2] := or$right
            end
        else begin
            turns[0] := or$right;
            turns[2] := or$left
            end;
        if diff_y > 0 then begin
            turns[1]:= or$up;
            turns[3] := or$down
            end
        else begin
            turns[1] := or$down;
            turns[3] := or$up
            end
        end
    else begin
        if diff_x > 0 then begin
            turns[1] := or$left;
            turns[3] := or$right
            end
        else begin
            turns[1] := or$right;
            turns[3] := or$left
            end;
        if diff_y > 0 then begin
            turns[0]:= or$up;
            turns[2] := or$down
            end
        else begin
            turns[0] := or$down;
            turns[2] := or$up
            end
    end;
    about_face := ((nasty^.orientation+2) mod 4);
    if turns[2] = about_face then begin
        i := turns[3];
        turns[3] := turns[2];
        turns[2] := i
        end;
    can_turn := false;
    for i := 0 to bound do begin
        orient := turns[i];
        if no_change or else
           orient <> about_face or else
           (nasty_rand mod allow_reverse) = 0 then begin
            turnpos := pos;
            board_$can_turn(turnpos, orient, can_turn);
            end;
        if can_turn then exit
        end;
    if can_turn and then orient <> nasty^.orientation then begin
        fig_$turn(nasty, orient);
        pos := turnpos
        end;
    fig_$move(nasty, pos);
    { check scores }
    if fig_$coincident(nasty, pac) then begin
        num_pacs := num_pacs - 1;
        t.high16 := 0;
        t.low32  := 10000;
        tone_$time(t);
        add_time(last_tick, 10000);
        pos.x_coord := pac_init_x;
        pos.y_coord := pac_init_y;
        fig_$move(pac, pos);
        pos.x_coord := nasty_init_x;
        pos.y_coord := nasty_init_y;
        fig_$move(nasty, pos);
        pregnant_pause;
        end
END;

PROCEDURE pacm_$tick_all_nasties;
VAR
    i: integer;
BEGIN
    for i := 1 to num_nasties do
        pacm_$tick_nasty(nasties[i]);
END;

PROCEDURE pacm_$tick;
VAR
    i           : linteger;
    status      : status_$t;
    unobsc      : boolean;
    release     : boolean;
BEGIN
    add_time(last_tick, clock_tick);
    release := (pac_time mod 16) = 0;
    pac_time := pac_time + 1;
    if release then gpr_$release_display(status);
    time_$wait( time_$absolute, last_tick, status );
    time_$clock(last_tick);
    if release then unobsc := gpr_$acquire_display(status);
END;


PROCEDURE pacm_$play;
VAR
    c           : char; 
    pos         : gpr_$position_t;
    event       : gpr_$event_t;
    cp          : ^char;
    status      : status_$t;
    unobsc      : boolean;
    wasdot      : boolean;
    special     : boolean;
    is_q_orient : boolean;
    can_turn    : boolean;
    q_orient    : board_$direction;
    num_dots    : integer;
    num_bigdots : integer;
    total_dots  : integer;
    total_sdots : integer;
    i           : integer;
    num_events  : integer;
    num_passes  : integer32;

    u1, u2      : univ_ptr;
BEGIN
    num_dots := 0;
    num_bigdots := 0;
    score_dots := 0;
    score_bigdots := 0;
    score := 0;
    pac_time := 0;
    num_pacs := 5;
    screen_rfs := 0;
    pacm_$init_gpr;
    pacm_$init_pac;
    pacm_$init_nasty;
    unobsc := gpr_$acquire_display(status);
    gpr_$set_bitmap(screen, status);
    u1 := addr(pacm_$refresh_part);
    u2 := addr(pacm_$noop);
    gpr_$set_refresh_entry(addr(pacm_$refresh_part), addr(pacm_$noop), status);
    board_$init(screen, screen_size, num_pacs);
    board_$get_num_dots(total_dots, total_sdots);
    pacm_$refresh_all;
    is_q_orient := false;
    c := chr(0);
    cp := addr(c);
    num_events := 2;
    num_passes := 0;
    REPEAT
        repeat
            unobsc := gpr_$cond_event_wait(event, c, pos, status);
            IF status.all <> status_$OK THEN
                BEGIN
                    error_$print(status);
                    pgm_$exit;
                END;
            IF event = gpr_$keystroke THEN
                CASE c OF
                  kbd_$right_arrow:
                        begin
                            is_q_orient := true;
                            q_orient := or$right
                        end;
                  kbd_$up_arrow:
                        begin
                            is_q_orient := true;
                            q_orient := or$up
                        end;
                  kbd_$left_arrow:
                        begin
                            is_q_orient := true;
                            q_orient := or$left
                        end;
                  kbd_$down_arrow:
                        begin
                            is_q_orient := true;
                            q_orient := or$down
                        end;
                  'f':
                        begin
                            clock_tick := clock_tick - 1000;
                            if clock_tick < 0 then clock_tick := 0;
                        end;
                  's':
                        clock_tick := clock_tick + 1000;
                  'l':
                        pacm_$refresh_all;
                  kbd_$up_box_arrow2:
                        begin
                            if pac^.velocity < (guage-1) then
                                fig_$set_velocity(pac, pac^.velocity+1)
                        end;
                  kbd_$down_box_arrow2:
                        begin
                            if pac^.velocity <> 0 then
                                fig_$set_velocity(pac, pac^.velocity - 1)
                        end;
                  kbd_$hold2:
                        begin
                            repeat
                                unobsc := gpr_$event_wait(event, c, pos, status);
                            until (event = gpr_$keystroke) AND (c = kbd_$hold2);
                            num_events := 2
                        end;
                  'q':;
                  OTHERWISE
                        { ignore other characters -- they are defined so }
                        { that pressing them by accident doesn't spoil   }
                        { the game.                                      }
                END;
                num_events := num_events + 1;
        UNTIL event <> gpr_$keystroke;

        { num_passes is to prevent an initial "spurt" when libraries }
        { are loaded the first time a pac is run in a process }
        num_passes := num_passes + 1;
        if (num_events > 1) or (num_passes < 10) then
            time_$clock(last_tick);
        num_events := 0;

        pacm_$tick;
        fig_$elapse_time(pac, 1, pos);

        { stop pac man at boundary }
        board_$try_pac_position(pos);
        board_$clear_dot(pos, wasdot, special);
        if wasdot then
            if special then
              begin
                num_bigdots := num_bigdots + 1;
                score_bigdots := score_bigdots + 1;
                score := score + 5;
              end
            else
              begin
                num_dots := num_dots + 1;
                score_dots := score_dots + 1;
                score := score + 1;
              end;
        board_$show_score( score, num_pacs );
        if is_q_orient then
          begin
            if pac^.orientation = q_orient then
                is_q_orient := false
            else
              begin
                board_$can_turn(pos, q_orient, can_turn);
                if can_turn then
                  begin
                    fig_$turn(pac, q_orient);
                    is_q_orient := false
                  end
              end
          end;
        fig_$move(pac, pos);

        { move /nasty/ }
        pacm_$tick_all_nasties;

        if num_dots = total_dots  THEN
          begin
            screen_rfs := screen_rfs + 1;
            board_$reinit;
            fig_$refresh(pac);
            for i := 1 to num_nasties do
                fig_$refresh(nasties[i]);
            num_bigdots := 0;
            num_dots := 0;
            for i := 1 to num_nasties do
                if nasties[i]^.velocity < (guage-1) then
                    fig_$set_velocity(nasties[i], nasties[i]^.velocity+1);
            if num_nasties < max_nasties then begin
                num_nasties := num_nasties + 1;
                pacm_$add_nasty(nasties[num_nasties])
                end;
            num_pacs := num_pacs + 1;
            time_$clock(last_tick);
          end
    UNTIL (c = 'q') OR ((num_pacs <= 0) AND (NOT play_forever));
    { Read any extra characters that were typed but not read yet }
    { This is necessary since otherwise special characters can get }
    { left in the input stream and kill the csh }
    repeat
        unobsc := gpr_$cond_event_wait(event, c, pos, status);
    until (event <> gpr_$keystroke) OR (status.all <> status_$ok);

    gpr_$release_display(status);
    gpr_$terminate(false, status);
    i := 6 + screen_rfs - num_pacs;
    if num_pacs <> 0 then
        writeln('PAC score after ', i:0, ' pacs:')
    else
        writeln('Final PAC score (', i:0, ' pacs):');
    writeln('    ', score:0, ' points, ', screen_rfs:0, ' entire screens consumed.')
END;

PROCEDURE pacm_$help;
VAR
    status      : status_$t;
    argv_ptr    : pgm_$argv_ptr;
    argc        : integer;
BEGIN
    pgm_$get_args(argc, argv_ptr);
    play_forever := false;
    if argc = 7 then
      begin
        writeln('Pac - play forever mode.');
        play_forever := true;
        argc := 0;
      end;
    if argc > 1 then
      begin
        writeln('pac - play pac man.');
        writeln('usage: pac');
        writeln('[An argument gives this help, no argument plays the game]');
        writeln('PAC is an adaptation of the ever-popular ATARI game, PACMAN(C).');
        writeln('You control a round PAC, which runs from the scurrying');
        writeln('NASTIES.  The nasties seek the PAC like heat-seeking missiles.');
        writeln('When they catch it, it is destroyed, and both nasty and pac');
        writeln('go to neutral corners.  You start the game with five PACs.');
        writeln;
        writeln('The PAC accumulates points by eating solid dots [1 point] and');
        writeln('hollow dots [5 points].  When all the dots on the screen are');
        writeln('eaten, the screen is re-filled, and you are given one more ');
        writeln('PAC "life."  The game also gets tougher each time the screen');
        writeln('refreshes, since a new nasty appears, and all existing nasties');
        writeln('get a bit faster.');
        writeln;
        writeln('Control the PAC by using the arrow keys.  Pressing an');
        writeln('arrow key queues a request to turn in that direction.');
        writeln('The request is processed when the turn is first possible.');
        writeln('For best results, do not hold down the arrow keys.');
        writeln;
        writeln('The UP and DOWN block arrows make the PAC get slower and faster,');
        writeln('respectively.  A slow PAC is more maneuverable, but must');
        writeln('be more strategic to escape the nasties.');
        writeln;
        writeln('Additional commands:');
        writeln('    ''q''  - quits the game immediately');
        writeln('    ''l''  - manual refresh of screen');
        writeln('    HOLD - stops action until you press hold again');
        writeln('    POP  - the game stops if the window is obscured');
        writeln('    ''f''  - speed  up the game clock (decrease tick time)');
        writeln('    ''s''  - slow down the game clock (increase tick time)');
        writeln('If you start with a window that is too small, just enlarge it.');
        writeln;
        writeln('Run this program again without arguments to play.');
        pgm_$exit
      end
    else
      begin
        writeln('Pac:  type ''q'' to quit, then ''pac help'' to get instructions');
        pregnant_pause;
      end
END;

BEGIN
    pacm_$help;
    pacm_$play;
END.
