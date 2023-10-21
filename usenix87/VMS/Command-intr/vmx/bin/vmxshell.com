!--> file VMXSHELL.COM -------- ---------- ---------- --------->
from_vmx = true
@vmx_dir:shell 'p1' 'from_vmx' 'p2'
!<-- end  ---------- ---------- ---------- ---------- <---------
