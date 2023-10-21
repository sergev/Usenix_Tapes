!--> file SLEEP.COM  ---------- ---------- ---------- --------->
if f$locate(":", p1) .ne. f$length(p1) then wait 0:'p1'
if f$locate(":", p1) .eq. f$length(p1) then wait 0:0:'p1'
!<-- end  ---------- ---------- ---------- ---------- <---------
