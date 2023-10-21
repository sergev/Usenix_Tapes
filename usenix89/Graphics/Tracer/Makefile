tracer: tracer.o shade.o find.o support.o refract.o g_bod.o g_bal.o
	cc tracer.o shade.o find.o support.o refract.o g_bod.o g_bal.o -lm -o tracer
find.o: find.c rtd.h extern.h macros.h
	cc -c find.c 
shade.o: shade.c rtd.h extern.h macros.h
	cc -c shade.c 
support.o: support.c rtd.h extern.h
	cc -c support.c 
tracer.o: tracer.c rtd.h bdata.i macros.h
	cc -c tracer.c  
refract.o: refract.c rtd.h extern.h macros.h
	cc -c refract.c  
g_bod.o: g_bod.c extern.h macros.h
	cc -c g_bod.c  
g_bal.o: g_bal.c extern.h rtd.h
	cc -c g_bal.c  
