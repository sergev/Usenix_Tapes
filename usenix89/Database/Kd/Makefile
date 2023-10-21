HOME=/u1/markp
KEYFILE=$(HOME)/.keys
DATADIR=$(HOME)/.kd

kd: kd.c
	cc -o kd kd.c

initialize:
	-mkdir $(DATADIR)
	cp /dev/null $(KEYFILE)
