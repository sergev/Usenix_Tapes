
{   PROGRAM TO COMPARE THE CRC'S OF THE FILE LISTS IN  }
{   CHECK$$$.NEW AND CHECK$$$.CRC                      }

{$G512,P512,U+,R+ }
Program Compare;

TYPE
  string255 = string[255];
  string64 = string[64];
  string12 = string[12];

  Registers = record
                ax, bx, cx, dx, bp, si, di, ds, es, flags : integer;
              end;
  Months = array [1..12] of string[3];

  Directory_record = record
                       directory : string64;
                       FileNum   : integer;
                     end;

  File_Rec = record
               name                : string12;
               time_of_day, date   : integer;
               low_size,high_size  : integer;
               attribute           : byte;
               crc                 : integer;
             end;


CONST
  month : Months = ('JAN','FEB','MAR','APR','MAY','JUN',
                    'JUL','AUG','SEP','OCT','NOV','DEC');
  Version = 1.00;
  Version_Date = '13 AUG 86';

VAR

  {  File Creation time and date  }
  TimeOfDay, FileDate : integer;
  directory_number, file_number : integer;
  {  Number of files in each category  }
  old_file, new_file, OK_file, Update_file, Mod_file : integer;

  old_filename, new_filename : string64;
  infile : TEXT[$0800];  { file for reading file lists }
  newfile : TEXT; { file for writing names of new files created }
  modfile : TEXT; { file for writing names of modified files }
  tempfile : file; { used in renaming files }

  CRC_value : Integer;

  filename : string12;
  Name_of_File, CRC_string, instring : string255;

  attribute : byte;
  lowsize, highsize : integer;
  new, new_dir : boolean;

  number_directories, direct_count : integer;

  this_directory, current_directory : string64;

  directories : array [1..200] of directory_record;
  fileinfo : array [1..1900] of file_rec;


function get_string  : string255;
{
  This function returns a string up to the first space from infile
}
var
  inchar : char;
  temp_string : string255;

begin
  {  Ignore any leading blanks  }
  Repeat
    read(infile, inchar);
  Until inchar <> ' ';

  temp_string := '';

  {  Now, add on to temp_string until a blank is found  }
  Repeat
    temp_string := temp_string + inchar;
    read(infile, inchar);
  Until inchar = ' ';

  get_string := temp_string;

end;

procedure read_old_file;
{
  Procedure to read in the old list of files and set up the list of
  directories (variable directories), and the list of files along with
  the various data (variable fileinfo).
  On return,
  old_file has the number of files in the list and
  number_directories has the number of directories.

  The variables directories and fileinfo have the following information:
  directories  directory : Name of the directory (up to 64 characters)
               FileNum   : Number of the name in fileinfo that contains
                           the information for the first file in this
                           directory.

  fileinfo     name        : Name of the file
               time_of_day : Time of day in DOS format
               date        : Date in DOS format
               low_size    : Low byte of the file size
               high_size   : High byte of the file size
               attribute   : Attribute of the file
               crc         : CRC of the file

}

begin
  Reset (infile);  {  Set to read Old List of Files  }
  old_file := 0;  {  Number of files in the list  }
  number_directories := 0;  {  Number of directories in the list  }
  While not eof(infile) do
  begin
    old_file := old_file + 1;  {  Another file  }
    this_directory := get_string;  {  Get the directory name  }
    fileinfo[old_file].name := get_string;  {  Get the file name  }
    if this_directory <> current_directory then
    begin
      current_directory := this_directory;
      number_directories := number_directories + 1;
      directories[number_directories].directory := this_directory;
      directories[number_directories].FileNum := old_file;
    end;
    With fileinfo[old_file] do
      Readln(infile,attribute, Time_of_day, date, low_size, high_size, crc);
  end;
  directories[number_directories + 1].FileNum := old_file + 1;
  Close (infile);
end;


function get_time(date1,date2 : integer) : string64;
{
  This function returns the time and date of file creation.
  date1 is the time of day in DOS format
  date2 is the date of creation in DOS format

  get_time is a string with the time and date (e.g., 14:31:42  8 AUG 1986)
}

var
  hour, minute, second : integer;
  temp, time : string64;
  year, n_month, day : integer;

begin

  hour := date1 shr 11;
  minute := (date1 shr 5) - (hour shl 6);
  second := (date1 - (minute shl 5) - (hour shl 11))*2;
  year := date2 shr 9;
  n_month := (date2 shr 5) - (year shl 4);
  day := date2 - (n_month shl 5) - (year shl 9);
  Str(hour:2,temp);
  time := temp + ':';
  Str(minute:2,temp);
  time := time + temp + ':';
  Str(second:2,temp);
  time := time + temp + '   ';
  Str(day:2,temp);
  time := time + temp + ' ' + month[n_month] + ' ';
  Str(year + 1980:4,temp);
  get_time := time + temp;

end;

procedure write_old_file ( file_number : integer);
{
  Procedure to write the attribute, size and CRC for a file from
  the old list

  file_number is the number of the file name

}

var
  filesize : real;
begin
  with fileinfo[file_number] do
  begin
    if low_size < 0 then
      filesize := int(high_size)*65536.0 + 32768.0 + int(low_size and $7FFF)
    else
      filesize := int(high_size)*65536.0 + int(low_size);
    Write ('Attribute = ',attribute:3,', Size = ',filesize:10:0);
    Writeln(', CRC = ',CRC);
  end;
end;


procedure write_new_file;
{
  Procedure to write the attribute, size and CRC for a file from
  the new list

}

var
  filesize : real;
begin
  if lowsize < 0 then
    filesize := int(highsize)*65536.0 + 32768.0 + int(lowsize and $7FFF)
  else
    filesize := int(highsize)*65536.0 + int(lowsize);
  Write ('Attribute = ',attribute:3,', Size = ',filesize:10:0);
  Writeln(', CRC = ', CRC_value)
end;


procedure find_directory( var number : integer; var newdir : boolean);
{
  Procedure to the the directory from the old list that matches the
  directory name from the new list

  If the directory name is the same as the current directory, then
  number and newdir are unchanged.

  If the directory name is not the same, and it exists on the old list,
  number will be the number of the old directory, and newdir is FALSE.
  The current directory will be updated.

  If the directory name is not the same, and it does not exist on the
  old list, newdir is FALSE.  Number is number of directories + 1, but
  is never used.

}
begin
  {  If the directory is the same, then the status of number and newdir  }
  {  will not change                                                     }
  if this_directory <> current_directory then
  begin  {  search from the beginning  --  nothing fancy  }
    number := 0;
    Repeat
      number := number + 1;
    Until (number > number_directories) or
      (this_directory = directories[number].directory);
    newdir := (number > number_directories);
    current_directory := this_directory;
  end;
end;

procedure find_file( var number : integer; var new : boolean;
                    number_begin, number_end : integer);
{
  Procedure to find the file name.  The directory name has been
  found prior to this time, so the starting point in the search
  has been found.  The search will continue until the first file
   name in the next directory.

}
begin
  number := number_begin -1;
  Repeat
    number := number + 1;
  Until (number = number_end) or (filename = fileinfo[number].name);
  new := (filename <> fileinfo[number].name);
end;

procedure file_new;
{
  This procedure processes the new files.  new_file is the counter
  for the number of new files.  The file name and information is
  written to the file assigned to newfile.
}

var
  filesize : real;

begin
  new_file := new_file + 1;
  Write (newfile,this_directory+filename);
  Writeln (newfile,' Date: ',get_time(TimeOfDay, FileDate));
  if lowsize < 0 then
    filesize := int(highsize)*65536.0 + 32768.0 + int(lowsize and $7FFF)
  else
    filesize := int(highsize)*65536.0 + int(lowsize);
  Writeln (newfile,'Attribute = ',attribute:3,
           ', Size = ',filesize:10:0,', CRC = ', CRC_value);
end;

procedure file_updated;
{
  This procedure processes the updated files.  Update_file is the counter
  for the number of updated files.
}

begin
  Update_file := Update_file + 1;
end;

procedure file_OK;
{
  This procedure processes the files that have not been changed, modified
  or deleted.  OK_file is the counter for the number of such files.
}

begin
  OK_file := OK_file + 1;
end;

procedure bad_CRC;
{
  This procedure processes the files that have been modified without
  changing the directory entry date or time.  Mod_file is the counter for
  the number of such files.  In normal operations, this should not happen,
  so for such files, the name and date information is shown on the console
  and sent to the file assigned to modfile.
}

begin
  Mod_file := Mod_file + 1;
  Writeln ('CRC''s do not match!  File: ',this_directory+filename);
  Writeln ('Date: ',get_time(TimeOfDay, FileDate));
  Write ('Old file: ');
  write_old_file(file_number);
  Write ('New file: ');
  write_new_file;
  Write (modfile, this_directory + filename);
  Writeln (modfile,' Date: ', get_time(TimeOfDay, FileDate));
end;

procedure read_new_file;
{
  Procedure to read the list of new files, and compare them to the
  old files.  The various comparison types are processed according to
  the preceeding routines.
}

begin
  current_directory := '';
  new_dir := FALSE;

  Assign (infile, new_filename);
  Reset (infile);

  While not eof(infile) do
  begin
    this_directory := get_string;  {  First is the directory name  }
    filename := get_string;  {  Next is the file name  }
    Readln(infile, attribute, TimeOfDay, FileDate, lowsize,
           highsize, crc_value);  {  Then the file parameters  }
    {  Find the entry in the list of old files with the same name  }
    find_directory(directory_number,new_dir);
    if not new_dir then
      find_file(file_number,new,
                directories[directory_number].FileNum,
                directories[directory_number + 1].FileNum-1);
    if (new_dir or new) then  {  New directory means new file  }
      file_new
    else  {  Existing file, compare the two  }
      if (fileinfo[file_number].Time_of_day <> TimeOfDay)
        or (fileinfo[file_number].date <> FileDate) then
          file_updated
      else
        if (fileinfo[file_number].crc <> CRC_value) then bad_CRC
        else
          file_OK;
  end;
  Close (infile);
end;


BEGIN  {  Compare  }

  Writeln('CRC file integrity comparison program');
  Writeln('Version ',version:5:2,', ',version_date);
  Write('Written by Ted H. Emigh -- ');
  Writeln('emigh@ecsvax.uucp or NEMIGH@TUCC.BITNET');

  number_directories := 1;
  current_directory := '';
  directories[1].directory := current_directory;
  directories[1].FileNum := 1;

  {  Reset the counters for the various comparisons  }

  New_file := 0;
  OK_file := 0;
  Update_file := 0;
  Mod_file := 0;

  {  Set up the input and output files  }

  Case ParamCount of
    0 : begin  {  No command line parameters, use default names  }
          old_filename := 'CHECK$$$.CRC';
          new_filename := 'CHECK$$$.NEW';
        end;
    1 : begin  {  File name with listing of new files has been given  }
          old_filename := 'CHECK$$$.CRC';
          new_filename := ParamStr(1);
        end;
    else
        begin  {  Both file names have been given  }
          old_filename := ParamStr(2);
          new_filename := ParamStr(1);
        end;
  end;

  {  Set up the various input and output files  }

  Assign (infile,old_filename);
  Assign(newfile,'FILES$$$.NEW');
  Rewrite (newfile);
  Writeln (newfile,'New files created on this disk');
  Assign(modfile,'FILES$$$.MOD');
  Rewrite (modfile);
  Writeln (modfile,'Files that were modified without updating the directory');


  Writeln ('Reading old CRC list, please wait ...');
  read_old_file;

  Writeln ('Reading new CRC list and checking, please wait ...');
  read_new_file;

  {  Print the summary numbers for this check  }

  Writeln ('Number of Files in the last CRC check:           ',old_file);
  Writeln ('Number of Files that are the same as last time:  ',OK_file);
  Writeln ('Number of New Files:                             ',new_file);
  Writeln ('Number of Deleted Files:                         ',
            old_file - update_file - OK_file - Mod_file);
  Writeln ('Number of Updated Files:                         ',update_file);
  Writeln ('Number of Invalidly Modified Files:              ',Mod_file);
  Writeln;
  Writeln;


  {  Erase the output files if they are empty  }

  Close (newfile);
  if new_file = 0 then Erase (newfile);
  Close (modfile);
  if Mod_file = 0 then Erase (modfile);

  {  No command line parameters  --  Rename the files with the file lists  }

  if ParamCount = 0 then
  begin
    Assign (tempfile, 'CHECK$$$.OLD');
    {$I-}
    Reset (tempfile);  {  See if the file already exists  }
    {$I+}
    if IOresult =0 then
      Erase (tempfile);  {  Yes, it exists -- delete it  }
    Close (tempfile);
    Assign (tempfile, 'CHECK$$$.CRC');
    Rename (tempfile, 'CHECK$$$.OLD');
    Assign (tempfile, 'CHECK$$$.NEW');
    Rename (tempfile, 'CHECK$$$.CRC');
    Writeln ('Old CRC file is now CHECK$$$.OLD');
    Writeln ('New CRC file is now CHECK$$$.CRC');
    Writeln;
  end;



end.
