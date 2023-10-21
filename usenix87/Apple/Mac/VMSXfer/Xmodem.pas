$ PASCAL sys$input /OBJECT=xmac.obj
PROGRAM xmodem_for_MAC_VAX_VMS (INPUT, OUTPUT, data_file, text_file, log_file);
(*****************************************************************************)
(*                                                                           *)
(*  DESCRIPTION :                                                            *)
(*    Macintosh  XMODEM protocol file transfer program                       *)
(*                                                                           *)
(*    (C) Copyright 1985 Kris Kreutzman.  All rights reserved.  Permission   *)
(*    is granted for noncommercial use and distribution.                     *)
(*                                                                           *)
(*    Kris Kreutzman                                                         *)
(*    225 Union Avenue #220A                                                 *)
(*    Campbell, CA 95008                                                     *)
(*                                                                           *)
(* also                                                                      *)
(*                                                                           *)
(*    ARPANET:  c/o  peirce@lll-crg                                          *)
(*                                                                           *)
(*  HISTORY :                                                                *)
(*                                                                           *)
(*    Date          Programmer        Comments                               *)
(*    -----------   --------------    ------------------------------------   *)
(*    12-JAN-1985   Kris Kreutzman    V1.0  First release.                   *)
(*    17-APR-1985   Kris Kreutzman    V2.0  9600 baud, error recovery,       *)
(*                                    /DEVICE, /BELLS, QIO timeouts.         *)
(*                                                                           *)
(*****************************************************************************)

CONST
  ss$_normal      = 1;
  ss$_timeout     = 556;

  io$_setmode     = 35;
  io$_sensemode   = 39;
  io$_writevblk   = 48;
  io$_readvblk    = 49;
  io$_ttyreadall  = 58;

  io$m_noecho     = 64;
  io$m_timed      = 128;
  io$m_noformat   = 256;
  io$m_nofiltr    = 512;

  tt$m_passall    = 1;
  tt$m_noecho     = 2;
  tt$m_eightbit   = 32768;
  tt$m_nobrdcst   = 131072;
  tt2$m_altypeahd = 128;

  NUL = ''(%x00);  
  SOH = ''(%x01);
  EOT = ''(%x04);
  ACK = ''(%x06);
  BEL = ''(%x07); 
  TAB = ''(%x09);
  FF  = ''(%x0C);
  CR  = ''(%x0D);
  NAK = ''(%x15);
  CAN = ''(%x18);
  EM  = ''(%x19);
  ESC = ''(%x1B);

  (* header block offsets *)
  start_offset = 0;
  name_length  = 1;
  name_buffer  = 2;
  file_type    = 65;
  file_author  = 69;
  file_lock    = 81;
  data_length  = 83;
  rsrc_length  = 87;
  create_time  = 91;
  modify_time  = 95;
  the_rest     = 99;

  max_device_name_length = 60;
  max_file_name_length   = 255;

TYPE
  $byte  = [BYTE] -128..127;
  $ubyte = [BYTE] 0..255;
  $word  = [WORD] -32768..32767;
  $quad  = [UNSAFE] RECORD
      l0 : UNSIGNED;
      l1 : UNSIGNED;
      l2 : UNSIGNED;
      l3 : INTEGER;
    END;
  $quad_word = PACKED ARRAY [0..3] OF $word;

  two_characters = PACKED ARRAY [1..2] OF CHAR;
  block_type     = PACKED ARRAY [1..128] OF CHAR;
  block_file_type= FILE OF block_type;
  package_type   = (pack_head, pack_data, pack_rsrc, pack_eof);
  file_direction_type = (transmit, receive);
  qio_string     = CHAR;
  log_string     = VARYING [128] OF CHAR;
  v_string       = VARYING [128] OF CHAR;

  mode_block_type = PACKED RECORD
      mode_class      : [BYTE, POS(0)]  0..%xFF;
      mode_type       : [BYTE, POS(8)]  0..%xFF;
      mode_width      : [WORD, POS(16)] 0..%xFFFF;
      characteristics : [POS(32)]       0..%xFFFFFF;
      mode_length     : [BYTE, POS(56)] 0..%xFF;
      extended        : [POS(64)]       UNSIGNED;
    END;

  receive_type = (r_start, r_head, r_blkA, r_blkB, r_blkX, r_csum,
                  r_chra, r_grbg);

      
VAR
  receive_mode   : receive_type;
  data_file      : FILE OF block_type;
  internal_file  : FILE OF block_type;
  text_file      : TEXT;
  log_file       : TEXT;
  log_kept       : BOOLEAN;
  return_status  : INTEGER;
  file_direction : file_direction_type;
  termination    : BOOLEAN;
  file_name      : PACKED ARRAY [1..max_file_name_length] OF CHAR;
  device_name    : VARYING [max_device_name_length] OF CHAR;
  data_char      : CHAR;
  line_channel   : $word;
  text_operation : BOOLEAN;
  text_line_len  : INTEGER;
  package_kind   : package_type;
  mode_block     : mode_block_type;
  old_mode       : mode_block_type;
  retransmited   : INTEGER;
  packets        : INTEGER;
  abort_transfer : BOOLEAN;
  system_error   : BOOLEAN;
  dummy_listen   : BOOLEAN;
  r_data_present : BOOLEAN := FALSE;
  io_status      : $quad_word;
  bells          : INTEGER;
  i              : INTEGER;


[EXTERNAL] FUNCTION cli$present
  (%STDESCR qual_name : PACKED ARRAY [$l2..$U2:INTEGER] OF CHAR)
  : INTEGER;    EXTERN;

[EXTERNAL] FUNCTION cli$get_value
  (%STDESCR qual_name : PACKED ARRAY [$l2..$u2:INTEGER] OF CHAR;
   VAR ret_val        : PACKED ARRAY [$l3..$u3:INTEGER] OF CHAR)
  : INTEGER;    EXTERN;


[EXTERNAL] FUNCTION sys$assign
   (devnam         : [CLASS_S] PACKED ARRAY [$l1..$u1:INTEGER] OF CHAR;
    VAR    chan    : [VOLATILE] $word;
    %IMMED acmode  : INTEGER := %IMMED 0;
    mbxnam         : [CLASS_S] PACKED ARRAY [$l2..$u2:INTEGER] OF CHAR := %IMMED
0)
   : INTEGER;  EXTERN;


[EXTERNAL] FUNCTION sys$qiow
   (%IMMED efn    : INTEGER := %IMMED 0;
    %IMMED chan   : INTEGER;
    %IMMED func   : INTEGER;
    VAR    isob   : [VOLATILE] $quad_word := %IMMED 0;
    %IMMED [UNBOUND] PROCEDURE astadr := %IMMED 0;
    %IMMED astprm : INTEGER := %IMMED 0;
    %REF   p1     : [UNSAFE] ARRAY [$l1..$u1:INTEGER] OF $ubyte := %IMMED 0;
    %IMMED p2     : INTEGER := %IMMED 0;
    %IMMED p3     : INTEGER := %IMMED 0;
    %REF   p4     : [UNSAFE] $quad_word := %IMMED 0;
    %IMMED p5     : INTEGER := %IMMED 0;
    %IMMED p6     : INTEGER := %IMMED 0)
   : INTEGER;  EXTERN;

[EXTERNAL] FUNCTION mth$iior
   (%REF par1 : UNSIGNED;
    %REF par2 : UNSIGNED) :
   INTEGER;  EXTERN;

[EXTERNAL] PROCEDURE sys$exit
   (%IMMED ret_status : INTEGER);
  EXTERN;




PROCEDURE log_write (q : INTEGER; s : log_string); 
(****************************************************************)
(*    Spools text strings to the log file.  Forces a new line   *)
(*    when the string meets or exceeded 80 characters.          *)
(****************************************************************)

VAR t_len : [STATIC] INTEGER := 0;
    s_len : INTEGER;

BEGIN
  s_len := LENGTH (s);
  IF t_len + s_len >= 80 THEN q := 2;
  IF (q = 3) OR (q = 2) THEN BEGIN WRITELN (log_file); t_len := 0; END;
  WRITE (log_file, s);
  IF ODD (q) THEN BEGIN WRITELN (log_file); t_len := 0; END
  ELSE t_len := t_len + s_len;
END;


PROCEDURE get_options_and_filename;
(****************************************************************)
(*    Picks up the command line options an sets internal flags  *)
(*    and variables.                                            *)
(****************************************************************)

VAR dummy_string : PACKED ARRAY [1..15] OF CHAR;
    i            : INTEGER;

BEGIN
  return_status := cli$get_value ('FILE_NAME', file_name);

  IF ODD (cli$present ('RECEIVE_FILE')) THEN file_direction := receive;
  IF ODD (cli$present ('SEND_FILE'))    THEN file_direction := transmit;

  IF ODD (cli$present ('BELLS')) THEN
    BEGIN
      return_status := cli$get_value ('BELLS', dummy_string);
      READV (dummy_string, bells);
    END
  ELSE bells := 0;

  log_kept       := ODD (cli$present ('LOG_FILE'));
  text_operation := ODD (cli$present ('TEXT'));
  IF text_operation THEN
    BEGIN
      return_status := cli$get_value ('TEXT', dummy_string);
      READV (dummy_string, text_line_len);
    END;

  return_status := cli$get_value ('DEVICE', device_name.body);
  i := max_device_name_length;
  WHILE (device_name.body[i] = ' ') DO i := i - 1;
  device_name.length := i;

END;


PROCEDURE check_for_error (ret_val : INTEGER; title : v_string);
(****************************************************************)

BEGIN
  IF return_status <> ss$_normal THEN
    BEGIN
      log_write (3, title + ' ' + HEX (ret_val,8));
      system_error := TRUE;
    END;
END;


PROCEDURE initialize_line;
(****************************************************************)
(*    Sets up the terminal line to allow file transfer without  *)
(*    interuptions.                                             *)
(****************************************************************)

VAR name_output : PACKED ARRAY [1..13] OF CHAR;
    temp        : UNSIGNED;

BEGIN
  return_status := sys$assign (device_name, line_channel);
  check_for_error (return_status, '%SYS$ASSIGN - bad device name');

  return_status := sys$qiow (32, line_channel, io$_sensemode,,,, mode_block,
12);
  check_for_error (return_status, '%SYS$QIOW - bad device channel');

  old_mode := mode_block;
  temp     := mode_block.characteristics;
  mode_block.characteristics := mth$iior (temp,
                                          tt$m_passall +
                                          tt$m_noecho +
                                          tt$m_eightbit +
                                          tt$m_nobrdcst);
  temp     := mode_block.extended;
  mode_block.extended := mth$iior (temp, tt2$m_altypeahd);

  return_status := sys$qiow (32, line_channel, io$_setmode,,,, mode_block, 12);
  check_for_error (return_status, '%SYS$QIOW - bad device channel');
END;



PROCEDURE decommit_line;
(****************************************************************)
(*    Removes the terminal port characteristics which allow     *)
(*    file transfer.                                            *)
(****************************************************************)

BEGIN
  return_status := sys$qiow (32, line_channel, io$_setmode,,,, old_mode, 8);
  check_for_error (return_status, '%SYS$QIOW - bad device channel');
END;


PROCEDURE transmit_character (out_going : CHAR; buffer_len : INTEGER := 0);
(****************************************************************)
(*    Send a character to the terminal port.                    *)
(****************************************************************)

VAR  internal_count : [STATIC] INTEGER := 0;
     buffer_count   : [STATIC] INTEGER := 0;
     t_buff         : [STATIC] ARRAY [1..200] OF $ubyte;

BEGIN
  IF buffer_len > 0 THEN
    BEGIN
      internal_count := 0;
      buffer_count   := buffer_len;
    END;
  IF buffer_count > 0 THEN
    BEGIN
      internal_count := internal_count + 1;
      t_buff[internal_count] := ORD (out_going);
      IF internal_count = buffer_count THEN
        BEGIN
          return_status := sys$qiow (1,line_channel,
            io$_writevblk + io$m_noformat,,,,t_buff,buffer_count);
          check_for_error (return_status, '%SYS$QIOW - Transmit buffer');
          buffer_count := 0;
        END;
    END
  ELSE
    BEGIN
      return_status := sys$qiow (1,line_channel,
        io$_writevblk + io$m_noformat,,,,ORD (out_going),1);
          check_for_error (return_status, '%SYS$QIOW - Transmit character');
    END;
  log_write (0, ' T' + HEX (ORD (out_going), 2));
END;


FUNCTION receive_character (blk_len : INTEGER := 1) : CHAR;
(****************************************************************)
(*    Wait for a character from the terminal port.              *)
(****************************************************************)

VAR dirt          : $ubyte;
    r_buff        : [STATIC] ARRAY [1..200] OF $ubyte;
    term_mask     : [STATIC] $quad_word := (0,0,0,0);
    block_count   : [STATIC] INTEGER := 0;
    block_length  : [STATIC] INTEGER := 0;
    temp_str      : VARYING [80] OF CHAR;
    r_timeout     : INTEGER;
    good_data     : BOOLEAN;

BEGIN
  IF receive_mode = r_start THEN r_timeout := 60
                            ELSE r_timeout := 5;

  IF NOT r_data_present THEN
    REPEAT
      good_data := TRUE;
      return_status := sys$qiow (32, line_channel, 
                                 io$m_noecho + io$m_nofiltr + io$_ttyreadall +
io$m_timed,
                                 io_status,,, r_buff, blk_len,
                                 r_timeout, term_mask);
      check_for_error (return_status, '%SYS$QIOW - Receive_char');
      block_count := 0;
      block_length := io_status[1] + io_status[3];
      r_data_present := TRUE;
      WRITEV (temp_str, 'read_io_status = ', io_status[0]:4,io_status[1]:4,
                                             io_status[2]:4,io_status[3]:4);
      IF io_status[0] <> ss$_normal THEN log_write (3, temp_str);
      IF io_status[0] = ss$_timeout THEN
        BEGIN
          log_write (3, 'TIMEOUT');
          transmit_character (NAK);
          receive_mode := r_head;
          r_data_present := FALSE;
          retransmited := retransmited + 1;
          good_data := FALSE;
        END
      ELSE IF io_status[0] <> ss$_normal THEN
        BEGIN
          log_write (3, 'r_c_io' + HEX (io_status[0], 8));
          receive_mode := r_grbg;
          r_data_present := FALSE;
        END;
    UNTIL good_data;

  IF r_data_present THEN
    BEGIN
      block_count := block_count + 1;
      dirt := r_buff[block_count];
      IF block_count = block_length THEN r_data_present := FALSE;
      log_write (0, ' R' + HEX (dirt,2));
      receive_character := CHR (dirt);
    END;
END;


PROCEDURE receive_file;
(****************************************************************)
(*    Main loop to receive a file from the terminal port.       *)
(****************************************************************)

VAR
  package_mode : package_type;
  pack_num_t   : INTEGER;
  pack_num_c   : INTEGER;
  checksum_c   : INTEGER;
  checksum_t   : INTEGER;
  data_buffer  : block_type;
  data_pos     : INTEGER;
  line_buffer  : VARYING [255] OF CHAR;
  data_char    : CHAR;
  text_length  : INTEGER;


PROCEDURE text_package_head;
(****************************************************************)
(*    Tests the Macintosh header package for a TEXT file.       *)
(*    Gets the file length.                                     *)
(****************************************************************)

BEGIN
  IF (data_buffer[file_type+1] = 'T') AND
     (data_buffer[file_type+2] = 'E') AND
     (data_buffer[file_type+3] = 'X') AND
     (data_buffer[file_type+4] = 'T') THEN

   text_length :=
   (*  data_buffer[data_length+1] * 16777216 + *)
      ORD (data_buffer[data_length+2]) * 65536 +
      ORD (data_buffer[data_length+3]) * 256 +
      ORD (data_buffer[data_length+4])

  ELSE
    BEGIN
      termination := TRUE;
      transmit_character (CAN);
      transmit_character (CAN);
      abort_transfer := TRUE;
    END
END;


PROCEDURE output_data_as_text;
(****************************************************************)
(*    Translates the XMODEM packets into TEXT strings, writes   *)
(*    to a VAX/VMS text file.                                   *)
(*    Forces a new line when the input line exceeds the line    *)
(*    length maximum.                                           *)
(****************************************************************)

BEGIN
  data_pos := 0;
  WHILE (data_pos < 128) AND (text_length > 0) DO
    BEGIN
      data_pos := data_pos + 1;
      text_length := text_length - 1;
      IF LENGTH (line_buffer) = text_line_len THEN
        BEGIN
          WRITELN (text_file, line_buffer);
          line_buffer := '';
        END;
      IF data_buffer[data_pos] = CR THEN
        BEGIN
          WRITELN (text_file, line_buffer);
          line_buffer := '';
        END
      ELSE line_buffer := line_buffer + data_buffer[data_pos];
    END;
END;


(****************************************************************)

BEGIN (* receive_file *)
  line_buffer  := '';
  pack_num_c   := %x01;
  package_mode := pack_head;
  receive_mode := r_start;
  termination  := FALSE;

  REPEAT
    IF receive_mode = r_blkA THEN data_char := receive_character (131)
                             ELSE data_char := receive_character;
    CASE receive_mode OF
      r_start, r_head : BEGIN
                 log_write (1,'_head ' + HEX (pack_num_c,2));
                 CASE data_char OF
                   SOH : receive_mode := r_blkA;
                   ESC : receive_mode := r_chra;
                   CAN : BEGIN
                           termination  := TRUE;
                           abort_transfer := TRUE;
                         END;
                   EM  : termination  := TRUE;
                   EOT : BEGIN
                           package_mode := SUCC (package_mode);
                           pack_num_c := 1;
                           transmit_character (ACK);
                           IF package_mode = pack_eof THEN termination := TRUE
                           ELSE transmit_character (NAK);
                           log_write (3, '');
                         END;
                   OTHERWISE BEGIN
                               receive_mode := r_grbg;
                               log_write (3, 'GARGAGE');
                             END;
                 END;
               END;

      r_blkA : BEGIN
                 pack_num_t := ORD (data_char);
                 log_write (1, '_blk1 ' + HEX (pack_num_t,2));
                 receive_mode := r_blkB;
                 data_pos     := 0;
                 checksum_c   := 0;
                 packets := packets + 1;
               END;

      r_blkB : BEGIN
                 log_write (1, '_blkB');
                 IF (pack_num_t = (255 - ORD (data_char))) AND
                    (pack_num_t = pack_num_c)
                   THEN receive_mode := r_blkX
                   ELSE
                     BEGIN
                       receive_mode := r_grbg;
                       log_write (3, 'GARGAGE');
                     END;
               END;

      r_blkX : BEGIN
                 data_pos := data_pos + 1;
                 data_buffer[data_pos] := data_char;
                 checksum_c := (checksum_c + ORD (data_buffer[data_pos])) MOD
256;
                 IF data_pos = 128 THEN receive_mode := r_csum;
               END;

      r_csum : BEGIN
                 checksum_t := ORD (data_char);
                 log_write (1, '_csum ' + HEX (checksum_c,2));
                 IF checksum_t = checksum_c THEN
                   BEGIN
                     receive_mode := r_head;
                     IF text_operation THEN
                       BEGIN
                         IF      package_mode = pack_head THEN text_package_head
                         ELSE IF package_mode = pack_data THEN
output_data_as_text
                         ELSE IF (package_mode = pack_rsrc) AND (LENGTH
(line_buffer) > 0) THEN
                            BEGIN
                              WRITELN (text_file, line_buffer);
                              line_buffer := '';
                            END;
                       END
                     ELSE
                       BEGIN
                         data_file^ := data_buffer;
                         put (data_file);
                       END;
                     transmit_character (ACK);
                     log_write (3, '');
                     pack_num_c := (pack_num_c + 1) MOD 256;
                   END;
               END;

      r_chra : BEGIN
                 log_write (0,'_chra');
                 IF data_char = 'a' THEN
                   BEGIN
                     receive_mode := r_head;
                     transmit_character (ACK);
                     log_write (3,'');
                   END
                 ELSE
                   BEGIN
                     receive_mode := r_grbg;
                     log_write (3, 'GARGAGE');
                   END;
               END;

      r_grbg : BEGIN
               END;
      OTHERWISE ;
    END; (* CASE receive_mode *)


  UNTIL termination;
END;  (* receive_file *)


PROCEDURE convert_text_file;
(****************************************************************)
(*    Converts text strings into XMODEM package blocks.         *)
(*    Suppresses all control characters except CR, & TAB.       *)
(*    Creates the Macintosh header packet for a TEXT file.      *)
(****************************************************************)

VAR   data_pos    : INTEGER;
      data_buffer : block_type;
      text_length : INTEGER;
      text_char   : CHAR;
      i,j,k       : INTEGER;

BEGIN
  internal_file^ := data_buffer;
  PUT (internal_file);
  data_pos := 0;
  text_length := 0;
  WHILE NOT EOF (text_file) DO
    BEGIN
      IF EOLN (text_file) THEN
        BEGIN
          READ (text_file, text_char);
          text_char := CR;
        END
      ELSE READ (text_file, text_char);
      IF data_pos = 128 THEN
        BEGIN
          internal_file^ := data_buffer;
          PUT (internal_file);
          data_pos := 0;
        END;
      data_pos    := data_pos + 1;
      text_length := text_length + 1;
      IF NOT (text_char IN [CR,TAB,' '..'~']) THEN
        text_char := '~';
      data_buffer[data_pos] := text_char;
    END;
  IF data_pos <> 0 THEN
    BEGIN
      internal_file^ := data_buffer;
      PUT (internal_file);
      data_pos := 0;
    END;

  LOCATE (internal_file, 1);
  FOR i := 1 TO 128 DO data_buffer[i] := NUL;

  j := 0;
  REPEAT
    j := j + 1;
  UNTIL (file_name[j] = ' ') OR (j = 255);
  j := j - 1;
  k := j + 1;
  REPEAT
    k := k - 1;
  UNTIL (file_name[k] IN [']',':','>']) OR (k = 1);
  IF k = 1 THEN k := 0;

  data_buffer[name_length+1] := CHR (j-k);
  FOR i := 1 TO j-k DO data_buffer [name_buffer + i] := file_name[k+i];
  data_buffer[file_type+1]   := 'T';
  data_buffer[file_type+2]   := 'E';
  data_buffer[file_type+3]   := 'X';
  data_buffer[file_type+4]   := 'T';
  data_buffer[file_author+1] := 'V';
  data_buffer[file_author+2] := 'A';
  data_buffer[file_author+3] := 'X';
  data_buffer[file_author+4] := ' ';
  data_buffer[file_lock+1]   := CHR (%x12);
  data_buffer[file_lock+2]   := CHR (%x2C);
  data_buffer[data_length+2] := CHR ((text_length DIV 65536) MOD 256);
  data_buffer[data_length+3] := CHR ((text_length DIV 256) MOD 256);
  data_buffer[data_length+4] := CHR (text_length MOD 256);
  internal_file^ := data_buffer;
  PUT (internal_file);
END;


PROCEDURE transmit_file;
(****************************************************************)
(*    Main loop for sending a file.                             *)
(****************************************************************)

VAR  pack_length : ARRAY [pack_head..pack_eof] OF INTEGER;
     eot_sent : BOOLEAN;
     pack_num_c : INTEGER;
     



PROCEDURE transmit_block;
(****************************************************************)
(*    Sends one block of the formated file, one character at a  *)
(*    time with XMODEM bindings.                                *)
(****************************************************************)

VAR
  checksum_c  : INTEGER;
  data_buffer : block_type;
  i           : INTEGER;

BEGIN
  log_write (3, 'block # ' + HEX (pack_num_c,2));
  transmit_character (SOH,132);
  transmit_character (CHR (pack_num_c));
  transmit_character (CHR (255-pack_num_c));
  log_write (1, '');
  IF text_operation THEN data_buffer := internal_file^
                    ELSE data_buffer := data_file^;
  checksum_c := 0;
  FOR i := 1 TO 128 DO
    BEGIN
      transmit_character (data_buffer[i]);
      checksum_c := (checksum_c + ORD (data_buffer[i])) MOD 256;
    END;
  log_write (2, 'csum=');
  transmit_character (CHR (checksum_c));
  log_write (0, ' =>');
END;


FUNCTION find_length (place : INTEGER) : INTEGER;
(****************************************************************)
(*    Pulls out of the header packet a length number.           *)
(****************************************************************)

VAR i,j : INTEGER;

BEGIN
  j := 0;
  FOR i := 1 TO 4 DO
    IF text_operation THEN j := j * 256 + ORD (internal_file^[place+i])
                      ELSE j := j * 256 + ORD (data_file^[place+i]);
  log_write (3, 'Length =' + HEX (j,4));
  j := (j + 127) DIV 128;
  find_length := j;
END;


(****************************************************************)

BEGIN (* transmit_file *)
  pack_length [pack_head] := 1;
  pack_length [pack_data] := find_length (data_length);
  pack_length [pack_rsrc] := find_length (rsrc_length);
  pack_length [pack_eof ] := 0;
  pack_num_c := 0;
  package_kind  := pack_head;
  eot_sent      := FALSE;
  transmit_character (ESC);
  transmit_character ('a');

  REPEAT
    data_char := receive_character;
    IF (eot_sent) AND (data_char = NAK) THEN
      BEGIN
        eot_sent  := FALSE;
        data_char := ACK;
      END;
    log_write (1, '');
    CASE data_char OF
      EOT : termination := TRUE;
      CAN : BEGIN
              termination := TRUE;
              abort_transfer := TRUE;
            END;
      ACK : BEGIN
              IF pack_length[package_kind] = 0 THEN
                BEGIN
                  package_kind := SUCC (package_kind);
                  IF package_kind = pack_eof THEN termination := TRUE;
                  transmit_character (EOT);
                  data_char := receive_character;
                  eot_sent := TRUE;
                  pack_num_c := 0;
                END
              ELSE
                BEGIN
                  IF package_kind <> pack_head THEN
                    IF text_operation THEN GET (internal_file)
                                      ELSE GET (data_file);
                  pack_length[package_kind] := pack_length[package_kind] - 1;
                  pack_num_c := (pack_num_c + 1) MOD 256;
                  transmit_block;
                  packets := packets + 1;
                END;
            END;
      NAK : BEGIN
              transmit_block;
              packets      := packets + 1;
              retransmited := retransmited + 1;
            END;

      OTHERWISE
        BEGIN
          log_write (0, '-grbg');    
        END;
    END; (* CASE data_char *)

  UNTIL termination;
END;  (* transmit_file *)

PROCEDURE set_receive_files;
BEGIN
  IF text_operation THEN
    BEGIN
      OPEN (text_file, file_name, NEW);
      REWRITE (text_file);
      receive_file;
      CLOSE (text_file);
    END
  ELSE
    BEGIN
      OPEN (data_file, file_name, NEW);
      REWRITE (data_file);
      receive_file;
      CLOSE (data_file);
    END;
END;


PROCEDURE set_trans_files;
BEGIN
  IF text_operation THEN
    BEGIN
      OPEN    (text_file, file_name, OLD, ERROR := CONTINUE);
      IF STATUS (text_file) = 0 THEN
        BEGIN
          RESET   (text_file);
          OPEN    (internal_file, RECORD_LENGTH := 128, ACCESS_METHOD :=
DIRECT);
          REWRITE (internal_file);
          convert_text_file;
          CLOSE   (text_file);
          RESET   (internal_file);
          transmit_file;
          CLOSE   (internal_file);
        END
      ELSE
        BEGIN
          WRITELN ('%XMAC-E-FILERR, error in file name');
          abort_transfer := TRUE;
        END;
    END
  ELSE
    BEGIN
      OPEN (data_file, file_name, OLD, ERROR := CONTINUE);
      IF STATUS (data_file) = 0 THEN
        BEGIN
          RESET (data_file);
          transmit_file;
          CLOSE (data_file);
        END
      ELSE
        BEGIN
          WRITELN ('%XMAC-E-FILERR, error in file name');
          abort_transfer := TRUE;
        END;
    END;
END;

(****************************************************************)

BEGIN (* xmac *)
  retransmited   := 0;
  packets        := 0;
  abort_transfer := FALSE;
  termination    := FALSE;
  system_error   := FALSE;

  get_options_and_filename;
  WRITE ('  XMAC Version 2.0 ');
  IF file_direction = transmit THEN WRITE ('  VAX to Mac')
                               ELSE WRITE ('  Mac to VAX');
  IF text_operation THEN WRITE (' text')
                    ELSE WRITE (' binary');
  WRITELN (' file transfer');
  WRITELN ('  IF locked, wait 20 sec. and press control-X to exit');

  IF log_kept THEN OPEN (log_file, 'xmac.log', NEW)
              ELSE OPEN (log_file, '_nla0:', NEW);
  REWRITE (log_file);
  WRITELN (log_file, 'start');

  initialize_line;

  IF NOT system_error THEN
    IF file_direction = transmit THEN set_trans_files
                                 ELSE set_receive_files;

  decommit_line;

  CLOSE (log_file);
  IF system_error THEN
    WRITELN ('%XMAC-F-SYSROUERR, system routine error.  Use /LOG to catch
errors.');
  abort_transfer := abort_transfer OR system_error;
  WRITE ('  File transfer was ');
  IF abort_transfer THEN WRITELN ('ABORTED')
                    ELSE WRITELN ('successful');
  WRITELN ('  Packets : ', packets:0, '  Retries/Timeouts : ',retransmited:0);
  FOR i := 1 TO bells DO WRITE (bel);
  WRITELN;
END.  (* xmac *)
$ LINK xmac
$ CREATE xmac.cld
DEFINE VERB xmac
  IMAGE sys$disk:[]xmac.exe
  PARAMETER P1,              LABEL=file_name,
                             PROMPT="file_name",        VALUE(required)
  QUALIFIER SEND_FILE
  QUALIFIER RECEIVE_FILE
  QUALIFIER TEXT,            VALUE(DEFAULT=80)
  QUALIFIER LOG_FILE
  QUALIFIER BELLS,           VALUE(DEFAULT=4)
  QUALIFIER DEVICE,          DEFAULT,  VALUE(DEFAULT=SYS$COMMAND)
$ RUNOFF sys$input /OUTPUT=xmac.doc
.lm 10
.rm 75
.set para 0,1,2
.autop

.c;XMAC Instructions
.c;=================
.bl
.c;XMAC release 2.0
.bl
.c;Kris Kreutzman
.c;OMNYC Software Associates
.skip 5

  This is the operating instructions manual for the Macintosh-VAX/VMS
XMODEM Protocol File Transfer Utility (XMAC).

  The XMAC utility is used on the VAX/VMS side of transferring files between
the Macintosh computer and any DEC VAX system running VMS software.
The program "MacTerminal" is run on the Macintosh as the other side of the
transfer link.
XMAC and MacTerminal send and receive files based upon a modified XMODEM
protocol.  This protocol insures a high reliabilily in file transmissions by
incorporating error checking and retransmission.



.hl1  XMAC command and options

.literal
  Format

    XMAC <filespec>

    Command Qualifiers        Defaults

    /SEND_FILE
    /RECEIVE_FILE
    /TEXT[=line_max]          /TEXT=80
    /LOG_FILE                 /NOLOG
    /BELLS=n                  /BELLS=0
    /DEVICE=x                 /DEVICE=sys$command
.end literal
.page
Command Parameters

FILESPEC
.lm 15
  Specifies the data or text file to be transfered.
.lm 10

Qualifiers

/SEND_FILE
.lm 15
  Send a VAX file to the Mac.
.lm 10

/RECEIVE_FILE
.lm 15
  Receive a Mac file to the VAX.
.lm 10

/TEXT
.lm 15
  Translate TEXT ONLY files between VAX and Mac formats.  The optional
parameter determines the point where a new line is inserted if Macintosh text
lines exceed this number per line.
.lm 10

/LOG_FILE
.lm 15
  Keep a log file of all transmissions, receptions, formating
and special exceptions.  Used in debugging and tracing command line errors.
.lm 10

/BELLS
.lm 15
  Beep the terminal when a file transfer is complete.
.lm 10

/DEVICE
.lm 15
  Determines which device XMAC transfers files through.
.lm 10


.hl1  XMAC operation

.hl2  Setup of XMAC

  Do the following once per login session (usually done in the LOGIN.COM
file) :     $ SET COMMAND XMAC

  If XMAC is to be shared between several persons and/or you want the image
in a special directory do the following :
.list
.le;EDIT the XMAC.CLD file.
.le;Change the line "IMAGE sys$disk:[]xmac.exe" to contain the new disk
and directory. ie "IMAGE dur1:[kris.public]xmac.exe"
.le;Move the files XMAC.CLD and XMAC.EXE into the directory.
.le;Set file protection to WORLD:READ,EXECUTE for both files and all
directories. (this is for sharing only)
.le;The SET COMMAND must reference now must reference the CLD file in the
new directory.  ie "$ SET COMMAND dur1:[kris.public]xmac"
.els

.hl2  Setup of MacTerminal

.list 1,"o"
.le;Set the XMODEM and MACINTOSH attributes under the FILE TRANSFER menu.
.le;DO NOT set the XON/XOFF attribute under the COMPATABILITY menu.
It interferes with the file transfer and causes MacTerminal to error.
.le;Call and login to the VAX.
.els


.hl2  Mac to VAX transfer

.list
.le;Start XMAC with a <filespec> to put the data to and /RECEIVE mode
with /TEXT being optional.
.le;Select SEND from the FILE menu.
.le;Select the file to send from the mini-finder.
.els

  MacTerminal will transmission with a running percentage indicator.

.hl2  VAX to Mac transfer

.list
.le;Start XMAC with the <filespec> to get the data from and /SEND mode with
/TEXT being optional.
.els

  MacTerminal will automatically intercept the incoming data and route it
to a file.



.hl1  Differences, Tips and Notes

  XMAC can down and upload any Macintosh file.  Because of the differences
between the computers, Macintosh files on the VAX can only be moved, examined,
stored and duplicated.

  The exception is TEXT files.  XMAC provides translation of pure text files
from the Macintosh format to VAX/VMS format and back.  This allows editing and
use of text on both systems.  However, due to the differences between the way
the two computers store text files, there are some guidelines to follow for
the best results.


.hl2  Macintosh vs. VAX/VMS TEXT file differences

.list 1,"o"
.le;Macintosh files use carriage returns to delimit paragraphs.
.le;Vax files use carriage returns to delimit lines.
.le;Vax files may contain any control character.
.le;It is unknown how most control characters will affect Macintosh files.
.els

.hl2  TIPS :  When creating text files on the Mac for the VAX

.list 1,"o"
.le;Use the MONACO font in 9 or 10 point.  This will ensure the proper spacing
perspective of text for the VAX system.
.le;Do not use the automatic line break by exceeding the window boundry.
Use the carriage return to terminate lines.
.le;Use right justify text. Do not use full justify, paragraph indent or
any other formating.  Formating is not saved in plain TEXT files and will not
be transmitted to the VAX (the VAX can't handle them anyway).
.le;SAVE all files as TEXT ONLY.
.els

.hl2  TIPS :  When creating text files on the VAX for the Mac

.list 1, "o"
.le;Do not use any control characters except TAB and CARRIAGE RETURN.  All
other control characters will be replaced by the tilde character "~" when
transfered to the Mac.
.le;Keep text line lengths under 255 characters.  This is the max which
XMAC can handle.
.els

.hl2  NOTES : On files and filenames

.list 1,"o"
.le;You must specify a VAX/VMS filename for all file transfers.  Macintosh
filenames may contain various characters unsuitable for use by VMS.
.le;Macintosh filenames of binary files are kept internal to the data file.
When the file is sent back to the Mac, the filename will be restored.
.le;A TEXT file sent from the VAX to the Mac will have the same filename on
the Mac as it did on the VAX, without any NODE::DISK:DIRECTORY information
if any was specified.
.le;Any user information associated with a Mac file as shown by the GET INFO
command will be lost in file transfer.  MacTerminal does not send it.
.els

.hl2  NOTES : On error recovery

.list 1,"o"
.le;If the file transfer takes too long, or is terminated early, or locks up
break out of MacTerminal using COMMAND-PERIOD.  Break out of XMAC using
CONTROL-X (COMMAND-X).  If this does not work, wait 10 to 30 seconds for XMAC
to time out and reset, then try again.
CONTROL-X will cause XMAC to terminate in an orderly
manner.  CONTROL-Y will not work with XMAC due to XMODEM contraints.
.le;IF XMAC crashes, VMS commands may not be echoed and CONTROL-Y may not work.
Use the command SET TERM /INTERACTIVE/ECHO to reset the terminal port.
.le;As a last resort, LOGOUT and login again.  If unable to logout then hang
up the phone to force the process to terminate.  If directly connected,
stop the process.
.le;The /LOG_FILE qualifier will create a text file of data transfered and
other useful information.  This file can be analyzed to determine why a
transfer did not work.
.els
$ CREATE xmac.not
INFORMATION ON MACTERMINAL XMODEM PROTOCOL
==========================================

NOTE :  This information may or may not be correct.  It is what I am
        basing my program (XMAC - VAX VMS file transfer program) on.

        Kris Kreutzman - OMNYC Software Associates.


XMODEM control bytes :

<SOH> = 01 HEX
<EOT> = 04 HEX
<ACK> = 05 HEX
<NAK> = 15 HEX
<CAN> = 18 HEX
<ESC> = 1A HEX


XMODEM packet :

+-----+------+-------+----+----+--//-+------+--------+
| hdr | blk# | blk#- | d0 | d1 | ... | d127 | chksum |
+-----+------+-------+----+----+--//-+------+--------+

hdr      = header byte <SOH> 01 HEX
blk#     = block number, 8 bits, start at 01 wrap 0FFH to 00H
blk#-    = ones complement of blk#, start at 0FEH
d0..d127 = 128 bytes of data, full 8 bits
chksum   = sum of data bytes (d0..d127), 8 bits, throw away carry


normal XMODEM protocol :

receiver   transmiter
--------   ----------
<NAK> ------->
      <------- packet #1
<ACK> ------->
      <------- packet #2 (garbled in transmition)
<NAK> ------->
      <------- packet #2
<ACK> ------->
      <------- <EOT>
<ACK> ------->


MacTerminal XMODEM protocol :

receiver   transmiter
--------   ----------
      <------- <ESC>
      <------- <'a'>
<ACK> ------->
      <------- packet #1 (header information)
<ACK> ------->
      <------- <EOT>
<ACK> ------->
<NAK> ------->
      <------- packet #1 (1st packet of data fork)
<ACK> ------->
      <------- packet #2
<ACK> ------->
      <------- <EOT>
<ACK> ------->
<NAK> ------->
      <------- packet #1 (1st packet of resource fork)
<ACK> ------->
      <------- packet #2
<ACK> ------->
      <------- <EOT>
<ACK> ------->


Header format :

Start Length Description
----- ------ -----------
  00     1    always 00 {???}
  01     1    length of filename  i.e. 8
  02    63    filename buffer     i.e. "Workfile to Vax"
  65     4    file type           i.e. "TEXT"
  69     4    file author         i.e. "WORD"
  81     2    unknown (122C HEX) {???}
  83     4    data fork length in bytes
  87     4    resource fork length in bytes
  91     4    create time
  95     4    modify time
  99    37    the rest of the header (assumed garbage) {???}


MacTerminal peculularities :

o  You can not NAK the header block.  It will blissfully continue on
   to the data fork transmision and not retransmit the header block.
o  The packet header number resets to one (01) before the data and
   resource fork transmisions.
o  The GET INFO information contained in the directory is not trans-
   mitted.
o  MacTerminal is TRANSMITER initiated while normal XMODEM protocol is
   RECEIVER initiated.
o  If there is no data or resource fork, then just the <EOT> control
   character is sent for that fork.
o  If the data or resourc
