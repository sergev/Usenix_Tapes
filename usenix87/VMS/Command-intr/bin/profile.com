!--> file PROFILE.COM --------- ---------- ---------- --------->
set protection=(system:wred, owner:wred, group:re, world:re)/default
@bin:def0
@bin:def1
@bin:def2
@bin:def3
write sys$error "start_up done"
!<-- end  ---------- ---------- ---------- ---------- <---------
