robots:		robots.c
		cc -O robots.c -o robots -lcurses -ltermcap

install:	robots
		cp robots.6 /usr/man/man6/robots.6
		install -s -m 4111 robots /usr/games/robots
		touch /usr/games/lib/robots_hof
		touch /usr/games/lib/robots_tmp

clean:
		rm -f robots *.o

lint:
		lint -phbxac robots.c -lcurses -ltermcap > linterrs
