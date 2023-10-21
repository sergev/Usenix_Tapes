!--> file UP.COM     ---------- ---------- ---------- --------->
if p1 .eqs. "" then p1 = 1
index = 0
LOOP:
  index = 1 + index
  if index .gt. p1 then goto RETURN
  set def [-]
END_LOOP: goto LOOP
RETURN: sho def
!<-- end  ---------- ---------- ---------- ---------- <---------
