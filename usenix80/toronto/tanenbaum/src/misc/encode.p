#include "../local.h"
#include "../em1.h"
{
(c) copyright 1978 by the Vrije Universiteit, Amsterdam, The Netherlands.
Explicit permission is hereby granted to universities to use or duplicate
this program for educational or research purposes.  All other use or dup-
lication  by universities,  and all use or duplication by other organiza-
tions is expressly prohibited unless written permission has been obtained
from the Vrije Universiteit. Requests for such permissions may be sent to
     Dr. Andrew S. Tanenbaum
     Wiskundig Seminarium
     Vrije Universiteit
     Postbox 7161
     1007 MC Amsterdam
     The Netherlands
}

{$r- : range check off}
{$c+ : C-type strings allowed}
program encode(input,output);
const
	MAGIC= 172;		{indicates compact EM1 code}

type
	byte	= 0..255;
	mnem	= packed array[1..4] of char;
var
	lino	: integer;
	mn	: array[byte] of mnem;
	hash	: array[byte] of byte;
	CAL	: byte;
	err	: text;

procedure exit(i:integer); extern;
procedure diag(var f:text); extern;
procedure popen(var f:text; s:string); extern;
procedure buff(var f:text); extern;

procedure name;
begin write(err,'encode: input line ',lino:0,': ') end;

procedure enter(b:byte;m:mnem);
var	h	: byte;
begin h:=(ord(m[1])*ord(m[2]) + ord(m[3])) mod 256;
    while hash[h] <> 0 do
	h:=(h+1) mod 256;
    hash[h]:=b; mn[b]:=m
end;

procedure init;
var	i,n	: integer;
	m	: mnem;
	b	: byte;
	f	: text;
