#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "defines.h"
#include "structs.h"

struct message message;

main(argc, argv)
	char *argv[];
{
	int key, id, i, stat;

	if (argc == 2)
		key = atoi (argv[1]);
	else if ((key = ftok("/tmp/SEARCH")) == -1) {
		perror("/tmp/SEARCH");
		exit(1);
	}
	printf("Key: 0x%x\n", key);
	if ((id = msgget(key, 0)) == -1) {
		perror("msgget");
		exit(1);
	}
	printf("id: %d\n", id);

	for (i=0;; i++) {
		message.mtype = 0;
		if ((stat = msgrcv(id, &message, sizeof message, 0, IPC_NOWAIT)) == -1) {
			printf ("msgrcv failed\n");
			exit(1);
		}
		printf("Msg#%d: typ=%d, id=%d, len=%d, ret=%d, str='%s'\n",
		i, message.mtype, message.ident, strlen(message.text), stat,
		message.text);
	}
}
