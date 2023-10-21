/* dirs.c              Thu Jul 31 16:48:52 1986  cxd  Author                */

#include <stdio.h>
#include <direct.h>
#include <stdlib.h>

#define     BUF_LEN     100


main(argc, argv)

int argc;
char *argv[];
{
   char buffer[BUF_LEN + 1];
   FILE *fid_in;
   char storage_for_stack_file[13], *stack_file;

   if ((stack_file = getenv("DIRS")) == NULL)
      {
      strcpy(storage_for_stack_file, "\\%%dirs.dat");
      stack_file = storage_for_stack_file;
      }

   if ((fid_in = fopen(stack_file, "r")) == NULL)
      {
      printf("dirs: directory stack empty.\n");
      exit(1);
      }

   while (!feof(fid_in))
      {
      if (fgets(buffer, BUF_LEN, fid_in) != NULL)
         {
         buffer[strlen(buffer) - 1] = '\0';
         printf("%s ", buffer);
         }
      }
   printf("\n");

   fclose(fid_in);
   exit(0);
}

