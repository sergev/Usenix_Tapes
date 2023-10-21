/************************************  notify.c  *************************/
#include <sunwindow/notify.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include "sdi.h"

/*
 * Copyright 1987 by Mark Weiser.
 * Permission to reproduce and use in any manner whatsoever on Suns is granted
 * so long as this copyright and other identifying marks of authorship
 * in the code and the game remain intact and visible.  Use of this code
 * in other products is reserved to me--I'm working on Mac and IBM versions.
 */

/*
 * notifier and timer specific code, including some input handling hacks.
 */

/*
 * Begin a series of minor kludges to get good response time at
 * heavy screen loads without running into various SunView bugs
 * involving input notification and non-blocking I/O.  There is
 * probably a bit of overkill here, but the obvious solution
 * (turn on non-blocking once in 'main') does not work.
 */

/*
 * See if any mouse clicks have occured.
 */
checkinput()
{
	input_helper(city_fd, citycanvas);
	input_helper(launch_fd, launchcanvas);
}

/*
 * Without blocking, exhaust all input on file descripter 'fd' and
 * send it to window 'w'.  
 */
input_helper(fd, w)
Window w;
{
	Event event;
	int old_flags = fcntl(fd, F_GETFL);
/*
 * Have to turn FNDELAY on and off because SunView really doesn't
 * work well with it on under heavy input load.  The result is that
 * we do too much work under light load (when we can afford it), and
 * much less work under heavy load (when it makes sense).  A good balance.
 */
	fcntl(fd, F_SETFL, old_flags | FNDELAY);
	while(! input_readevent(fd, &event)) {
		canvas_event(w, &event);
		event_x(&event) = event_x(&event)-FIELD_MARGIN;
		event_y(&event) = event_y(&event)-FIELD_MARGIN;
		main_event_proc(w, &event, 0);
	}
	fcntl(fd, F_SETFL, old_flags);
}

/*
 * Receive notifications of pending input.
 */
Notify_value 
input_notify(me, fd)
{
	if (fd == city_fd) {
		input_helper(city_fd, citycanvas);
	} else if (fd == launch_fd) {
		input_helper(launch_fd, launchcanvas);
	} 
}

/*
 * Schedule things from input clients before timers.  The effect
 * is to get all the input before starting to update the screen.
*/
Notify_value
scheduler(n, clients)
Notify_client *clients;
{
	int i;
	for (i=0; i < n; i += 1) {
		if (
			clients[i] == (Notify_client)launchcanvas ||
			clients[i] == (Notify_client)citycanvas ||
			clients[i] == (Notify_client)launchframe ||
			clients[i] == (Notify_client)cityframe
			) {
				notify_client(clients[i]);
				clients[i] = NOTIFY_CLIENT_NULL;
		}
	}
	for (i=0; i < n; i += 1) {
		if (clients[i] != NOTIFY_CLIENT_NULL) {
				notify_client(clients[i]);
				clients[i] = NOTIFY_CLIENT_NULL;
		}
	}
	return NOTIFY_DONE;
}

/*
 * Call procedure f in a little while.
 */

struct call_wrapper {
	/* Dynamically allocating a wrapper ensures unique notifier id's. */
	void (*f)();
}

do_with_delay(f, secs, usecs)
void (*f)();
int secs, usecs;
{
	Notify_value do_the_call();
	struct call_wrapper *w;
	struct itimerval timer;

	/* Sigh, so much work just to wait a bit before starting up. */
	timer.it_interval.tv_usec = 0;
	timer.it_interval.tv_sec = 0;
	timer.it_value.tv_usec = usecs;
	timer.it_value.tv_sec = secs;
 	w = (struct call_wrapper *)calloc(sizeof(struct call_wrapper), 1);
	w->f = f;
	notify_set_itimer_func(w, do_the_call,
		ITIMER_REAL, &timer, NULL);
}

/*
 * Wrapper to make sure procedures from do_with_delay return good values
 * to the notifier.
 */
Notify_value
do_the_call(w, which)
struct call_wrapper *w;
{
	(*(w->f))();
	free(w);
	return NOTIFY_DONE;
}

/* Call this routine when we are suspended, and
 * we want to put off notification.  The parameters
 * are those to notify_set_itimer_func.
 */
suspendor(func, me, which, when)
Notify_value (*func)();
int *me, which;
{
	struct itimerval timer;
	timer.it_interval.tv_usec = 0;
	timer.it_interval.tv_sec = 0;
	timer.it_value.tv_usec = 0;
	timer.it_value.tv_sec = when;
	notify_set_itimer_func(me, func, ITIMER_REAL, &timer, which);
}
