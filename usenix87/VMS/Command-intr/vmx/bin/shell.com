!--> file SHELL.COM  ---------- ---------- ---------- --------->
is_verify = f$verify(is_debug_shell)

$! procedure SHELL (P1 : STRING  := "";
$!                  P2 : BOOLEAN;
$!                  P3 : NATURAL := 0) is

on warning       then goto WHEN_WARNING
on error         then goto WHEN_ERROR
on severe_error  then goto WHEN_SEVERE_ERROR
on control_y     then goto WHEN_CONTROL_Y

shell_max_control_y = 3
control_y_count     = 0
shell_count         = -1

argument_string := 'p1'
if p2       then command_line  = 'argument_string
if .not. p2 then command_line := 'p1'

if p3 .nes. "" then second_argument = 'p3
if p3 .eqs. "" then second_argument = "0"
count_string    := 'second_argument

initial_name = count_string
message_in   = "shell: <" + in_file

LOOP_ON_SPACE:
  length         = f$length(command_line)
  position       = f$locate("\S", command_line)
  exists_space   = position .ne. length
  if .not. exists_space then goto EXIT_LOOP_ON_SPACE
  command_line  = f$extract(0, position, command_line) + " " + -
                  f$extract(2 + position, length, command_line)
END_LOOP_ON_SPACE: goto LOOP_ON_SPACE
EXIT_LOOP_ON_SPACE:

length_line  = f$length(command_line)

SEND_A_JOB:
  position = f$locate("^", command_line)
  is_a_job = position .ne. length_line 
  if is_a_job then vmx_send command_line count_string
  if is_a_job then goto RETURN

FORK_A_PROCESS:
  position     = f$locate("&", command_line)
  is_a_process = position .ne. length_line 
  if is_a_process then vmx_fork command_line count_string
  if is_a_process then goto RETURN

RUN_A_PIPE: 
  position  = f$locate("|", command_line)
  is_a_pipe = position .ne. length_line 
  if is_a_pipe then vmx_pipe command_line count_string
  if is_a_pipe then goto RETURN

LOOP:
  on warning       then goto WHEN_WARNING
  on error         then goto WHEN_ERROR
  on severe_error  then goto WHEN_SEVERE_ERROR
  on control_y     then goto WHEN_CONTROL_Y

  is_new_input  = false
  is_new_output = false
  is_new_error  = false
  is_a_job      = false

  shell_count   = 1 + shell_count
  prefix_file   = initial_name + "x" + f$string(shell_count)
  com_file      = prefix_file + vmx_extension
  out_file      = prefix_file + out_extension
  err_file      = prefix_file + err_extension

  length_line   = f$length(command_line)
  position      = f$locate(";", command_line)
  is_multiple   = position .ne. length_line  
  p = length_line - position - 1
  first_command = f$extract(0, position, command_line)
  command_line  = f$extract(1 + position, p, command_line)
  if first_command .eqs. "" then goto END_EXEC_COM

  INNER_LOOP:
    first_char = f$extract(0, 1, first_command)
    is_special = first_char .eqs. "<" .or. first_char .eqs. ">" .or. -
                 first_char .eqs. "?" .or. first_char .eqs. "+" .or. -
                 first_char .eqs. " "
    if .not. is_special then goto EXIT_INNER_LOOP

    if first_char .eqs. "<" then is_new_input  = true
    if first_char .eqs. ">" then is_new_output = true
    if first_char .eqs. "?" then is_new_error  = true
    if first_char .eqs. "+" then is_a_job      = true

    first_command = f$extract(1, f$length(first_command), first_command)
  END_INNER_LOOP: goto INNER_LOOP
  EXIT_INNER_LOOP:
  if .not. is_new_output .and. .not. is_a_job then goto STANDARD_OUTPUT

  REDIRECT_OUTPUT:
  change_dir = "set def " + f$logical("sys$disk") + f$directory()
  open/write logical_file 'com_file
       if is_a_job then write logical_file change_dir
       write logical_file first_command
  close      logical_file
  if is_new_output then first_command = "@" + com_file + "/out=" + out_file
  if is_a_job then first_command = "vmx_nice " + com_file

  STANDARD_OUTPUT:
  message_out = "shell: >" + out_file
  message_err = "shell: ?" + err_file

  if is_new_input  then deassign sys$input
  if is_new_input  then assign/user_mode in_file sys$input

  if is_new_input  then vmx_put_line message_in
  if is_new_output then vmx_put_line message_out
  if is_new_error  then vmx_put_line message_err

  if is_new_error  then open/write sys$error: 'err_file'
  if is_new_error  then define sys$error sys$error:

  EXEC_COM:
  vmx_exec first_command
$
  if is_new_error then deassign sys$error
  if is_new_error then close sys$error:
  if is_new_output .and. .not. is_a_job then vmx_delete 'com_file.*
$
  if .not. is_multiple then goto RETURN
  END_EXEC_COM:
  if command_line .eqs. "" then goto RETURN

  EXCEPTION: goto END_LOOP
  WHEN_WARNING:
    vmx_put_error "shell: warning"
    goto END_LOOP
  WHEN_ERROR:
    vmx_put_error "shell: error"
    goto END_LOOP
  WHEN_SEVERE_ERROR:
    vmx_put_error "shell: severe_error"
    goto END_LOOP
  WHEN_CONTROL_Y:
    control_y_string = "shell: control_y"
    control_y_count = 1 + control_y_count
    if control_y_count .eq. 1 then -
      control_y_string = control_y_string + " (only " + -
      shell_max_control_y + " allowed)"
    if control_y_count .eq. shell_max_control_y - 1 then -
      control_y_string = control_y_string + " (next is last)"
    if control_y_count .eq. shell_max_control_y then -
      control_y_string = control_y_string + " (last)"
    if control_y_count .gt. shell_max_control_y then -
      control_y_string = control_y_string + " (exit)"
    vmx_put_error control_y_string
    if control_y_count .gt. shell_max_control_y then goto RETURN
END_LOOP: goto LOOP
RETURN: 
$! end SHELL;
is_verify = f$verify(is_verify)
!<-- end  ---------- ---------- ---------- ---------- <---------
