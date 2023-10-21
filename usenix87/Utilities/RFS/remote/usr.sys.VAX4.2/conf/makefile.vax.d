This change teaches the makefile how to make rmt_data.c from
../remote/remotefs.h, ../remote/rmt_data_template and ../sys/init_sysent.c.
***************
*** 166,168
  
  %RULES
  

--- 166,175 -----
  
+ ../remote/rmt_data.c: ../remote/rmt_data_template ../remote/remotefs.h \
+     ../sys/init_sysent.c
+ 	cat ../remote/rmt_data_template > ../remote/nrmt_data.c
+ 	/lib/cpp ${CFLAGS} ../sys/init_sysent.c | sh ../remote/remote_mkdata \
+ 		../remote/remotefs.h >> ../remote/nrmt_data.c
+ 	mv ../remote/nrmt_data.c ../remote/rmt_data.c
+ 
  %RULES
  
