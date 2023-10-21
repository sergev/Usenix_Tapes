implementation module dverror;


from io import
    terminal, Writef;

from dvdefs import
    BELL;

(* export *)
from dvdefs import
    DrawingView;

from dvwindops import
    BannerMsg;

from unix import
    fflush;

(* export const WRITEFILEERR = "couldn't write file "; *)
(* export const READFILEERR = "couldn't read file "; *)
(* export const PRINTERR = "couldn't write file "; *)


(* export *)
procedure ReportError(const moduleName, errorCode, param : array of char);
begin
    Writef(terminal, "%s:  %s%s\n", moduleName, errorCode, param);
end ReportError;


(* export *)
procedure ReportErrorBanner(
    const view : DrawingView;
    const errorCode, param : array of char
);
begin
    Writef(terminal, "%c", BELL);
    fflush(terminal);
    BannerMsg(view, errorCode, param);
end ReportErrorBanner;


begin
end dverror.
