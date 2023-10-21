implementation module dialog;

import
    vdi,
    picture;

(* export *)
from picture import
    Picture, XCoord, YCoord;

(* export Picture; *)

(* export type Dialog; *)

from rubband import
    Rubberband;

from utilities import
    round, IntersectBoxes;

const
    MSGLINESPACING = 5;     (* spacing between message lines *)
    MSGTEXTBOXSPACE = 5;    (* space between message and textbox *)
    TEXTBOXVSPACE = 6;	    (* total space above and below text in textbox *)
    MAXSTRINGLEN = 100;

    NULL = 0C;
    BACKSPACE = 10C;
    DELETE = 177C;
    DELETELINE = 25C;
    LINEFEED = 12C;
    CARRIAGERTN = 15C;

type
    Dialog =  pointer to DialogRecord;
    DialogRecord = record
	pict : Picture;
	frame : Picture;
	msg : Picture;
	textBox : Picture;
	text : array [1..MAXSTRINGLEN] of char;
	count : integer;
	fontHt : YCoord;
	msgFont : integer;
	warning : Picture;
    end;

type CharSet = set of char;


(* export *)
procedure Create(
    var d : Dialog;
    const width : integer;
    const height : integer;
    msg : Picture;
    textBoxHScale : real
);
    var textBoxWidth, textBoxHeight : integer;
	textx : XCoord;
	texty : YCoord;
	subpict : Picture;
begin
    new(d);
    d^.count := 0;
    picture.Create(d^.frame);
    picture.DefaultRenderingStyle(d^.frame, picture.Fill);
    picture.DefaultPattern(d^.frame, vdi.solidPattern);
    picture.DefaultColor(d^.frame, vdi.WHITE);
    picture.Rectangle(d^.frame, 0, 0, width - 1, height -1);
    picture.DefaultColor(d^.frame, vdi.BLACK);
    picture.DefaultRenderingStyle(d^.frame, picture.Stroke);
    picture.Rectangle(d^.frame, 0, 0, width - 1, height -1);
    picture.Rectangle(d^.frame, 2, 2, width - 3, height -3);
    picture.Rectangle(d^.frame, 3, 3, width - 4, height -4);

    d^.msg := msg;
    d^.msgFont := picture.GetFont(msg);
    
    d^.fontHt := picture.GetFontHeight(d^.msgFont);
    textBoxWidth := round(textBoxHScale * float(width));
    textBoxHeight := d^.fontHt + TEXTBOXVSPACE;
    picture.Create(d^.textBox);
    picture.DefaultRenderingStyle(d^.textBox, picture.Fill);
    picture.DefaultPattern(d^.textBox, vdi.solidPattern);
    picture.DefaultColor(d^.textBox, vdi.WHITE);
    picture.Rectangle(d^.textBox, 0, 0, textBoxWidth - 1, textBoxHeight - 1);
    picture.DefaultColor(d^.textBox, vdi.BLACK);
    picture.DefaultRenderingStyle(d^.textBox, picture.Stroke);
    picture.Rectangle(d^.textBox, 0, 0, textBoxWidth - 1, textBoxHeight - 1);
    
    picture.Create(subpict);
    picture.Nest(d^.msg, subpict);
    picture.Nest(d^.textBox, subpict);
    picture.AlignTopToBottom(d^.msg, d^.textBox);
    picture.Translate(d^.textBox, 0.0, -float(MSGTEXTBOXSPACE));
    picture.AlignHorizCenters(d^.msg, d^.textBox);

    picture.AlignCenters(d^.frame, subpict);
    picture.Create(d^.pict);
    picture.DisableRedraw(d^.pict);
    picture.Nest(d^.frame, d^.pict);
    picture.Nest(subpict, d^.pict);

    d^.warning := Picture(nil);
end Create;


(* export *)
procedure Destroy(var d : Dialog);
begin
    picture.Destroy(d^.pict);
end Destroy;


(* export *)
procedure GetPicture(const d : Dialog) : Picture;
begin
    return d^.pict;
end GetPicture;


(* export *)
procedure SetOrigin(
    const d : Dialog;
    const x0 : XCoord;
    const y0 : YCoord
);
begin
    picture.SetOrigin(d^.pict, x0, y0);
    picture.SetOrigin(d^.frame, x0, y0);
    picture.SetOrigin(d^.msg, x0, y0);
    picture.SetOrigin(d^.textBox, x0, y0);
    if d^.warning # Picture(nil) then
	picture.SetOrigin(d^.warning, x0, y0);
    end;
end SetOrigin;


(* export *)
procedure AddWarning(
    const d : Dialog;
    const warning : Picture
);
    var x0 : XCoord;
	y0 : YCoord;
begin
    d^.warning := warning;
    picture.GetOrigin(d^.pict, x0, y0);
    picture.SetOrigin(d^.warning, x0, y0);
    picture.Insert(d^.warning, d^.msg);
    picture.AlignHorizCenters(d^.msg, d^.warning);
    picture.AlignBottomToTop(d^.msg, d^.warning);
    picture.Translate(d^.warning, 0.0, float(MSGLINESPACING));
