!--> file PIPE.COM   ---------- ---------- ---------- --------->
is_verify = f$verify(is_debug_pipe)

$! procedure PIPE (P1 : STRING  := "";
$!                 P2 : NATURAL := 0) is

pipe_max_control_y = 3
control_y_count    = 0
pipe_count         = -1

on warning       then goto WHEN_WARNING
on error         then goto WHEN_ERROR
on severe_error  then goto WHEN_SEVERE_ERROR
on control_y     then goto WHEN_CONTROL_Y

argument_string := 'p1'
command_line     = 'argument_string

second_argument  = 'p2
if p2 .eqs. "" then second_argument = "000"
count_string    := 'second_argument

initial_name = "pip" + count_string + "x"
message_in   = "pipe: <" + in_file

in_pipe  = "inpip"  + count_string + vmx_extension
out_pipe = "outpip" + count_string + vmx_extension
com_pipe = "compip" + count_string + vmx_extension

deassign_sys_input = "deassign sys$input"
pipe_sys_command   = "assign/user_mode " + in_pipe + " sys$command"
assign_sys_command = "assign/user_mode " + in_file + " sys$command"
assign_sys_input   = "assign/user_mode sys$command: sys$input"

open_sys_error     = "open/write sys$error: "
define_sys_error   = "define sys$error sys$error:"
deassign_sys_error = "$deassign sys$error"
close_sys_error    = "$close sys$error:"

is_a_pipe = false

LOOP:
  was_a_pipe    = is_a_pipe
  length_line   = f$length(command_line)
  position_pipe = f$locate("|", command_line)
  is_a_pipe     = position_pipe .ne. length_line  

  p = length_line - position_pipe - 1
  first_command = f$extract(0, position_pipe, command_line)
  command_line  = f$extract(1 + position_pipe, p, command_line)
  if first_command .eqs. "" then goto END_EXEC_COM
  
  open/write pipe_file 'com_pipe
  if was_a_pipe then write pipe_file pipe_sys_command
  write pipe_file deassign_sys_input
  write pipe_file assign_sys_input
  command_all = first_command

  OUTER_LOOP:
    is_new_input   = false
    is_new_output  = false
    is_new_error   = false
    is_a_job       = false

    pipe_count     = 1 + pipe_count
    prefix_file    = initial_name  + f$string(pipe_count)
    com_file       = prefix_file + vmx_extension
    job_file       = prefix_file + job_extension
    out_file       = prefix_file + out_extension
    err_file       = prefix_file + err_extension
    open_sys_error_string = open_sys_error + err_file

    length_all     = f$length(command_all)
    position       = f$locate(";", command_all)
    is_multiple    = position .ne. length_all

    p = length_line - position - 1
    command_one    = f$extract(0, position, command_all)
    command_all    = f$extract(1 + position, p, command_all)
    if command_one .eqs. "" then goto END_MULTIPLE_COMMAND

    INNER_LOOP:
      first_char = f$extract(0, 1, command_one)
      is_special = first_char .eqs. "<" .or. first_char .eqs. ">" .or. -
                   first_char .eqs. "?" .or. first_char .eqs. "+" .or. -
                   first_char .eqs. " "
      if .not. is_special then goto EXIT_INNER_LOOP
  
      if first_char .eqs. "<" then is_new_input  = true
      if first_char .eqs. ">" then is_new_output = true
      if first_char .eqs. "?" then is_new_error  = true
      if first_char .eqs. "+" then is_a_job      = true

      command_one = f$extract(1, f$length(command_one), command_one)
    END_INNER_LOOP: goto INNER_LOOP
    EXIT_INNER_LOOP:
    if .not. is_new_output .and. .not. is_a_job then goto STANDARD_OUTPUT

    REDIRECT_OUTPUT:
    change_dir = "$set def " + f$logical("sys$disk") + f$directory()
    open/write logical_file 'com_file
         if is_a_job then write logical_file change_dir
         write logical_file "$''command_one'"
    close      logical_file
    if is_new_output then command_one = "@" + com_file + "/out=" + out_file
    if is_a_job then vmx_rename 'com_file 'job_file
$   if is_a_job then command_one = "vmx_nice " + job_file

    STANDARD_OUTPUT:
    message_out = "pipe: >" + out_file
    message_err = "pipe: ?" + err_file

    if is_new_input  then deassign sys$input
    if is_new_input  then assign/user_mode in_file sys$input

    if is_new_input  then vmx_put_line message_in
    if is_new_output then vmx_put_line message_out
    if is_new_error  then vmx_put_line message_err

    if is_new_error  then write pipe_file open_sys_error_string
    if is_new_error  then write pipe_file define_sys_error
    write pipe_file "$''command_one'"
    if is_new_error  then write pipe_file deassign_sys_error
    if is_new_error  then write pipe_file close_sys_error
    if .not. is_multiple then goto EXIT_OUTER_LOOP
  END_MULTIPLE_COMMAND:
    if command_all .eqs. "" then goto EXIT_OUTER_LOOP
  END_OUTER_LOOP: goto OUTER_LOOP
  EXIT_OUTER_LOOP:
  close pipe_file

  EXEC_COM:
  command_one = "@''com_pipe'"
  if is_a_pipe then command_one = command_one + "/out=''out_pipe'"
  vmx_exec command_one
$ if is_a_pipe then vmx_rename 'out_pipe 'in_pipe
$ vmx_delete 'initial_name'*'vmx_extension'.*
$ vmx_delete 'com_pipe.*
$ if .not. is_a_pipe then vmx_delete 'in_pipe.*
$ if .not. is_a_pipe then goto RETURN
  END_EXEC_COM:
  if command_line .eqs. "" then goto RETURN

  EXCEPTION: goto END_LOOP
  WHEN_WARNING:
    vmx_put_error "pipe: warning"
    goto END_LOOP
  WHEN_ERROR:
    vmx_put_error "pipe: error"
    goto END_LOOP
  WHEN_SEVERE_ERROR:
    vmx_put_error "pipe: severe_error"
    goto END_LOOP
  WHEN_CONTROL_Y:
    control_y_string = "pipe: control_y"
    control_y_count = 1 + control_y_count
    if control_y_count .eq. 1 then -
      control_y_string = control_y_string + " (only " + -
      pipe_max_control_y + " allowed)"
    if control_y_count .eq. pipe_max_control_y - 1 then -
      control_y_string = control_y_string + " (next is last)"
     if control_y_count .eq. pipe_max_control_y then -
      control_y_string = control_y_string + " (last)"
     if control_y_count .gt. pipe_max_control_y then -
      control_y_string = control_y_string + " (exit)"
    vmx_put_error control_y_string
    if control_y_count .gt. pipe_max_control_y then goto RETURN
END_LOOP: goto LOOP

$! end PIPE;

RETURN: is_verify = f$verify(is_verify)
!<-- end  ---------- ---------- ---------- ---------- <---------
