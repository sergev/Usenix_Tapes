
program convert_vfont;
{

   cvtfont.pas - converts fonts in Berkeley vfont format to apollo font files.

    John W. Peterson
    Computer Science Dept.
    University of Utah
   (c) 1986 John W. Peterson

}
{
  This program converts files in BSD Unix VFONT format (see VFONT(5)) into
  apollo format font files.  The usage is:

    cvtfont vfontfile apollofontfile

  The program assumes that the VFONT file is in Vax format, this may cause
  problems if the vfonts were generated on other machines (e.g, 68K boxes)
  due to byte swapping.

  If byte swapping problems do occur, the structure definition of
  vf_dispatch_t will need to be changed (see comment) and the byte swapping
  code in procedure read_data will need to be removed.
}

%include '/sys/ins/base.ins.pas';
%include '/sys/ins/pgm.ins.pas';
%include '/sys/ins/error.ins.pas';
%include '/sys/ins/ios.ins.pas';
%include '/sys/ins/vfmt.ins.pas';
%include '/sys/ins/ms.ins.pas';
%include '/sys/ins/smdu.ins.pas';

{ include '/us/ins/cl.ins.pas'; One of these days apollo should release this }
{----------------------------------------------------------------------------}
TYPE    cl_$opt_t = (cl_$wildcards, cl_$no_wildcards);
        cl_$opt_set_t  = SET OF cl_$opt_t ;
        cl_$arg_select_t = (cl_$first, cl_$next) ;
