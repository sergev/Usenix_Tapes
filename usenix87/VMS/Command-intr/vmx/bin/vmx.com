!--> file VMX.COM    ---------- ---------- ---------- --------->
INIT:
  vmx_init_file  = "vmx_dir:initvmx.com"
  if f$search(vmx_init_file) -
    .eqs. "" then exit
  @'vmx_init_file
$
  user_id        = f$getjpi("", "username")
  user_name     := 'user_id
  user_init_file = "vmx_users_dir:" + user_name + ini_extension
  if f$search(user_init_file) -
    .eqs. "" then -
    vmx_put_warning "vmx: can't open ''user_init_file'; use EDIX"
  if f$search(user_init_file) -
    .nes. "" then @'user_init_file
$
END_INIT:

modulo_for_hs  = max_lines_for_hs / 5
control_y_count = 0
count           = init_count - 1

if f$mode() .nes. "INTERACTIVE" then exit
if .not. is_vmx_wanted          then exit
vmx_put_help "vmx: type HX for help, EX to exit"

LOOP:
  is_verify = f$verify(is_debug_vmx)

  $! procedure VMX is

  on warning       then goto WHEN_WARNING
  on error         then goto WHEN_ERROR
  on severe_error  then goto WHEN_SEVERE_ERROR
  on control_y     then goto WHEN_CONTROL_Y

  if count .ge. max_count then count = init_count - 1
  count        = 1 + count
  count_string = f$string(count)

  input_mode = f$logical("sys$input")
  if input_mode .nes. "SYS$COMMAND:" then -
    define/user_mode sys$input sys$command:

  global_prompt      = f$string('count) + PS1
  prompt            := 'global_prompt
  string_array'count = "control_y"
  inquire/nopunctuation command_line "''prompt' "

  string_array'count = command_line
  first_char  = f$extract(0, 1, command_line)
  is_a_digit  = "0" .les. first_char .and. first_char .les. "9"
  is_to_edit  = first_char .eqs. "/"
  is_previous = first_char .eqs. "\"

  if is_previous then index = count - 1
  if is_previous then command_line = string_array'index
  if is_a_digit  then command_line = string_array'command_line
  if is_a_digit .or. is_previous then string_array'count = command_line
  if is_a_digit .or. is_previous then vmx_put_line "''prompt' ''command_line'"
  if .not. is_to_edit then goto NOT_TO_EDIT

  BLOCK_EDIT:
    old_com      = "oldcom''count_string'" + vmx_extension
    new_com      = "newcom''count_string'" + vmx_extension
    command_line = f$extract(1, f$length(command_line), command_line)
    if command_line .eqs. "" then position = count - 1
    if command_line .nes. "" then position = command_line
    command_line = string_array'position
    open/write logical_file 'new_com
    close      logical_file
    open/write logical_file 'old_com
         write logical_file command_line
    close      logical_file
    command_line = ""
    vmx_edit/nojournal/output='new_com 'old_com
$   open/read logical_file 'new_com
         read logical_file command_line /end_of_file=WHEN_END_OF_FILE
    input_mode = f$logical("sys$input")
    if input_mode .nes. "SYS$COMMAND:" then -
      define/user_mode sys$input sys$command:
  EXCEPTION:
  WHEN_END_OF_FILE:
    close logical_file
    string_array'count = command_line
    vmx_delete 'old_com.*
$   vmx_delete 'new_com.*
$   vmx_put_line "''prompt' ''command_line'"
  END_BLOCK_EDIT:

  NOT_TO_EDIT:
  length_line        = f$length(command_line)
  position_semicolon = f$locate(";", command_line)
  exists_semicolon   = position_semicolon .ne. length_line 
  is_for_shell       = exists_semicolon
  if is_for_shell then goto CALL_THE_SHELL

  if command_line .eqs. "EX"   then goto RETURN
  if command_line .eqs. "INIX" then count = INIT_COUNT - 1
  if command_line .eqs. "INYX" then control_y_count = 0

  if command_line .eqs. "INIX" .or. -
     command_line .eqs. "INYX" then goto END_LOOP

VMS_COMMAND:
  vmx_exec command_line
$END_VMS_COMMAND: goto END_LOOP

CALL_THE_SHELL:
  vmx_shell command_line count_string
END_CALL_THE_SHELL: goto END_LOOP

EXCEPTION:
  WHEN_WARNING:
    vmx_put_error "vmx: warning"
    goto END_LOOP
  WHEN_ERROR:
    vmx_put_error "vmx: error"
    goto END_LOOP
  WHEN_SEVERE_ERROR:
    vmx_put_error "vmx: severe_error"
    goto END_LOOP
  WHEN_CONTROL_Y:
    control_y_string = "vmx: control_y"
    control_y_count = 1 + control_y_count
    if control_y_count .eq. 1 then -
      control_y_string = control_y_string + " (only " + -
      max_control_y + " allowed)"
    if control_y_count .eq. max_control_y - 1 then -
      control_y_string = control_y_string + " (next is last)"
    if control_y_count .eq. max_control_y then -
      control_y_string = control_y_string + " (last)"
    if control_y_count .gt. max_control_y then -
      control_y_string = control_y_string + " (exit)
    vmx_put_error control_y_string
    if control_y_count .gt. max_control_y then goto RETURN
END_LOOP: 

$! end VMX;

goto LOOP
RETURN: is_verify = f$verify(is_verify)
!<-- end  ---------- ---------- ---------- ---------- <---------
