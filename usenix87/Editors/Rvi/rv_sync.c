#include "rv.h"

/*
 * Synchronize communication by sending a command
 * with a recognizable response.
 */

void
xmit_sync()
/*
 * Transmit sync
 *
 * A sync consists of transmitting two f commands followed
 * by an illegal command, and waiting to receive the file name echoed
 * twice followed by a question mark.
 */
{
	if (fputs("f\nf\nzzz\n", file.fi_fpout) == NULL)
		panic("Error while writing to ed\n\n");
}

boolean
recv_sync(disp_flag)
/*
 * Wait for sync to come back
 *
 * Returns TRUE if no errors received prior to sync, else
 *	 displays errors if disp_flag is TRUE.
 */
boolean disp_flag;
{
	register char buf[512];
	register INT i, namematch = 0, errcount = 0;
	INT old_alarm;
	extern alarmring();

	old_alarm = alarm(RV_TIMEOUT);

	fflush(file.fi_fpout);
	for (;;) {
		if (fgets(buf, 511, file.fi_fpin) == NULL)
			panic("Error while reading from ed\n\n");
		/*
		 * Strip trailing \n
		 */
		if ((i = strlen(buf)) > 0 && buf[i-1] == '\n')
			buf[--i] = '\0';
		if (buf[0] == '?') {
			if (namematch > 1)
				goto found;
			/*
			 * Display error
			 */
			++errcount;
			if (file.fi_sysv) {
				if (buf[1] != ' ' ||
						file.fi_cray == FALSE)
					fgets(buf, 511, file.fi_fpin);
				if (disp_flag)
					botprint(TRUE, "%s", buf);
			}
			else
				if (disp_flag)
					botprint(TRUE, "Failed ");
			namematch = 0;
			continue;
		}
		/*
		 * Look for file name echoed twice
		 */
		if (strcmp(buf, file.fi_name) == 0) 
			++namematch;
		else
			namematch = 0;
		if (namematch >= 2) {
			(void) fgets(buf, 511, file.fi_fpin);
			if ((i = strlen(buf)) > 0 && buf[i-1] == '\n')
				buf[--i] = '\0';
			/*
			 * Look for question mark
			 */
			if (buf[0] == '?') {
		found:
				if (file.fi_sysv &&
				    (buf[1] != ' ' || file.fi_cray == FALSE))
					/*
					 * Skip error msg
					 */
					fgets(buf, 511, file.fi_fpin);
				break;
			}
			else {
				if (disp_flag > 1)
					botprint(FALSE, "%s", file.fi_name);
				if (strcmp(buf, file.fi_name)) {
					if (disp_flag > 1)
					    botprint(FALSE, "%s", file.fi_name);
					namematch = 0;
				}
			}
		}
		if (disp_flag > 1 && namematch == 0)
			botprint(FALSE, "%s", buf);
	}
	alarm(old_alarm);

	return (errcount == 0);
}
