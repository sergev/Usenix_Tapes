{ filter to replace the error numbers in pem.p by question marks }
{ qn replaces question marks by consecutive numbers }

program nq(input,output);
var i:integer; c:char;
begin get(input); i:=1;
  while not eof do
    begin
      while not eoln do
	begin read(c);
	  if c='+' then
	    begin read(c);
	      if c='0' then
		begin
		  while input^ in ['0'..'9'] do
		    get(input);
		  write('?');
		end
	      else write('+',c)
	    end
	  else write(c);
	end;
      readln; writeln
    end;
  {funny nesting}
end.
