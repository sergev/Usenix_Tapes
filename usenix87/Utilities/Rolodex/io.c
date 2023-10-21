#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <ctype.h>

#include "sys5.h"

#ifdef TMC
#include <ctools.h>
#else
#include "ctools.h"
#endif

#include "datadef.h"
#include "rolofiles.h"


char *Field_Names[N_BASIC_FIELDS] = {
        
        "Name: ", "Work Phone: ", "Home Phone: ", "Company: ",
        "Work Address: ", "Home Address: ", "Remarks: ", "Date Updated: "
        
     };

Ptr_Rolo_List Begin_Rlist = 0;
Ptr_Rolo_List End_Rlist = 0;
Ptr_Rolo_List Current_Entry = 0;

static char *rolofiledata;

read_rolodex (fd) int fd;

{
  struct stat statdata;
  int filesize,i,j,k,start_of_others,warning_given;
  Ptr_Rolo_Entry newentry,oldentry,currententry;
  Ptr_Rolo_List newlink,rptr;
  char *next_field,*next_other,*oldname,*currentname;
  char **other_pointers;
  int n_entries = 0;
  
  /* find out how many bytes are in the file */
  
  fstat(fd,&statdata);
  if ((filesize = statdata.st_size) == 0) {
     return(0);
  }

  /* create an array of characters that big */
  
  rolofiledata = rolo_emalloc(filesize);

  /* read them all in at once for efficiency */
  
  if (filesize != read(fd,rolofiledata,filesize)) {
     fprintf(stderr,"rolodex read failed\n");
     exit(-1);
  }

  j = 0;
  
  /* for each entry in the rolodex file */
  
  while (j < filesize) {

      n_entries++;
        
      /* create the link and space for the data entry */
        
      newlink = new_link_with_entry();
      newentry = get_entry(newlink);
      if (j == 0) {
         Begin_Rlist = newlink;
         set_prev_link(newlink,0);
         set_next_link(newlink,0);
      }
      else {
          set_next_link(End_Rlist,newlink);
          set_prev_link(newlink,End_Rlist);
          set_next_link(newlink,0);
      }
      End_Rlist = newlink;

      /* locate each required field in the character array and change */
      /* the ending line feed to a null.  Insert a pointer to the */
      /* beginning of the field into the data entry */

      for (i = 0; i < N_BASIC_FIELDS; i++) {
          next_field = rolofiledata + j;
          while (rolofiledata[j] != '\n') {
            j++;
          }
          rolofiledata[j] = '\0';
          j++;
          set_basic_rolo_field(i,newentry,next_field);
      }

      /* the end of an entry is indicated by two adjacent newlines */

      if (rolofiledata[j] == '\n') {
         j++;
         newentry -> other_fields = 0;
         continue;
      }

      /* there must be additional, user-inserted fields. Find out how many. */

      start_of_others = j;
      while (1) {
        while (rolofiledata[j] != '\n') {
          j++;
        }
        incr_n_others(newentry);
        j++;
        if (rolofiledata[j] == '\n') {
           j++;
           break;
        }
     }

     /* allocate an array of character pointers to hold these fields */

     other_pointers = (char **)rolo_emalloc(get_n_others(newentry)*sizeof(char *));

     /* separate each field and insert a pointer to it in the char array */

     k = start_of_others;
     for (i = 0; i < get_n_others(newentry); i++) {
         next_other = rolofiledata + k;
         while (rolofiledata[k] != '\n') {
           k++;
         }
         rolofiledata[k] = '\0';
         other_pointers[i] = next_other;
         k++;
     }

     /* insert the pointer to this character array into the data entry */

     newentry -> other_fields = other_pointers;

  }

  /* check that all the entries are in alphabetical order by name */
  
  warning_given = 0;
  rptr = get_next_link(Begin_Rlist);
  while (rptr != 0) {
    if (1 == compare_links(get_prev_link(rptr),rptr)) {
       if (!warning_given) fprintf(stderr,"Warning, rolodex out of order\n");
       warning_given = 1;
       reorder_file = 1;
    }
    rptr = get_next_link(rptr);
  }    
    
  return(n_entries);
  
}


write_rolo_list (fp) FILE *fp; 

/* write the entire in-core rolodex to a file */

{

  Ptr_Rolo_List rptr;
  Ptr_Rolo_Entry entry;
  int j;

  rptr = Begin_Rlist;

  while (rptr != 0) {
    entry = get_entry(rptr);
    for (j = 0; j < N_BASIC_FIELDS; j++) {
        fprintf(fp,"%s\n",get_basic_rolo_field(j,entry));
    }
    for (j = 0; j < get_n_others(entry); j++) {
        fprintf(fp,"%s\n",get_other_field(j,entry));
    }
    fprintf(fp,"\n");
    rptr = get_next_link(rptr);
  }

}


