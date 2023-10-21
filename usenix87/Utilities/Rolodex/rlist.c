#include "datadef.h"


int rlength (rlist) Ptr_Rolo_List rlist;
{
  return((rlist == 0) ? 0 : 1 + rlength(get_next_link(rlist)));
}  
  

Ptr_Rolo_List new_link_with_entry ()
{
  Ptr_Rolo_List newlink;        
  Ptr_Rolo_Entry newentry;
  newlink = (Ptr_Rolo_List) rolo_emalloc(sizeof(Rolo_List));
  unset_matched(newlink);
  newentry = (Ptr_Rolo_Entry) rolo_emalloc(sizeof(Rolo_Entry));
  set_n_others(newentry,0);        
  newentry -> other_fields = 0;
  set_entry(newlink,newentry);
  return(newlink);
}


rolo_insert (link,compare) Ptr_Rolo_List link; int (*compare)();

{
  Ptr_Rolo_List rptr;        
        
  if (Begin_Rlist == 0) {
     Begin_Rlist = link;
     End_Rlist = link;
     set_prev_link(link,0);
     set_next_link(link,0);
     return;
  }
  
  /* find the element it goes before, alphabetically, and insert it */

  rptr = Begin_Rlist;
  while (rptr != 0) {
    if (1 == (*compare)(rptr,link)) {
       set_prev_link(link,get_prev_link(rptr));
       set_next_link(link,rptr);
       if (get_prev_link(rptr) != 0) 
          set_next_link(get_prev_link(rptr),link);
       else
          Begin_Rlist = link;
       set_prev_link(rptr,link);
       return;
    }
    rptr = get_next_link(rptr);
  }

  /* it goes at the end */
  
  set_next_link(End_Rlist,link);
  set_prev_link(link,End_Rlist);
  set_next_link(link,0);
  End_Rlist = link;
  return;

}


rolo_delete (link) Ptr_Rolo_List link;

{
  if (get_next_link(link) == 0 && get_prev_link(link) == 0) {
     Begin_Rlist = 0;
     End_Rlist = 0;
     return;
  }

  if (get_prev_link(link) == 0) {
     Begin_Rlist = get_next_link(link);
     set_prev_link(Begin_Rlist,0);
     return;
  }

  if (get_next_link(link) == 0) {
     End_Rlist = get_prev_link(link);
     set_next_link(End_Rlist,0);
     return;
  }

  set_next_link(get_prev_link(link),get_next_link(link));
  set_prev_link(get_next_link(link),get_prev_link(link));
  return;

}


compare_links (l1,l2) Ptr_Rolo_List l1,l2;

{
  Ptr_Rolo_Entry e1,e2;        
  char *n1,*n2;
  e1 = get_entry(l1);        
  e2 = get_entry(l2);        
  n1 = get_basic_rolo_field((int) R_NAME,e1);
  n2 = get_basic_rolo_field((int) R_NAME,e2);
  return(nocase_compare(n1,strlen(n1),n2,strlen(n2)));
}  
  

rolo_reorder ()
{
  Ptr_Rolo_List rptr,oldlink;
  rptr = Begin_Rlist;
  Begin_Rlist = 0;
  while (rptr != 0) {
    oldlink = get_next_link(rptr);
    rolo_insert(rptr,compare_links);
    rptr = oldlink;
  }
}


