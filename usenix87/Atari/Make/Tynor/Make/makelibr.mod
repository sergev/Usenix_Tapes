IMPLEMENTATION MODULE MakeLibraries;

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

 (* NOTE: this module is implementation specific.   Substitute the library
  *       modules supplied by your compiler.
  *)
IMPORT Strings;

CONST
  NumLibraryModules = 41;

VAR
  LibraryModule : ARRAY [1 .. NumLibraryModules] OF
                      ARRAY [0 .. 20] OF CHAR;

  (*-----------------------------------------------------------------------*)
  PROCEDURE IsALibraryModule (VAR modulename : ARRAY OF CHAR) : BOOLEAN;
    (* binary search... *)
  VAR
    low, mid, high : CARDINAL;
  BEGIN
    low := 1;
    high := NumLibraryModules;
    WHILE low <= high DO
      mid := low + (high - low) DIV 2;
      CASE Strings.Compare (modulename, LibraryModule[mid]) OF
        Strings.Equal : 
          RETURN TRUE;
      | Strings.Less :
          high := mid - 1;
      ELSE
          low := mid + 1;
      END; (* CASE *)
    END; (* WHILE *)
    RETURN FALSE;
  END IsALibraryModule;

BEGIN
   (* NOTE: we're doing a binary search, so these must be in sorted order: *)

   (* TDI supplied libraries: *)
  LibraryModule [ 1] := 'AESApplications';
  LibraryModule [ 2] := 'AESEvents';
  LibraryModule [ 3] := 'AESForms';
  LibraryModule [ 4] := 'AESGraphics';
  LibraryModule [ 5] := 'AESMenus';
  LibraryModule [ 6] := 'AESObjects';
  LibraryModule [ 7] := 'AESResources';
  LibraryModule [ 8] := 'AESScraps';
  LibraryModule [ 9] := 'AESShells';
  LibraryModule [10] := 'AESWindows';
  LibraryModule [11] := 'ASCII';
  LibraryModule [12] := 'BIOS';
  LibraryModule [13] := 'Conversions';
  LibraryModule [14] := 'Display';
  LibraryModule [15] := 'FileSystem';
  LibraryModule [16] := 'GEMAESbase';
  LibraryModule [17] := 'GEMDOS';
  LibraryModule [18] := 'GEMError';
  LibraryModule [19] := 'GEMVDIbase';
  LibraryModule [20] := 'GEMX';
  LibraryModule [21] := 'InOut';
  LibraryModule [22] := 'Keyboard';
  LibraryModule [23] := 'LongInOut';
  LibraryModule [24] := 'M2Conversions';
  LibraryModule [25] := 'MathLib0';
  LibraryModule [26] := 'RealInOut';
    (* note : Strings.Compare thinks 'SY' < 'St'. (it just compares ASCII
     *        values...) So this is not exactly alphabetical... just sorted 
     *        as Strings.Compare would...
     *)
  LibraryModule [27] := 'SYSTEM';
  LibraryModule [28] := 'Storage';
  LibraryModule [29] := 'Streams';
  LibraryModule [30] := 'Strings';
  LibraryModule [31] := 'TermBase';
  LibraryModule [32] := 'Terminal';
  LibraryModule [33] := 'TextIO';
  LibraryModule [34] := 'VDIAttribs';
  LibraryModule [35] := 'VDIControls';
  LibraryModule [36] := 'VDIEscapes';
  LibraryModule [37] := 'VDIInputs';
  LibraryModule [38] := 'VDIInquires';
  LibraryModule [39] := 'VDIOutputs';
  LibraryModule [40] := 'VDIRasters';
  LibraryModule [41] := 'XBIOS';

END MakeLibraries.
