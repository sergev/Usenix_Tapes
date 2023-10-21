CFLAGS= -O
all: hdate hcal

hdate: hdate.o hcom.o
	cc -o hdate hdate.o hcom.o

hcal: hcal.o hcom.o
	cc -o hcal hcal.o hcom.o

hdate.o hcal.o hcom.o: hdate.h

clean:
	rm -f hdate.o hcal.o hcom.o
