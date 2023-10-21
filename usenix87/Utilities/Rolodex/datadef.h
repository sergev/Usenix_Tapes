#define ABORTSTRING "\\"
#define ABORTCHAR '\\'

#define MAXMATCHES 17

#define N_BASIC_FIELDS 8
#define OTHER -1

typedef enum Basic_Field {

    R_NAME = 0, R_WORK_PHONE, R_HOME_PHONE, R_COMPANY, R_WORK_ADDRESS,
    R_HOME_ADDRESS, R_REMARKS, R_UPDATED

  };    
    
extern char *Field_Names[];  
  
/* A Rolodex entry */

typedef struct {

    char *basicfields[N_BASIC_FIELDS];
    int n_others;
    char **other_fields;

  } Rolo_Entry, *Ptr_Rolo_Entry;

  
#define get_basic_rolo_field(n,x) (((x) -> basicfields)[(n)])
#define get_n_others(x) ((x) -> n_others)  
#define get_other_field(n,x) (((x) -> other_fields)[n])
  
#define set_basic_rolo_field(n,x,s) (((x) -> basicfields[(n)]) = (s))
#define set_n_others(x,n) (((x) -> n_others) = (n))
#define incr_n_others(x) (((x) -> n_others)++)
#define set_other_field(n,x,s) ((((x) -> other_fields)[n]) = (s))

typedef struct link {

    Ptr_Rolo_Entry entry;
    int matched;
    struct link *prev;
    struct link *next;

  } Rolo_List, *Ptr_Rolo_List;


#define get_next_link(x) ((x) -> next)
#define get_prev_link(x) ((x) -> prev)
#define get_entry(x)     ((x) -> entry)
#define get_matched(x) ((x) -> matched)

#define set_next_link(x,y) (((x) -> next) = (y))
#define set_prev_link(x,y) (((x) -> prev) = (y))
#define set_entry(x,y) (((x) -> entry) = (y))
#define set_matched(x) (((x) -> matched) = 1)
#define unset_matched(x) (((x) -> matched) = 0);

extern Ptr_Rolo_List Begin_Rlist;
extern Ptr_Rolo_List End_Rlist;

#define MAXLINELEN 80
#define DIRPATHLEN 100

extern int changed;
extern int reorder_file;
extern int rololocked;

extern char *rolo_emalloc();
extern char *malloc();
extern Ptr_Rolo_List new_link_with_entry();
extern char *copystr();
extern int compare_links();
extern char *timestring();
extern char *homedir(), *libdir();
extern char *getenv();
extern char *ctime();
extern char *select_search_string();
extern int in_search_mode;
