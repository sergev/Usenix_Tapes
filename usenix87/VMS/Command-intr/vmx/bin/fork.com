!--> file FORK.COM   ---------- ---------- ---------- --------->
is_verify = f$verify(is_debug_fork)

$! procedure FORK (P1 : STRING  := "";
$!                 P2 : NATURAL := 0) is

is_new_input  = false
is_new_output = false
is_to_fork    = false

on warning       then goto WHEN_WARNING
on error         then goto WHEN_ERROR
on severe_error  then goto WHEN_SEVERE_ERROR
on control_y     then goto WHEN_CONTROL_Y

argument_string := 'p1'
fork_line        = 'argument_string

second_argument  = 'p2
if p2 .eqs. "" then second_argument = "000"
count_string    := 'second_argument

initial_name  = "fork" + count_string
fork_out_file = initial_name + out_extension
fork_com_file = initial_name + vmx_extension

message_in    = "fork: <" + in_file
message_out   = "fork: >" + fork_out_file

LOOP:
  first_char = f$extract(0, 1, fork_line)
  is_special = first_char .eqs. "<" .or. first_char .eqs. ">" .or. -
               first_char .eqs. "&" .or. first_char .eqs. " "
  if .not. is_special then goto EXIT_LOOP

  if first_char .eqs. "<" then is_new_input  = true
  if first_char .eqs. ">" then is_new_output = true
  if first_char .eqs. "&" then is_to_fork    = true

  fork_line = f$extract(1, f$length(fork_line), fork_line)
  if first_char .eqs. "&" then goto EXIT_LOOP
END_LOOP: goto LOOP
EXIT_LOOP: if .not. is_to_fork then goto RETURN

if is_new_input  then vmx_put_line message_in
if is_new_output then vmx_put_line message_out

first_def  = "first_argument := " + """""""" + fork_line + """"""""
first_arg  = "line_arg        = 'first_argument"
second_def = "second_argument = " + count_string
second_arg = "count_arg      := 'second_argument"
shell_call = "vmx_shell line_arg count_arg"

open/write logical_file 'fork_com_file
     write logical_file first_def
     write logical_file first_arg
     write logical_file second_def
     write logical_file second_arg
     write logical_file shell_call
close      logical_file

fork_line := 'vmx_spawn
if is_new_input  then fork_line = fork_line + "/input="  + in_file
if is_new_output then fork_line = fork_line + "/output=" + fork_out_file
fork_line = fork_line + " @" + fork_com_file
vmx_exec fork_line
$
vmx_delete 'fork_com_file.*
$
EXCEPTION: goto RETURN
WHEN_WARNING:
  vmx_put_error "fork: warning"
  goto RETURN
WHEN_ERROR:
  vmx_put_error "fork: error"
  goto RETURN
WHEN_SEVERE_ERROR:
  vmx_put_error "fork: severe_error"
  goto RETURN
WHEN_CONTROL_Y:
  vmx_put_error "fork: control_y"

$! end FORK;

RETURN: is_verify = f$verify(is_verify)
!<-- end  ---------- ---------- ---------- ---------- <---------