PROCEDURE   cl_$init (
                IN  opt:  cl_$opt_set_t ;       { option-set }
                IN  my_name:  UNIV string ;     { caller's name }
                IN  my_name_len:  integer       { length of caller's name }
            ) ;  EXTERN ;
PROCEDURE   cl_$set_verb (
                IN  verb:  UNIV string ;
                IN  len:  integer
            ) ;  EXTERN ;
FUNCTION    cl_$get_arg (
                IN  select:  cl_$arg_select_t ; { first or next }
                OUT value:  UNIV string ;       { returned value }
                OUT len:  integer ;             { length, in characters }
                IN  maxlen:  integer            { maximum length }
            ): boolean ;  EXTERN ;
{----------------------------------------------------------------------------}

type

  { Vfont file header }
  vf_head_t = record
    magic: integer16;
    size: pinteger;
    maxx, maxy: pinteger;	    { Man page says these aren't implemented }
    xtnd: integer16;
  end;

  { Vfont dispatch vectors }
  vf_dispatch_t = record
    addr: pinteger;			{ Offset to bitmap }
    nbytes: integer16;			{ # bytes in char (0 if not in font) }
    down, up, right, left: char;	{ --Note these are byte swapped.--- }
    width: integer16;
  end;

  { Apollo character image storage }
  ap_font_image_t = packed array [0..1023, 0..27] of char;

var
  vf_strm : ios_$id_t;
  i: integer;
  status: status_$t;
  vf_header: vf_head_t;
  vf_dispatches: array[0..255] of vf_dispatch_t;
  vfont_image_data: array[0..65535] of char;	{ oink }
  ap_font_info : ^smd_$font_table_t;  

procedure CHECK ( in  Proc_name: univ char );
begin
  if status.ALL <> status_$ok then
  begin
    error_$std_format(status, proc_name, 0);
    pgm_$exit;  
  end;
end;

{ Convert char to sign-extended int }
function iord(ch: char): integer;
begin
  { total sleaze }
  if ord(ch) & 16#80 <> 0 then iord := 16#ff00 ! ord(ch)
    else iord := ord(ch);
end;

function max(a,b: integer): integer;
begin if a > b then max := a else max := b end;

function min(a,b: integer): integer;
begin if a < b then min := a else min := b end;

{ Initialize command line processing and open up the font file }

procedure open_vf_file;
var name: name_$pname_t;
    namelen: integer;
begin
  { Init / get argument }
  cl_$init( [cl_$no_wildcards], 'font', 4) ;
  cl_$set_verb('cvtfont', 7);
  error_$init_std_format( stream_$errout, '?', 'cvtfont', 7 );

  if not cl_$get_arg( cl_$first, name, namelen, name_$pnamlen_max ) then
  begin
    writeln('Must specify vfont name');
    pgm_$exit;
  end;

  vf_strm := ios_$open( name, namelen, [], status );
  check('Opening font file %$');
end;

{ Create a mapped object for the apollo font file }

procedure make_ap_file;
var name: name_$pname_t;
    namelen: integer;
    file_length: integer32;
begin
  if not cl_$get_arg( cl_$next, name, namelen, name_$pnamlen_max ) then
  begin
    writeln('Must specify output font name');
    pgm_$exit;
  end;
  { assume worst case }
  file_length := sizeof( ap_font_image_t )
                 + sizeof( smd_$font_table_t ) + 65535;
  UNIV_PTR(ap_font_info) :=
    ms_$crmapl( name, namelen, 0, file_length, ms_$nr_xor_1w, status );
  check('Creating apollo font file %$');
end;

{ Read the vfont file.  Note: this file assumes vax-format data.  If it's
  from another source (e.g., 68K), you may need to take out the byte
  swapping code. }

procedure read_data( var ptr: univ string; amount: integer32; swap: boolean );
var actually_read, i: integer32;
  tmp: char;
begin
  actually_read := ios_$get( vf_strm, [ios_$no_rec_bndry_opt], ptr,
                             amount, status );
  if actually_read <> amount then writeln('misread?');
  check('Reading data %$');

  { Must byte swap data 'cuz of the vax. }
  if swap then 
    for i := 1 to actually_read div 2 do begin
      tmp := ptr[i*2-1];
      ptr[i*2-1] := ptr[i*2];
      ptr[i*2] := tmp;
    end;
end;

{ Create the apollo font header }

procedure convert_header;
var
  i: integer;
  char_count, image_bytes: integer;
begin
  { Note - vfont's may have 255 characters vs. 127 in apollo fonts.  So
    we may lose a few.  And so it goes... }

  with ap_font_info^ do begin

    v_spacing := 0;		{ Vfont's already have spacing per char }
    h_spacing := 0;		

    version := 1;
    char_count := 0;
    max_up := 0;
    max_width := 0;
    max_right := 0;
    max_down := 0;
    max_height := 0;

    { Scan all of the (ASCII) characters in the vfont to find the minimum
      and maximum values. }

    for i:=0 to 127 do
      if vf_dispatches[i].nbytes <> 0 then begin

	char_count := char_count + 1;
	index_table[i] := chr(char_count);

	max_up := max( max_up, iord(vf_dispatches[i].up) );
	max_down := max( max_down, iord(vf_dispatches[i].down) );
	max_width := max( max_width, vf_dispatches[i].width );
	max_right := max( max_right, iord(vf_dispatches[i].right) );

	{ Note we add one so the characters in the apollo bitmap
	  don't bump into each other. }

	max_height := max( max_height, (max_up + max_down)+1 );

	if i = iord('n') then		{ Approximate space size }
	  space_size := vf_dispatches[i].width;

      end;

    chars_in_font := char_count;
    image_offset := smd_$font_header_size + smd_$font_index_table_size +
                          (smd_$v1_fdte_size * char_count) ;
  end; {with}
end;

{ Convert the individual characters }

procedure convert_characters;
const
  max_line_width = 224;			{ Max width of font image (1024-800) }
var
  cur_xpos, cur_ypos: integer;
  ind, i, j, rowbits, rowbytes, abit, bit, byte, rows: integer;
  ap_font_img : ^ap_font_image_t;
begin
  cur_xpos := 0;
  cur_ypos := 0;

  with ap_font_info^ do begin
    UNIV_PTR(ap_font_img) := addr(desc_table[chars_in_font + 1]);

    for ind := 0 to 127 do
      if vf_dispatches[ind].nbytes <> 0 then begin

        { Fill in per-char description info }

        with desc_table[ord(index_table[ind])] do begin
          left := vf_dispatches[ind].left;
          right := vf_dispatches[ind].right;
          up := vf_dispatches[ind].up;
          down := vf_dispatches[ind].down;
	  width := chr(vf_dispatches[ind].width);

          x_pos := chr(cur_xpos);
	  y_pos := cur_ypos;
	end;

        { Convert the character image, bit at a time.  It would be a little
	  more efficient to do it byte at a time, but I didn't want to do
	  that much bit-twiddling. }

	with vf_dispatches[ind] do begin
	  rowbits := iord(left)+iord(right);
	  rowbytes := rowbits div 8;
	  if rowbits mod 8 <> 0 then
	    rowbytes := rowbytes + 1;		{ round up }
	  rows := iord(up)+iord(down);

	  for i := 0 to rows-1 do
	    for j := 0 to rowbits - 1 do begin
	      byte := i * rowbytes + j div 8;
	      bit := j mod 8;
              abit := (cur_xpos + j) mod 8;

   	      if (lshft( iord( vfont_image_data[addr + byte] ), bit )
	         & 16#80 ) <> 0
              then
	        ap_font_img^[cur_ypos+i, (cur_xpos + j) div 8] :=
  	          chr(ord(ap_font_img^[cur_ypos+i, (cur_xpos + j) div 8])
		      ! lshft(1,7-abit))
              else
	        ap_font_img^[cur_ypos+i, (cur_xpos + j) div 8] :=
  	          chr(ord(ap_font_img^[cur_ypos+i, (cur_xpos + j) div 8])
		      & (~ lshft(1,7-abit)));
          end; {fors}
        end; {with vf_dispatches}
	
        cur_xpos := cur_xpos + vf_dispatches[ind].width;
	if cur_xpos + vf_dispatches[ind+1].width > max_line_width then begin
	  cur_xpos := 0;
  	  cur_ypos := cur_ypos + max_height;
        end;
      end; { if vf... }

      { Take care of some final details in the font header }

      raster_lines := cur_ypos + max_height;
      if raster_lines <= (65535 div 28) then
        image_size := raster_lines * 28
      else image_size := 65535;

      ms_$unmap( ap_font_info, image_offset + image_size, status );
      check('Unmapping font file %$');
   end; {with}
end;

{main}
begin
  open_vf_file;
  make_ap_file;

  read_data( vf_header, sizeof(vf_header), true );
  for i := 0 to 255 do
    read_data( vf_dispatches[i], sizeof(vf_dispatch_t), true );
  read_data( vfont_image_data, vf_header.size, false );

  convert_header;
  convert_characters;
end.
