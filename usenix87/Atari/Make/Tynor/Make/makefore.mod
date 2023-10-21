IMPLEMENTATION MODULE MakeForest;

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

IMPORT Strings;
FROM   Storage IMPORT ALLOCATE;

  (*----------------------------------------------------------------------*)
  PROCEDURE FindFile (VAR fname          : ARRAY OF CHAR; 
                      VAR fileDefinition : FileDefinitionList);
  VAR
    found : BOOLEAN;
  BEGIN
    fileDefinition := FileForest;
    found := FALSE;
    WHILE (fileDefinition <> NIL) AND (NOT found) DO
      found := Strings.Compare (fname, fileDefinition^.filename) 
                            = Strings.Equal;
      IF NOT found THEN
        fileDefinition := fileDefinition^.next;
      END; (* IF *)
    END; (* WHILE *)
  END FindFile;


  (*----------------------------------------------------------------------*)
  PROCEDURE AddFile (VAR fname          : ARRAY OF CHAR; 
                     VAR fileDefinition : FileDefinitionList) : BOOLEAN;
  BEGIN
    FindFile (fname, fileDefinition);
    IF fileDefinition = NIL THEN
      NEW (fileDefinition);
      Strings.Assign (fileDefinition^.filename, fname);
      fileDefinition^.next    := FileForest;
      fileDefinition^.imports := NIL;
      IF FileForest = NIL THEN
        FileForestEnd := fileDefinition;
      END; (* IF *)
      FileForest      := fileDefinition;
      RETURN TRUE;
    ELSE
      RETURN FALSE;
    END; (* IF *)
  END AddFile;


  (*----------------------------------------------------------------------*)
  PROCEDURE AddImport (VAR fileDefinition :  FileDefinitionList;
                       VAR fname          : ARRAY OF CHAR ); 
  VAR
    iPtr : ImportList;
  BEGIN
    NEW (iPtr);
    iPtr^.next              := fileDefinition^.imports;
    fileDefinition^.imports := iPtr;
    IF AddFile (fname, iPtr^.file) THEN END;
  END AddImport;


BEGIN
  FileForest := NIL;
  FileForestEnd := NIL;
END MakeForest.
