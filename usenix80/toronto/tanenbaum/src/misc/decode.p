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

{$c+	: allow C-type strings}
{$i256	: integer sets of 256 elements}
program decode(input,output);
const
	MAGIC0 = 172;
	MAGIC1 = 428;
	MAGIC2 = 684;
type
	byte	= 0..255;
	alfa2	= packed array[1..4] of char;
var
	mn	: array[byte] of alfa2;
	oneops	: set of byte;
	CAL	: byte;
	err	: text;

procedure notext(var f:text); extern;
procedure popen(var f:text; s:string); extern;
procedure diag(var f:text); extern;
procedure exit(i:integer); extern;

procedure name(b:integer);
begin write(err,'decode: input number ',b:0,': ') end;

procedure init;
var	b	: byte;
	c	: char;
	i,n	: integer;
	f	: text;
	m	: alfa2;
	zseen	: boolean;
begin
    notext(input);  {do not check for valid char range and end of line}
    get(input);
    diag(err);
    popen(f,TABLES_PATH);
    for b:=0 to 255 do
	mn[b]:=' ???';
    n:=sp_fmnem; oneops:=[];
    while not eoln(f) do readln(f);
    readln(f);
    while not eoln(f) do readln(f);
    readln(f);
    while not eoln(f) do
	begin m[1]:=' '; zseen:=false;
	    for i:=2 to 4 do
		read(f,m[i]);
	    while not eoln(f) do
		begin read(f,c);
		    if c='z' then zseen:=true;
		end;
	    readln(f);
	    mn[n]:=m;
	    if not zseen then oneops:=oneops+[n];
	    if m=' cal' then CAL:=n;
	    n:=n+1;
	end;
    { pseudo's }
    mn[ps_bss ]:=' bss';	mn[ps_con ]:=' con';
    mn[ps_end ]:=' end';	mn[ps_eof ]:=' eof';
    mn[ps_mes ]:=' mes';	mn[ps_exc ]:=' exc';
    mn[ps_exd ]:=' exd';	mn[ps_hol ]:=' hol';
    mn[ps_let ]:=' let';	mn[ps_pro ]:=' pro';
    mn[ps_rom ]:=' rom';	mn[ps_imd ]:=' imd';
    mn[ps_fwp ]:=' fwp';
end;

procedure readword(var i:integer);
var t	: packed record
	      case boolean of
		  false	: (b1,b2:char);
		  true	: (j:integer)
	  end;
begin read(t.b1,t.b2); i:=t.j end;

procedure getilb(b:byte);
var l	: integer;
    c	: char;
begin
    if b=sp_ilb1 then
	begin read(c); l:=ord(c) end
    else readword(l);
    write(l:0)
end;

procedure getdlb(b:byte);
var l	: integer;
    c	: char;
begin
    if b=sp_dlb1 then
	begin read(c); l:=ord(c) end
    else readword(l);
    write('.',l:0)
end;

procedure getcst(b:byte);
var l	: integer;
    c	: char;
begin
    if b<>sp_cst2 then
	begin read(c); l:=ord(c) end
    else readword(l);
    if b=sp_cstm then l:=-l;
    write(l:0)
end;

procedure copystring;
var	l	: integer;
	c	: char;
begin read(c); l:=ord(c);
    if l=255 then readword(l);
    while l>0 do
	begin l:=l-1; read(c);
	    if (c<chr(32)) or (c>chr(127)) then
		begin write('\');
		    write(chr(ord(c) div 64 + ord('0')));
		    write(chr(ord(c) mod 64 div 8 + ord('0')));
		    write(chr(ord(c) mod 8 + ord('0')))
		end
	    else
		begin
		    if (c='"') or (c='\') then
			write('\');
		    write(c)
		end
	end
end;

procedure copyalpha;
var	i	: integer;
	c	: char;
begin
    write(';   ');
    for i:=1 to 8 do
	begin read(c); if ord(c)<>0 then write(c) end;
    writeln
end;

procedure header;
var	i,g,p	:integer;
begin
    readword(i);
    if i=MAGIC1 then
	begin readword(g); readword(p);
	    if g>0 then
		begin writeln('; exported data labels:');
		    for i:=1 to g do copyalpha;
		    writeln
		end;
	    if p>0 then
		begin writeln('; exported procedures :');
		    for i:=1 to p do copyalpha;
		    writeln
		end;
	    readword(i);
	    if i<>MAGIC2 then
		begin name(i); writeln(err,'end of header (684) expected');
		    exit(-1)
		end;
	end
    else
	if i<>MAGIC0 then
	    begin name(i); writeln(err,'magic number (172) expected');
		exit(-1)
	    end;
end;

procedure getoperand(c1:char; opb:byte);
var	c	: char;
begin write(c1); read(c);
    if (ord(c) >= sp_fcst0) and (ord(c) < sp_fcst0 + sp_ncst0) then
	write(ord(c):0)
    else case ord(c) of
	sp_dlb1,sp_dlb2:
	    getdlb(ord(c));
	sp_dnam:
	    copystring;
	sp_pnam:
	    begin
		if (opb<>CAL) and (opb<>ps_fwp) then write('$');
		copystring
	    end;
	sp_cst1,sp_cstm,sp_cst2:
	    getcst(ord(c));
	end
end;

procedure pseudo(b:byte);
var	no	: integer;
	c	: char;
begin
    case b of
	ps_end,ps_eof:
	    ;
	ps_mes,ps_imd,ps_exd,ps_hol,ps_fwp:
	    getoperand(' ',b);
	ps_bss,ps_exc,ps_let:
	    begin getoperand(' ',b); getoperand(',',b) end;
	ps_pro:
	    begin read(c);
		if ord(c)<>sp_pnam then
		    begin name(ord(c)); writeln(err,'procedure name expected');
			exit(-1)
		    end;
		write(' '); copystring;
		getoperand(',',ord(c));
		getoperand(',',ord(c));
		getoperand(',',ord(c));
	    end;
	ps_rom,ps_con:
	    begin no:=0; read(c);
		while ord(c)<>sp_cend do
		    begin
			if ((ord(c)=sp_scon) and (no>0)) or (no=8) then
			    begin no:=0; writeln; write(mn[b]) end;
			if no=0 then write(' ') else write(',');
			if (ord(c) >= sp_fcst0) and
				(ord(c) < sp_fcst0 + sp_ncst0) then
			    write(ord(c):0)
			else case ord(c) of
			    sp_ilb1,sp_ilb2:
				begin write('*'); getilb(ord(c)) end;
			    sp_dlb1,sp_dlb2:
				getdlb(ord(c));
			    sp_cst1,sp_cstm,sp_cst2:
				getcst(ord(c));
			    sp_pnam:
				begin write('$'); copystring end;
			    sp_scon:
				begin write('"'); copystring; write('"') end;
			    sp_dnam,sp_rcon:
				copystring
			    end;
			read(c); no:=no+1;
		    end;
	    end;
    end
end;

procedure decode;
var	c	: char;
begin read(c);
    if (ord(c)>=sp_filb0) and (ord(c)<sp_filb0+sp_nilb0) then
	write(ord(c) - sp_filb0:0)
    else if (ord(c)=sp_ilb1) or (ord(c)=sp_ilb2) then
	getilb(ord(c))
    else
	begin
	    if (ord(c)=sp_dlb1) or (ord(c)=sp_dlb2) then
		begin getdlb(ord(c)); read(c) end
	    else if ord(c)=sp_dnam then
		begin copystring; read(c) end;
	    if (ord(c)>=sp_fmnem) and (ord(c)<=sp_lmnem) then
		begin write(mn[ord(c)]);
		    if ord(c) in oneops then getoperand(' ',ord(c))
		end
	    else if (ord(c)>=sp_fpseu) and (ord(c)<=sp_lpseu) then
		begin write(mn[ord(c)]); pseudo(ord(c)) end
	    else
		begin name(ord(c));
		    writeln(err,'instruction or pseudo expected');
		    exit(-1)
		end;
	end
end;

begin { main }
    init;
    header;
    while not eof do
	begin decode; writeln end;
end.