end AddWarning;


(* export *)
procedure RemoveWarning(const d : Dialog);
begin
    if d^.warning # Picture(nil) then
	picture.Unnest(d^.warning);
	d^.warning := Picture(nil);
    end;
end RemoveWarning;


(* export *)
procedure DrawWarning(const d : Dialog);
begin
    if d^.warning # Picture(nil) then
	picture.Draw(d^.warning);
    end;
end DrawWarning;


(* export *)
procedure EraseWarning(const d : Dialog);
begin
    if d^.warning # Picture(nil) then
	picture.Disable(d^.warning);
	picture.Redraw(d^.warning);
	picture.Enable(d^.warning);
    end;
end EraseWarning;


(* export *)
procedure DrawInBoundingBox(
    const d : Dialog;
	  x0 : XCoord;
	  y0 : YCoord;
	  x1 : XCoord;
	  y1 : YCoord
);
    var left, right : XCoord;
	bottom, top : YCoord;
begin
    picture.GetBoundingBox(d^.frame, left, bottom, right, top);
    if IntersectBoxes(x0, y0, x1, y1, left, bottom, right, top) then
	picture.DrawInBoundingBox(d^.frame, x0, y0, x1, y1);
	picture.DrawInBoundingBox(d^.msg, x0, y0, x1, y1);
	picture.DrawInBoundingBox(d^.textBox, x0, y0, x1, y1);
	picture.GetBoundingBox(d^.textBox, left, bottom, right, top);
	dec(right);
	if IntersectBoxes(left, bottom, right, top, x0, y0, x1, y1) then
	    DrawText(d, 0);
	end;
	if d^.warning # Picture(nil) then
	    picture.DrawInBoundingBox(d^.warning, x0, y0, x1, y1);
	end;
    end;
end DrawInBoundingBox;


(* export *)
procedure Draw(const d : Dialog);
begin
    DrawInBoundingBox(d, 0, 0, vdi.XMAXSCREEN, vdi.YMAXSCREEN);
end Draw;


(* export *)
procedure GetBoundingBox(
    const d : Dialog;
    var x0 : XCoord;
    var y0 : YCoord;
    var x1 : XCoord;
    var y1 : YCoord
);
begin
    picture.GetBoundingBox(d^.frame, x0, y0, x1, y1);
end GetBoundingBox;


procedure DrawText (
    const d : Dialog;
    const firstEraseChar : integer
);
    var left, right, eraseLeft : XCoord;
	bottom, top : YCoord;
begin
    picture.GetBoundingBox(d^.textBox, left, bottom, right, top);
    picture.ConvertToWorldCoord(d^.textBox, left, bottom);
    picture.ConvertToWorldCoord(d^.textBox, right, top);
    vdi.Writable(left, bottom, right-1, top);
    vdi.SetFont(d^.msgFont);

    if firstEraseChar # 0 then
	vdi.SetColors(vdi.WHITE, vdi.WHITE);
	vdi.SetPattern(vdi.solidPattern);
	if firstEraseChar > 1 then
	    eraseLeft := left + 3 + 
		vdi.StrWidth(d^.text[1:min(d^.count, firstEraseChar)]);
	else
	    eraseLeft := left + 3;
	end;
	vdi.FilledRectangle(eraseLeft, bottom+1, right-1, top-1);
    end;

    vdi.SetColors(vdi.BLACK, vdi.WHITE);
    vdi.SetRendering(vdi.PAINTFGBG);
    left := left + TEXTBOXVSPACE div 2;
    bottom := bottom + TEXTBOXVSPACE div 2;
    vdi.MoveTo(left, bottom);
    if d^.count # 0 then
	if firstEraseChar = 0 then
	    vdi.CharStr(d^.text[1:d^.count]);
	end;
	vdi.GetPosition(left, bottom);
	left := left + vdi.StrWidth(d^.text[1:d^.count]) + 1;
	vdi.MoveTo(left, bottom);
    end;
    vdi.LineTo(left, bottom + d^.fontHt-1);
    vdi.AllWritable();
end DrawText;


(* export *)
procedure UpdateText(
    const d : Dialog;
    const c : char
);
begin
    if c in CharSet{BACKSPACE, DELETE} then
	if d^.count # 0 then
	    dec(d^.count);
	    DrawText(d, d^.count+1);
	end;
    elsif c = DELETELINE then
	ResetInputString(d);
	DrawText(d, 1);
    else
	inc(d^.count);
	d^.text[d^.count] := c;
	DrawText(d, 0);
    end;
end UpdateText;


(* export *)
procedure GetInputString(
    const d : Dialog;
    var c : array of char
);
    var i : integer;
begin
    for i := 1 to d^.count do
	c[i-1] := d^.text[i];
    end;
    i := d^.count;	(* hack to please the optimizer *)
    c[i] := NULL;
end GetInputString;


(* export *)
procedure ResetInputString(const d : Dialog);
begin
    d^.count := 0;
end ResetInputString;


begin

end dialog.