begin
    get(input);
    diag(err);
    popen(f,TABLES_PATH);
    lino:=0;
    n:=1;
    for b:=0 to 255 do
	begin mn[b]:='????'; hash[b]:=0 end;
    while not eoln(f) do readln(f);
    readln(f);
    while not eoln(f) do readln(f);
    readln(f);
    while not eoln(f) do
	begin m[4]:=' ';
	    for i:=1 to 3 do
		read(f,m[i]);
	    if m='cal ' then CAL:=n;
	    enter(n,m);
	    readln(f); n:=n+1;
	end;
    { pseudo's }
    enter(ps_bss,'bss ');	enter(ps_con,'con ');
    enter(ps_end,'end ');	enter(ps_eof,'eof ');
    enter(ps_mes,'mes ');	enter(ps_exc,'exc ');
    enter(ps_exd,'exd ');	enter(ps_hol,'hol ');
    enter(ps_let,'let ');	enter(ps_pro,'pro ');
    enter(ps_rom,'rom ');	enter(ps_imd,'imd ');
    enter(ps_fwp,'fwp ');
end;

procedure skipsp;
begin
    while ((input^=' ') or (input^=chr(9))) and not eoln do
	get(input);
    if input^ = ';' then
	while not eoln do get(input);
end;


function getmnem:byte;
var	h	: integer;
	m	: mnem;
	b	: byte;
begin
    m[4]:=' ';
    for h:=1 to 3 do
	read(m[h]);
    h:=(ord(m[1])*ord(m[2]) + ord(m[3])) mod 256;
    repeat
	b:=hash[h];
	h:=(h+1) mod 256;
    until (b=0) or (mn[b]=m);
    if b=0 then
	begin name; writeln(err,'bad mnemonic ',m); exit(-1) end;
    getmnem:=b
end;

procedure putword(i:integer);
var	t:	packed record
		    case boolean of
		    false	: (j:integer);
		    true	: (b1,b2:char)
		end;
begin t.j:=i; write(t.b1,t.b2) end;

procedure putilb(i:integer);
begin
    if i<256 then
	write(chr(sp_ilb1),chr(i))
    else
      begin write(chr(sp_ilb2)); putword(i) end
end;

procedure putdlb(i:integer);
begin
    if i<256 then
	write(chr(sp_dlb1),chr(i))
    else
	begin write(chr(sp_dlb2)); putword(i) end
end;

procedure putcst(i:integer);
begin
    if i>=0 then
	if i<256 then write(chr(sp_cst1),chr(i)) else
	    begin write(chr(sp_cst2)); putword(i) end
    else
	if i>-256 then write(chr(sp_cstm),chr(-i)) else
	    begin write(chr(sp_cst2)); putword(i) end
end;

procedure copyname(b:byte);
const	SLEN = 20;
var	s	: packed array[1..SLEN] of char;
	i,j,k	: integer;
	short	: boolean;
begin i:=0;
    if not (input^ in ['A'..'Z','a'..'z','_','.']) then
	begin name; writeln(err,'bad identifier'); exit(-1) end;
    short:=input^='.';
    repeat
	i:=i+1; read(s[i]);
	short:=short and not (input^ in ['A'..'Z','a'..'z','_','.']);
    until not (input^ in ['A'..'Z','a'..'z','0'..'9','_','.']);
    if short and (b=sp_dnam) then
	begin k:=0;
	    for j:=2 to i do
		k:=k*10 + ord(s[j]) - ord('0');
	    putdlb(k)
	end
    else
	begin
	    write(chr(b),chr(i));
	    for j:=1 to i do
		write(s[j]);
	end;
    skipsp;
end;

procedure copystring;
const SLEN = 512;
var	s	: packed array[1..SLEN] of byte;
	c	: char;
	i,j	: integer;
	b	: byte;
begin get(input); i:=0;
    while input^<>'"' do
	begin read(c); i:=i+1; b:=ord(c);
	    if c='\' then
		if (input^='"') or (input^='\') then
		    begin read(c); b:=ord(c) end
		else
		    begin b:=0;
			for j:=1 to 3 do
			    begin
				if (input^ < '0') or (input^ > '7') then
				    begin name;
					writeln(err,'three digits required');
					exit(-1)
				    end;
				read(c); b:=8*b + ord(c) - ord('0')
			    end;
		    end;
	    s[i]:=b;
	    if eoln then
		begin name; writeln(err,'newline in string not allowed');
		    exit(-1);
		end;
	end;
    get(input); write(chr(sp_scon));
    if i<255 then write(chr(i)) else
	begin write(chr(255)); putword(i) end;
    for j:=1 to i do
	write(chr(s[j]));
end;

procedure constant;
const	RLEN = 72;
var	i,j,k : integer;
	ca	: packed array[1..RLEN] of char;
	s	: boolean;
	rc	: boolean;
	c	: char;
begin i:=0; rc:=false;
    repeat i:=i+1; read(c); ca[i]:=c;
	rc:=rc or (c='.') or (c='e');
    until not (input^ in ['0'..'9','+','-','.','e']);
    if rc then
	begin write(chr(sp_rcon),chr(i));
	    for j:=1 to i do write(ca[j])
	end
    else
	begin j:=1; s:=false; k:=0;
	    if (ca[1]='+') or (ca[1]='-') then
		begin j:=j+1; s:=ca[1]='-' end;
	    while j<=i do
		begin k:=k*10 + ord(ca[j]) - ord('0'); j:=j+1 end;
	    if s then k:= -k;
	    if (k >= sp_fcst0) and (k < sp_fcst0 + sp_ncst0) then
		write(chr(k))
	    else
		putcst(k);
	end
end;

procedure operand(opb:byte);
var	b	: byte;
	i	: integer;
begin skipsp;
    if input^ in ['0'..'9','+','-'] then
	constant
    else if input^ = '"' then
	copystring
    else if input^ = '*' then
	begin get(input); read(i); putilb(i) end
    else
	begin b:=sp_dnam;
	    if input^ = '$' then begin get(input); b:=sp_pnam end;
	    if opb=CAL then b:=sp_pnam;
	    copyname(b);
	end;
    skipsp;
end;

procedure pseudo(b:byte);
var	i	: integer;
begin
    case b of
	ps_end:
	    buff(output);  {flush the buffer}
	ps_eof:
	    halt;
	ps_mes,ps_imd,ps_exd,ps_hol:
	    operand(b);
	ps_fwp:
	    begin skipsp; copyname(sp_pnam) end;
	ps_bss,ps_exc,ps_let:
	    begin operand(b);
		if input^ <> ',' then
		    begin name; writeln(err,'comma required'); exit(-1) end;
		get(input);
		operand(b)
	    end;
	ps_pro:
	    begin skipsp; copyname(sp_pnam);
		for i:=1 to 3 do
		    begin
			if input^ <> ',' then
			    begin name; writeln(err,'comma required');
				exit(-1);
			    end;
			get(input); operand(b)
		    end
	    end;
	ps_rom,ps_con:
	    begin operand(b);
		while input^ = ',' do
		    begin get(input); operand(b) end;
		write(chr(sp_cend))
	    end;
    end
end;

procedure encode;
var	b	: byte;
	i	: integer;
begin
    if input^ in ['0'..'9'] then
	begin read(i);
	    if i<sp_nilb0 then
		write(chr(i+sp_filb0))
	    else
		putilb(i)
	end
    else
	begin
	    if input^ in ['A'..'Z','a'..'z','_','.'] then
		copyname(sp_dnam);
	    skipsp;
	    if not eoln then
		begin
		    b:=getmnem; write(chr(b));
		    if b<=sp_lmnem then
			begin skipsp;
			   if not eoln then operand(b)
			end
		    else
			pseudo(b);
		    skipsp
		end
	end
end;

{ main }
begin init; putword(MAGIC);
    while not eof do
	begin lino:=lino+1;
	    if not eoln then encode;
	    if not eoln then
		begin name; writeln(err,'end of line expected',ord(input^));
		    exit(-1)
		end;
	    readln
	end
end.
