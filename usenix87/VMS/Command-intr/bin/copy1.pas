(* COPY1.PAS *)
program copy1 (input, output);
var c : char;
begin
  while not EOF do
  begin
    while not EOLN do
    begin
      read(c);
      write('[', c, ']');
    end;
    readln;
    writeln;
  end;
end.
(* END *)
