{ ifdef.pas [gh] 25 feb 87
+----------------------------------------------------------------------
| Abstract:
|    Very simple preprocessor that may be used to simulate something
|    like conditional compilation under Turbo Pascal.
|
| History:
|    1.0  25 feb 87 [gh]  Wrote it.
|
| Authorship:
|    [gh]  Gisle Hannemyr, Bragevn. 3a, 0452 Oslo, Norway;
|    (gisle@ifi.uio.no.arpa).
|
|    This hack is hereby placed in the public domain. You may use, copy
|    and distribute it in any way you like. Please remember to include
|    the manual page file (IFDEF.DOC) in any redistribution. If you
|    improve or extend it in any way,  I would appreiciate to receive a
|    copy of of your version.
|
| Environment:
|    Should work under any version of Borlands Turbo-Pascal, but I've
|    only tested it with Turbo version 3.01A (under generic MS-DOS,
|    PC-DOS, CP/M-80, CP/M-86 and C-DOS).
+---------------------------------------------------------------------}

{$V-}
program ifdef;

const
   VERSION = '1.0';

type
   genstring = string[255];
   pathname  = string[64];

   pattern = string[6];
   delpntr = ^delimit;
   delimit = record
      dbeg: pattern;
      dend: pattern;
      bcnt: integer;
      ecnt: integer;
      icnt: integer;
      deff: boolean;
      next: delpntr;
   end; { delimit }

var
   delimitlist, delimitlast: delpntr;
   lines: integer;
   doverbose, dokeep: boolean;


{---( Messages )------------------------------------------------------}

procedure abort(nn: integer);
begin
   write('ifdef: ');
   case nn of
      1: write('Symbols should be exactly 3 characters.');
      2: write('Not enough arguments.  Try "ifdef -?" for help.');
      3: write('Could not open file.');
   end; { case }
   halt;
end;


procedure presentation;
begin
   writeln('IFDEF    Ver. ',VERSION,'     --     A feeble pre-processor for Turbo-Pascal.');
   writeln('Another utility by Gisle Hannemyr, The Brage Software Workshop, 1987.');
   writeln;
end;


procedure dohelp;
begin
   presentation;
   writeln('Usage: ifdef [option ... ] filename');
   writeln;
   writeln('Options:');
   writeln('   -f filename : read definitions from file');
   writeln('   -d symbol   : "define"   symbol');
   writeln('   -u symbol   : "undefine" symbol');
   writeln('   -k          : keep backup copy');
   writeln('   -s          : run silent');
   halt;
end;


procedure writepattern(var ss: pattern);
var
  tt: string[3];
begin
   tt := copy(ss,3,3);
   write(tt);
end;


procedure matchup;
var
   nd: delpntr;
begin
   nd := delimitlist;
   while nd <> NIL do
   begin
      writepattern(nd^.dbeg); write(': ');
      write('commented ');
      if nd^.deff then write('in:  ') else write('out: ');
      if nd^.bcnt = nd^.ecnt then write(nd^.bcnt)
      else writeln('started: ',nd^.bcnt,'; ended: ',nd^.ecnt);
      writeln(' block(s); and ',nd^.icnt,' include file(s).');
      nd := nd^.next;
   end;
end;  { matchup }


{---( utils )---------------------------------------------------------}

procedure cvup(var ss: genstring);
var
   ii: integer;
begin
   for ii := 1 to length(ss) do ss[ii] := upcase(ss[ii]);
end;


procedure makebak(var fname, bname: pathname);
var
   ii: integer;
   ff: file;
begin
   bname := fname;
   ii := pos('.',bname);
   if ii <> 0 then bname[0] := chr(ii-1) else ii := length(bname);
   bname := bname + '.BAK';
   assign(ff,bname);
   {$I-} erase(ff); {$I+} ii := ioresult;
end;


{---( lists )---------------------------------------------------------}

procedure buildlist(cc: char; var ss: genstring);
var
   nd: delpntr;
begin
   if length(ss) <> 3 then abort(1);
   new(nd);
   nd^.dbeg := '(*' + ss +  '<';
   nd^.dend := '>'  + ss + '*)';
   nd^.bcnt := 0;
   nd^.ecnt := 0;
   nd^.icnt := 0;
   nd^.deff := cc = 'D';
   nd^.next := NIL;
   if delimitlist = NIL then delimitlist := nd
   else delimitlast^.next := nd;
   delimitlast := nd;
end;


procedure showlist;
var
   nd: delpntr;
begin
   nd := delimitlist;
   while nd <> NIL do
   begin
      if nd^.deff then write('#define ')
      else             write('#undef  ');
      writepattern(nd^.dbeg);
      writeln;
      nd := nd^.next;
   end;
end;


procedure cmdparse(var iname, dname: pathname);
var
   pc, ii: integer;
   ss: genstring;
   cc: char;
