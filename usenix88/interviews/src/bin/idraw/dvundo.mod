implementation module dvundo;

import
    picture,
    dvselect;

(* export *)
from dvdefs import
    DrawingView, UndoObj, ModifOp, SelecInfo;

from dvdefs import
    UndoOp, UndoOpSet, Relation, UndoObjRecord, SelecInfoRecord;

from dvupict import
    DrawInClipBox, RedrawInClipBox, DrawPageInClipBox, ClearAndDrawInClipBox,
    ClipToClipBox, CenterPage, CenterPointOnPage, CalcCenterPictClipBox,
    GetNearestGridPoint;

from dvselect import
    CreateRubberbands, HighlightSelections, DeleteRubberbands,
    RecalcRubberbands, QuickReleaseSelections, SelectAll,
    ReleaseSelections, BringSelectionsToFront, SendSelectionsToBack,
    DisableSelections, EnableSelections, CopySelectionList,
    TranslateSelections, ScaleSelections, StretchSelections, RotateSelections,
    AlignSelectionsToGrid;

from dvwindops import
    MakeDirty;

from vdi import
    AllWritable;

from input import
    hourglassCursor, GetCursor, SetCursor;

(* export *)
from system import
    Word;

(* export *)
from picture import
    Picture, XCoord, YCoord;

(* export *)
from genlists import
    GenericList, Iterator, BeginIteration, BeginReverseIteration,
    MoreElements, PrevElement, Create,
    EndIteration, GetElement, Head, Tail, Append, Prepend, Insert,
    Delete, DeleteCur, Release;

from math import
    ldexp;

from util import
    round;

var
    undoObjFreelist, selecInfoFreelist : GenericList;


(* export *)
procedure AllocUndoObj(var undoObj : UndoObj);
begin
    undoObj := UndoObj(Head(undoObjFreelist));
    if undoObj = UndoObj(nil) then
        new(undoObj);
    else
        Delete(undoObj, undoObjFreelist);
    end;
    undoObj^.selecList := GenericList(nil);
end AllocUndoObj;


(* export *)
procedure AllocSelecInfo(var selecInfo : SelecInfo);
begin
    selecInfo := SelecInfo(Head(selecInfoFreelist));
    if selecInfo = SelecInfo(nil) then
        new(selecInfo);
    else
        Delete(selecInfo, selecInfoFreelist);
    end;
end AllocSelecInfo;


(* export *)
procedure DeallocUndoObj(var u : UndoObj);
begin
    Prepend(u, undoObjFreelist);
    u := UndoObj(nil);
end DeallocUndoObj;


(* export *)
procedure DeallocSelecInfo(var selecInfo : SelecInfo);
begin
    Prepend(selecInfo, selecInfoFreelist);
    selecInfo := SelecInfo(nil);
end DeallocSelecInfo;


procedure ResetSelecList(const view : DrawingView);
    var u : UndoObj;
	i : Iterator;
	p : Picture;
	s : SelecInfo;
