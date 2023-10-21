MODULE MakeMake;

  (*
   * MAKEMAKE.  Create a MAKEFILE for a MODULA-2 program.
   *
   * Written by Steve Tynor, 30 September 1986.
   *            UUCP  : tynor@gitpyr
   *            USNAIL: 2550 Akers Mill Rd. T-2, Atlanta GA. 30339
   *
   * Permission is granted to distribute, copy and change this program as long
   * as this notice remains...
   *)

  (*
   * MAKEMAKE.  Create a MAKEFILE for a MODULA-2 program.
   * Usage:
   *      MAKEMAKE "main-module-modulename"
   *
   *    Even though module dependencies are explicit in MODULA-2, it can still
   * be quite a chore to figure them out and create a MAKEFILE.  This program 
   * reads the text file associated with the module name given on the command 
   * line and recursively checks dependencies for all imported files.  It then
   * creates a MAKEFILE suitable for use with the Unix MAKE utility.  If a 
   * file is not found and is not a recognized library module, then the
   * MAKEFILE will include a comment:
   *     ## File NOT Found ##
   * on the right hand side of the dependency.
   *
   *
   * This program was written on the Atari ST using TDI Modula-2. 
   *
   * BUGS:
   *     the CommandLine module dies when a null command line is seen.  
   * Unfortunately, I no longer have the source to the module, so I can't
   * fix it.  This bug also seems to consider a command line consisting of
   * a single character as a reason for dieing.
   *
   * Suggestions for porting to another compiler:
   *
   *    1) module CommandLine is non-standard.  It exports 2 procedures:
   *          PROCEDURE GetArgument (    num      : CARDINAL; 
   *                                 VAR filename : ARRAY OF CHAR);
   *          PROCEDURE NumberOfArguments() : CARDINAL;
   *       they do just what their names imply. (ie. like C's argv, argc)
   *
   *    2) the IMPLEMENTATION MODULE MakeLibraries will have to be edited to
   *       reflect the standard library that your compiler knows about.  List
   *       any modules that you don't want included in your MAKEFILE.  Library
   *       modules are listed in the MAKEFILE, but are commented out.
   *
   *    3) the procedure ModulenameToFilename in MakeParse will have to be
   *       modified so that it creates the correct filename for your machine.
   *
   *    4) if your MAKE program has default MODULA-2 rules, set the constant
   *       MAKEHasModulaRules (in this module) to TRUE.  This will prevent
   *       MAKEMAKE from producing remake lines. 
   *    
   *    5) the procedure WriteFilename takes a parameter: extension.  Trace
   *       all the calls to this function and make sure the proper extentions
   *       are used.  in TDI Modula, '.DEF' = definition module text
   *                                 '.MOD' = implementation module text
   *                                 '.SYM' = symbol file (compiled DEF)
   *                                 '.LNK' = symbol file (compiled MOD)
   *)

IMPORT InOut;
IMPORT CommandLine;
IMPORT MakeForest;
IMPORT MakeParse;
IMPORT ASCII;
IMPORT Streams;
IMPORT Strings;

CONST
  MAKEHasModulaRules = FALSE;
  CompilerPathname   = 'C:\BIN\MOD.PRG ';
  LinkerPathname     = 'C:\BIN\LNK.PRG ';