begin
   pc := paramcount;
   for ii := 1 to pc do
   begin
      ss := paramstr(ii);
      cvup(ss);
      if ss[1] = '-' then
      begin
         cc := upcase(ss[2]);
         delete(ss,1,2);
         case cc of
            '?': dohelp;
            'D', 'U': buildlist(cc,ss);
            'F': dname := ss;
            'K': dokeep := TRUE;
            'S': doverbose := FALSE;
            else abort(2);
         end; { case }
      end
      else
      begin
         if ss[1] = '<' then delete(ss,1,1);
         iname := ss;
      end;
   end;
end;


procedure defparse(var dname: pathname);
var
   dfile: text;
   cc: char;
   ss: genstring;
   tt: string[3];
begin { defparse }
   if dname = '' then exit;
   assign(dfile,dname); {$I-} reset(dfile); {$I+}
   if ioresult <> 0 then abort(3);
   while not eof(dfile) do
   begin
      readln(dfile,ss);
      cvup(ss);
      cc := ss[2];
      if ((ss[1] = '-') and ((cc = 'D') or (cc = 'U'))) then
      begin
         tt := copy(ss,3,3);
         buildlist(cc,tt);
      end;
   end;
   close(dfile);
end;  { defparse }


{---( Hackit )--------------------------------------------------------}

procedure hackit( var ss: genstring);
var
   nd: delpntr;
   ii: integer;
begin
   nd := delimitlist;
   while nd <> NIL do
   begin
      ii := pos(nd^.dbeg,ss);
      if ((ii <> 0) and (pos('<IFDEF',ss) = (ii + 5))) then
      begin
         if (ss[2] = '$') or (ss[2] = ' ') then
         begin
            if nd^.deff then ss[2] := '$' else ss[2] := ' ';
            nd^.icnt := succ(nd^.icnt);
         end;
      end
      else
      begin
         if ii <> 0 then
         begin
            if nd^.deff then begin ss[ii+6] := '*'; ss[ii+7] := ')'; end
            else             begin ss[ii+6] := '-'; ss[ii+7] := '-'; end;
            nd^.bcnt := succ(nd^.bcnt);
         end;
         ii := pos(nd^.dend,ss);
         if ii <> 0 then
         begin
            if nd^.deff then begin ss[ii-2] := '('; ss[ii-1] := '*'; end
            else             begin ss[ii-2] := '-'; ss[ii-1] := '-'; end;
            nd^.ecnt := succ(nd^.ecnt);
         end;
      end;
      nd := nd^.next;
   end; { while }
end; { hackit }



function incp(var ss: genstring): boolean;
var
   pp, qq: integer;
begin
   incp := FALSE;
   pp := pos('{$I',ss);
   if pp = 0 then exit;

   pp := pp + 3;
   if ((ss[pp] = '+') or (ss[pp] = '-')) then exit;

   while ss[pp] = ' ' do pp := succ(pp);
   qq := 0;
   while ((ss[pp] <> ' ') and (ss[pp] <> '}')) do
   begin
      qq := succ(qq);
      ss[qq] := ss[pp];
      pp := succ(pp);
   end;
   ss[0] := chr(qq);
   incp := TRUE;
end;  { incp }


procedure procinclude(var cname: pathname);
var
   cfile, ofile: text;
   bname: pathname;
   ss: genstring;
begin
   if doverbose then writeln(' Including file: ',cname);
   assign(cfile,cname); {$I-} reset(cfile); {$I+}
   if ioresult <> 0 then abort(3);
   assign(ofile,'IFDEF2.TMP'); rewrite(ofile);

   while not eof(cfile) do
   begin
      readln(cfile,ss); lines := succ(lines);
      hackit(ss);
      writeln(ofile,ss);
   end;

   close(ofile);
   close(cfile);
   if dokeep then
   begin
      makebak(cname,bname);
      rename(cfile,bname)
   end
   else erase(cfile);
   rename(ofile,cname);
end;  { procinclude }


procedure procmain(var iname: pathname);
var
   ifile, ofile: text;
   bname: pathname;
   ss: genstring;
begin
   assign(ifile,iname); {$I-} reset(ifile); {$I+}
   if ioresult <> 0 then abort(3);
   assign(ofile,'IFDEF1.TMP'); rewrite(ofile);

   while not eof(ifile) do
   begin
      readln(ifile,ss); lines := succ(lines);
      hackit(ss);
      writeln(ofile,ss);
      if incp(ss) then procinclude(ss);
   end;

   close(ofile);
   close(ifile);
   if dokeep then
   begin
      makebak(iname,bname);
      rename(ifile,bname)
   end
   else erase(ifile);
   rename(ofile,iname);
end; { procmain }

var
   iname, dname: pathname;

begin { main }
   delimitlist := NIL;
   doverbose := TRUE;
   dokeep    := FALSE;
   dname := '';
   iname := '';
   lines := 0;

   cmdparse(iname, dname);
   if iname = '' then abort(2);
   defparse(dname);

   if doverbose then
   begin
      presentation;
      writeln;
      if dname <> '' then writeln('Definition file: ',dname); showlist;
      writeln;
      writeln('Input file:      ',iname);
   end;

   procmain(iname);

   if doverbose then
   begin
      writeln;
      writeln('Processed ',lines,' source lines.');
      matchup;
   end;

end.

(* EOF *)
