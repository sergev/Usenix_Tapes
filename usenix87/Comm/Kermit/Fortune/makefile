.c.o: ; cc -c 9091 main.c.c

modcommand = kermit

modkermit =     main.o connect.o interact.o receive.o send.o X                utils.o

kermit: $(modkermit)
        cc -o kermit $(modkermit)

$(modkermit):   kermit.h

clean:
        -rm -f  *.o kermit

force:
        -rm -f  $(modcommand)

install: kermit
        -cp kermit /bin

xref:
        -rm -f kermit.xref
        -xref main.c    >> kermit.xref
        -xref connect.c >> kermit.xref
        -xref interact.c>> kermit.xref
        -xref receive.c >> kermit.xref
        -xref send.c    >> kermit.xref
        -xref utils.c   >> kermit.xref


lint:
        -lint main.c connect.c interact.c receive.c send.c utils.c >kermit.lint
