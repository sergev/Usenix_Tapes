!--> file DEF0.COM   ---------- ---------- ---------- --------->
assign disk0:[root.z.vmx.bin]    vmx_dir
assign disk0:[root.z.vmx.users]  vmx_users_dir
assign disk0:[root.z.vmx.help]   vmx_help_dir
!<-- end  ---------- ---------- ---------- ---------- <---------
