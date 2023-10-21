/* pop.c               Thu Jul 31 16:48:52 1986  cxd  Author                */

#include <stdio.h>
#include <direct.h>
#include <stdlib.h>

#define     BUF_LEN     100


main(argc, argv)

int argc;
char *argv[];
{
   char buffer[BUF_LEN + 1], dir[BUF_LEN + 1];
   int wrote_out = 0;
   FILE *fid_in, *fid_out;
   char storage_for_stack_file[13], *stack_file;

   if ((stack_file = getenv("DIRS")) == NULL)
      {
      strcpy(storage_for_stack_file, "\\%%dirs.dat");
      stack_file = storage_for_stack_file;
      }

   if ((fid_in = fopen(stack_file, "r")) == NULL)
      {
      printf("pop: directory stack empty.\n");
      exit(1);
      }

   if ((fid_out = fopen(stack_file, "w")) == NULL)
      {
      perror("couldn't open data file");
      exit(1);
      }
    
   fgets(buffer, BUF_LEN, fid_in);
   buffer[strlen(buffer) - 1] = '\0';
   strcpy(dir, buffer);

   while (!feof(fid_in))
      {
      if (fgets(buffer, BUF_LEN, fid_in) != NULL)
         {
         buffer[strlen(buffer) - 1] = '\0';
         fprintf(fid_out, "%s\n", buffer);
         printf("%s ", buffer);
         wrote_out = 1;
         }
      }
   printf("\n");

   fclose(fid_in);
   fclose(fid_out);
   if (!wrote_out)
      {
      if (unlink(stack_file))
         {
         perror("couldn't delete empty data file");
         exit(1);
         }
      }

   if (chdir(dir) != NULL)
      {
      perror("couldn't change directory");
      exit(1);
      }

   exit(0);
}
