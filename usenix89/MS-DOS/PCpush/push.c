/* push.c              Mon Jul 28 16:59:59 1986  cxd  Author                */

#include <stdio.h>
#include <direct.h>
#include <stdlib.h>

#define     BUF_LEN     100


main(argc, argv)

int argc;
char *argv[];
{
   char buffer[BUF_LEN + 1], dir[BUF_LEN + 1];
   FILE *fid_in, *fid_out;
   int file_there = 1, first_dir = 1;
   char storage_for_stack_file[13], *stack_file;
   void chkdir();

   if (argc > 2)
      {
      printf("usage: push [dirspec]\n");
      exit(0);
      }

   if (argc == 2)
      {
      chkdir(argv[1]);
      }

   if ((stack_file = getenv("DIRS")) == NULL)
      {
      strcpy(storage_for_stack_file, "\\%%dirs.dat");
      stack_file = storage_for_stack_file;
      }

   if ((fid_in = fopen(stack_file, "r")) == NULL)
      {
      if (argc == 1)
         {
         printf("push: directory stack empty.\n");
         exit(1);
         }
      else
         file_there = 0;
      }

   if ((fid_out = fopen(stack_file, "w")) == NULL)
      {
      perror("couldn't open data file");
      exit(1);
      }
    
   if (getcwd(buffer,BUF_LEN) == NULL)
      {
      perror("error getting current working directory");
      exit(1);
      }

   fprintf(fid_out, "%s\n", buffer);
   printf("%s ", buffer);

   if (file_there)
      {
      while (!feof(fid_in))
         {
         if (fgets(buffer, BUF_LEN, fid_in) != NULL)
            {
            buffer[strlen(buffer) - 1] = '\0';
            if (!(first_dir && argc == 1))
               {
               fprintf(fid_out, "%s\n", buffer);
               printf("%s ", buffer);
               }
            if (first_dir)
               strcpy(dir, buffer);
            first_dir = 0;
            }
         }
      }
   printf("\n");

   if (argc > 1)
      strcpy(dir, argv[1]);

   if (chdir(dir) != NULL)
      {
      perror("couldn't change directory");
      exit(1);
      }

   if (file_there)
      fclose(fid_in);
   fclose(fid_out);
   exit(0);
}


void chkdir( dirnm )

char *dirnm;
{
   char buffer[100];
   FILE *stream;

   strcpy(buffer, dirnm);
   if (strcmp(buffer, "\\") == 0)
      strcat(buffer, "%%dirs.tst", 11);
   else
      strcat(buffer, "\\%%dirs.tst", 13);
   if ((stream = fopen(buffer, "w")) == NULL)
      {
      perror(dirnm);
      exit(1);
      }
   fclose(stream);
   unlink(buffer);
   return;
}
