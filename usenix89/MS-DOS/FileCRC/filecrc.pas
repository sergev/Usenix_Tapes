
{  PROGRAM TO CREATE OF FILE OF  THE CRC'S OF THE FILES ON THE DEFAULT DISK  }

{

  This program was written by Ted H. Emigh, and has been placed in the public
  domain, to be used at the user's discretion.  The CRC routines and the
  discussion of the CRC were written by David Dantowitz, Digital Equipment
  Corporation,  Dantowitz%eagle1.dec@decwrl.

  This program calculates the CRC (cyclic redundancy check) for all the files
  on the disk (with the exception of files that are hidden system files).  The
  CRC's are placed in a file (CHECK$$$.NEW) to be compared with the CRC's
  calculated at a previous time in the file CHECK$$$.CRC.  The comparison is
  done with the program COMPARE.PAS.  This program is set to automatically
  chain to COMPARE.PAS to automate the procedure, but this can be turned off
  by deleting the lines:
    Assign (chain_file,'COMPARE.CHN');
    Chain(chain_file);
  at the end of this program.


   For a good discussion of polynomial selection see "Cyclic
   Codes for Error Detection", by W. W. Peterson and
   D. T. Brown, Proceedings of the IEEE, volume 49, pp 228-235,
   January 1961.

   A reference on table driven CRC computation is "A Cyclic
   Redundancy Checking (CRC) Algorithm" by A. B. Marton and
   T. K. Frambs, The Honeywell Computer Journal, volume 5,
   number 3, 1971.

   Also used to prepare these examples was "Computer Networks",
   by Andrew S. Tanenbaum, Prentice Hall, Inc.  Englewood Cliffs,
   New Jersey, 1981.

   The following three polynomials are international standards:


        CRC-12 = X^12 + X^11 + X^3 + X^2 + X^1 + 1
        CRC-16 = X^16 + X^15 + X^2 + 1
        CRC-CCITT = X^16 + X^12 + X^5 + 1

   In Binary and hexadecimal :

                   Binary                     Hex

        CRC-12    = 1111 0000 0001           $0F01
        CRC-16    = 1010 0000 0000 0001      $A001
        CRC-CCITT = 1000 0100 0000 1000      $8404    (Used below)

   The first is used with 6-bit characters and the second two
   with 8-bit characters.  All of the above will detect any
   odd number of errors.  The second two will catch all 16-bit
   bursts, a high percentage of 17-bit bursts (~99.997%) and
   also a large percentage of 18-bit or larger bursts (~99.998%).
   The paper mentioned above (Peterson and Brown) discusses how 
   to compute the statistics presented which have been quoted 
   from Tanenbaum.

   (A burst of length N is defined a sequence of N bits, where
   the first and last bits are incorrect and the bits in the
   middle are any possible combination of correct and incorrect.
   See the paper by Peterson and Brown for more information)

}

{$G512,P512,U+,R+ }
Program FILECRC;

Const
  BufSize = 192;  { Number of 128 byte sectors in the CRC buffer }
  Buffer_Length = 24576;  { BufSize * 128 = Length of the CRC buffer }
  Version = 1.00;
  Version_Date = '13 AUG 86';
  POLY = $8404;  {  CRC Polynomial Used  }

Type
  Bytes = Array [1..24576] of Byte;  {  Length is 1..Buffer_Length  }

  Registers = record  {  Registers for 8088/8086/80286  }
                ax, bx, cx, dx, bp, si, di, ds, es, flags : integer;
              end;

  DTA_record = record  {  DTA as used by MSDOS  }
                 dos : array [1..21] of char;
                 attribute : byte;  {  Attribute byte  }
                 time_of_day : integer;  {  Time of Day of File Creation  }
                 date : integer;  {  Date of File Creation  }
                 low_size, high_size : integer;  {  Size of the File  }
                 filename: array [1..13] of char;  { File Name  }
                 junk : array [1..85] of byte;
               end;

  string255 = string[255];

Var
  {  Variables used in Calculating the CRC  }

  str_length, RecsRead, CRC_value : integer;
  table_256 : Array [0 .. 255] of Integer;  {CRC Table to speed computations}
  byte_string : Bytes;

  {  Variables used in setting up the input and output files  }

  filvar : file;
  chain_file : file;
  outfile : TEXT[$4000];
  check_crc : boolean;

  {  Misc. Variables  }

  root : string255;  {  Contains the default drive and root directory }
  global_reg : registers;  {  Registers for the DOS calls  }


Procedure generate_table_256(POLY : Integer);

{
    This routine computes the remainder values of 0 through 255 divided
  by the polynomial represented by POLY.  These values are placed in a
  table and used to compute the CRC of a block of data efficiently.
  More space is used, but the CRC computation will be faster.



    This implementation only permits polynomials up to degree 16.
}


Var
   val, i, result : Integer;

Begin
For val := 0 to 255 Do
  Begin
     result := val;
     For i := 1 to 8 Do
        Begin
           If (result and 1) = 1
              then result := (result shr 1) xor POLY
              else result :=  result shr 1;
        End;

     table_256[val] := result;
  End
End;


Function crc_string_256(Var s : Bytes; s_length, initial_crc : Integer)
                        : Integer;

{
     This routine computes the CRC value and returns it as the function
  value.  The routine takes an array of Bytes, a length and an initial
  value for the CRC.  The routine requires that a table of 256 values
  be set up by a previous call to Generate_table_256.

      This routine uses table_256.
}

Begin

inline(

$c4/$7e/<s/                {les di,s[bp]            (es:di points to array)  }
$8b/$46/<initial_crc/      {mov ax,initial_crc[bp]  (initial CRC value)      }
$8b/$4e/<s_length/         {mov cx,s_length[bp]     (count)                  }
$be/table_256/             {mov si,offset table_256 (table address)          }


{ next:  }

$26/$32/$05/               {xor al,es:[di]          CRC = CRC XOR next byte  }
$47/                       {inc di                  (point to next byte)     }

{ intermediate steps, see comments for overall effect }

$31/$db/                   {xor bx,bx               (bx <- 0)                }
$86/$d8/                   {xchg al,bl              (bx <- ax and 0FF)       }
$86/$e0/                   {xchg al,ah              (ax <- ax shr 8)         }
$d1/$e3/                   {shl bx,1                (bx <- bx+bx)            }

$33/$00/                   {xor ax,[bx+si]          CRC = (CRC shr 8) XOR
                                                          table[CRC and 0FF] }

$e2/$f0/                   {loop next               (count <- count -1)      }

$89/$46/<s+4);             {mov s+4[bp],ax          (crc_string_256 := CRC)  }


{  basic algorithm expressed above

crc := initial_crc

For each byte Do
Begin
  crc := crc XOR next_byte;
  crc := (crc shr 8) XOR table_256 [crc and $FF];
End;

crc_string_256 := crc;
}
End;



Procedure set_attr (attr : byte; asciiz : string255);
{

  This routine sets the file attributes.  Uses Function $43 in
  Interrupt $21.

  Turbo Pascal is unable to open and read various types files
  (e.g., r/o and files that are both hidden and system).  This
  gets around that by always setting the attribute to 0, then
  reseting it to the original value.

  attr  is the attribute to be set on the file
  asciiz is a string variable with the file name

}

begin
  asciiz := asciiz + chr(0);  {  Make a valid DOS ASCIIZ name  }
  {  Set up the registers for the interrupt  }
  global_reg.ax := $4301;
  global_reg.ds := seg(asciiz);
  global_reg.dx := ofs(asciiz)+1;
  global_reg.cx := attr;
  intr ($21, global_reg);
end;


Procedure get_crc(this_file : string255; dta : DTA_record);
{
  This procedure computes the CRC for a file.  Value is returned
  in the global variable CRC_value.

  this_file is a string variable containing the file name
  dta is a DTA_Record containing the file's DTA

}

var
  length  : real;  {  Length of the File  }

begin

  {  Change the Attribute byte so we can always open it  }
  {    To save some time, this is only done if the file  }
  {    Has any attribute other than ARCHIVE              }

  if (dta.attribute and $DF <> 0) then
    set_attr ( 0, this_file);

  {  Get the size of the file  }

  if dta.low_size < 0 then
    {  Negative low_size is really number between 32768 and 65536  }
    length := int(dta.high_size)*65536.0 + 32768.0
              + int(dta.low_size and $7FFF)
  else
    length := int(dta.high_size)*65536.0 + int(dta.low_size);

  {  Open the file as untyped  }

  Assign (Filvar, this_file);
  Reset (Filvar);

  {  Calculate the CRC  }

  CRC_value := 0;
  While length > 0.5 do
  Begin
    {  Read a segment of the file to process  }
    BlockRead(filvar,byte_string,BufSize,RecsRead);
    {  Get the correct number of bytes to process  }
    if length >= Buffer_Length then
      str_length := Buffer_Length
    else
      str_length := round(length);
    {  Compute the CRC  }
    CRC_value := crc_string_256(byte_string, str_length, CRC_value);
    {  Adjust the file length  }
    length := length - Buffer_Length;
  End;

  Close (Filvar);

  {  Restore the correct Attribute Byte  }
  if (dta.attribute and $DF <> 0) then
    set_attr ( dta.attribute, this_file);

end;


Procedure directory(current_directory : string255);

{
  Procedure to calculate the CRC of all the files in a directory,
  then all subdirectories in that directory

  current_directory contains the directory name (including drive)

}

var
  DTA_ofs, DTA_seg : integer;  {  Contains the current DTA address  }
  reg : Registers;  {  Local 8088/8086/80286 registers  }
  DTA : DTA_record;  {  Local DTA  }
  this_directory, this_file, asciiz : string255;  { directory and file names }


function get_file : string255;

{  Get the file name from the DTA  }

var
  i : integer;
  temp_file : string255;

begin
  i := 1;
  temp_file := '';
  repeat
    temp_file := temp_file + DTA.filename[i];
    i := i+1;
  until dta.filename[i] = chr(0);

  get_file := temp_file;

end;


function is_directory : boolean;

{  Function to tell if the file is a directory entry  }

begin
  is_directory := ((dta.attribute and $10) <> 0)
                   and (dta.filename[1] <> '.');
end;

Procedure set_DTA(offset, segment : integer);

{   sets the disk DTA
    Uses MSDOS Function $1A with interrupt $21
    offset is the offset of the new DTA
    segment is the segment of the new DTA
}

begin
  reg.ax := $1a00;
  reg.ds := segment;
  reg.dx := offset;
  intr($21, reg);
end;

Procedure get_DTA(var offset, segment : integer);

{   gets the disk DTA
    Uses MSDOS Function $2F with Interrupt $21
    offset will return with the current DTA offset
    segment will return with the current DTA segment
}

begin
  reg.ax := $2f00;
  intr($21, reg);
  offset := reg.bx;
  segment := reg.es;
end;


Function find_first (attr_mask : byte) : boolean;

{
    Find the first file matching the ASCIIZ string.
    attr_mask is $27 for files only and $37 for directories & files

    INT 21 function 4EH
    Returns TRUE if found, FALSE if not found
}

begin
  reg.ax := $4e00;
  reg.ds := seg(asciiz);
  reg.dx := ofs(asciiz)+1;
  reg.cx := attr_mask;
  intr($21, reg);
  find_first := (lo(reg.ax) <> 18);

end;


Function find_next (attr_mask : byte) : boolean;

{
    Find the next file matching the ASCIIZ string.
    attr_mask is $27 for files only and $37 for directories & files

    Returns TRUE if found, FALSE if not found
}

begin
  reg.ax := $4f00;
  reg.cx := attr_mask;
  intr($21, reg);
  find_next := (lo(reg.ax) <> 18);
end;


begin { directory }

  get_DTA(DTA_ofs, DTA_seg); { Save the current DTA location }

  set_DTA(ofs(DTA), seg(DTA)); { Set the DTA location to local area }

{
  Find and print the files in the current directory
}

  asciiz := current_directory + '\*.*' + CHR(0);  {  CHR(0) to make proper  }

{  Process all the files before doing any directories  }

  if find_first ($27) then
    repeat
      if dta.filename[1] <> '.' then
        begin
          this_file := get_file;
          get_crc(current_directory + '\' + this_file, dta);
          writeln(outfile,current_directory,' ',this_file,' ',
                dta. attribute,' ',dta.time_of_day,' ',dta.date,' ',
                dta.low_size,' ',dta.high_size,' ',CRC_value);
        end;
    until not find_next ($27);

{  Now process all the directories  }

  if find_first ($37) then
    repeat
      if is_directory then
      begin
        this_directory := current_directory + '\' + get_file;
        Writeln(this_directory);
        directory(this_directory);  {  Now do all subdirectories  }
      end;
    until not find_next ($37);

  set_dta(DTA_ofs, DTA_seg); { restore the old DTA }

end;


Function current_drive : byte;
{
  Function to return the current drive
  Uses MSDOS Function $19 with Interrupt $21
  current_drive is 1 if A, 2 if B, 3 if C, etc.

}

begin
  global_reg.ax := $1900;
  intr($21, global_reg);
  current_drive := 1 + lo(global_reg.ax);
end;


BEGIN  {  FILECRC  }

  {  root will have the current drive designation  }
  root := chr(current_drive + ord('A') - 1) + ':';

  Writeln('CRC file integrity program');
  Writeln('Version ',version:5:2,', ',version_date);
  Write('Written by Ted H. Emigh -- ');
  Writeln('emigh@ecsvax.uucp or NEMIGH@TUCC.BITNET');

  Assign (filvar,'CHECK$$$.CRC');
  {$I-}
  Reset (filvar);   {  See if CHECK$$$.CRC exists  }
  {$I+}
  {  check_crc will be TRUE if CHECK$$$.CRC exists  }
  check_crc := (IOresult = 0);
  if check_crc then
  begin
    Assign (outfile,'CHECK$$$.NEW');
    Writeln ('Creating File CHECK$$$.NEW');
  end
  else
  begin
    Assign (outfile,'CHECK$$$.CRC');
    Writeln ('Creating File CHECK$$$.CRC');
  end;
  Close (filvar);
  Rewrite (outfile);  {  Open the output file  }

  Generate_table_256(POLY);  {  Generate the table for CRC check  }

  Writeln(root+'\');
  directory(root);  {  Now, do the CRC check  }

  Close (outfile);

  { Now compare this with the previous CRC's  }

  if check_crc then
  begin
    Assign (chain_file,'COMPARE.CHN');
    Chain(chain_file);
  end;
end.
