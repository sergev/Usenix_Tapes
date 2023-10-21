# include <stdio.h>
# include <strings.h>

# define DB "areacodes"

main(argc, argv)
  int    argc;
  char **argv;
{

   int i,j;
   int glen, llen;
   char grep[255],line[255];
   FILE *db;

   if(argc <= 1) {
       printf("USAGE: %s areacode\nor     %s city\nor     %s state\n\n",argv[0],
                   argv[0],argv[0]);
       exit(0);
   }

   grep[0] = '\0';
   for( i = 1; i < argc; i++) {
      strcat(grep,argv[i]);
      j = strlen(grep);
      grep[j] = ' ';
      grep[j + 1] = '\0';
    }
   grep[j] = '\0';

   if ( (db = fopen(DB,"r")) == NULL)  {
        perror(DB);
        exit(0);
   }
   printf("%s\n","");
   glen = strlen(grep);
   while( (fgets(line,255,db)) != NULL) {
       llen = strlen(line);
       if( glen > llen)
         continue;

       j = 0;
       for( i = 0; i < llen ; i++) {
         if (line[i] == grep[j])  {
             j++ ;
             if ( j == glen)
                break;
             else
                continue;
         }
         else
             j = 0 ;
      }

      if( j == glen )
         printf("%s",line);
      continue;
    }

   fclose(db);
   exit(0);
}
