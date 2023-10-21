This change teaches the Makefile how to make remote/rmt_data.c from
sys/init_sysent.c and remote/rmt_data_template.
***************
*** 145,150
  param.o: param.c makefile
  	${CC} -c ${CFLAGS} ${PARAM} param.c
  
  %RULES
  
  # DO NOT DELETE THIS LINE -- make depend uses it

--- 145,157 -----
  param.o: param.c Makefile
  	${CC} -c ${CFLAGS} ${PARAM} param.c
  
+ ../remote/rmt_data.c: ../remote/rmt_data_template ../remote/remotefs.h \
+     ../sys/init_sysent.c
+ 	cat ../remote/rmt_data_template > ../remote/nrmt_data.c
+ 	/lib/cpp ${CFLAGS} ../sys/init_sysent.c | sh ../remote/remote_mkdata \
+ 		../remote/remotefs.h >> ../remote/nrmt_data.c
+ 	mv ../remote/nrmt_data.c ../remote/rmt_data.c
+ 
  %RULES
  
  # DO NOT DELETE THIS LINE -- make depend uses it
