!--> file EXEC.COM   ---------- ---------- ---------- --------->
is_verify = f$verify(is_debug_exec)

$! procedure EXEC (P1 : STRING  := "") is

on warning      then goto WHEN_WARNING
on error        then goto WHEN_ERROR
on severe_error then goto WHEN_SEVERE_ERROR
on control_y    then goto WHEN_CONTROL_Y

argument_string := 'p1'
command_line = 'argument_string
'command_line 'p2' 'p3' 'p4' 'p5' 'p6' 'p7' 'p8'
$
EXCEPTION: goto RETURN
WHEN_WARNING:
  vmx_put_error "exec: warning"
  exit
WHEN_ERROR:
  vmx_put_error "exec: error"
  exit
WHEN_SEVERE_ERROR:
  vmx_put_error "exec: severe_error"
  exit
WHEN_CONTROL_Y:
  vmx_put_error "exec: control_y"

$! end EXEC;

RETURN: is_verify = f$verify(is_verify)
!<-- end  ---------- ---------- ---------- ---------- <---------
