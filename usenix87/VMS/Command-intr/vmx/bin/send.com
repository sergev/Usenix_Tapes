!--> file SEND.COM   ---------- ---------- ---------- --------->
is_verify = f$verify(is_debug_send)

$! procedure SEND (P1 : STRING  := "";
$!                 P2 : NATURAL := 0) is

on warning       then goto WHEN_WARNING
on error         then goto WHEN_ERROR
on severe_error  then goto WHEN_SEVERE_ERROR
on control_y     then goto WHEN_CONTROL_Y

argument_string := 'p1'
send_line        = 'argument_string

second_argument  = 'p2
if p2 .eqs. "" then second_argument = "000"
count_string    := 'second_argument

is_to_send       = false
initial_name     = "send" + count_string
send_com_file    = initial_name + vmx_extension

LOOP:
  first_char = f$extract(0, 1, send_line)
  is_special = first_char .eqs. "^" .or. first_char .eqs. " "
  if .not. is_special then goto EXIT_LOOP

  if first_char .eqs. "^" then is_to_send = true

  send_line = f$extract(1, f$length(send_line), send_line)
END_LOOP: goto LOOP
EXIT_LOOP: if .not. is_to_send then goto RETURN

change_dir = "set def " + f$logical("sys$disk") + f$directory()
first_def  = "first_argument := " + """""""" + send_line + """"""""
first_arg  = "line_arg        = 'first_argument"
second_def = "second_argument = " + count_string
second_arg = "count_arg      := 'second_argument"
shell_call = "vmx_shell line_arg count_arg"

open/write logical_file 'send_com_file
     write logical_file change_dir
     write logical_file first_def
     write logical_file first_arg
     write logical_file second_def
     write logical_file second_arg
     write logical_file shell_call
close      logical_file

send_line := "vmx_nice ''send_com_file'"
vmx_exec send_line
$
EXCEPTION: goto RETURN
WHEN_WARNING:
  vmx_put_error "send: warning"
  goto RETURN
WHEN_ERROR:
  vmx_put_error "send: error"
  goto RETURN
WHEN_SEVERE_ERROR:
  vmx_put_error "send: severe_error"
  goto RETURN
WHEN_CONTROL_Y:
  vmx_put_error "send: control_y"

$! end SEND;

RETURN: is_verify = f$verify(is_verify)
!<-- end  ---------- ---------- ---------- ---------- <---------
