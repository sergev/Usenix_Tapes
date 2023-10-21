(*
 *	Graphic Procedures for the Digital VT100 Terminal
 *      Most of these Procedures will work for any ANSI Terminal
 *
 *      If VT100 Does Not have the Advanced Video Option
 *      Some Functions will not work
 *
 *	Written By: Bryan K. DeLaney  
 *     
 *      
 *	To use These procedures #include this file
 *      in The Procedure section of your Unix Pascal Program
 *
 *)

(* For more information Consult VT100 Users Guide *)

procedure CursorBack(x :integer);
begin
	write('[',x:1,'D');
	(* write('ESC[',x:1,'D') *)
end;

(* ********************************************************** *)

procedure CursorDown(x :integer);
begin
	write('[',x:1,'B');
	(* write('ESC[',x:1,'B') *)
end;

(* ********************************************************** *)

procedure CursorUp(x :integer);
begin
	write('[',x:1,'A');
	(* write('ESC[',x:1,'A') *)
end;

(* ********************************************************** *)

procedure CursorForward(x :integer);
begin
	write('[',x:1,'C');
	(* write('ESC[',x:1,'C') *)
end;

(* ********************************************************** *)

procedure ClearHome;
begin
	write('[;H[2J');
	(* write('ESC[;H ESC[2J') *)
end;

(* ********************************************************** *)

procedure Clear(x :integer);
begin
	if x > 2 then
	begin
		x := 2;
	end;
	
	write('[',x:1,'J');
	(* write('ESC[',x:1,'J') *)
end;

(* ********************************************************** *)

procedure LineClear(x :integer);
begin
	if x > 2 then
	begin
		x := 2;
	end;
	
	write('[',x:1,'K');
	(* write('ESC[',x:1,'K') *)
end;

(* ********************************************************** *)

procedure Inverse;
begin
	write('[;7m');
	(* write('ESC[;7m') *)
end;
(* ********************************************************** *)

procedure Bold;
begin
	write('[;1m');
	(* write('ESC[;1m') *)
end;

(* ********************************************************** *)

procedure UnderScore;
begin
	write('[;4m');
	(* write('ESC[;4m') *)
end;

(* ********************************************************** *)

procedure Blink;
begin
	write('[;5m');
	(* write('ESC[;5m') *)
end;

(* ********************************************************** *)

procedure Normal;
begin
	write('[;0m');
	(* write('ESC[;0m') *)
end;

(* ********************************************************** *)

procedure Double;
begin
	write('#6');
	(* write('ESC#6') *)
end;

(* ********************************************************** *)

procedure DoubleHighUpper;
begin
	write('#3');
	(* write('ESC#3') *)
end;

(* ********************************************************** *)

procedure DoubleHighLower;
begin
	write('#4');
	(* write('ESC#4') *)
end;

(* ********************************************************** *)

procedure GraphicOn;
begin
	write('(0');
	(* write('ESC(0') *)
end;

(* ********************************************************** *)

procedure GraphicOff;
begin
	write('(B');
	(* write(ESC(B') *)
end;

(* ********************************************************** *)

procedure Position(x,y :integer);
begin
	write('[',y:1,';',x:1,'H');
	(* write('ESC[',y:1,';',x:1,'H') *)
end;

(* ********************************************************** *)

procedure Hline(x,y,length,scanline :integer);
var
	l :integer;
	error :char;
begin	
	GraphicOn;
	Position(x,y);
	
	if (scanline = 1) or (scanline =0) then
	begin
		for l := 1 to length do
		begin
			write('o');
		end;
	end;
	
	if (scanline = 3) or (scanline = 2) then
	begin
		for l := 1 to length do
		begin
			write('p');
		end;
	end;
	
	if (scanline = 5) or (scanline = 4) then
	begin
		for l := 1 to length do
		begin
			write('q');
		end;
	end;
	
	if (scanline = 7) or (scanline = 6) then
	begin
		for l := 1 to length do
		begin
			write('r');
		end;
	end;
	
	if (scanline = 9) or (scanline =8) then
	begin
		for l := 1 to length do
		begin
			write('s');
		end;
	end;
	if scanline > 9 then
	begin
		GraphicOff;
		ClearHome;
		writeln('Error Value for Scanline not in range');
		write('Hit <cr> to leave Hline');
		read(error);
		ClearHome;
	end;
	GraphicOff;
end; (* Procedure Hline *)

(* ********************************************************** *)

procedure Line(scanline :integer);
var
	length :integer;
	error :char;
begin	
	GraphicOn;
	write('[80D');
	(* write('ESC[80D') *)

	if (scanline = 1) or (scanline =0) then
	begin
		for length := 1 to 80 do
		begin
			write('o');
		end;
	end;
	
	if (scanline = 3) or (scanline = 2) then
	begin
		for length := 1 to 80 do
		begin
			write('p');
		end;
	end;
	
	if (scanline = 5) or (scanline = 4) then
	begin
		for length := 1 to 80 do
		begin
			write('q');
		end;
	end;
	
	if (scanline = 7) or (scanline = 6) then
	begin
		for length := 1 to 80 do
		begin
			write('r');
		end;
	end;
	
	if (scanline = 9) or (scanline =8) then
	begin
		for length := 1 to 80 do
		begin
			write('s');
		end;
	end;
	if scanline > 9 then
	begin
		GraphicOff;
		ClearHome;
		writeln('Error Value for Scanline not in range');
		write('Hit <cr> to leave Hline');
		read(error);
		ClearHome;
	end;
	GraphicOff;
	writeln;
end; (* procedure Hine *)

(* ********************************************************** *)

procedure Vline(x,y,length :integer);
var
	l,Newy :integer;
begin
	Newy := y;

	GraphicOn;
	Position(x,y);
	for l := 1 to length do
	begin
		write('x');
		Newy: = Newy + 1;
		Position(x,Newy);
	end;
	GraphicOff;
end;  (* Procedure Vline *)

(* ********************************************************** *)

procedure Box(x,y,High,Wide :integer);
begin
	if x = 0 then
	begin
		x :=1;
	end;
	if y = 0 then
	begin
		y :=1;
	end;
	
	Hline(x,y,Wide,5);
	Hline(x,y+High,Wide,5);
	Vline(x,y,High);
	Vline(x+Wide,y,High);
	GraphicOn;
	Position(x,y);
	write('l');
	Position(x,y+High);
	write('m');
	Position(x+Wide,y);
	write('k');
	Position(x+Wide,y+High);
	write('j');
	GraphicOff;
	Position(x+1,y+1);

end;  (* Procedure Box *)

(* ********************************************************** *)

procedure StoreCursor;
begin
	write('7');
	(* write('ESC7') *)
end;

(* ********************************************************** *)

procedure RestoreCursor;
begin
	write('8');
	(* write('ESC8') *)
end;

(* ********************************************************** *)

procedure LoadLED(x :integer);
begin
	write('[',x:1,'q');
	(* write('ESC[',x:1,'q') *)
end;	

(* ********************************************************** *)

procedure SetScrollRegion(t,b :integer);
begin
	write('[',t:1,';',b:1,'r');
	(* write('ESC[',t:1,';',b:1,'r') *)
end;

(* ********************************************************** *)
procedure ResetScroll;
begin
	write('[0;24r');
	(* write('ESC[0;24r') *)
end;
(* ********************************************************** *)
