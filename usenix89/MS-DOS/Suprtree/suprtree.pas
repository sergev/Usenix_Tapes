{$P1024}{$U+}
Program SuprTree;

  {Analyzes directory tree for space allocation.}

  {Copyright 1984, Halff Resources, Inc.}

  const
    MaxDepth = 100;                  {Maximum depth of directory tree}
    DefaultDepth = 6;                {Initial maximum depth of listing}
    debug = false;                   {Set to true to swamp yourself with output}
    NoName = '            ';         {Empty file name array}

    {Attribute bits in directory}
    ReadOnly = $1;
    Hidden = $2;
    System = $4;
    VolLabel = $8;
    SubDirectory = $10;
    Archive = $20;

  type
    LongIndex = array [0..1] of integer;     {16 bit address}
    AsciizArray = array [1..13] of char;     {ASCIIZ file name}
    FileNameString = array [1..12] of char;  {Printable file name}
    String80 = string[80];

    {The directory is stored in core as an n-ary tree reflecting the
     directory structure on the disk.  DirNode is the structure that
     holds the nodes of this tree.  The first six fields of a DirNode
     hold data on a directory entry.  The branches field is a binary tree
     that, in the case of directories, holds the sorted entries of
     the directory.}

    BinTreePtr = ^BinTree;

    DirNode = record
                name: FileNameString;
                size: real;
                Nsubdir, Nhidden, Nsystem, Nreadonly: boolean;
                branches: BinTreePtr
              end;

    BinTree = record
                node: DirNode;
                left: BinTreePtr;
                right: BinTreePtr
              end;

    {regpack is TURBO structure for machine register handling}
    regpack      = record
                     ax,bx,cx,dx,bp,si,di,ds,es,flags: integer;
                   end;

    {FileRecord is the structure returned by DOS functions 4E and 4F, which
     find directory entries}

    FileRecord   = record
                     reserved: array [1..21] of byte;
                     attribute: byte;
                     time: integer;
                     date: integer;
                     FileSize: LongIndex;
                     name: AsciizArray
                   end;

    FileRecPtr = ^FileRecord;

  var
    root: DirNode;                                    {Root directory}
    IPath: String80;                                  {Saves initial path}
    ClusterSize,                                      {size of DOS cluster}
    DirCnt,                                           {number of subdirectories}
    FileCnt: integer;                                 {number of files}
    Stack: array [1..MaxDepth] of real;               {ancestor sizes}
    ShowFiles, SeparateDirs, SortBySize: boolean;     {user options}
    depth: integer;                                   {print depth}
    previous: boolean;                                {used for formatting}
    DiskSpace, FreeSpace: real;                       {disk parameters}

  {$U-}
  function FindSize(size: LongIndex): real;
  {Converts file size to space on disk}

    var s, t: real;

    begin {FindSize}
      if size[0] < 0 then s := 32768.0
                     else s := 0;
      size[0] := size[0] and $7FFF;
      if (size[0] mod ClusterSize) <> 0
        then t := succ(size[0] div ClusterSize)
        else t := size[0] div ClusterSize;
      FindSize := s + ClusterSize*t + size[1]*65536.0
    end; {FindSize}


  procedure AsciizToString(asciiz: AsciizArray; var target: fileNamestring);
  {Converts ASCIIZ to Array of Char}

    var
      i,j: integer;

    begin {AsciizToString}
      target := NoName;
      i := 1;
      while not (asciiz[i] in [chr(0), '.']) do
        begin
          target[i] := asciiz[i];
          i := succ(i)
        end;
      if asciiz[i] = '.' then
        begin
          i := succ(i);
          j := 10;
          while asciiz[i] <> chr(0) do
            begin
              target[j] := asciiz[i];
              i := succ(i);
              j := succ(j)
            end
        end
    end; {AsciizToString}

  procedure FindDTA(var DTA: FileRecPtr);
  {Finds Disk Transfer Area}

    var
      R: regpack;

    begin {FindDTA}
      R.ax := $2F shl 8;
      MsDos(R);
      DTA := ptr(r.es,r.bx)
    end; {FindDTA}

  procedure SetDTA(DTA: FileRecPtr);
  {Sets Disk Transfer Area}

    var
      R: regpack;

    begin {FindDTA}
      R.ax := $1A shl 8;
      R.ds := seg(DTA^);
      R.dx := ofs(DTA^);
      MsDos(R);
    end; {FindDTA}

