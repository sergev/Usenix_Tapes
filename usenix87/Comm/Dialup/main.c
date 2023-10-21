#include <signal.h>
#include <stdio.h>

#define   USAGE     "usage: dialup [-s] service.\n"

char service[40],
     phone[20],
     baud[10],
     login[20],
     password[20];
main(argc, argv)
int  argc;
char *argv[];
{
     int  silent = 0;
     char name[21];

     signal(SIGINT,  SIG_DFL);      /* I need this at my location */
     signal(SIGQUIT, SIG_DFL);      /* I don't know about the rest of you */

     switch (argc) {

     case 1:
          printf("\nEnter network name: ");
          scanf("%20s", name);
          break;

     case 2:
          strncpy(name, argv[1], 21);
          break;

     case 3:
          if (strcmp(argv[1], "-s") == 0) {
               silent = 1;
               strncpy(name, argv[2], 21);
          }
          else {
               fprintf(stderr, USAGE);
               exit(1);
          }
          break;

     default:
          fprintf(stderr, USAGE);
          exit(1);
          break;
     }

     get_service(name);

     printf("\n");
     printf("Service : %s\n", service);
     if (!silent) {
          printf("Phone # : %s\n", phone);
          printf("Baud    : %s\n", baud);
          printf("Login   : %s\n", login);
          printf("Password: %s\n", password);
     }
     printf("\n");

     execlp("cu", "cu", phone, "-s", baud, 0);
}
