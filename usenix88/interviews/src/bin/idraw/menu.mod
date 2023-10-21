implementation module menu;

import
    picture,
    genlists,
    vdi;

(* export *)
from system import
    Word;
    
from system import
    Byte;
    
from utilities import
    PointInBox;

(* export *)
from picture import
    Picture,
    XCoord,
    YCoord;

(* export Picture; *)
(* export XCoord; *)
(* export YCoord; *)

(* export type Menu; *)
(* export type
    Positioning = (Below, Above, LeftSide, RightSide, FreePositioning);
 *)
(* export type
    Alignment = (
        FlushLeft, FlushRight, FlushTop, FlushBottom, 
	HorizCentered, VertCentered,  FreeAlignment
    );
 *)

(* export type
    HighlightType = (Invert, Outline, None);
 *)

(* export type
    MenuEntryHandler = pointer to procedure(Word);
 *)
    
type
    Menu = pointer to MenuRecord;
    MenuRecord = record
        handler	: MenuEntryHandler;
	pict : Picture;
	parentEntryDwg : Picture;
	positioning : Positioning;
	alignment : Alignment;
	numEntries : cardinal;
        entries : genlists.GenericList;
	highlightType : HighlightType;
	destructive : boolean;
	menuCached : boolean;
	pixmap, menuPixmap : vdi.Pixmap;
	userData : Word;
    end;
    


(* export *)
procedure Create(var m : Menu);
begin
    new(m);
    m^.handler := nil;
    m^.pict := Picture(nil);
    m^.parentEntryDwg := Picture(nil);
    m^.numEntries := 0;
    m^.entries := genlists.GenericList(nil);
    m^.destructive := false;
    m^.menuCached := false;
end Create;


(* export *)
procedure SetDestructive (m : Menu; toggle : boolean);
    var i : genlists.Iterator;
        entry : Menu;
begin
    if (m = Menu(nil)) or (m^.pict = Picture(nil)) then
        return;
    end;

    m^.destructive := toggle;
    i := genlists.BeginIteration(m^.entries);
    while genlists.MoreElements(i, entry) do
        SetDestructive(entry, toggle);
    end;
    genlists.EndIteration(i);
end SetDestructive;

(* export *)
procedure Destructive (const m : Menu) : boolean;
begin
    return m^.destructive;
end Destructive;


(* export *)
procedure SetUserData(
    const m : Menu;
    const data : Word
);
begin
    if m # Menu(nil) then
        m^.userData := data;
    end;
end SetUserData;


(* export *)
procedure GetUserData(const m : Menu) : Word;
begin
    if m # Menu(nil) then
        return m^.userData;
    else
        return Word(m);
    end;
end GetUserData;    


(* export *)
procedure SetLayout(
    const m : Menu;
    const positioning : Positioning;
    const alignment : Alignment
);
begin
    if m # Menu(nil) then
        m^.positioning := positioning;
        m^.alignment := alignment;
    end;
end SetLayout;


(* export *)
procedure GetLayout(
    const m : Menu;
    var   positioning : Positioning;
    var   alignment : Alignment
);
begin
    if m # Menu(nil) then
        positioning := m^.positioning;
	alignment := m^.alignment;
    end;
end GetLayout;    


(* export *)
procedure Destroy(const m : Menu);
    var i : genlists.Iterator;
    	entry : Menu;
begin
    if m # Menu(nil) then
        picture.Destroy(m^.pict);
        if m^.entries # genlists.GenericList(nil) then
            i := genlists.BeginIteration(m^.entries);
	    while genlists.MoreElements(i, entry) do
	        Destroy(entry);
    	    end;
	    genlists.EndIteration(i);
            genlists.Release(m^.entries);
	end;
    end;
end Destroy;


(* export *)
procedure AppendEntry(
    const m : Menu;
    var pict : Picture;
    const highlightType : HighlightType;
    const handler : MenuEntryHandler;
    const userData : Word
) : Menu;
    var entry : Menu;
	posRef, alignRef : Picture;
	autoLayout : boolean;
