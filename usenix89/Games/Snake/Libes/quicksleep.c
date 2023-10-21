/* quicksleep.c - Don Libes
quick_sleep() - sleep for less than a second
*/

#ifdef CMU_IPC
quick_sleep(timeout)
int timeout;		/* milliseconds */
{
	int sop[8];
	sop[0] = 1;

	ipcmessagewait(sop,timeout);	/* huh, message?  what message? */
#endif

#ifdef UNISOFT_SELECT
/* My documentation says that this doesn't work yet, but someday it might */
quick_sleep(timeout)
int timeout;		/* milliseconds */
{
	(void) select(20, 0, 0, timeout);
}
#endif

#ifdef BSD_SELECT
#include <sys/time.h>
quick_sleep(timeout)
int timeout;		/* microseconds */
{
	struct timeval t;

	t.tv_sec = timeout / 1000000;
	t.tv_usec = timeout % 1000000;
	(void) select(32, 0, 0, 0, &t);
}
#endif

#ifdef VMS
quick_sleep(timeout)
char *timeout;
{
	int rc;
	char *vms_timeout[8];

	struct {
		int length;
		char *string;
	} timeout_desc;
	timeout_desc.string = timeout;
	timeout_desc.length = strlen(timeout);

	if (1 != (rc = sys$bintim(&timeout_desc,vms_timeout))) {
		printf("$bintim() = %x\n",rc);
		cleanup();
		exit(0);
	}
	if (1 != (rc = sys$schdwk(0,0,vms_timeout,0))) {
		printf("$schdwk() = %x\n",rc);
		cleanup();
		exit(0);
	}
	sys$hiber(0);
}
#endif
