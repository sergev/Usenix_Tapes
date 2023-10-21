program interrupt(input,output);
{this program uses the high resolution mode, 640X350 by 16 color graphics
 that are available with the enhanced graphics adapter and enhanced color
 monitor.  This was written by Alan Curtis, 1985.                        }

var
  j,k,page : integer;

{////////////////////////////}

procedure super_hires;
{set resolution to 640X350 by 16 color mode}
type
  result = record
    al,ah,bl,bh : byte;
    cx,dx,bp,si,di,ds,es,flags : integer;
  end;
var
  int_result : result;
  col : byte;
begin
  with int_result do begin
    al := $10;  {graphics mode}
    ah := $0;
  end;
  intr($10,int_result);  {high res mode}
end;

{////////////////////////////}

procedure set_page(page : integer);
{sets active display page - must have 128K of graphics memory}
type
  result = record
    al,ah,bl,bh : byte;
    cx,dx,bp,si,di,ds,es,flags : integer;
  end;
var
  int_result : result;
  b_page : byte;
begin
  b_page := page;
  int_result.ah := $5;
  int_result.al := b_page;  {active page}
  intr($10,int_result);
end;

{////////////////////////////}

procedure plot_point(row,col,color,page : integer);
{plots a point at row,col in the specified color (0-15), at the
 specific page}
type
  result = record
    al,ah,bl,bh : byte;
    cx,dx,bp,si,di,ds,es,flags : integer;
  end;
var
  int_result : result;
  b_page,b_color : byte;

begin
  b_page := page;
  b_color := color;
  int_result.ah := $C;
  int_result.bh := b_page;  {active page}
  int_result.al := b_color;
  int_result.dx := row;
  int_result.cx := col;
  intr($10,int_result);
end;

{////////////////////////////}

procedure plot_line(x1,y1,x2,y2 : real; color,page : integer);
{draws a line from x1,y1 to x2,y2 in the specified color}
type
  result = record
    al,ah,bl,bh : byte;
    cx,dx,bp,si,di,ds,es,flags : integer;
  end;
var
  counter : real;
  int_result : result;
  dx,dy,ddx,ddy,newx,newy : real;
  b_page,b_color : byte;
  xmult,ymult : integer;

begin
  b_page := page;
  b_color := color;
  int_result.ah := $C;
  int_result.bh := b_page;  {active page}
  int_result.al := b_color;
  int_result.dx := round(y1);
  int_result.cx := round(x1);
  intr($10,int_result);

  dx := x2 - x1;
  if dx < 0.0 then xmult := -1 else xmult := 1; {increment can be negative}
  dy := y2 - y1;
  if dy < 0.0 then ymult := -1 else ymult := 1;

  {before getting ratios, check for zeros in dx and dy}
  if dy = 0.0 then begin
    {x will be incremented by 1 through every loop, y by 0}
    ddx := 1.0 * xmult;
    ymult := 0;
    dy := abs(dx);    {fool loop counter below}
  end
  else if dx = 0.0 then begin
    ddx := 2;
    ddy := 1 * ymult;
    xmult := 0;
    dx := abs(dy);    {fool loop counter}
  end
  else begin
    ddx := abs(dx/dy);  {get ratio}
    ddy := abs(dy/dx);
    ddx := ddx*xmult;
    ddy := ddy*ymult;
  end;

  {the following initializations are used in the proceeding loops}
  newy := y1;
  newx := x1;
  counter := 0.0;
  dy := abs(dy);
  dx := abs(dx);

  if abs(ddx) <= 1.0 then
  while counter < dy do begin
    counter := counter + 1.0;
    newx := newx + ddx;
    newy := newy + ymult;
    int_result.ah := $C;
    int_result.bh := b_page;  {active page}
    int_result.al := b_color;
    int_result.dx := round(newy);
    int_result.cx := round(newx);
    intr($10,int_result);
  end

  else while counter < dx do begin  {x will be incremented by 1}
    counter := counter + 1.0;
    newx := newx + xmult;
    newy := newy + ddy;
    int_result.ah := $C;
    int_result.bh := b_page;  {active page}
    int_result.al := b_color;
    int_result.dx := round(newy);
    int_result.cx := round(newx);
    intr($10,int_result);
  end;
end;

{****************************}
{begin main}
begin
  super_hires;
  page := 0;
  set_page(page);
  plot_line(0.0,0.0,639.0,0.0,9,0);
  plot_line(639.0,0.0,639.0,349.0,9,0);
  plot_line(639.0,349.0,0.0,349.0,9,0);
  plot_line(0.0,349.0,0.0,0.0,9,0);
  for j := 1 to 15 do
   for k := 1 to 10 do
     plot_line(j*10.0+k,1.0,349.0+j*10.0+k,350.0,j,0);
end.
