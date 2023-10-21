# the rules for constructing the library.

LIB = libarg.a

$(LIB): $(LIB)(arg_code.o) $(LIB)(arg_process.o) $(LIB)(arg_decode.o)\
 $(LIB)(arg_init.o) $(LIB)(arg_status.o) $(LIB)(arg_man.o)\
 $(LIB)(arg_synopsis.o) $(LIB)(arg_help.o) $(LIB)(arg_show.o)\
 $(LIB)(arg_hs.o) $(LIB)(arg_bhelp.o) $(LIB)(arg_describe.o)\
 $(LIB)(arg_error.o) $(LIB)(arg_wrong.o) $(LIB)(arg_fprint.o)\
 $(LIB)(arg_formal.o) $(LIB)(arg_spec.o) $(LIB)(arg_stdio.o) $(LIB)(arg_null.o)\
 $(LIB)(argv_init.o) $(LIB)(argv_file.o) $(LIB)(argv_line.o) $(LIB)(argv_get.o)\
 $(LIB)(ecmderr.o) $(LIB)(cmderr.o) $(LIB)(vcmderr.o)\
 $(LIB)(error.o) $(LIB)(errhead.o) $(LIB)(errmsg.o)

.c.a:
	$(CC) -c -O $<
#	ld -r -x $*.o
#	mv a.out $*.o
	ar rv $@ $*.o
	rm -f $*.o

clean:
	rm -f $(LIB)
	rm -f *.o
	rm -f z*

install:
	cp libarg.a /usr/lib/libarg.a
	chmod 644 /usr/lib/libarg.a

argument.doc: argument.nr
	n.nroff -ms argument.nr > argument.doc
