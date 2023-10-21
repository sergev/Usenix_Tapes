{**************************************************************************
*   Maps system memory blocks for MS/PCDOS 2.0 and higher.                *
*   Copyright (c) 1986 Kim Kokkonen, TurboPower Software.                 *
*   Released to the public domain for personal, non-commercial use only.  *
***************************************************************************
*   written 1/2/86                                                        *
*   revised 1/10/86 for                                                   *
*     running under DOS 2.X, where block owner names are unknown          *
*   revised 1/22/86 for                                                   *
*     a bug in parsing the owner name of the block                        *
*     a quirk in the way that the DOS PRINT buffer installs itself        *
*     minor cosmetic changes                                              *
*   revised 2/6/86 for (version 1.3)                                      *
*     smarter filtering for processes that deallocate their environment   *
***************************************************************************
*   telephone: 408-378-3672, CompuServe: 72457,2131.                      *
*   requires Turbo version 3 to compile.                                  *
*   Compile with mAx dynamic memory = A000.                               *
*   limited to environment sizes of 255 bytes (default is 128 bytes)      *
***************************************************************************}

{$P128}

PROGRAM MapMem;
  {-look at the system memory map using DOS memory control blocks}
CONST
  MaxBlocks = 100;
  Version = '1.3';
TYPE
  Block = RECORD              {store info about each memory block as it is found}
            idbyte : Byte;
            mcb : Integer;
            psp : Integer;
            len : Integer;
            psplen : Integer;
            env : Integer;
            cnt : Integer;
          END;
  BlockType = 0..MaxBlocks;
  BlockArray = ARRAY[BlockType] OF Block;

