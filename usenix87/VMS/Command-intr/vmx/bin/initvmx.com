!--> file INITVMX.COM --------- ---------- ---------- --------->
vmx_extension    :== ".vmx"
ini_extension    :== ".ini"
com_extension    :== ".com"
job_extension    :== ".job"

in_file          :== "temp.in"
out_extension    :== ".out"
err_extension    :== ".err"
res_extension    :== ".res"

hx               :== @vmx_dir:help
hs               :== @vmx_dir:history
edix             :== @vmx_dir:edix
debug            :== @vmx_dir:debug
no_debug         :== @vmx_dir:nodebug

sh               :== @vmx_dir:sh
vmx_shell        :== @vmx_dir:vmxshell
vmx_send         :== @vmx_dir:send
vmx_fork         :== @vmx_dir:fork
vmx_pipe         :== @vmx_dir:pipe
vmx_exec         :== @vmx_dir:exec
vmx_delete       :== @vmx_dir:delete

vmx_nice         :== submit/noprinter/delete/noid/log_file=[]'res_extension
vmx_spawn        :== spawn/nowait/nolog
vmx_rename       :== rename
vmx_copy         :== copy
vmx_edit         :== edit/edt/command=vmx_dir:edtini.edt

vmx_put_line     :== write sys$error
vmx_put_help     :== if is_help_message    then write sys$error
vmx_put_warning  :== if is_warning_message then write sys$error
vmx_put_error    :== if is_error_message   then write sys$error

max_control_y       :== 3
max_count           :== 333
init_count          :== 0
max_lines_for_hs    :== 15

constant_ps1        :== "_"
ps1                 :== "''constant_ps1'"
constant_tab        :== "     "
tab                 :== "''constant_tab'"

false               :== 0
true                :== 1

is_help_message      :== true
is_warning_message   :== true
is_error_message     :== true
is_vmx_wanted        :== true

is_debug_vmx         :== false
is_debug_shell       :== false
is_debug_pipe        :== false
is_debug_fork        :== false
is_debug_send        :== false
is_debug_nice        :== false
is_debug_exec        :== false
is_debug_history     :== false
!<-- end  ---------- ---------- ---------- ---------- <---------