procedure WriteAsciiz(asciiz: AsciizArray);
  {Prints an ASCIIZ string}

    var i: integer;

    begin {WriteAsciiz}
      i := 1;
      While asciiz[i] <> chr (0) do
        begin
          write(asciiz[i]);
          i := succ(i)
        end
    end; {WriteAsciiz}

  function GetFirstEntry(atr: byte; var FilePoop: FileRecord): boolean;
  {Gets first entry in directory; returns true iff successful}

     var mask: array [1..4] of char;
         R: regpack;
         temp: FileRecPtr;

     begin {GetFirstEntry}
       mask := '*.* ';
       mask[4] := chr(0);
       FindDTA(temp);
       SetDTA(Addr(FilePoop));
       R.ds := seg(mask);
       R.dx := ofs(mask);
       R.cx := atr;
       R.ax := $4E shl 8;   { get first entry in dir }
       msdos(R);
       if debug then
         begin
           Write('First Entry: ');
           WriteAsciiz(FilePoop.name);
           Writeln
         end;
       GetFirstEntry := not (R.ax in [2, 18]);
       SetDTA(temp)
     end; {GetFirstEntry}

   function GetNextEntry(var FilePoop: FileRecord): boolean;
   {Gets next entry in directory; returns true if successful}

     var R: regpack;
         temp: FileRecPtr;

     begin {GetNextEntry}
       FindDTA(temp);
       SetDTA(Addr(FilePoop));
       R.ax := $4F shl 8;
       msdos(R);
       GetNextEntry := R.ax <> 18;
       SetDTA(temp)
     end; {GetNextEntry}

 {$U+}
  procedure init;
  {Sets user options}

    var path: String80;
        prefix: string[1];
        param: string80;
        i: integer;

    procedure help;
    {displays help screen}

      begin {help}
        writeln;
        writeln('SUPRTREE Disk Space Allocation Analysis Program');
        writeln('     Copyright 1984, Halff Resources, Inc.');
        writeln;
        writeln('SUPRTREE Options:');
        writeln;
        writeln('SUPRTREE [d:[path]] [/S] [/F[M]] [/D[n]] [/H]');
        writeln('                                                      Default');
        writeln('     d:path   -- start tree at d(rive), path          current drive:path');
        writeln('     /S       -- sort by size (descending)            alphabetically');
        writeln('     /F       -- list files & directories separately  directories only');
        writeln('     /FM      -- list files & directories merged');
        writeln('     /Dn      -- list tree to depth n (0-100)         n=6');
        writeln('     /D       -- list tree to depth 100');
        writeln('     /H       -- display this help                    output tree');
        writeln;
        writeln('Output may be redirected to printer (>PRN:) or disk (>filename).');
        halt
      end; {help}

    Procedure ProcessParam;
    {processes 1 user option}

      var
        code, j: integer;

      begin
        param := prefix + param;
        prefix := '';
        for j := 1 to length(Param) do Param[j] := UpCase(Param[j]);
        if param = '/H' then help;
        if param = '/S' then SortBySize := True;
        if param = '/F' then ShowFiles := True;
        if param = '/FM' then
          begin
            ShowFiles := True;
            SeparateDirs := False;
          end;
        if copy (param, 1, 2) = '/D'then
          if param = '/D'
            then depth := MaxDepth
              else
                begin
                  Val (Copy(param, 3, length(param) - 2), depth, code);
                  if (code <> 0) or (not (depth in [1..MaxDepth]))
                     then depth := DefaultDepth
                end;
        if not (param[1] in ['/', '<', '>', '|']) then path := param
      end; {ProcessParam}

    Procedure GetFreeSpace;
    {gets disk parameters}

      var R: regpack;

      begin {GetFreeSpace}
        R.ax := $36 shl 8;
        R.dx := 0;
        msdos(R);
        ClusterSize := R.cx*R.ax;
        FreeSpace := R.bx;
        FreeSpace := FreeSpace * ClusterSize;
        DiskSpace := R.dx;
        DiskSpace := DiskSpace*ClusterSize
      End; {GetFreeSpace}

    begin {init}

      SortBySize := false;
      ShowFiles := false;
      depth := DefaultDepth;
      SeparateDirs := True;
      path := '';
      prefix := '';
      for i := 1 to ParamCount do
        begin
          Param := ParamStr(i);
          if (length(param) = 1) and (param[1] in ['/', '<', '>', '|'])
            then prefix := param
            else ProcessParam
        end;

      GetDir(0, IPath);
      if path <> '' then
        begin
          {$I-} chdir(path); {$I+}
          if IOResult <> 0 then
            begin
              Writeln (trm, path, ' is not a valid path.');
              halt
            end
        end;
      GetFreeSpace
    end; {init}

