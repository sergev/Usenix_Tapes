
--------------------------------------------------------------------------------
--                                                                            --
--                   "DHRYSTONE" Benchmark Program                            --
--                   -----------------------------                            --
--                                                                            --
--  Version:   ADA / 1 (Reference version, all parts in one file)             --
--                                                                            --
--  File:      dhry-ref-all.a                                                 --
--                                                                            --
--  Date:      Oct. 1984                                                      --
--                                                                            --
--  Author:    Reinhold P. Weicker                                            --
--                                                                            --
--------------------------------------------------------------------------------
--                                                                            --
--  The following program contains statements of a high level programming     --
--  language (here: Ada) in a distribution considered representative:         --
--                                                                            --
--    assignments                  53 %                                       --
--    control statements           32 %                                       --
--    procedure, function calls    15 %                                       --
--                                                                            --
--  100 statements are dynamically executed. The program is balanced with     --
--  respect to the three aspects:                                             --
--                                                                            --
--    - statement type                                                        --
--    - operand type (for simple data types)                                  --
--    - operand access                                                        --
--         operand global, local, parameter, or constant.                     --
--                                                                            --
--  The combination of these three aspects is balanced only approximately.    --
--                                                                            --
--  1. Statement Type:                                                        --
--  -----------------             number (= percentage)                       --
--                                                                            --
--     V1 := V2                   10                                          --
--     V := Constant              12                                          --
--       (incl. V1 := F(..)                                                   --
--     Assignment,                 7                                          --
--       with array element                                                   --
--     Assignment,                 6                                          --
--       with record component                                                --
--                                --                                          --
--                                35       35                                 --
--                                                                            --
--     X := Y +|-|and|or Z         5                                          --
--     X := Y +|-|"=" Constant     6                                          --
--     X := X +|- 1                3                                          --
--     X := Y *|/ Z                2                                          --
--     X := Expression,            1                                          --
--          two operators                                                     --
--     X := Expression,            1                                          --
--          three operators                                                   --
--                                --                                          --
--                                18       18                                 --
--                                                                            --
--     if .... then ....          14                                          --
--       with "else"      7                                                   --
--       without "else"   7                                                   --
--           executed        3                                                --
--           not executed    4                                                --
--     for  ...  loop              6  |  counted every time                   --
--     while ... loop              4  |  the loop condition                   --
--     loop .... exit              1  |  is evaluated                         --
--     case ... end case           1                                          --
--     return                      5                                          --
--     rename                      1                                          --
--                                --                                          --
--                                32       32                                 --
--                                                                            --
--     P (...)  proced. call      10                                          --
--       same package        5                                                --
--       other package       5                                                --
--                                                                            --
--     X := F (...)                                                           --
--          function  call         5                                          --
--       same package        2                                                --
--       other package       3                                                --
--                                --                                          --
--                                15       15                                 --
--                                        ---                                 --
--                                        100                                 --
--                                                                            --
--    22 of the 58 assignments have a variable of a constrained               --
--    (sub-)type as their destination. In general, discriminant checks        --
--    will be necessary in these cases; however, the compiler may             --
--    optimize out some of these checks.                                      --
--                                                                            --
--    The average number of parameters in procedure or function calls         --
--    is 1.80 (not counting the function values as implicit parameters).      --
--                                                                            --
--                                                                            --
--  2. Operators                                                              --
--  ------------                                                              --
--                          number    approximate                             --
--                                    percentage                              --
--                                                                            --
--    Arithmetic             27          52.9                                 --
--                                                                            --
--       +                     16          31.4                               --
--       -                      7          13.7                               --
--       *                      3           5.9                               --
--       /                      1           2.0                               --
--                                                                            --
--    Comparison             20           39.2                                --
--                                                                            --
--       =                      9           17.6                              --
--       /=                     4            7.8                              --
--       >                      1            2.0                              --
--       <                      3            5.9                              --
--       >=                     1            2.0                              --
--       <=                     2            3.9                              --
--                                                                            --
--    Logic                   4            7.8                                --
--                                                                            --
--       AND                    1            2.0                              --
--       OR                     1            2.0                              --
--       NOT                    2            3.9                              --
--                                                                            --
--                           --          -----                                --
--                           51           99.9                                --
--                                                                            --
--                                                                            --
--  3. Operand Type (counted once per operand reference):                     --
--  ---------------                                                           --
--                          number    approximate                             --
--                                    percentage                              --
--                                                                            --
--     Integer               131        54.4 %                                --
--     Character              47        19.5 %                                --
--     Enumeration            31        12.4 %                                --
--     Boolean                11         5.0 %                                --
--     Pointer                12         4.6 %                                --
--     String30                6         2.5 %                                --
--     Array                   2         0.8 %                                --
--     Record                  2         0.8 %                                --
--                           ---       -------                                --
--                           241       100.0 %                                --
--                                                                            --
--  When there is an access path leading to the final operand (e.g. a record  --
--  component), only the final data type on the access path is counted.       --
--                                                                            --
--  There are 16 accesses to components of a record, 9 of them go to          --
--  a component in a variant part. For some of these accesses, the            --
--  compiler may suppress generation of code checking the tag field           --
--  during optimization.                                                      --
--                                                                            --
--                                                                            --
--  3. Operand Locality:                                                      --
--  -------------------                                                       --
--                                                                            --
--     local variable              117        48.5 %                          --
--     global variable              19         7.9 %                          --
--        same package                 18         11.2 %                      --
--        other package                 1          0.4 %                      --
--     parameter                    45        18.7 %                          --
--        in                           27         11.2 %                      --
--        inout                        12          5.0 %                      --
--        out                           6          2.5 %                      --
--     function result               5         2.1 %                          --
--     constant                     55        22.8 %                          --
--                                 ---       -------                          --
--                                 243       100.0 %                          --
--                                                                            --
--                                                                            --
--  There may be cases where a highly optimizing compiler may recognize       --
--  unnecessary statements and may not generate code for them.                --
--                                                                            --
--  There has been no explicit effort to account for the effects of a         --
--  cache, or to balance the use of long or short displacements for code or   --
--  data.                                                                     --
--                                                                            --
--  The program does not compute anything meaningful, but it is syntactically --
--  and semantically correct. All variables have a value assigned to them     --
--  before they are used as a source operand.                                 --
--                                                                            --
--------------------------------------------------------------------------------
--                                                                            --
--  The "Dhrystone" program consists of six library units in the sense of     --
--  Ada: Two packages (Pack_1 and Pack_2) consisting of both specification    --
--  and body, one package (Global_Def) with specification only, and one       --
--  procedure (Main). The package structure is intended since this is the     --
--  natural ways for programming in Ada, and there can be systems where       --
--  calls and data accesses across package boundaries have execution          --
--  times different from those within a package.                              --
--  In addition, the execution time may depend upon the way the program       --
--  is compiled and executed; there are two possible ways:                    --
--  (1) the six units are compiled separately and linked together later,      --
--  (2) the six units are submitted to the compiler in one source file.       --
--                                                                            --
--  For many Ada systems there will be no difference in execution time,       --
--  as far as the two compilation models are concerned.                       --
--  However, for some machines there may be a difference since different      --
--  representations may be used for addresses (16 bit or 32 bit), depending   --
--  on the compilation model used.                                            --
--                                                                            --
--  If there is a difference in execution time, the times for both models     --
--  should be measured; model (1) is the one that corresponds more to actual  --
--  software development. Ada was explicitly designed for large software      --
--  projects that use separate compilation.                                   --
--                                                                            --
--  This file contains the "reference" version of Dhrystone/Ada (i.e. no      --
--  statement for measurement are included), with the whole program in        --
--  one file. For a version with separate source files for all library        --
--  units, see the files dhry-ref-pt1.ada, ..., dhry-ref-pt6.ada.             --
--                                                                            --
--------------------------------------------------------------------------------

package Global_Def is
------------------

  -- Global type definitions

  type Enumeration is (Ident_1, Ident_2, Ident_3, Ident_4, Ident_5);

  subtype One_To_Thirty is integer range 1..30;
  subtype One_To_Fifty is integer range 1..50;
  subtype Capital_Letter is character range 'A'..'Z';

  type String_30 is array (One_To_Thirty) of character;
    pragma Pack (String_30);

  type Array_1_Dim_Integer is array (One_To_Fifty) of integer;
  type Array_2_Dim_Integer is array (One_To_Fifty,
                                     One_To_Fifty) of integer;

  type Record_Type (Discr: Enumeration := Ident_1);

  type Record_Pointer is access Record_Type;

  type Record_Type (Discr: Enumeration := Ident_1) is
               record
                Pointer_Comp:   Record_Pointer;
                case Discr is
                  when Ident_1 =>     -- only this variant is used,
                                      -- but in some cases discriminant
                                      -- checks are necessary
                    Enum_Comp:      Enumeration;
                    Int_Comp:       One_To_Fifty;
                    String_Comp:    String_30;
                  when Ident_2 =>
                    Enum_Comp_2:    Enumeration;
                    String_Comp_2:  String_30;
                  when others =>
                    Char_Comp_1,
                    Char_Comp_2:    character;
                end case;
              end record;

end Global_Def;


  with Global_Def;
  use Global_Def;

package Pack_1 is
--------------

  procedure Proc_0;
  procedure Proc_1 (Pointer_Par_In:     in     Record_Pointer);
  procedure Proc_2 (Int_Par_In_Out:     in out One_To_Fifty);
  procedure Proc_3 (Pointer_Par_Out:    out    Record_Pointer);

  Int_Glob:          integer;

end Pack_1;


  with Global_Def;
  use Global_Def;

package Pack_2 is
--------------

  procedure Proc_6 (Enum_Par_In:        in     Enumeration;
                    Enum_Par_Out:       out    Enumeration);
  procedure Proc_7 (Int_Par_In_1,
                    Int_Par_In_2:       in     One_To_Fifty;
                    Int_Par_Out:        out    One_To_Fifty);
  procedure Proc_8 (Array_Par_In_Out_1: in out Array_1_Dim_Integer;
                    Array_Par_In_Out_2: in out Array_2_Dim_Integer;
                    Int_Par_In_1,
                    Int_Par_In_2:       in     integer);
  function Func_1 (Char_Par_In_1,
                   Char_Par_In_2:       in     Capital_Letter)
                                                    return Enumeration;
  function Func_2 (String_Par_In_1,
                   String_Par_In_2:     in     String_30)
                                                    return boolean;

end Pack_2;


  with Global_Def, Pack_1;
  use Global_Def;

procedure Main is
--------------

begin

  Pack_1.Proc_0;   -- Proc_0 is actually the main program, but it is part
                   -- of a package, and a program within a package can
                   -- not be designated as the main program for execution.
                   -- Therefore Proc_0 is activated by a call from "Main".

end Main;


with Global_Def, Pack_2;
use Global_Def;

package body Pack_1 is
-------------------

  Bool_Glob:         boolean;
  Char_Glob_1,
  Char_Glob_2:       character;
  Array_Glob_1:      Array_1_Dim_Integer;
  Array_Glob_2:      Array_2_Dim_Integer;
  Pointer_Glob,
  Pointer_Glob_Next: Record_Pointer;

  procedure Proc_4;
  procedure Proc_5;

procedure Proc_0
is
  Int_Loc_1,
  Int_Loc_2,
  Int_Loc_3:     One_To_Fifty;
  Char_Loc:      character;
  Enum_Loc:      Enumeration;
  String_Loc_1,
  String_Loc_2:  String_30;
begin

  -- Initializations

  Pack_1.Pointer_Glob_Next := new Record_Type;

  Pack_1.Pointer_Glob := new Record_Type
                        '(
                          Pointer_Comp   => Pack_1.Pointer_Glob_Next,
                          Discr          => Ident_1,
                          Enum_Comp      => Ident_3,
                          Int_Comp       => 40,
                          String_Comp    => "DHRYSTONE PROGRAM, SOME STRING"
                         );

  String_Loc_1 := "DHRYSTONE PROGRAM, 1'ST STRING";

  -----------------
  -- Start timer --
  -----------------

  Proc_5;
  Proc_4;
    -- Char_Glob_1 = 'A', Char_Glob_2 = 'B', Bool_Glob = false
  Int_Loc_1 := 2;
  Int_Loc_2 := 3;
  String_Loc_2 := "DHRYSTONE PROGRAM, 2'ND STRING";
  Enum_Loc := Ident_2;
  Bool_Glob := not Pack_2.Func_2 (String_Loc_1, String_Loc_2);
    -- Bool_Glob = true
  while Int_Loc_1 < Int_Loc_2 loop  -- loop body executed once
    Int_Loc_3 := 5 * Int_Loc_1 - Int_Loc_2;
      -- Int_Loc_3 = 7
    Pack_2.Proc_7 (Int_Loc_1, Int_Loc_2, Int_Loc_3);
      -- Int_Loc_3 = 7
    Int_Loc_1 := Int_Loc_1 + 1;
  end loop;
      -- Int_Loc_1 = 3
  Pack_2.Proc_8 (Array_Glob_1, Array_Glob_2, Int_Loc_1, Int_Loc_3);
    -- Int_Glob = 5
  Proc_1 (Pointer_Glob);
  for Char_Index in 'A' .. Char_Glob_2 loop -- loop body executed twice
    if Enum_Loc = Pack_2.Func_1 (Char_Index, 'C')
    then -- not executed
      Pack_2.Proc_6 (Ident_1, Enum_Loc);
    end if;
  end loop;
    -- Enum_Loc = Ident_1
    -- Int_Loc_1 = 3, Int_Loc_2 = 3, Int_Loc_3 = 7
  Int_Loc_3 := Int_Loc_2 * Int_Loc_1;
  Int_Loc_2 := Int_Loc_3 / Int_Loc_1;
  Int_Loc_2 := 7 * (Int_Loc_3 - Int_Loc_2) - Int_Loc_1;
  Proc_2 (Int_Loc_1);

  ----------------
  -- Stop timer --
  ----------------

