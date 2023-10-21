!--> file EDIX.COM   ---------- ---------- ---------- --------->
user_id        = f$getjpi("", "username")
user_name     := 'user_id
user_init_file = "vmx_users_dir:" + user_name + ini_extension

if f$search(user_init_file) -
  .eqs. "" then vmx_put_line "edix: creating ''user_init_file'"
if f$search(user_init_file) -
  .eqs. "" then wait 0:0:3
if f$search(user_init_file) -
  .eqs. "" then vmx_copy vmx_dir:initvmx.com 'user_init_file'
$
vmx_put_line "edix: opening  ''user_init_file'"
vmx_put_line "      type ^Z, then QUIT or EXIT to exit the editor"
wait 0:0:3
input_mode = f$logical("sys$input")
if input_mode .nes. "SYS$COMMAND:" then -
  define/user_mode sys$input sys$command:
vmx_edit 'user_init_file'
!<-- end  ---------- ---------- ---------- ---------- <---------