write_rolo (fp1,fp2) FILE *fp1; FILE *fp2;

{
  write_rolo_list(fp1);
  write_rolo_list(fp2);
}


display_basic_field (name,value,show,up) char *name; char *value; int show,up;
{
  int semi = 0;        
  int i;
  if (all_whitespace(value) && !show) return;
  printf("%-25s",name);
  while (*value != '\0') {
    if (*value == ';') {
       while (*++value == ' '); 
       putchar('\n');
       for (i = 0; i < (up ? 28 : 25); i++) putchar(' ');
       semi = 1;
    }
    else {
       semi = 0;
       putchar(*value++);
    }
  }
  putchar('\n');
}


display_other_field (fieldstring) char *fieldstring;
{
  int already_put_sep = 0;        
  int count = 0;
  int i;
  while (*fieldstring != '\0') {
    if (*fieldstring == ';' && already_put_sep) {
       while (*++fieldstring == ' ');
       putchar('\n');
       for (i = 0; i < 25; i++) putchar(' ');
       continue;
    }
    putchar(*fieldstring);
    count++;
    if (*fieldstring == ':' && !already_put_sep) {
       for (i = count; i < 24; i++) putchar(' ');
       already_put_sep = 1;
    }
    fieldstring++;
  }
  putchar('\n');
}


summarize_entry_list (rlist,ss) Ptr_Rolo_List rlist; char *ss;

/* print out the Name field for each entry that is tagged as matched */
/* and number each entry. */

{
  int count = 1;
  clear_the_screen();
  printf("Entries that match '%s' :\n\n",ss);
  while (rlist != 0) {
    if (get_matched(rlist)) {
       printf (
          "%d. \t%s\n",
          count++,
          get_basic_rolo_field((int) R_NAME,get_entry(rlist))
       );
    }
    rlist = get_next_link(rlist);    
  }
  putchar('\n');
}


display_field_names ()

/* display and number each standard field name. */

{
  int j;
  char *name;
  clear_the_screen();
  for (j = 0; j < N_BASIC_FIELDS - 1; j++) {        
      name = Field_Names[j];        
      printf("%d. ",j+1);
      while (*name != ':') putchar(*name++);
      putchar('\n');
  }
  printf("%d. ",N_BASIC_FIELDS);
  printf("A user created item name\n\n");
}  
  

display_entry (entry) Ptr_Rolo_Entry entry;

{
  int j,n_others;
  char *string;
  
  clear_the_screen();
  
  /* display the standard fields other than Date Updated */
  
  for (j = 0; j < N_BASIC_FIELDS - 1; j++) {
      string = get_basic_rolo_field(j,entry);
      display_basic_field(Field_Names[j],string,0,0);
  }        
      
  /* display any additional fields the user has defined for this entry */
  
  n_others = get_n_others(entry);
  for (j = 0; j < n_others; j++) {
      string = get_other_field(j,entry);
      display_other_field(string);
   }

   /* display the Date Updated field */
   
   j = N_BASIC_FIELDS - 1;
   display_basic_field(Field_Names[j],get_basic_rolo_field(j,entry),0,0);
   fprintf(stdout,"\n");

}


display_entry_for_update (entry) Ptr_Rolo_Entry entry;

/* same as display_entry, except each item is numbered and the Date Updated */
/* item is not displayed */

{
  int j,n_others;
  char *string;
  int count = 1;
  
  clear_the_screen();
  
  for (j = 0; j < N_BASIC_FIELDS - 1; j++) {
      string = get_basic_rolo_field(j,entry);
      printf("%d. ",count++);
      display_basic_field(Field_Names[j],string,1,1);
  }        
      
  n_others = get_n_others(entry);
  for (j = 0; j < n_others; j++) {
      string = get_other_field(j,entry);
      printf("%d. ",count++);
      display_other_field(string);
  }
  
  printf("%d. Add a new user defined field\n",count);

  fprintf(stdout,"\n");

}


int cathelpfile (filepath,helptopic,clear) 

  char *filepath, *helptopic; 
  int clear;

{
  FILE *fp;          
  char buffer[MAXLINELEN];
  if (clear) clear_the_screen();
  if (NULL == (fp = fopen(filepath,"r"))) {
     if (helptopic) {
        printf("No help available on %s, sorry.\n\n",helptopic); 
     }
     else {
        fprintf(stderr,"Fatal error, can't open %s\n",filepath);
        exit(-1);
     }
     return;
  }
  while (NULL != fgets(buffer,MAXLINELEN,fp)) printf("%s",buffer);  
  printf("\n");
  fclose(fp);
  return;
}


any_char_to_continue ()
{
  char buffer[80];
  printf("RETURN to continue: ");
  fgets(buffer,80,stdin);
  return;
}
