IMPLEMENTATION MODULE MakeParse;

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

IMPORT MakeToken;
IMPORT MakeForest;
IMPORT Strings;
IMPORT MakeLibraries;

  (*----------------------------------------------------------------------*)
  PROCEDURE ModulenameToFilename (VAR modulename : ARRAY OF CHAR;
                                      defOrMod   : FileType;
                                  VAR fname      : ARRAY OF CHAR);
  CONST
    EOS = 0C;
  VAR
    c : CARDINAL;
  BEGIN
    c := 0;
    WHILE (c < HIGH (modulename)) AND (c < 8) AND (modulename[c] <> EOS) DO
      fname[c] := modulename[c];
      INC (c);
    END; (* WHILE *)
    fname[c] := '.';
    IF defOrMod = def THEN
      fname[c+1] := 'D'; fname[c+2] := 'E'; fname[c+3] := 'F'; 
      fname[c+4] := EOS;
    ELSE
      fname[c+1] := 'M'; fname[c+2] := 'O'; fname[c+3] := 'D';
      fname[c+4] := EOS;
    END; (* IF *)
  END ModulenameToFilename;


  (*----------------------------------------------------------------------*)
  PROCEDURE TerminatingToken(VAR token : ARRAY OF CHAR) : BOOLEAN;
  VAR
    temp : BOOLEAN;
  BEGIN
    (* had to split up due to compiler limitation... *)
    temp := (Strings.Compare (token, 'CONST')     = Strings.Equal) OR
            (Strings.Compare (token, 'TYPE')      = Strings.Equal) OR
            (Strings.Compare (token, 'VAR')       = Strings.Equal) OR
            (Strings.Compare (token, 'MODULE')    = Strings.Equal);
    RETURN  temp OR
            (Strings.Compare (token, 'PROCEDURE') = Strings.Equal) OR
            (Strings.Compare (token, 'BEGIN')     = Strings.Equal) OR
            (Strings.Compare (token, 'END')       = Strings.Equal);
  END TerminatingToken;


  (*----------------------------------------------------------------------*)
  PROCEDURE ParseFROM (      fileDef : MakeForest.FileDefinitionList;
                         VAR eof     : BOOLEAN );
  VAR
    modulename, filename : ARRAY [0 .. 100] OF CHAR;
  BEGIN
    MakeToken.NextToken (modulename, eof);
    ParseModule (modulename, def, filename);
    MakeForest.AddImport (fileDef, filename);
    MakeToken.SkipTillSemicolon (eof);

    ParseModule (modulename, mod, filename);
  END ParseFROM;

  (*----------------------------------------------------------------------*)
  PROCEDURE ParseIMPORT (    fileDef : MakeForest.FileDefinitionList;
                         VAR eof     : BOOLEAN );
  VAR
    modulename, filename : ARRAY [0 .. 100] OF CHAR;
  BEGIN
    MakeToken.NextToken (modulename, eof);
    WHILE modulename[0] <> ';' DO
      ParseModule (modulename, def, filename);
      MakeForest.AddImport (fileDef, filename);

      ParseModule (modulename, mod, filename);
      MakeToken.NextToken (modulename, eof);
    END; (* WHILE *)
  END ParseIMPORT;

  (*----------------------------------------------------------------------*)
  PROCEDURE ParseModule (VAR modulename : ARRAY OF CHAR;
                             type       : FileType;
                         VAR filename   : ARRAY OF CHAR);
  VAR 
    token    : ARRAY [0 .. 132] OF CHAR;
    fileDef  : MakeForest.FileDefinitionList;
    success,
    eof      : BOOLEAN;

  BEGIN
    ModulenameToFilename (modulename, type, filename);

    IF MakeForest.AddFile (filename, fileDef) THEN
      MakeToken.OpenFile (filename, success);

      IF NOT success THEN
        IF MakeLibraries.IsALibraryModule (modulename) THEN
          fileDef^.library := TRUE;
        ELSE
          MakeForest.AddImport (fileDef, "## SOURCE FILE NOT FOUND ##");
        END; (* IF *)
      ELSE
        MakeToken.NextToken (token, eof);
        IF    (type = def) AND NOT
              (Strings.Compare (token, 'DEFINITION') = Strings.Equal) THEN
          MakeForest.AddImport (fileDef, 
                                "## Expected a 'DEFINITION' module ##");
        ELSIF (type = mod) AND NOT
              (Strings.Compare (token, 'IMPLEMENTATION') = Strings.Equal) THEN
          MakeForest.AddImport (fileDef, 
                                "## Expected an 'IMPLEMENTATION' module ##");
        ELSIF (type = main) AND NOT
              (Strings.Compare (token, 'MODULE') = Strings.Equal) THEN
          MakeForest.AddImport (fileDef, 
                                "## Expected a 'MODULE' module ##");
        END; (* IF / ELSE / ENDIF *)
        MakeToken.SkipTillSemicolon (eof);
        token := ';';
        WHILE (NOT eof) AND (NOT TerminatingToken(token)) DO
          IF Strings.Compare (token, 'FROM') = Strings.Equal THEN
            ParseFROM (fileDef, eof);
          ELSIF Strings.Compare (token, 'IMPORT') = Strings.Equal THEN
            ParseIMPORT (fileDef, eof);
          END; (* IF *)
          MakeToken.NextToken (token, eof);
        END; (* WHILE *)
      END; (* IF file opened *)
      MakeToken.CloseFile;
    END; (* IF we need to check this one *)

  END ParseModule;

END MakeParse.