begin
    if m = Menu(nil) then
        return m;
    end;

    Create(entry);
    entry^.highlightType := highlightType;
    entry^.handler := handler;
    entry^.userData := userData;

    if (m^.positioning # FreePositioning) and
        (m^.alignment # FreeAlignment) then
        autoLayout := true;
    else
        autoLayout := false;
    end;

    if m^.entries = genlists.GenericList(nil) then  (* => no entries *)
        genlists.Create(m^.entries);
	picture.Create(m^.pict);
(*
 *  If the entry is the first one in the menu, then we base its POSITIONING
 *  on the parent menu of the menu containing the entry.  We base its 
 *  ALIGNMENT on the menu containing the entry.
 *)
	if (m^.parentEntryDwg # Picture(nil)) and autoLayout then
	    posRef := picture.GetParent(m^.parentEntryDwg);
	    if posRef = Picture(nil) then
	        posRef := m^.parentEntryDwg;
	    end;
	    alignRef := m^.parentEntryDwg;
	else (*  assume free orientation *)
	    autoLayout := false;
	end;
    elsif autoLayout then  (* entries exist on which to base layout *)
	posRef := m^.pict;
	alignRef := m^.pict;
    end;

    if  autoLayout then
	case m^.positioning of
	    Below:
		picture.AlignTopToBottom(posRef, pict);
	        |
	    Above:
		picture.AlignBottomToTop(posRef, pict);
		|
	    LeftSide:
		picture.AlignRightToLeft(posRef, pict);
		|
	    RightSide:
		picture.AlignLeftToRight(posRef, pict);
	end;

	case m^.alignment of
	    FlushLeft:
		picture.AlignLeftSides(alignRef, pict);
		|
	    FlushRight:
		picture.AlignRightSides(alignRef, pict);
		|
	    FlushTop:
		picture.AlignTops(alignRef, pict);
		|
	    FlushBottom:
		picture.AlignBottoms(alignRef, pict);
		|
	    HorizCentered:
		picture.AlignHorizCenters(alignRef, pict);
		|
	    VertCentered:
		picture.AlignVertCenters(alignRef, pict);
	end;
    end;
    entry^.parentEntryDwg := pict;
    picture.Nest(pict, m^.pict);
    genlists.Append(entry, m^.entries);        
    m^.numEntries := m^.numEntries + 1;
    return entry;
end AppendEntry;


(* export *)
procedure GetPicture(const m : Menu) : Picture;
begin
    if m = Menu(nil) then
        return Picture(nil);
    else
        return m^.pict;
    end;
end GetPicture;


(* export *)
procedure SetPicture(
    const m : Menu;
    const pict : Picture
);
begin 
    if m # Menu(nil) then
        m^.pict := pict;
	if m^.menuCached then
	    vdi.DestroyPixmap(m^.menuPixmap);
	    m^.menuCached := false;
	end;
    end;
end SetPicture;    


(* export *)
procedure GetHighlightType(const m : Menu) : HighlightType;
begin
    if m = Menu(nil) then
        return Invert;
    else
        return m^.highlightType;
    end;
end GetHighlightType;


(* export *)
procedure SetHighlightType(
    const m : Menu;
    const highlightType : HighlightType
);
begin
    if m # Menu(nil) then
        m^.highlightType := highlightType;
    end;
end SetHighlightType;


(* export *)
procedure GetHandler(const m : Menu) : MenuEntryHandler;
begin
    if m = Menu(nil) then
        return MenuEntryHandler(nil);
    else
        return m^.handler;
    end;
end GetHandler;


(* export *)
procedure SetHandler(
    const m : Menu;
    const handler : MenuEntryHandler
);
begin
    if m # Menu(nil) then
        m^.handler := handler;
    end;
end SetHandler;


(* export *)
procedure GetEntryByCoord(
    const m : Menu;
    const x : XCoord;
    const y : YCoord
) : Menu;
    var entry : Menu;
        pict : Picture;
        number : integer;
	i : genlists.Iterator;
	found, dummy : boolean;
	left, right : XCoord;
	bottom, top : YCoord;
begin
    if m = Menu(nil) then
        return m;
    end;

    number := 1;
    found := false;
    i := genlists.BeginReverseIteration(m^.entries);
    while 
        not found and 
        genlists.PrevElement(i, entry) and 
	(number <= m^.numEntries) 
    do
        pict := picture.GetNestedPicture(m^.pict, -number);
	picture.GetBoundingBox(pict, left, bottom, right, top);
	found := PointInBox(x, y, left, bottom, right, top);
	if found then
	    return entry;
	end;
	number := number + 1;
    end;
    genlists.EndIteration(i);
    return Menu(nil);
end GetEntryByCoord;


(* export *)
procedure Execute(
    const m : Menu;
    const parameter : Word
);
begin
    if (m # Menu(nil)) and (m^.handler # nil) then
        m^.handler^(parameter);
    end;
end Execute;


(* export *)
procedure GetEntry(
    const m : Menu;
    const number : integer
) : Menu;
    var i : genlists.Iterator;
        entry : Menu;
        n : integer;
begin
    if m = Menu(nil) then
        return m;
    end;

    n := 1;
    i := genlists.BeginIteration(m^.entries);
    while (n <= number) and genlists.MoreElements(i, entry) do
    	if n = number then
	    return entry;
	else
            n := n + 1;
	end;
    end;
    genlists.EndIteration(i);
    return Menu(nil);
end GetEntry;


(* export *)
procedure Draw(const m : Menu);
begin
    picture.Draw(m^.pict);
end Draw;


(* export *)
procedure DrawInBoundingBox(
    const m : Menu;
    const left : XCoord;
    const bottom : YCoord;
    const right : XCoord;
    const top : YCoord
);
begin
    picture.DrawInBoundingBox(m^.pict, left, bottom, right, top);
end DrawInBoundingBox;


(* export *)
procedure Redraw(const m : Menu);
begin
    picture.Redraw(m^.pict);
end Redraw;


(* export *)
procedure DisableRedraw(const m : Menu);
begin
    picture.DisableRedraw(m^.pict);
end DisableRedraw;


(* export *)
procedure EnableRedraw(const m : Menu);
begin
    picture.EnableRedraw(m^.pict);
end EnableRedraw;


(* export *)
procedure Expose(const m : Menu);
    var left, right : XCoord;
        bottom, top : YCoord;
begin
    if m = Menu(nil) then
        return;
    end;

    picture.GetBoundingBox(m^.pict, left, bottom, right, top);
    picture.ConvertToWorldCoord(m^.pict, left, bottom);
    picture.ConvertToWorldCoord(m^.pict, right, top);
    left := min(max(0, left), vdi.XMAXSCREEN-1);
    bottom := min(max(0, bottom), vdi.YMAXSCREEN-1);
    right := min(max(0, right), vdi.XMAXSCREEN-1);
    top := min(max(0, top), vdi.YMAXSCREEN-1);
    vdi.SetRendering(vdi.PAINTFGBG);
    vdi.SetColors(vdi.BLACK, vdi.WHITE);

    if not m^.destructive then
	m^.pixmap := vdi.CreatePixmap(
	    integer(right - left) + 1, integer(top - bottom) + 1
	);
	vdi.SaveArea(m^.pixmap, left, bottom, right, top);
    end;
    
    if m^.menuCached then
        vdi.RestoreArea(m^.menuPixmap, left, bottom, right, top);
    else
	Draw(m);    
	m^.menuPixmap := vdi.CreatePixmap(
	    integer(right - left) + 1, integer(top - bottom) + 1
	);
	vdi.SaveArea(m^.menuPixmap, left, bottom, right, top);
	m^.menuCached := true;
    end;
    vdi.Sync();
end Expose;


(* export *)
procedure Hide(const m : Menu);
    var left, right : XCoord;
        bottom, top : YCoord;
begin
    if (m # Menu(nil)) and not m^.destructive then
        picture.GetBoundingBox(m^.pict, left, bottom, right, top);
        picture.ConvertToWorldCoord(m^.pict, left, bottom);
        picture.ConvertToWorldCoord(m^.pict, right, top);
	left := min(max(0, left), vdi.XMAXSCREEN-1);
	bottom := min(max(0, bottom), vdi.YMAXSCREEN-1);
	right := min(max(0, right), vdi.XMAXSCREEN-1);
	top := min(max(0, top), vdi.YMAXSCREEN-1);
 	vdi.SetRendering(vdi.PAINTFGBG);
	vdi.SetColors(vdi.BLACK, vdi.WHITE);
        vdi.RestoreArea(m^.pixmap, left, bottom, right, top);
        vdi.DestroyPixmap(m^.pixmap);
	vdi.Sync();
    end;
end Hide;


(* export *)
procedure Highlight(const m : Menu);
    var left, right : XCoord;
        bottom, top : YCoord;
begin
    if 
        (m # Menu(nil)) and 
	(m^.parentEntryDwg # Picture(nil)) and
	(m^.highlightType # None)
    then
        picture.GetBoundingBox(m^.parentEntryDwg, left, bottom, right, top);
	picture.ConvertToWorldCoord(m^.parentEntryDwg, left, bottom);
	picture.ConvertToWorldCoord(m^.parentEntryDwg, right, top);

	case m^.highlightType of
	    Invert:
	        vdi.InvertArea(left, bottom, right, top);
		|
	    Outline:
	        vdi.SetRendering(vdi.INVERT);
		vdi.Rectangle(left, bottom, right, top);
	    else
	end;
	vdi.Sync();
    end;
end Highlight;


(* export *)
procedure HighlightInBoundingBox(
    const m : Menu;
    const left : XCoord;
    const bottom : YCoord;
    const right : XCoord;
    const top : YCoord
);
    var x0, x1 : XCoord;
        y0, y1 : YCoord;
begin
    vdi.Writable(left, bottom, right, top);
    Highlight(m);
    vdi.AllWritable();
end HighlightInBoundingBox;


(* export *)
procedure Translate(
    const m : Menu;
    const dx : XCoord;
    const dy : YCoord
);
    var i : genlists.Iterator;
        entry : Menu;
begin
    if (m = Menu(nil)) or (m^.pict = Picture(nil)) then
        return;
    end;

    picture.Translate(m^.pict, float(dx), float(dy));
    i := genlists.BeginIteration(m^.entries);
    while genlists.MoreElements(i, entry) do
        Translate(entry, dx, dy);
    end;
    genlists.EndIteration(i);
end Translate;


(* export *)
procedure SetOrigin(
    const m : Menu;
    const x : XCoord;
    const y : YCoord
);
    var i : genlists.Iterator;
        entry : Menu;
begin
    if (m = Menu(nil)) or (m^.pict = Picture(nil)) then
        return;
    end;

    picture.SetOrigin(m^.pict, x, y);
    i := genlists.BeginIteration(m^.entries);
    while genlists.MoreElements(i, entry) do
        SetOrigin(entry, x, y);
    end;
    genlists.EndIteration(i);
end SetOrigin;


(* export *)
procedure Uncache(const m : Menu);
    var i : genlists.Iterator;
        entry : Menu;
begin
    if (m = Menu(nil)) or (m^.pict = Picture(nil)) then
        return;
    end;

    if m^.menuCached then
	vdi.DestroyPixmap(m^.menuPixmap);
	m^.menuCached := false;
    end;
    i := genlists.BeginIteration(m^.entries);
    while genlists.MoreElements(i, entry) do
        Uncache(entry);
    end;
    genlists.EndIteration(i);
end Uncache;


(* export *)
procedure AddBorder(const m : Menu);
    var left, right : XCoord;
        bottom, top : YCoord;
	border : Picture;
begin
    if (m = Menu(nil)) or (m^.pict = Picture(nil)) then
        return;
    end;
    
    picture.GetBoundingBox(m^.pict, left, bottom, right, top);
    picture.Create(border);
    picture.DefaultRenderingStyle(border, picture.Stroke);
    picture.Rectangle(border, left - 1, bottom - 1, right + 1, top + 1);
    picture.Nest(border, m^.pict);
    picture.SendToBack(m^.pict, border);
end AddBorder;


(* export *)
procedure AddShadow(const m : Menu);
    var left, right : XCoord;
        bottom, top : YCoord;
	shadow : Picture;
begin
    if (m = Menu(nil)) or (m^.pict = Picture(nil)) then
        return;
    end;
    
    picture.GetBoundingBox(m^.pict, left, bottom, right, top);
    picture.Create(shadow);
    picture.DefaultRenderingStyle(shadow, picture.Stroke);
    picture.MoveTo(shadow, left + 1, bottom - 1);
    picture.LineTo(shadow, right + 1, bottom - 1);
    picture.LineTo(shadow, right + 1, top - 1);
    picture.Nest(shadow, m^.pict);
    picture.SendToBack(m^.pict, shadow);
end AddShadow;


end menu.