end Proc_0;

procedure Proc_1 (Pointer_Par_In: in Record_Pointer)
is  -- executed once
  Next_Record: Record_Type
    renames Pointer_Par_In.Pointer_Comp.all; -- = Pointer_Glob_Next.all
begin
  Next_Record := Pointer_Glob.all;
  Pointer_Par_In.Int_Comp := 5;
  Next_Record.Int_Comp := Pointer_Par_In.Int_Comp;
  Next_Record.Pointer_Comp := Pointer_Par_In.Pointer_Comp;
  Proc_3 (Next_Record.Pointer_Comp);
    -- Next_Record.Pointer_Comp = Pointer_Glob.Pointer_Comp = Pointer_Glob_Next
  if Next_Record.Discr = Ident_1
  then -- executed
    Next_Record.Int_Comp := 6;
    Pack_2.Proc_6 (Pointer_Par_In.Enum_Comp, Next_Record.Enum_Comp);
    Next_Record.Pointer_Comp := Pointer_Glob.Pointer_Comp;
    Pack_2.Proc_7 (Next_Record.Int_Comp, 10, Next_Record.Int_Comp);
  else -- not executed
    Pointer_Par_In.all := Next_Record;
  end if;
end Proc_1;

procedure Proc_2 (Int_Par_In_Out: in out One_To_Fifty)
is  -- executed once
    -- In_Par_In_Out = 3, becomes 7
  Int_Loc:  One_To_Fifty;
  Enum_Loc: Enumeration;
