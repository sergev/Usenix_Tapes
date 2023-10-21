
procedure
    cursor(on: boolean);
const
    cursor_mask = $2000;
    lo_register = 10;
    hi_register = 11;
var
    cursmode: integer absolute $0:$460;
    monitor: integer absolute $0:$463;
begin
    if not on then
        cursmode := cursmode or cursor_mask
    else
        cursmode := cursmode and not cursor_mask;
    port[monitor] := hi_register;
    port[monitor + 1] := hi(cursmode);
    port[monitor] := lo_register;
    port[monitor + 1] := lo(cursmode)
end;

(*
 * That's it!  DOS insures that 0000:0463 contains the number of the port where
 * the current monitor adapter (monochrome or CGA) resides; 0000:0460 is the
 * current cursor state (which scan lines and is it visible).
 *
 * Usage:
 *
 *	cursor(true);		turns cursor on
 *	cursor(false);		turns cursor off
 *
 *This function doesn't affect the scan lines used by the cursor, etc.
 *)
-- 
ihnp4!sun!cwruecmp!ncoast!allbery ncoast!allbery@Case.CSNET ncoast!tdi2!brandon
(ncoast!tdi2!root for business) 6615 Center St. #A1-105, Mentor, OH 44060-4101
Phone: +01 216 974 9210      CIS 74106,1032      MCI MAIL BALLBERY (part-time)
