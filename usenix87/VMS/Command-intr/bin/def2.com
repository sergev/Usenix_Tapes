!--> file DEF2.COM   ---------- ---------- ---------- --------->
sleep   :== @bin:sleep
up      :== @bin:up
cd      :== @bin:cd
cwd     :== @bin:cwd
rmdir   :== @bin:rmdir
mkdir   :== @bin:mkdir
rm      :== @bin:rm
go      :== @bin:profile
d0      :== @bin:def0
d1      :== @bin:def1
d2      :== @bin:def2
d3      :== @bin:def3

copy_1  :== run bin:copy1
copy_2  :== run bin:copy2
!<-- end  ---------- ---------- ---------- ---------- <---------
