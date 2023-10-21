!--> file HISTORY.COM --------- ---------- ---------- --------->
is_verify = f$verify(is_debug_history)

$! procedure HISTORY (P1 : NATURAL) is

index = count - p1
if p1 .eqs. "" .or. p1 .gt. count then index = init_count - 1
if p1 .eqs. "" .and. count .ge. max_lines_for_hs - 1 then -
  index = count - max_lines_for_hs 
if p1 .nes. "" .and. 0  .ge. p1 then goto RETURN
  
LOOP_HISTORY:
  index = 1 + index
  history_string = tab + "when " + f$string(index) + -
    " => " + tab + string_array'index
  vmx_put_line history_string

  modulo = index - modulo_for_hs * (index / modulo_for_hs)
  if modulo .eq. modulo_for_hs - 1 then vmx_put_line ""
  if index .ge. count then goto RETURN
END_LOOP_HISTORY: goto LOOP_HISTORY

$! end HISTORY;

RETURN: is_verify = f$verify(is_verify)
!<-- end  ---------- ---------- ---------- ---------- <---------
