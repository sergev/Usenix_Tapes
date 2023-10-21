{this program replaces question marks by consecutive numbers from 1 up}
{used together with nq.p to maintain the error numbers in pem.p}

program test(input,output);
var i:integer; c:char;
begin get(input); i:=1;
  while not eof do
    begin
      while not eoln do
	begin read(c);
	  if c='?' then begin write('+0',i:0); i:=i+1 end else write(c)
	end;
      readln; writeln
    end;
end.