VAR
  outputStream : Streams.Stream;

  (*----------------------------------------------------------------------*)
  PROCEDURE CloseOutput;
  VAR
    reply : INTEGER;
  BEGIN
    Streams.CloseStream (outputStream, reply);
  END CloseOutput;

  (*----------------------------------------------------------------------*)
  PROCEDURE OpenOutput (VAR filename : ARRAY OF CHAR) : BOOLEAN;
  VAR
    reply : INTEGER;
  BEGIN
    Streams.OpenStream (outputStream, filename, Streams.READWRITE, reply);
    RETURN reply = 0;
  END OpenOutput;


  (*----------------------------------------------------------------------*)
  PROCEDURE Write (ch : CHAR);
  BEGIN
    Streams.Write8Bit (outputStream, ch);
  END Write;


  (*----------------------------------------------------------------------*)
  PROCEDURE WriteString (VAR str : ARRAY OF CHAR);
  VAR
    c : CARDINAL;
  BEGIN
    c := 0;
    WHILE (c <= HIGH(str)) AND (str[c] <> 0C) DO
      Write(str[c]);
      INC(c);
    END; (* WHILE *)
  END WriteString;


  (*----------------------------------------------------------------------*)
  PROCEDURE WriteLn;
  BEGIN
    Write ( ASCII.CR );
    Write ( ASCII.LF );
  END WriteLn;


  (*----------------------------------------------------------------------*)
  PROCEDURE WriteMakefile;

    (*--------------------------------------------------------------------*)
     PROCEDURE WriteFileDependency (file : MakeForest.FileDefinitionList;
                                    VAR lhsExtention : ARRAY OF CHAR);
     VAR
       c    : CARDINAL;
       iptr : MakeForest.ImportList;
     BEGIN
       IF file <> NIL THEN
         IF file^.filename[0] = '#' THEN
           RETURN;
         END; (* IF *) 
         WriteFilename (file^.filename, lhsExtention);
         Write (ASCII.HT);
         WriteString (': ');
         WriteString (file^.filename);
           (* if this is a .LNK file, then don't forget the .SYM file: *)
           (* but don't do it for the main module! *)
         IF (file <> MakeForest.FileForestEnd) AND
            (Strings.Compare (lhsExtention, '.LNK') = Strings.Equal) THEN
           Write (' ');
           WriteFilename (file^.filename, '.SYM');
           c := 3;
         ELSE
           c := 2;
         END; (* IF *)
         iptr := file^.imports;
         WHILE iptr <> NIL DO
           IF NOT iptr^.file^.library THEN
             IF c > 4 THEN
               WriteString (' \');
               WriteLn;
               WriteString ('  ');
               c := 0;
             END; (* IF *)
             Write (' ');
             WriteFilename (iptr^.file^.filename, '.SYM'); 
             INC (c);
           END; (* IF *)
           iptr := iptr^.next;
         END; (* WHILE *)

         iptr := file^.imports;
         c := 5;  (* <-- force a WriteLn *)
         WHILE iptr <> NIL DO
           IF iptr^.file^.library THEN
             IF c > 4 THEN
               WriteLn;
               WriteString ('  #libs:  ');
               c := 0;
             END; (* IF *)
             Write (' ');
             WriteFilename (iptr^.file^.filename, '.SYM'); 
             INC (c);
           END; (* IF *)
           iptr := iptr^.next;
         END; (* WHILE *)

         WriteLn;
         IF NOT MAKEHasModulaRules THEN
           Write (ASCII.HT);
           WriteString (CompilerPathname);
           WriteString (file^.filename);
           WriteLn;
         END; (* IF *)
       END; (* IF *)
     END WriteFileDependency;

    (*--------------------------------------------------------------------*)
    PROCEDURE WriteAllLnkFiles;
    VAR
      c, dummy    : CARDINAL;
      ptr : MakeForest.FileDefinitionList;
    BEGIN
      ptr := MakeForest.FileForest;
      c := 2;
      REPEAT
        IF Strings.Pos (ptr^.filename, '.MOD', 0, dummy) THEN
          IF NOT ptr^.library THEN
            WriteFilename (ptr^.filename, '.LNK');  
            Write (' ');
            IF c > 4 THEN
              WriteString (' \');
              WriteLn;
              WriteString ('  ');
              c := 0;
            END; (* IF *)
            INC (c);
          END; (* IF *)
        END; (* IF *)
        ptr := ptr^.next;
      UNTIL ptr = NIL;
      WriteLn;
    END WriteAllLnkFiles;

    (*--------------------------------------------------------------------*)
    PROCEDURE WriteFilename (VAR filename, extention : ARRAY OF CHAR);
    VAR
      c : CARDINAL;
    BEGIN
      c := 0;
      REPEAT
        Write (filename[c]);
        INC(c);
      UNTIL (c > HIGH (filename)) OR (filename[c] = '.');
      WriteString (extention);
    END WriteFilename;

    (*--------------------------------------------------------------------*)
    PROCEDURE WritePRGDependency (file : MakeForest.FileDefinitionList);
    BEGIN
      WriteLn;
      WriteFilename (file^.filename, '.PRG');
      Write (ASCII.HT);  WriteString (': ');
      WriteAllLnkFiles;
      IF NOT MAKEHasModulaRules THEN
        Write (ASCII.HT);  WriteString (LinkerPathname);
        WriteFilename (file^.filename, '.LNK');
        WriteLn;
      END; (* IF *)
    END WritePRGDependency;

  VAR
    ptr    : MakeForest.FileDefinitionList;
    c      : CARDINAL;
  BEGIN
    IF OpenOutput ('MAKEFILE') THEN
        (* print out the .PRG dependency first.... *)
      WritePRGDependency (MakeForest.FileForestEnd);
      WriteFileDependency (MakeForest.FileForestEnd, '.LNK');
      ptr := MakeForest.FileForest;
      WHILE ptr <> MakeForest.FileForestEnd DO
        IF NOT ptr^.library THEN
          IF Strings.Pos (ptr^.filename, '.MOD', 0, c) THEN
            WriteFileDependency (ptr, '.LNK');
          ELSE
            WriteFileDependency (ptr, '.SYM');
          END; (* IF *)
        END; (* IF *)
        ptr     := ptr^.next;
      END; (* WHILE *)
    ELSE
      InOut.WriteString ('Error opening MAKEFILE');
      InOut.WriteLn;
    END; (* IF *)
    CloseOutput;
  END WriteMakefile;

VAR
  modulename,
  filename : ARRAY [0 .. 132] OF CHAR;

BEGIN
  IF CommandLine.NumberOfArguments() < 1 THEN  
    InOut.WriteString ("Usage:   MAKEMAKE main-module-modulename");
    InOut.WriteLn;
  ELSE
    CommandLine.GetArgument (1, filename);
    MakeParse.ParseModule (filename, MakeParse.main, filename);
    WriteMakefile;
  END; (* IF *)
END MakeMake.
