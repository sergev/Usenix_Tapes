
{ EXECUTE.INC }

{ *************************************************************************** }
{ *                                                                         * }
{ *                TURBO SCREEN INPUT PRE-PROCESSOR TOOLKIT                 * }
{ *                                                                         * }
{ *                    EXECUTE SUBPROGRAM INCLUDE FILE                      * }
{ *                                                                         * }
{ *                             Version  1.07                               * }
{ *                                                                         * }
{ *                                                                         * }
{ *    This include file was written with routines supplied by Bela Lubkin. * }
{ *    He has asked that his routines be for non-commercial use only EXCEPT * }
{ *    with permission from him.                                            * }
{ *                                                                         * }
{ *    This include file contains routines that allow you to run other      * }
{ *    programs directly from Turbo Pascal, get the return codes when       * }
{ *    those programs terminate, and be able to temporarily exit your       * }
{ *    Turbo program to issue DOS commands.                                 * }
{ *                                                                         * }
{ *    All of these routines return to your next line of Pascal code once   * }
{ *    they have completed.                                                 * }
{ *                                                                         * }
{ *    You could, for example, use these routines within this include file  * }
{ *    to execute a child program, perhaps written in Fortran.  The user    * }
{ *    would be totally unaware that control had been passed to a different * }
{ *    program.  Once the child program terminates, control would then be   * }
{ *    passed back to the parent program, specifically to the next line of  * }
{ *    Pascal Code.  Thus one could write a Turbo Pascal shell program to   * }
{ *    execute other programs, thereby preventing the user from having to   * }
{ *    deal with DOS.  Please see the documentation file 'Tsipp.Doc' for    * }
{ *    more discussion on this interesting topic.                           * }
{ *                                                                         * }
{ *    VERY IMPORTANT NOTES                                                 * }
{ *                                                                         * }
{ *    The Exec calls (SubProcess, SubProcessViaCOMMAND, ShellToDOS,        * }
{ *    ExecuteChildProgram) will not work unless you restrict Turbo's       * }
{ *    allocated heap.  To do this, lower "mAximum dynamic free memory"     * }
{ *    on the compiler Options menu to a reasonable value.  What a          * }
{ *    reasonable value is depends on your program's use of the heap and    * }
{ *    the stack, and must be determined by you.  If you use neither the    * }
{ *    heap (pointers) nor recursion, as low as 0400h (16K bytes) is        * }
{ *    probably more than enough.                                           * }
{ *                                                                         * }
{ *    The Exec calls CANNOT be called from within the interactive Turbo    * }
{ *    compiler system.  They can only be called from .COM or .CHN files    * }
{ *    running outside of the Turbo environment.                            * }
{ *                                                                         * }
{ *    Revision history                                                     * }
{ *    ----------------                                                     * }
{ *    Version 1.5 1/14/86 fixes the memory freeing bug by removing         * }
{ *                support for Turbo 2.0.  String types changed to          * }
{ *                minimize chances of collision.  General environment      * }
{ *                support added.  Explicit calls for Exec-via-COMMAND.COM  * }
{ *                and Exec-to-DOS-prompt added.  Support for getting the   * }
{ *                subprocess return code added.  Major documentation       * }
{ *                overhaul.  NOW REQUIRES TURBO 3.0!  (Thanks to Stu       * }
{ *                Fuller 76703,501 for pointing out how easy it was to add * }
{ *                full environment support).                               * }
{ *    Version 1.4 attempts to fix a bug in the freeing of memory before    * }
{ *                the Exec call.                                           * }
{ *    Version 1.3 works with MS-DOS 2.0 and up, TURBO PASCAL version 1.0   * }
{ *                and up.                                                  * }
{ *    Version 1.2 had a subtle but dangerous bug: I set a variable that    * }
{ *                was addressed relative to BP, using a destroyed BP!      * }
{ *    Version 1.1 didn't work with Turbo 2.0 because I used Turbo 3.0      * }
{ *                features.                                                * }
{ *    Version 1.0 only worked with DOS 3.0 due to a subtle bug in DOS 2.x  * }
{ *                                                                         * }
{ *    -  Bela Lubkin                                                       * }
{ *       CompuServe 76703,3015                                             * }
{ *                                                                         * }
{ *************************************************************************** }

Type
  ExecuteStr66=String[66];
  ExecuteStr255=String[255];



Function SubProcess(    CommandLine:ExecuteStr255):Integer;

{ Calls an executable image (.COM or .EXE file) using MS-DOS function
  4Bh, Exec.  The parameter CommandLine must contain both the name of the
  program to run and the arguments to be passed to it, seperated by a
  space.  Path searching and other amenities are not performed; the passed
  in name must be specific enough to allow the file to be found, i.e.
  'CHKDSK' will NOT work.  At least 'CHKDSK.COM' must be specified, and a
  drive and path name will help even more.  For example,

        'C:\SYSTEM\CHKDSK.COM'
        'A:\WS.COM DOCUMENT.1'
        'C:\DOS\LINK.EXE TEST;'
        'D:\ASSEM\MASM.EXE PROG1 PROG1.OBJ NUL PROG1.MAP'
        'C:\COMMAND.COM /C COPY *.* B:\BACKUP >FILESCOP.IED'

  The last example uses COMMAND.COM to invoke a DOS internal command and
  to perform redirection.  Only with the use of COMMAND.COM can the
  following be done: redirection; piping; path searching; searching for
  the extension of a program (.COM, .EXE, or .BAT); batch files; and
  internal DOS commands.

  Because the COMMAND-assisted Exec function is so useful, a seperate
  function, SubProcessViaCOMMAND, is provided for that purpose.
  The integer return value of SubProcess is the error value returned by
  DOS on completion of the Exec call.  If it is nonzero, the call failed.
  Here is a list of likely error values:

         0: Success
         2: File/path not found
         3: Path not found
         4: Too many files open (no handles left)
         5: Access denied
         8: Not enough memory to load program
        10: Illegal environment (greater than 32K)
        11: Illegal .EXE file format
        32: Sharing violation
        33: Lock violation

  If you get any other result, consult an MS-DOS Technical Reference
  manual. }

Const
  STK_SEG: Integer=0;                  { structured constant to store stack segment }
  STK_PTR: Integer=0;                  { structured constant to store stack pointer }

Var
  Register:
    Record
      Case Integer Of { Variant Record Type }
        1 : (AX,BX,CX,DX,BP,SI,DI,DS,ES,Flags: Integer); { store 16 bit register values }
        2 : (AL,AH,BL,BH,CL,CH,DL,DH: Byte);             { store 8 bit register values }
      End; { Register Record }
  FileControlBlock1: Array [0..36] Of Byte;
  FileControlBlock2: Array [0..36] Of Byte;
  PathName: ExecuteStr66;
  CommandTail: ExecuteStr255;
  ParmTable:
    Record
      EnvSeg: Integer;
      ComLin: ^Integer;
      FileControlBlock1Ptr: ^Integer;
      FileControlBlock2Ptr: ^Integer;
    End; { ParmTable Record }
  RegisterFlags: Integer;

Begin   { SubProcess }
  If Pos(' ',CommandLine)=0 Then
    Begin
      PathName:=CommandLine+#0;
      CommandTail:=^M;
    End { If Pos }
  Else
    Begin
      PathName:=Copy(CommandLine,1,Pred(Pos(' ',CommandLine)))+#0;
      CommandTail:=Copy(CommandLine,Pos(' ',CommandLine),255)+^M;
    End; { Else }
  CommandTail[0]:=Pred(CommandTail[0]);
  With Register Do
    Begin
      FillChar(FileControlBlock1,Sizeof(FileControlBlock1),0);
      AX:=$2901;
      DS:=Seg(CommandTail[1]);
      SI:=Ofs(CommandTail[1]);
      ES:=Seg(FileControlBlock1);
      DI:=Ofs(FileControlBlock1);
      MsDos(Register); { Create FileControlBlock 1 }
      FillChar(FileControlBlock2,Sizeof(FileControlBlock2),0);
      AX:=$2901;
      ES:=Seg(FileControlBlock2);
      DI:=Ofs(FileControlBlock2);
      MsDos(Register); { Create FileControlBlock 2 }
      With ParmTable Do
        Begin
          EnvSeg:=MemW[CSeg:$002C];
          ComLin:=Addr(CommandTail);
          FileControlBlock1Ptr:=Addr(FileControlBlock1);
          FileControlBlock2Ptr:=Addr(FileControlBlock2);
        End; { With ParmTable }
      InLine($8D/$96/ PathName /$42/        { <DX>:=Ofs(PathName[1]); }
             $8D/$9E/ ParmTable /           { <BX>:=Ofs(ParmTable);   }
             $B8/$00/$4B/                   { <AX>:=$4B00;            }
             $1E/$55/                       { Save <DS>, <BP>         }
             $16/$1F/                       { <DS>:=Seg(PathName[1]); }
             $16/$07/                       { <ES>:=Seg(ParmTable);   }
             $2E/$8C/$16/ STK_SEG /         { Save <SS> in STK_SEG     }
             $2E/$89/$26/ STK_PTR /         { Save <SP> in STK_PTR     }
             $FA/                           { Disable interrupts      }
             $CD/$21/                       { Call MS-DOS             }
             $FA/                           { Disable interrupts      }
             $2E/$8B/$26/ STK_PTR /         { Restore <SP>            }
             $2E/$8E/$16/ STK_SEG /         { Restore <SS>            }
             $FB/                           { Enable interrupts       }
             $5D/$1F/                       { Restore <BP>,<DS>       }
             $9C/$8F/$86/ RegisterFlags /   { Flags:=<CPU flags>      }
             $89/$86/ Register );           { Register.AX:=<AX>;      }
      { The messing around with SS and SP is necessary because under DOS 2.x,
        after returning from an EXEC call, ALL registers are destroyed except
        CS and IP!  I wish I'd known that before I released this package the
        first time... }
      If (RegisterFlags And 1)<>0 Then
        SubProcess:=AX
      Else
        SubProcess:=0;
    End; { With Register }
End;    { SubProcess }



Function GetEnvStr(    SearchString: ExecuteStr255): ExecuteStr255;

{ Gets a string from the MS-DOS environment.  The parameter SearchString
  specifies the desired environment string.  The function result returns
  the value of that string from the environment.  If the string is not
  found, a null string is returned.  SearchString may have one special
  value, '='.  This returns garbage under MS-DOS 2.x.  Under MS-DOS 3.x,
  it returns the pathname under which the currently running program was
  invoked.  Examples:

        GetEnvStr('COMSPEC')   might = 'C:\COMMAND.COM'
        GetEnvStr('PROMPT')    might = '$p $g'
        GetEnvStr('REFLEX')    might = 'Herc'
        GetEnvStr('=')         might = 'C:\TURBO\exectest.COM'

  Only an exact match will succeed; case IS significant.  Do not include
  an equal sign in the search string (GetEnvStr('COMSPEC=') will fail).
  Note: if you are wondering why there is no SetEnvStr procedure, read
  an MS-DOS Technical Reference manual. }

Type
  Env=Array [0..32767] Of Char;

Var
  EPtr: ^Env;
  EStr: ExecuteStr255;
  Done: Boolean;
  I: Integer;

Begin   { GetEnvStr }
  GetEnvStr:='';
  If SearchString<>'' Then
    Begin
      EPtr:=Ptr(MemW[CSeg:$002C],0);
      I:=0;
      SearchString:=SearchString+'=';
      Done:=False;
      EStr:='';
      Repeat
        If EPtr^[I]=#0 Then
          Begin
            If EPtr^[Succ(I)]=#0 Then
              Begin
                Done:=True;
                If SearchString='==' Then
                  Begin
                    EStr:='';
                    I:=I+4;
                    While EPtr^[I]<>#0 Do
                      Begin
                        EStr:=EStr+EPtr^[I];
                        I:=Succ(I);
                      End; { While EPtr^ }
                    GetEnvStr:=EStr;
                  End; { If SearchString }
              End; { If EPtr^ }
            If Copy(EStr,1,Length(SearchString))=SearchString Then
              Begin
                GetEnvStr:=Copy(EStr,Succ(Length(SearchString)),255);
                Done:=True;
              End; { If Copy }
            EStr:='';
          End { If EPtr^ }
        Else
          EStr:=EStr+EPtr^[I];
        I:=Succ(I);
      Until Done;
    End; { If SearchString }
End;    { GetEnvStr }



Function GetComSpec: ExecuteStr66;

{ This is a special case of GetEnvStr and simply returns the COMSPEC
  environment string.  It is included for compatability with previous
  EXEC.PAS versions. }

Begin   { GetComSpec }
  GetComSpec:=GetEnvStr('COMSPEC');
End;    { GetComSpec }



Function SubProcessViaCOMMAND(    CommandLine: ExecuteStr255): Integer;

{ This is a special case of SubProcess.  The CommandLine is passed to
  COMMAND.COM, which does all further processing.  Command lines invoked
  via this procedure can do redirection and piping; undergo the normal DOS
  PATH search; may be batch files; and may be internal DOS commands such
  as COPY and RENAME.

  Disadvantages of this approach are: a copy of COMMAND.COM must be
  present (not always true on a floppy-based system); a slight time and
  memory penalty is involved due to the loading of an extra copy of
  COMMAND.COM (about 3K under DOS 3.1); the subprocess return code
  (Errorlevel) is lost.  In most cases the benefits will outweight the
  disadvantages.

  The integer return code is the same as for SubProcess. }

Begin   { SubProcessViaCOMMAND }
  SubProcessViaCOMMAND:=SubProcess(GetComSpec+' /C '+CommandLine);
End;    { SubProcessViaCOMMAND }



Function SubProcessReturnCode: Integer;

{ This function calls MS-DOS function 4Dh, Get Return Code of a
  Sub-process.  The integer return value is the return code set by the
  last subprocess you called.  Like Turbo's IOResult, SubProcessReturnCode
  is only valid once after a SubProcess call, reverting to 0 on successive
  calls.  The return code obtained after using SubProcessViaCOMMAND is the code
  returned by COMMAND.COM, not by any other program, and is not likely to be
  useful.

  Note: Turbo programs can set the return code by using the Halt
  procedure with a parameter, e.g. Halt(20);.  Other languages can call
  DOS function 4Ch (Terminate) with the return code in AL. }

Var
  Register:
    Record
      Case Integer Of { Variant Record Type }
        1 : (AX,BX,CX,DX,BP,SI,DI,DS,ES,Flags: Integer); { store 16 bit register values }
        2 : (AL,AH,BL,BH,CL,CH,DL,DH: Byte);             { store 8 bit register values }
      End; { Register Record }

Begin   { SubProcessReturnCode }
  Register.AH:=$4D;
  MsDos(Register);
  SubProcessReturnCode:=Register.AX;
End;    { SubProcessReturnCode }



Procedure ShellToDOS;

{ This procedure allows you to temporarily exit your Turbo program and execute
  DOS commands, and then re-enter your Turbo program where you left off
  simply by typing 'exit'. }

Var
  Command:ExecuteStr255;               { string used for issuing DOS commands }
  CurrentDirectory:ExecuteStr66;       { string used to store the current logged directory }
  ErrorCode:Integer;                   { variable used to determine error code from SubProcess function }
  Exit:Boolean;                        { flag used in determining when to re-enter the user's Turbo program }

Begin   { ShellToDOS }
  WriteLn('To re-enter "place program name here" type "exit"');
  Exit:=False;                         { initialize flag }
  Repeat
    Command:='';                       { initialize to prevent duplicate commands to be issued by single CR }
    GetDir(0,CurrentDirectory);        { get the current logged directory }
    Write(CurrentDirectory,'=->');     { display turbo DOS prompt }
    ReadLn(Command);                   { read user's DOS command }
    If (Command='EXIT') Or (Command='Exit') Or (Command='exit') Then
      Exit:=True                       { user has requested to return to Turbo program }
    Else
      If Command<>'' Then              { check for empty entry (CR) }
        ErrorCode:=SubProcessViaCOMMAND(Command);
  Until Exit;
End;    { ShellToDOS }



Procedure ExecuteChildProgram(    ProgramNameAndPassedParameters:ExecuteStr255);

{ This procedure allows you to call and run a child program directly from a
  parent Turbo Pascal program, passing all required parameters to the child
  program to be executed.  Control is then returned to the next line of Pascal
  code in the parent program once the child program has terminated. }

Var
  ErrorCode:Integer;                   { variable used to determine error code from SubProcess function }

Begin   { ExecuteChildProgram }
  ErrorCode:=SubProcess(ProgramNameAndPassedParameters);
End;    { ExecuteChildProgram }
