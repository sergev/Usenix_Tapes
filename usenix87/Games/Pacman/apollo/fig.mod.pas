MODULE fig;
                                                  
{ ******************************************************** }
{ ******************************************************** }
{ *********                                      ********* }
{ *********    FIG.MOD.PAS                       ********* }
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
                                                  
%include '/sys/ins/base.ins.pas';
%include '/sys/ins/error.ins.pas';
%include '/sys/ins/kbd.ins.pas';
%include '/sys/ins/gpr.ins.pas';
%include '/sys/ins/pgm.ins.pas';
%include '/sys/ins/pad.ins.pas';

DEFINE fig_$create, fig_$refresh, fig_$move, fig_$elapse_time,
       fig_$turn, fig_$set_velocity, fig_$alloc_fig_bitmaps,
       fig_$coincident;

%include 'fig.ins.pas';

VAR
    { pre-stored for fast bit_blt's }
    fig_$wind: gpr_$window_t := [ [ 0, 0 ], [ guage, guage ] ];


PROCEDURE fig_$cs(status: status_$t; position: integer);
BEGIN
    if status.all <> status_$ok then
    begin
        gpr_$terminate(false, status);
        writeln('fig_$error(', position:0, ')');
        error_$print(status);
        pgm_$exit
    end
END;


{ Error 10 }
PROCEDURE fig_$alloc_fig_bitmaps(* OUT f: fig_$orientations *);
VAR
    i           : integer;
    attr        : gpr_$attribute_desc_t;
    status      : status_$t;
    size        : gpr_$offset_t;
BEGIN
    gpr_$allocate_attribute_block(attr, status);
    fig_$cs(status, 11);
    size.x_size := guage;
    size.y_size := guage;
    for i := 0 to num_orientations do
        begin
            gpr_$allocate_bitmap(size, 0, attr, f[i], status);
            fig_$cs(status, 12);
        end
END;

PROCEDURE fig_$create(* IN figures: fig_$orientations;
                        IN pos_x, pos_y: Integer;
                        OUT r: fig_$t *);
VAR           
    status  : status_$t;

BEGIN
    new(r);

    r^.figures     := figures;
    r^.orientation := or$right;
    r^.velocity    := 0;
    r^.position.x_coord := pos_x;
    r^.position.y_coord := pos_y;
END {fig_$create};

{ Error 30 }
PROCEDURE fig_$refresh(* IN r: fig_$t *);
VAR
    status: status_$t;
BEGIN
    gpr_$bit_blt(r^.figures[r^.orientation], fig_$wind, 0,
                 r^.position, 0, status);
    fig_$cs(status, 21);
END;

{ Error 40 }
PROCEDURE fig_$move(* IN r: fig_$t; IN pos: gpr_$position_t *);
{ ASSUMES that raster op is XOR }
VAR
    status: status_$t;
BEGIN
    { write it once in its old place, to erase it }
    fig_$refresh(r);
    { and write it into its new place, to redraw it }
    r^.position := pos;
    fig_$refresh(r);
END;

{ Error 50 }
{ find new position based on current velocity }
PROCEDURE fig_$elapse_time(* IN r: fig_$t; IN t: PInteger;
                             OUT newpos: gpr_$position_t *);
VAR
    incr: INTEGER;
BEGIN
    incr := t * r^.velocity;
    newpos := r^.position;
    CASE r^.orientation OF
      or$up:   newpos.y_coord := newpos.y_coord - incr;
      or$down: newpos.y_coord := newpos.y_coord + incr;
      or$right:newpos.x_coord := newpos.x_coord + incr;
      or$left: newpos.x_coord := newpos.x_coord - incr
    END
END;

{ Error 60 }
PROCEDURE fig_$turn(* IN r: fig_$t; IN orient: Integer *);
VAR
    status: status_$t;
BEGIN
    if r^.orientation <> orient then begin
        { write it once in its old place, to erase it }
        fig_$refresh(r);

        { change orientation }
        r^.orientation := orient;

        { write it again to show it }
        fig_$refresh(r)
        end
END;

{ Error 70 }
PROCEDURE fig_$set_velocity(* IN r: fig_$t;
                             IN velocity: PInteger *);
BEGIN
    r^.velocity := velocity;
END;


FUNCTION fig_$coincident(* IN r1, r2: fig_$t *){: BOOLEAN};
CONST
    halfguage = guage div 2;
VAR
    pos1_x, pos1_y, pos2_x, pos2_y: integer;
BEGIN
    pos1_x := r1^.position.x_coord + halfguage;
    pos1_y := r1^.position.y_coord + halfguage;
    pos2_x := r2^.position.x_coord + halfguage;
    pos2_y := r2^.position.y_coord + halfguage;
    fig_$coincident := (abs(pos1_x - pos2_x) < guage)
                                  AND
                       (abs(pos1_y - pos2_y) < guage)
END;