VAR
  Blocks : BlockArray;
  BlockNum : BlockType;

  PROCEDURE FindTheBlocks;
    {-scan memory for the allocated memory blocks}
  CONST
    MidBlockID = $4D;         {byte DOS uses to identify part of MCB chain}
    EndBlockID = $5A;         {byte DOS uses to identify last block of MCB chain}
  VAR
    mcbSeg : Integer;         {potential segment address of an MCB}
    nextSeg : Integer;        {computed segment address for the next MCB}
    gotFirst : Boolean;       {true after first MCB is found}
    gotLast : Boolean;        {true after last MCB is found}
    idbyte : Byte;            {byte that DOS uses to identify an MCB}

    PROCEDURE StoreTheBlock(VAR mcbSeg, nextSeg : Integer;
                            VAR gotFirst, gotLast : Boolean);
      {-store information regarding the memory block}
    VAR
      nextID : Byte;
      pspAdd : Integer;       {segment address of the current PSP}
      mcbLen : Integer;       {size of the current memory block in paragraphs}
    BEGIN

      mcbLen := MemW[mcbSeg:3]; {size of the MCB in paragraphs}
      nextSeg := Succ(mcbSeg+mcbLen); {where the next MCB should be}
      pspAdd := MemW[mcbSeg:1]; {address of program segment prefix for MCB}
      nextID := Mem[nextSeg:0];

      IF gotLast OR (nextID = EndBlockID) OR (nextID = MidBlockID) THEN BEGIN
        BlockNum := Succ(BlockNum);
        gotFirst := True;
        WITH Blocks[BlockNum] DO BEGIN
          idbyte := Mem[mcbSeg:0];
          mcb := mcbSeg;
          psp := pspAdd;
          env := MemW[pspAdd:$2C];
          len := mcbLen;
          psplen := 0;
          cnt := 1;
        END;
      END;

    END {storetheblock} ;

  BEGIN
    {start above the Basic work area, could probably start even higher}
    {there must be a magic address to start from, but it is not documented}
    mcbSeg := $50;
    gotFirst := False;
    gotLast := False;
    BlockNum := 0;

    {scan all memory until the last block is found}
    REPEAT
      idbyte := Mem[mcbSeg:0];
      IF idbyte = MidBlockID THEN BEGIN
        StoreTheBlock(mcbSeg, nextSeg, gotFirst, gotLast);
        IF gotFirst THEN mcbSeg := nextSeg ELSE mcbSeg := Succ(mcbSeg);
      END ELSE IF gotFirst AND (idbyte = EndBlockID) THEN BEGIN
        gotLast := True;
        StoreTheBlock(mcbSeg, nextSeg, gotFirst, gotLast);
      END ELSE
        {still looking for first block, try every paragraph boundary}
        mcbSeg := Succ(mcbSeg);
    UNTIL gotLast;

  END {findtheblocks} ;


  PROCEDURE ShowTheBlocks;
    {-analyze and display the blocks found}
  CONST
    MaxVector = $40;          {highest interrupt vector checked for trapping}
  TYPE
    Pathname = STRING[64];
    HexString = STRING[4];
    Address = RECORD
                offset, segment : Integer;
              END;
    VectorType = 0..MaxVector;
  VAR
    st : Pathname;
    b : BlockType;
    dosV : Byte;
    Vectors : ARRAY[VectorType] OF Address ABSOLUTE 0 : 0;
    vTable : ARRAY[VectorType] OF Real;
    SumBlocks : BlockType;
    Sum : BlockArray;

    FUNCTION Hex(i : Integer) : HexString;
      {-return hex representation of integer}
    CONST
      hc : ARRAY[0..15] OF Char = '0123456789ABCDEF';
    VAR
      l, h : Byte;
    BEGIN
      l := Lo(i); h := Hi(i);
      Hex := hc[h SHR 4]+hc[h AND $F]+hc[l SHR 4]+hc[l AND $F];
    END {hex} ;

    FUNCTION DOSversion : Byte;
      {-return the major version number of DOS}
    VAR
      reg : RECORD
              CASE Byte OF
                1 : (ax, bx, cx, dx, bp, si, di, ds, es, flags : Integer);
                2 : (al, ah, bl, bh, cl, ch, dl, dh : Byte);
            END;
    BEGIN
      reg.ah := $30;
      MsDos(reg);
      DOSversion := reg.al;
    END {dosversion} ;

    FUNCTION Cardinal(i : Integer) : Real;
      {-return an unsigned integer 0..65535}
    BEGIN
      Cardinal := 256.0*Hi(i)+Lo(i);
    END {cardinal} ;

    FUNCTION Owner(startadd : Integer) : Pathname;
      {-return the name of the owner program of an MCB}
    VAR
      e : STRING[255];
      i : Integer;
      t : Pathname;

      PROCEDURE StripNonAscii(VAR t : Pathname);
        {-return an empty string if t contains any non-printable characters}
      VAR
        ipos : Byte;
        goodname : Boolean;
      BEGIN
        goodname := True;
        FOR ipos := 1 TO Length(t) DO
          IF (t[ipos] < ' ') OR (t[ipos] > '}') THEN
            goodname := False;
        IF NOT(goodname) THEN t := '';
      END {stripnonascii} ;

      PROCEDURE StripPathname(VAR pname : Pathname);
        {-remove leading drive or path name from the input}
      VAR
        spos, cpos, rpos : Byte;
      BEGIN
        spos := Pos('\', pname);
        cpos := Pos(':', pname);
        IF spos+cpos = 0 THEN Exit;
        IF spos <> 0 THEN BEGIN
          {find the last slash in the pathname}
          rpos := Length(pname);
          WHILE (rpos > 0) AND (pname[rpos] <> '\') DO rpos := Pred(rpos);
        END ELSE
          rpos := cpos;
        Delete(pname, 1, rpos);
      END {strippathname} ;

    BEGIN
      {get the environment string to scan}
      e[0] := #255;
      Move(Mem[startadd:0], e[1], 255);

      {find end of the standard environment}
      i := Pos(#0#0, e);
      IF i = 0 THEN BEGIN
        {something's wrong, exit gracefully}
        Owner := '';
        Exit;
      END;

      {end of environment found, get the program name that follows it}
      t := '';
      i := i+3;               {skip over #0#0#args}
      REPEAT
        t := t+Chr(Mem[startadd:i]);
        i := Succ(i);
      UNTIL (Length(t) > 64) OR (Mem[startadd:i] = 0);

      StripNonAscii(t);
      IF Length(t) = 0 THEN
        Owner := 'N/A'
      ELSE BEGIN
        StripPathname(t);
        IF t = '' THEN t := 'N/A';
        Owner := t;
      END;

    END {owner} ;

    PROCEDURE InitVectorTable;
      {-build real equivalent of vector addresses}
    VAR
      v : VectorType;

      FUNCTION RealAdd(a : Address) : Real;
        {-return the real equivalent of an address (pointer)}
      BEGIN
        WITH a DO
          RealAdd := 16.0*Cardinal(segment)+Cardinal(offset);
      END {realadd} ;

    BEGIN
      FOR v := 0 TO MaxVector DO
        vTable[v] := RealAdd(Vectors[v]);
    END {initvectortable} ;

    PROCEDURE WriteHooks(start, stop : Integer);
      {-show the trapped interrupt vectors}
    VAR
      v : VectorType;
      sadd, eadd : Real;
    BEGIN
      sadd := 16.0*Cardinal(start);
      eadd := 16.0*Cardinal(stop);
      FOR v := 0 TO MaxVector DO BEGIN
        IF (vTable[v] >= sadd) AND (vTable[v] <= eadd) THEN
          Write(Copy(Hex(v), 3, 2), ' ');
      END;
    END {writehooks} ;

    PROCEDURE SortByPSP(VAR Blocks : BlockArray; BlockNum : BlockType);
      {-sort in order of ascending PSP}
    VAR
      i, j : BlockType;
      temp : Block;
    BEGIN
      FOR i := 1 TO Pred(BlockNum) DO
        FOR j := BlockNum DOWNTO Succ(i) DO
          IF Cardinal(Blocks[j].psp) < Cardinal(Blocks[Pred(j)].psp) THEN BEGIN
            temp := Blocks[j];
            Blocks[j] := Blocks[Pred(j)];
            Blocks[Pred(j)] := temp;
          END;
    END {SortByPSP} ;

    PROCEDURE SumTheBlocks(VAR Blocks : BlockArray;
                           BlockNum : BlockType;
                           VAR Sum : BlockArray;
                           VAR SumBlocks : BlockType);
      {-combine the blocks with equivalent PSPs}
    VAR
      prevpsp : Integer;
      b : BlockType;
    BEGIN
      SumBlocks := 0;
      prevpsp := $FFFF;
      FOR b := 1 TO BlockNum DO BEGIN
        IF Blocks[b].psp <> prevpsp THEN BEGIN
          SumBlocks := Succ(SumBlocks);
          Sum[SumBlocks] := Blocks[b];
          prevpsp := Blocks[b].psp;
        END ELSE
          WITH Sum[SumBlocks] DO BEGIN
            cnt := Succ(cnt);
            len := len+Blocks[b].len;
          END;
        {get length of the block which owns the executable program}
        {for checking vector trapping next}
        IF Succ(Blocks[b].mcb) = Blocks[b].psp THEN
          Sum[SumBlocks].psplen := Blocks[b].len;
      END;
    END {sumblocks} ;

  BEGIN
    WriteLn;
    WriteLn('      Allocated Memory Map - by TurboPower Software - Version ', Version);
    WriteLn;
    WriteLn('PSP adr MCB adr  paras   bytes   owner        hooked vectors');
    WriteLn('------- ------- ------- ------- ----------   ------------------------------');

    dosV := DOSversion;
    InitVectorTable;
    SortByPSP(Blocks, BlockNum);
    SumTheBlocks(Blocks, BlockNum, Sum, SumBlocks);

    FOR b := 1 TO SumBlocks DO WITH Sum[b] DO BEGIN
      Write(' ',
      Hex(psp), '    ',       {PSP address}
      Hex(mcb), '    ',       {MCB address}
      Hex(len), '   ',        {size of block in paragraphs}
      16.0*Cardinal(len):6:0, '  '); {size of block in bytes}

      {get the program owning this block by scanning the environment}
      IF (dosV >= 3) AND (cnt > 1) THEN
        st := Owner(env)
      ELSE
        st := 'N/A';
      WHILE Length(st) < 13 DO st := st+' ';
      Write(st);
      WriteHooks(psp, psp+psplen);
      WriteLn;
    END;

  END {showtheblocks} ;

BEGIN
  FindTheBlocks;
  ShowTheBlocks;
END.