begin
  Int_Loc := Int_Par_In_Out + 10;
  loop -- executed once
    if Char_Glob_1 = 'A'
    then -- executed
      Int_Loc := Int_Loc - 1;
      Int_Par_In_Out := Int_Loc - Int_Glob;
      Enum_Loc := Ident_1;
    end if;
  exit when Enum_Loc = Ident_1; -- true
  end loop;
end Proc_2;

procedure Proc_3 (Pointer_Par_Out: out Record_Pointer)
is  -- executed once
    -- Pointer_Par_Out becomes Pointer_Glob
begin
  if Pointer_Glob /= null
  then -- executed
    Pointer_Par_Out := Pointer_Glob.Pointer_Comp;
  else -- not executed
    Int_Glob := 100;
  end if;
  Pack_2.Proc_7 (10, Int_Glob, Pointer_Glob.Int_Comp);
end Proc_3;

procedure Proc_4 -- without parameters
is  -- executed once
  Bool_Loc: boolean;
begin
  Bool_Loc := Char_Glob_1 = 'A';
  Bool_Loc := Bool_Loc or Bool_Glob;
  Char_Glob_2 := 'B';
end Proc_4;

procedure Proc_5 -- without parameters
is  -- executed once
begin
  Char_Glob_1 := 'A';
  Bool_Glob := false;
