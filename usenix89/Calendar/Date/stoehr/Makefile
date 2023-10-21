MONTH_OBJS = display.o lunar.o month.o schedule.o time.o user.o

CC = cc

CFLAGS = -c

month: $(MONTH_OBJS)
	$(CC) $(MONTH_OBJS) -lm -lcurses -ltermlib -o month

display.o: display.c month.h
	$(CC) $(CFLAGS) display.c

lunar.o: lunar.c 
	$(CC) $(CFLAGS) lunar.c

month.o: month.c month.h
	$(CC) $(CFLAGS) month.c

schedule.o: schedule.c month.h
	$(CC) $(CFLAGS) schedule.c

time.o: time.c month.h
	$(CC) $(CFLAGS) time.c

user.o: user.c month.h
	$(CC) $(CFLAGS) user.c
