implementation module alloc;

(* export *)
from main import
    Picture, XformPtr, BboxPtr;

from genlists import
    GenericList, Create, Head, Prepend, Delete;


var
    pictFreeList, xformFreeList, bboxFreeList : GenericList;


(* export *)
procedure AllocPicture(var pict : Picture);
begin
    pict := Picture(Head(pictFreeList));
    if pict = Picture(nil) then
        new(pict);
    else
        Delete(pict, pictFreeList);
    end;
end AllocPicture;


(* export *)
procedure AllocXform(var xform : XformPtr);
begin
    xform := XformPtr(Head(xformFreeList));
    if xform = XformPtr(nil) then
        new(xform);
    else
        Delete(xform, xformFreeList);
    end;
end AllocXform;


(* export *)
procedure AllocBbox(var bbox : BboxPtr);
begin
    bbox := BboxPtr(Head(bboxFreeList));
    if bbox = BboxPtr(nil) then
        new(bbox);
    else
        Delete(bbox, bboxFreeList);
    end;
end AllocBbox;


(* export *)
procedure DeallocPicture(var pict : Picture);
begin
    Prepend(pict, pictFreeList);
    pict := Picture(nil);
end DeallocPicture;


(* export *)
procedure DeallocXform(var xform : XformPtr);
begin
    Prepend(xform, xformFreeList);
    xform := XformPtr(nil);
end DeallocXform;


(* export *)
procedure DeallocBbox(var bbox : BboxPtr);
begin
    Prepend(bbox, bboxFreeList);
    bbox := BboxPtr(nil);
end DeallocBbox;


begin
    Create(pictFreeList);
    Create(xformFreeList);
    Create(bboxFreeList);
end alloc.
