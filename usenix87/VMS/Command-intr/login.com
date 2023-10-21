!-->file LOGIN.COM  ---------- ---------- ---------- --------->
$
set noverify
set term/dev=vt100
$
assign disk0:[root.z.bin] bin
@bin:profile
vmx
!<-- end  ---------- ---------- ---------- ---------- <---------
