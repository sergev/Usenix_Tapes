!--> file RMDIR.COM  ---------- ---------- ---------- --------->
set prot=(owner:wred) 'p1'.dir
$
del 'p1'.dir.*
$
!<-- end  ---------- ---------- ---------- ---------- <---------
