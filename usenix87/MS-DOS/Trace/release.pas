{**************************************************************************
*   Releases memory above the last MARK call made.                        *
*   Copyright (c) 1986 Kim Kokkonen, TurboPower Software.                 *
*   Released to the public domain for personal, non-commercial use only.  *
***************************************************************************
*   written 2/8/86                                                        *
***************************************************************************
*   telephone: 408-378-3672, CompuServe: 72457,2131.                      *
*   requires Turbo version 3 to compile.                                  *
*   Compile with mAx dynamic memory = A000.                               *
***************************************************************************}

{$P128}

PROGRAM ReleaseTSR;
  {-release system memory above the last mark call}
CONST
  MaxBlocks = 100;
  Version = '1.0';
  markID = 'MARK PARAMETER BLOCK FOLLOWS'; {marking string for TSR}
  markOffset = $103;          {offset into MARK.COM where markID is found in TSR}
  vectoroffset = $120;        {offset into MARK.COM where vector table is stored}
TYPE
  Block = RECORD              {store info about each memory block as it is found}
            mcb : Integer;
            psp : Integer;
          END;
  BlockType = 0..MaxBlocks;
  BlockArray = ARRAY[BlockType] OF Block;
  allstrings = STRING[255];

VAR
  Blocks : BlockArray;
  BottomBlock, BlockNum : BlockType;

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
          mcb := mcbSeg;
          psp := pspAdd;
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

  FUNCTION findmark(idstring : allstrings; idoffset : Integer) : Integer;
    {-find the last memory block matching idstring at offset idoffset}
  VAR
    b : BlockType;
    foundit : Boolean;

    FUNCTION MatchString(segment : Integer; idstring : allstrings; idoffset : Integer)
      : Boolean;
      {-return true if idstring is found at segment:idoffset}
    VAR
      tstring : allstrings;
      len : Byte;
    BEGIN
      len := Length(idstring);
      tstring[0] := Chr(len);
      Move(Mem[segment:idoffset], tstring[1], len);
      MatchString := (tstring = idstring);
    END {matchstring};

  BEGIN
    {scan from the last block-1 down to find the last MARK TSR}
    b := Pred(BlockNum);
    REPEAT
      foundit := MatchString(Blocks[b].psp, idstring, idoffset);
      IF NOT(foundit) THEN
        b := Pred(b);
    UNTIL (b < 1) OR foundit;
    IF NOT(foundit) THEN BEGIN
      WriteLn('No memory marker found. Mark memory by calling MARK.COM');
      Halt(1);
    END;
    findmark := b;
  END {findmark} ;

  PROCEDURE CopyVectors(BottomBlock : BlockType; vectoroffset : Integer);
    {-put interrupt vectors back into table}
  BEGIN
    {interrupts off}
    INLINE($FA);
    {replace vectors}
    Move(Mem[Blocks[BottomBlock].psp:vectoroffset], Mem[0:0], 1024);
    {interrupts on}
    INLINE($FB);
  END {copyvectors} ;

  PROCEDURE ReleaseMem(BottomBlock : BlockType);
    {release memory starting at block b, up to but not including this program}
  TYPE
    hexstring = STRING[4];
  VAR
    b : BlockType;
    regs : RECORD
             CASE Byte OF
               1 : (ax, bx, cx, dx, bp, si, di, ds, es, flags : Integer);
               2 : (al, ah, bl, bh, cl, ch, dl, dh : Byte);
           END;

    FUNCTION Hex(i : Integer) : hexstring;
      {-return hex representation of integer}
    CONST
      hc : ARRAY[0..15] OF Char = '0123456789ABCDEF';
    VAR
      l, h : Byte;
    BEGIN
      l := Lo(i); h := Hi(i);
      Hex := hc[h SHR 4]+hc[h AND $F]+hc[l SHR 4]+hc[l AND $F];
    END {hex} ;

  BEGIN
    WITH regs DO
      FOR b := BottomBlock TO BlockNum DO
        IF Blocks[b].psp <> CSeg THEN BEGIN
          ah := $49;
          {the block is always 1 paragraph above the MCB}
          es := Succ(Blocks[b].mcb);
          MsDos(regs);
          IF Odd(flags) THEN BEGIN
            WriteLn('Could not release block at segment ', Hex(es));
            WriteLn('Memory is now a mess... Please reboot');
            Halt(1);
          END;
      END;
  END {releasemem} ;

BEGIN
  WriteLn;
  {get all allocated memory blocks}
  FindTheBlocks;
  {find the last one marked with the MARK idstring}
  BottomBlock := findmark(markID, markOffset);
  {copy the vector table from the MARK resident}
  CopyVectors(BottomBlock, vectoroffset);
  {release memory at and above the mark resident}
  ReleaseMem(Pred(BottomBlock));
  {DOS will release this program's memory when it exits}
  {write success message}
  WriteLn('Memory released above last MARK');
END.
