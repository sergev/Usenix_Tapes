#include <stdio.h>

extern char    *getenv();

extern char    service[40],
               phone[20],
               baud[10],
               login[20],
               password[20];

get_service(name)
char *name;
{
     char dialup[128], c_name[20];
     FILE *du_file;

     strcpy(dialup, getenv("HOME"));   /* build file name */
     strcat(dialup, "/.dialup");

     if ((du_file = fopen(dialup, "r")) == NULL) {    /* open it */
          fprintf(stderr, "dialup: can't open %s.\n", dialup);
          exit(1);
     }

     while (!feof(du_file)) {     /* loop till the end of the file */
          c_name[0] = 0;
          phone[0] = 0;
          baud[0] = 0;
          login[0] = 0;
          password[0] = 0;
          service[0] = 0;

          fscanf(du_file, "%[^:]%*c", c_name);
          fscanf(du_file, "%[^:]%*c", phone);
          fscanf(du_file, "%[^:]%*c", baud);
          fscanf(du_file, "%[^:]%*c", login);
          fscanf(du_file, "%[^:]%*c", password);
          fscanf(du_file, "%[^\n]%*c", service);

          if (strcmp(name, c_name) == 0) {       /* match? */
               fclose(du_file);                  /* if so, close up early */
               return;                           /* and return */
          }
     }

     fclose(du_file);                            /* No matches in file */
     fprintf(stderr, "dialup: %s network unknown.\n", name);
     exit(1);
}