begin
    u := view^.stateInfo.undoInfo;
    if 
	(u^.undoOp = Deletion) and (u^.selecList # view^.stateInfo.selecList)
    then
	i := BeginIteration(u^.selecList);
	while MoreElements(i, p) do
	    DeleteCur(i);
	    picture.Destroy(p);
	end;
	EndIteration(i);
	Release(u^.selecList)
    elsif u^.selecList # view^.stateInfo.selecList then
	i := BeginIteration(u^.selecList);
	while MoreElements(i, p) do
	    DeleteCur(i);
	end;
	EndIteration(i);
	Release(u^.selecList);
    else
	u^.selecList := GenericList(nil);
    end;
end ResetSelecList;


procedure ResetInfoList(const view : DrawingView);
    var u : UndoObj;
	i : Iterator;
	p : Picture;
	s : SelecInfo;
begin
    u := view^.stateInfo.undoInfo;
    if u^.infoList # GenericList(nil) then
	if
	    u^.undoOp in 
	    UndoOpSet{Alignment, SendToBack, BringToFront, Group, Ungroup}
	then
	    i := BeginIteration(u^.infoList);
	    while MoreElements(i, s) do
		DeleteCur(i);
		DeallocSelecInfo(s);
	    end;
	    EndIteration(i);
	    Release(u^.infoList);
	elsif u^.undoOp = Modification then
	    i := BeginIteration(u^.infoList);
	    while MoreElements(i, p) do
		DeleteCur(i);
		picture.Destroy(p);
	    end;
	    EndIteration(i);
	    Release(u^.infoList);
	end;
    end;
end ResetInfoList;


(* export *)
procedure ResetLog (const view : DrawingView);
    var u : UndoObj;
begin
    u := view^.stateInfo.undoInfo;
    if (u # UndoObj(nil)) and (u^.selecList # GenericList(nil)) then
	ResetSelecList(view);
	ResetInfoList(view);
    elsif u = UndoObj(nil) then
	AllocUndoObj(view^.stateInfo.undoInfo);
    end;
    u := view^.stateInfo.undoInfo;
    u^.zoom := view^.stateInfo.zoom;
    u^.undone := false;
end ResetLog;


(* export *)
procedure SelectionsLogged(const v : DrawingView) : boolean;
begin
    return 
	(v^.stateInfo.undoInfo # UndoObj(nil)) and
	(v^.stateInfo.undoInfo^.selecList = v^.stateInfo.selecList);
end SelectionsLogged;


(* export *)
procedure LogMove(
    const view : DrawingView;
    const dx, dy : real
);
begin
    if (dx # 0.0) or (dy # 0.0) then
	ResetLog(view);
	view^.stateInfo.undoInfo^.undoOp := Move;
	view^.stateInfo.undoInfo^.selecList := view^.stateInfo.selecList;
	view^.stateInfo.undoInfo^.dx := dx;
	view^.stateInfo.undoInfo^.dy := dy;
    end;
end LogMove;


(* export *)
procedure LogMoveVertex(
    const view : DrawingView;
    const selection : Picture;
    const vertex : integer;
    const vdx : XCoord;
    const vdy : YCoord
);
begin
    if (vdx # 0) or (vdy # 0) then
	ResetLog(view);
	view^.stateInfo.undoInfo^.undoOp := MoveVertex;
	view^.stateInfo.undoInfo^.selecList := view^.stateInfo.selecList;
	view^.stateInfo.undoInfo^.selection := selection;
	view^.stateInfo.undoInfo^.vertex := vertex;
	view^.stateInfo.undoInfo^.vdx := vdx;
	view^.stateInfo.undoInfo^.vdy := vdy;
    end;
end LogMoveVertex;


(* export *)
procedure LogScale(
    const view : DrawingView;
    const sx, sy : real
);
begin
    if (sx # 1.0) or (sy # 1.0) then
	ResetLog(view);
	view^.stateInfo.undoInfo^.undoOp := Scale;
	view^.stateInfo.undoInfo^.selecList := view^.stateInfo.selecList;
	view^.stateInfo.undoInfo^.sx := sx;
	view^.stateInfo.undoInfo^.sy := sy;
    end;
end LogScale;


(* export *)
procedure LogStretch(
    const view : DrawingView;
    const stx, sty : real;
    const codex, codey : integer
);
begin
    if (stx # 1.0) or (sty # 1.0) then
	ResetLog(view);
	view^.stateInfo.undoInfo^.undoOp := Stretch;
	view^.stateInfo.undoInfo^.selecList := view^.stateInfo.selecList;
	view^.stateInfo.undoInfo^.stx := stx;
	view^.stateInfo.undoInfo^.sty := sty;
	view^.stateInfo.undoInfo^.codex := codex;
	view^.stateInfo.undoInfo^.codey := codey;
    end;
end LogStretch;


(* export *)
procedure LogRotate(
    const view : DrawingView;
    const angle : real
);
begin
    if (angle # 0.0) or (angle / 360.0 # 1.0) then
	ResetLog(view);
	view^.stateInfo.undoInfo^.undoOp := Rotate;
	view^.stateInfo.undoInfo^.selecList := view^.stateInfo.selecList;
	view^.stateInfo.undoInfo^.angle := angle;
    end;
end LogRotate;


(* export *)
procedure LogInsertion(const view : DrawingView);
begin
    ResetLog(view);
    view^.stateInfo.undoInfo^.undoOp := Insertion;
    view^.stateInfo.undoInfo^.selecList := view^.stateInfo.selecList;
end LogInsertion;


(* export *)
procedure LogDeletion(const view : DrawingView);
begin
    ResetLog(view);
    view^.stateInfo.undoInfo^.undoOp := Deletion;
    view^.stateInfo.undoInfo^.selecList := view^.stateInfo.selecList;
end LogDeletion;


(* export *)
procedure LogAlignment(
    const view : DrawingView;
    const dx, dy : real
);
    var selecInfo : SelecInfo;
begin
    if view^.stateInfo.undoInfo^.selecList = GenericList(nil) then
	view^.stateInfo.undoInfo^.undoOp := Alignment;
	Create(view^.stateInfo.undoInfo^.infoList);
	view^.stateInfo.undoInfo^.selecList := view^.stateInfo.selecList;
    end;
    AllocSelecInfo(selecInfo);
    selecInfo^.undoOp := Alignment;
    selecInfo^.dx := dx;
    selecInfo^.dy := dy;
    Append(selecInfo, view^.stateInfo.undoInfo^.infoList);
end LogAlignment;


(* export *)
procedure LogModification(
    const view : DrawingView;
    const cmd : ModifOp;
    const curpict : Picture
);
    var undopict : Picture;
begin
    if view^.stateInfo.undoInfo^.selecList = GenericList(nil) then
	view^.stateInfo.undoInfo^.undoOp := Modification;
	Create(view^.stateInfo.undoInfo^.infoList);
	view^.stateInfo.undoInfo^.selecList := view^.stateInfo.selecList;
    end;
    undopict := picture.Copy(curpict);
    picture.Disable(undopict);
    picture.Insert(undopict, curpict);
    Append(undopict, view^.stateInfo.undoInfo^.infoList);
end LogModification;


procedure CalcRelatives(
    const view : DrawingView;
    const undoOp : UndoOp
);
    var i : Iterator;
	p, sib : Picture;
	selecInfo : SelecInfo;
begin
    i := BeginIteration(view^.stateInfo.undoInfo^.selecList);
    while MoreElements(i, p) do
	sib := picture.GetNextSibling(p);
	AllocSelecInfo(selecInfo);
	selecInfo^.undoOp := undoOp;
	if sib = Picture(nil) then
	    selecInfo^.relative := picture.GetParent(p);
	    selecInfo^.relation := Parent;
	else
	    selecInfo^.relative := sib;
	    selecInfo^.relation := Sibling;
	end;
	Append(selecInfo, view^.stateInfo.undoInfo^.infoList);
    end;
    EndIteration(i);
end CalcRelatives;


(* export *)
procedure LogSendToBack(const view : DrawingView);
begin
(*
 * Assume log has been reset prior to ordering of selections, to avoid
 * double clobbering of selection list.
 *)
    view^.stateInfo.undoInfo^.undoOp := SendToBack;
    view^.stateInfo.undoInfo^.selecList := view^.stateInfo.selecList;
    Create(view^.stateInfo.undoInfo^.infoList);
    CalcRelatives(view, SendToBack);
end LogSendToBack;


(* export *)
procedure LogBringToFront(const view : DrawingView);
begin
(*
 * Assume log has been reset prior to ordering of selections, to avoid
 * double clobbering of selection list.
 *)
    view^.stateInfo.undoInfo^.undoOp := BringToFront;
    view^.stateInfo.undoInfo^.selecList := view^.stateInfo.selecList;
    Create(view^.stateInfo.undoInfo^.infoList);
    CalcRelatives(view, BringToFront);
end LogBringToFront;


(* export *)
procedure LogGroup(const view : DrawingView);
begin
(*
 * Assume log has been reset prior to ordering of selections, to avoid
 * double clobbering of selection list.
 *)
    view^.stateInfo.undoInfo^.undoOp := Group;
    view^.stateInfo.undoInfo^.selecList := view^.stateInfo.selecList;
    Create(view^.stateInfo.undoInfo^.infoList);
    CalcRelatives(view, Group);
end LogGroup;


procedure CalcNumsInGroups(
    const view : DrawingView;
    const undoOp : UndoOp
);
    var i : Iterator;
	parent : Picture;
	selecInfo : SelecInfo;
begin
    i := BeginIteration(view^.stateInfo.undoInfo^.selecList);
    while MoreElements(i, parent) do
	AllocSelecInfo(selecInfo);
	selecInfo^.undoOp := undoOp;
	selecInfo^.numInGroup := 0;
	if not picture.ContainsPrimitives(parent) then
            selecInfo^.numInGroup := picture.NumberOfNestedPictures(parent);
	end;
	Append(selecInfo, view^.stateInfo.undoInfo^.infoList);
    end;
    EndIteration(i);
end CalcNumsInGroups;


(* export *)
procedure LogUngroup(const view : DrawingView);
begin
    ResetLog(view);
    view^.stateInfo.undoInfo^.undoOp := Ungroup;
    view^.stateInfo.undoInfo^.selecList := view^.stateInfo.selecList;
    Create(view^.stateInfo.undoInfo^.infoList);
    CalcNumsInGroups(view, Ungroup);
end LogUngroup;


(* export *)
procedure LogShowStructure(const view : DrawingView);
begin
    ResetLog(view);
    view^.stateInfo.undoInfo^.undoOp := ShowStructure;
    view^.stateInfo.undoInfo^.selecList := CopySelectionList(view);
end LogShowStructure;


procedure ZoomCorrection(const view : DrawingView) : real;
begin
    return ldexp(1.0, view^.stateInfo.zoom - view^.stateInfo.undoInfo^.zoom);
end ZoomCorrection;


procedure UndoMove(const view : DrawingView);
    var u : UndoObj;
	zc : real;
begin
    u := view^.stateInfo.undoInfo;
    view^.stateInfo.selecList := u^.selecList;
    zc := ZoomCorrection(view);
    u^.dx := zc * u^.dx;
    u^.dy := zc * u^.dy;
    TranslateSelections(view, -u^.dx, -u^.dy);
    LogMove(view, -u^.dx, -u^.dy);
    RedrawInClipBox(view^.pict, view);
end UndoMove;


procedure UndoMoveVertex(const view : DrawingView);
    var u : UndoObj;
	zc : real;
begin
    u := view^.stateInfo.undoInfo;
    view^.stateInfo.selecList := u^.selecList;
    zc := ZoomCorrection(view);
    u^.vdx := round(zc * float(u^.vdx));
    u^.vdy := round(zc * float(u^.vdy));
    picture.MoveVertex(u^.selection, 1, u^.vertex, -u^.vdx, -u^.vdy);
    if picture.GetPrimitiveType(u^.selection, 2) # picture.None then
	picture.MoveVertex(u^.selection, 2, u^.vertex, -u^.vdx, -u^.vdy);
    end;
    LogMoveVertex(view, u^.selection, u^.vertex, -u^.vdx, -u^.vdy);
    RedrawInClipBox(view^.pict, view);
end UndoMoveVertex;


procedure UndoScale(const view : DrawingView);
    var u : UndoObj;
	isx, isy : real;
begin
    u := view^.stateInfo.undoInfo;
    view^.stateInfo.selecList := u^.selecList;
    isx := 1.0 / u^.sx;
    isy := 1.0 / u^.sy;
    ScaleSelections(view, isx, isy);
    LogScale(view, isx, isy);
    RedrawInClipBox(view^.pict, view);
end UndoScale;


procedure UndoStretch(const view : DrawingView);
    var u : UndoObj;
	isx, isy : real;
begin
    u := view^.stateInfo.undoInfo;
    view^.stateInfo.selecList := u^.selecList;
    isx := 1.0 / u^.sx;
    isy := 1.0 / u^.sy;
    if isx < 0.0 then
	u^.codex := 2 - u^.codex;   (* "invert" code if stretch was negative *)
    end;
    if isy < 0.0 then
	u^.codey := 2 - u^.codey;   (* "invert" code if stretch was negative *)
    end;
    StretchSelections(view, isx, isy, u^.codex, u^.codey);
    LogStretch(view, isx, isy, u^.codex, u^.codey);
    RedrawInClipBox(view^.pict, view);
end UndoStretch;


procedure UndoRotate(const view : DrawingView);
    var u : UndoObj;
begin
    u := view^.stateInfo.undoInfo;
    view^.stateInfo.selecList := u^.selecList;
    RotateSelections(view, -u^.angle);
    LogRotate(view, -u^.angle);
    RedrawInClipBox(view^.pict, view);
end UndoRotate;


procedure UndoInsertion(const view : DrawingView);
    var u : UndoObj;
begin
    u := view^.stateInfo.undoInfo;
    view^.stateInfo.selecList := u^.selecList;
    DisableSelections(view);
    LogDeletion(view);
    Create(view^.stateInfo.selecList);
    RedrawInClipBox(view^.pict, view);
end UndoInsertion;


procedure UndoDeletion(const view : DrawingView);
    var u : UndoObj;
begin
    u := view^.stateInfo.undoInfo;
    view^.stateInfo.selecList := u^.selecList;
    EnableSelections(view);
    LogInsertion(view);
    RedrawInClipBox(view^.pict, view);
end UndoDeletion;


procedure UndoAlignment(const view : DrawingView);
    var iSelecs, iInfo : Iterator;
	p : Picture;
	info : SelecInfo;
	zc : real;
begin
    view^.stateInfo.selecList := view^.stateInfo.undoInfo^.selecList;
    zc := ZoomCorrection(view);
    iSelecs := BeginIteration(view^.stateInfo.undoInfo^.selecList);
    iInfo := BeginIteration(view^.stateInfo.undoInfo^.infoList);
    while MoreElements(iSelecs, p) and MoreElements(iInfo, info) do
	info^.dx := -info^.dx * zc;
	info^.dy := -info^.dy * zc;
	picture.Translate(p, info^.dx, info^.dy);
    end;
    EndIteration(iInfo);
    EndIteration(iSelecs);
    RedrawInClipBox(view^.pict, view);
    view^.stateInfo.undoInfo^.zoom := view^.stateInfo.zoom;
end UndoAlignment;


procedure UndoModification(const view : DrawingView);
    var iSelecs, iInfo : Iterator;
	tmp : GenericList;
	undopict, curpict : Picture;
	info : SelecInfo;
begin
    iSelecs := BeginIteration(view^.stateInfo.undoInfo^.selecList);
    iInfo := BeginIteration(view^.stateInfo.undoInfo^.infoList);
    while MoreElements(iSelecs, curpict) and MoreElements(iInfo, undopict) do
	picture.Disable(curpict);
	picture.Enable(undopict);
    end;
    EndIteration(iInfo);
    EndIteration(iSelecs);
    RedrawInClipBox(view^.pict, view);
    with view^.stateInfo.undoInfo^ do
	tmp := selecList;
	selecList := infoList;
	infoList := tmp;
	view^.stateInfo.selecList := selecList;
    end;
end UndoModification;


procedure ReplaceRelatives(const view : DrawingView);
    var iSelecs, iInfo : Iterator;
	p : Picture;
	info : SelecInfo;
begin
    iSelecs := BeginReverseIteration(view^.stateInfo.undoInfo^.selecList);
    iInfo := BeginReverseIteration(view^.stateInfo.undoInfo^.infoList);
    while PrevElement(iSelecs, p) and PrevElement(iInfo, info) do
	picture.Unnest(p);
	if info^.relation = Parent then
	    picture.Nest(p, info^.relative);
	elsif info^.relation = Sibling then
	    picture.Insert(p, info^.relative);
	end;
	picture.Touch(p);
    end;
    EndIteration(iInfo);
    EndIteration(iSelecs);
end ReplaceRelatives;


procedure UndoSendToBack(const view : DrawingView);
    var u : UndoObj;
begin
    u := view^.stateInfo.undoInfo;
    view^.stateInfo.selecList := u^.selecList;
    if u^.undone then
	SendSelectionsToBack(view);
    else
	ReplaceRelatives(view);
	RedrawInClipBox(view^.pict, view);
    end;
end UndoSendToBack;


procedure UndoBringToFront(const view : DrawingView);
    var u : UndoObj;
begin
    u := view^.stateInfo.undoInfo;
    view^.stateInfo.selecList := u^.selecList;
    if u^.undone then
	BringSelectionsToFront(view);
    else
	ReplaceRelatives(view);
	RedrawInClipBox(view^.pict, view);
    end;
end UndoBringToFront;


procedure UndoGroup(const view : DrawingView);
    var parent : Picture;
	u : UndoObj;
begin
    u := view^.stateInfo.undoInfo;
    view^.stateInfo.selecList := u^.selecList;
    if u^.undone then
	picture.Create(parent);
	dvselect.Group(view, parent);
    else
	parent := picture.GetParent(Picture(Head(u^.selecList)));
	picture.Unnest(parent);
	ReplaceRelatives(view);
	RedrawInClipBox(view^.pict, view);
	picture.Destroy(parent);
    end;
end UndoGroup;


procedure RegroupSelections(const view : DrawingView);
    var iSelecs, iInfo : Iterator;
	p, parent : Picture;
	info : SelecInfo;
	u : UndoObj;
	i : integer;
	dummy : boolean;    (* should be an error check *)
begin
    u := view^.stateInfo.undoInfo;
    iInfo := BeginIteration(u^.infoList);
    iSelecs := BeginIteration(u^.selecList);
    while MoreElements(iInfo, info) do
	dummy := MoreElements(iSelecs, p);
	if info^.numInGroup = 0 then
	    parent := p;
	else
	    parent := picture.GetParent(p);
	    for i := 2 to info^.numInGroup do
		dummy := MoreElements(iSelecs, p);
	    end;
	end;
	Append(parent, view^.stateInfo.selecList);
    end;
    EndIteration(iSelecs);
    EndIteration(iInfo);
end RegroupSelections;


procedure Regroup(const view : DrawingView);
    var iSelecs, iInfo : Iterator;
	p, parent : Picture;
	info : SelecInfo;
	u : UndoObj;
	i : integer;
	dummy : boolean;    (* should be an error check *)
begin
    u := view^.stateInfo.undoInfo;
    iInfo := BeginIteration(u^.infoList);
    iSelecs := BeginIteration(u^.selecList);
    while MoreElements(iInfo, info) do
	dummy := MoreElements(iSelecs, p);
	if info^.numInGroup = 0 then
	    Append(p, view^.stateInfo.selecList);
	else
	    picture.Create(parent);
	    picture.Insert(parent, p);
	    picture.Unnest(p);
	    picture.Nest(p, parent);
	    for i := 2 to info^.numInGroup do
		dummy := MoreElements(iSelecs, p);
		picture.Unnest(p);
		picture.Nest(p, parent);
	    end;
	    Append(parent, view^.stateInfo.selecList);
	end;
    end;
    EndIteration(iSelecs);
    EndIteration(iInfo);
end Regroup;


procedure UndoUngroup(const view : DrawingView);
    var u : UndoObj;
begin
    u := view^.stateInfo.undoInfo;
    Create(view^.stateInfo.selecList);
    if u^.undone then
	RegroupSelections(view);
	dvselect.Ungroup(view);
    else
	Regroup(view);
    end;
end UndoUngroup;


(* export *)
procedure UndoShowStructure(const view : DrawingView);
    var u : UndoObj;
	tmp : GenericList;
begin
    u := view^.stateInfo.undoInfo;
    view^.stateInfo.selecList := u^.selecList;
    if u^.undone then
	view^.stateInfo.selecList := CopySelectionList(view);
	dvselect.ShowStructure(view);	(* creates its own rubberbands *)
    end;
end UndoShowStructure;


(* export *)
procedure UndoHandler(const w : Word);
    var view : DrawingView;
	origCursor : integer;
begin
    view := DrawingView(w);
    if 
	(view^.stateInfo.undoInfo = UndoObj(nil)) or
	(view^.stateInfo.undoInfo^.selecList = GenericList(nil))
    then
	return;
    end;
    origCursor := GetCursor();
    SetCursor(hourglassCursor);
    MakeDirty(view);

    if SelectionsLogged(view) then
	ClipToClipBox(view);
	DeleteRubberbands(view);
	AllWritable();
    elsif Picture(Head(view^.stateInfo.selecList)) # Picture(nil) then
	ClipToClipBox(view);
	ReleaseSelections(view);
	Release(view^.stateInfo.selecList);
	AllWritable();
    end;

    case view^.stateInfo.undoInfo^.undoOp of
	Move:		    UndoMove(view);		|
	MoveVertex:	    UndoMoveVertex(view);	|
	Scale:		    UndoScale(view);		|
	Stretch:	    UndoStretch(view);		|
	Rotate:		    UndoRotate(view);		|
	Insertion:	    UndoInsertion(view);	|
	Deletion:	    UndoDeletion(view);		|
	Alignment:	    UndoAlignment(view);	|
	Modification:	    UndoModification(view);	|
	SendToBack:	    UndoSendToBack(view);	|
	BringToFront:	    UndoBringToFront(view);	|
	Group:		    UndoGroup(view);		|
	Ungroup:	    UndoUngroup(view);		|
	ShowStructure:	    UndoShowStructure(view);    
    end;

    DrawPageInClipBox(view);
    ClipToClipBox(view);
    if 
	(view^.stateInfo.undoInfo^.undoOp # ShowStructure) or
	not view^.stateInfo.undoInfo^.undone
    then
	CreateRubberbands(view);
    end;
    HighlightSelections(view);
    AllWritable();
    SetCursor(origCursor);
    view^.stateInfo.undoInfo^.undone := not view^.stateInfo.undoInfo^.undone;
end UndoHandler;


begin
    Create(undoObjFreelist);
    Create(selecInfoFreelist);
end dvundo.