end Proc_5;

end Pack_1;


with Global_Def, Pack_1;
use Global_Def;

package body Pack_2 is
-------------------

function Func_3 (Enum_Par_In: in Enumeration) return boolean;
         -- forward declaration

procedure Proc_6 (Enum_Par_In:  in  Enumeration;
                  Enum_Par_Out: out Enumeration)
is  -- executed once
    -- Enum_Par_In = Ident_3, Enum_Par_Out becomes Ident_2
begin
  Enum_Par_Out := Enum_Par_In;
  if not Func_3 (Enum_Par_In)
  then -- not executed
    Enum_Par_Out := Ident_4;
  end if;
  case Enum_Par_In is
    when Ident_1 => Enum_Par_Out := Ident_1;
    when Ident_2 => if Pack_1.Int_Glob > 100
                      then Enum_Par_Out := Ident_1;
                      else Enum_Par_Out := Ident_4;
                    end if;
    when Ident_3 => Enum_Par_Out := Ident_2;    -- executed
    when Ident_4 => null;
    when Ident_5 => Enum_Par_Out := Ident_3;
  end case;
end Proc_6;

procedure Proc_7 (Int_Par_In_1,
                  Int_Par_In_2:   in  One_To_Fifty;
                  Int_Par_Out:    out One_To_Fifty)
