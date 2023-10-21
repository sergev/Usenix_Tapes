CFLAGS =	-DHELP -O
LIBS =		-lpml -lm -ldbg

all :		d2d dd2d c2d c2c cc2c

d2d :		d2d.o
		cc -o d2d d2d.o $(LIBS)

dd2d :		dd2d.o
		cc -o dd2d dd2d.o $(LIBS)
		
c2d :		c2d.o
		cc -o c2d c2d.o $(LIBS)

c2c :		c2c.o
		cc -o c2c c2c.o $(LIBS)
		
cc2c :		cc2c.o
		cc -o cc2c cc2c.o $(LIBS)
		

#
#	Clean up the directory.
#

clean:
	rm -f c2c c2d cc2c d2d dd2d *.BAK *.bak *.tmp nohup.out *.o