{$U-}
  procedure BuildTree(Var Dir: DirNode; depth: integer);
  {constructs tree representing disk directory}

    var EntryThere: boolean;
        Entry: integer;
        EntryPoop: FileRecord;
        NewNode: DirNode;

     procedure ProcessEntry;
     {fills in node for a single entry}

       begin {ProcessEntry}
         with NewNode do
           begin
             AsciizToString(EntryPoop.name, name);
             branches := nil;
             Nsubdir := (EntryPoop.attribute and SubDirectory) > 0;
             Nhidden := (EntryPoop.attribute and hidden) > 0;
             Nsystem := (EntryPoop.attribute and system) > 0;
             NReadOnly := (EntryPoop.attribute and ReadOnly) > 0;
             if debug then Write(name) else write (trm, '.');
             if Nsubdir
               then
                 begin
                   DirCnt := succ(DirCnt);
                   if debug then Writeln(' is a subdirectory.');
                   size := ClusterSize;
                   BuildTree(NewNode, pred(depth))
                 end
               else
                 begin
                   FileCnt := succ(FileCnt);
                   size := FindSize(EntryPoop.FileSize);
                   if debug then writeln(' is a file; size = ', size:7:0)
                 end;
             dir.size := dir.size + size
           end;
         Entry := succ(Entry)
       end; {ProcessEntry}

    Procedure InsertEntry(var br: bintreeptr);
    {inserts entry in binary tree}

      function before(OldNode: DirNode): boolean;
      {returns true if NewNode should be listed before OldNode}

        begin {before}
          if SeparateDirs and (NewNode.Nsubdir <> OldNode.Nsubdir)
              then before := NewNode.Nsubdir < OldNode.nSubdir
              else
                if SortBySize and (NewNode.size <> OldNode.size)
                  then before := NewNode.size > OldNode.size
                  else before := NewNode.name < OldNode.name
        end; {before}

      begin {InsertEntry}
        if br = nil
          then
            begin
              if debug then writeln('Inserting ', NewNode.name, ' depth ', depth);
              new(br);
              br^.node := NewNode;
              br^.left := nil;
              br^.right := nil
            end
          else
            if Before(br^.node)
              then InsertEntry(br^.left)
              else InsertEntry(br^.right)
       end; {InsertEntry}

    procedure NewDir(name: FileNameString);
    {changes directory}

      var s: string[12];
          i: integer;

      begin {NewDir}
        s := '';
        i := 1;
        while name[i] <> ' ' do
          begin
            s := s + name[i];
            i := succ(i)
          end;
        if name[10] <> ' ' then
          begin
            s := s + '.';
            for i := 10 to 12 do
              if name[i] <> ' ' then s := s + name[i]
          end;
        chdir(s)
      end; {NewDir}

    begin {BuildTree}
      if debug then Writeln('Starting on ', dir.name, ' at depth ', depth);
      if dir.name <> NoName then NewDir(dir.name);
      EntryThere := GetFirstEntry(hidden+system+SubDirectory, EntryPoop);
      Entry := 0;
      while EntryThere do
        begin
          if EntryPoop.name[1] <> '.' then
            begin
              ProcessEntry;
              if depth > 0
                then if NewNode.Nsubdir or ShowFiles
                  then InsertEntry(dir.branches)
            end;
          EntryThere := GetNextEntry(EntryPoop)
        end;
      if debug then Writeln('Ending ', dir.name, '; size = ', dir.size:7:0,
                            '; entries = ', Entry);
      if dir.name <> NoName then chdir('..');
    end; {BuildTree}

{U+}
  procedure GetVolName(var name: FileNameString);
  {Gets volume label}

    var poop: FileRecord;
        test: boolean;

    begin {GetVolName}
      chdir('\');
      test := GetFirstEntry(VolLabel, poop);
      while test and (poop.attribute <> VolLabel) do test := GetNextEntry(poop);
      if poop.attribute = VolLabel then
        begin
          AsciizToString(poop.name, name);
          move(name[10], name[9], 3);
          name[12] := ' '
        end
    end; {GetVolName}

  procedure PrintTree(dir: DirNode; StackPos: integer);
  {prints directory tree}

    var i: integer;

    Procedure PrintBinTree(tree: BinTree);
    {prints entries in subdirectory}

      begin {PrintBinTree}
        with tree do
          begin
            if left <> nil then PrintBinTree(left^);
            PrintTree(node, succ(stackpos));
            if right <> nil then PrintBinTree(right^)
          end
      end; {PrintBinTree}

    begin {PrintTree}
      with dir do
        begin
          if previous and Nsubdir then writeln;
          previous := not Nsubdir;
          for i := 1 to stackpos do
            write(100*size/stack[i]:13:2);
          write(' ',name, size:7:0, ' ');
          If NReadOnly then write('R');
          if Nsystem then write('S');
          if Nhidden then write('H');
          writeln;
          if Nsubdir then
            begin
              for i := 1 to stackpos do write(' ', NoName);
              writeln(' ------------')
            end;
          if branches<> nil
            then
              begin
                stack[succ(stackpos)] := size;
                previous := false;
                PrintBinTree(branches^);
              end;
          if Nsubdir and ShowFiles then writeln;
        end
    end; {PrintTree}

  begin {SuprTree}
    rewrite(output);
    init;
    DirCnt := 0; FileCnt := 0;
    with root do
      begin
        size := 0;
        name := NoName;
        Nsubdir := true;
        branches := nil
      end;
    BuildTree (Root, depth);

    previous := false;
    Writeln;
    GetVolName(root.name);
    chdir(IPath);
    PrintTree (Root, 0);

    Writeln(Dircnt, ' directories.');
    Writeln(FileCnt, ' files.');
    Writeln(root.size:7:0, ' bytes used.');
    Writeln(FreeSpace:7:0, ' bytes left');
    Writeln(DiskSpace:7:0, ' bytes on disk')
  end. {SuprTree}
-- 
Henry M. Halff                                       Halff Resources, Inc.
halff@utah-cs.ARPA                 4918 33rd Road, N., Arlington, VA 22207