is  -- executed three times
    -- first call:      Int_Par_In_1 = 2, Int_Par_In_2 = 3,
    --                  Int_Par_Out becomes 7
    -- second call:     Int_Par_In_1 = 6, Int_Par_In_2 = 10,
    --                  Int_Par_Out becomes 18
    -- third call:      Int_Par_In_1 = 10, Int_Par_In_2 = 5,
    --                  Int_Par_Out becomes 17
  Int_Loc: One_To_Fifty;
begin
  Int_Loc := Int_Par_In_1 + 2;
  Int_Par_Out := Int_Par_In_2 + Int_Loc;
end Proc_7;

procedure Proc_8 (Array_Par_In_Out_1: in out Array_1_Dim_Integer;
                  Array_Par_In_Out_2: in out Array_2_Dim_Integer;
                  Int_Par_In_1,
                  Int_Par_In_2:       in     integer)
is  -- executed once
    -- Int_Par_In_1 = 3
    -- Int_Par_In_2 = 7
  Int_Loc: One_To_Fifty;
begin
  Int_Loc := Int_Par_In_1 + 5;
  Array_Par_In_Out_1 (Int_Loc) := Int_Par_In_2;
  Array_Par_In_Out_1 (Int_Loc+1) :=
                          Array_Par_In_Out_1 (Int_Loc);
  Array_Par_In_Out_1 (Int_Loc+30) := Int_Loc;
  for Int_Index in Int_Loc .. Int_Loc+1 loop -- loop body executed twice
    Array_Par_In_Out_2 (Int_Loc, Int_Index) := Int_Loc;
  end loop;
  Array_Par_In_Out_2 (Int_Loc, Int_Loc-1) :=
                          Array_Par_In_Out_2 (Int_Loc, Int_Loc-1) + 1;
  Array_Par_In_Out_2 (Int_Loc+20, Int_Loc) :=
                          Array_Par_In_Out_1 (Int_Loc);
  Pack_1.Int_Glob := 5;
end Proc_8;

function Func_1 (Char_Par_In_1,
                 Char_Par_In_2: in Capital_Letter)
                                                return Enumeration
is  -- executed three times, returns Ident_1 each time
    -- first call:      Char_Par_In_1 = 'H', Char_Par_In_2 = 'R'
    -- second call:     Char_Par_In_1 = 'A', Char_Par_In_2 = 'C'
    -- third call:      Char_Par_In_1 = 'B', Char_Par_In_2 = 'C'
  Char_Loc_1, Char_Loc_2: Capital_Letter;
begin
  Char_Loc_1 := Char_Par_In_1;
  Char_Loc_2 := Char_Loc_1;
  if Char_Loc_2 /= Char_Par_In_2
  then  -- executed
    return Ident_1;
  else  -- not executed
    return Ident_2;
  end if;
  end Func_1;

function Func_2 (String_Par_In_1,
                 String_Par_In_2: in String_30) return boolean
is  -- executed once, returns false
    -- String_Par_In_1 = "DHRYSTONE PROGRAM, 1'ST STRING"
    -- String_Par_In_2 = "DHRYSTONE PROGRAM, 2'ND STRING"
  Int_Loc:  One_To_Thirty;
  Char_Loc: Capital_Letter;
begin
  Int_Loc := 2;
  while Int_Loc <= 2 loop -- loop body executed once
    if Func_1 (String_Par_In_1(Int_Loc),
               String_Par_In_2(Int_Loc+1)) = Ident_1
    then -- executed
      Char_Loc := 'A';
      Int_Loc := Int_Loc + 1;
    end if;
  end loop;
  if Char_Loc >= 'W' and Char_Loc < 'Z'
  then -- not executed
    Int_Loc := 7;
  end if;
  if Char_Loc = 'X'
  then -- not executed
    return true;
  else -- executed
    if String_Par_In_1 > String_Par_In_2
    then -- not executed
      Int_Loc := Int_Loc + 7;
      return true;
    else -- executed
      return false;
    end if;
  end if;
end Func_2;

function Func_3 (Enum_Par_In: in Enumeration) return boolean
is  -- executed once, returns true
    -- Enum_Par_In = Ident_3
  Enum_Loc:  Enumeration;
begin
  Enum_Loc := Enum_Par_In;
  if Enum_Loc = Ident_3
  then -- executed
    return true;
  end if;
end Func_3;

end Pack_2;
