/*  list_sort( list, compare_routine ) -> sorted_list

    Assumes a list of structures, with a pointer to "next" as the first
    field.  It reorders the list into ascending order, and returns the
    new first node's address.  It is order N log(N).

    The compare routine should return 0 if the items are in order, and 1
    if they are not.  If the compare routine returns 0 in case of
    equality, the sort will be stable.
*/

/*  This routine depends upon compiler-dependant struct layout, but this
    assumption is likely to be fairly commonly valid.
*/

typedef struct node_struct { struct node_struct *next; } node_type;
typedef node_type *(list_pair_type[2]);

node_type *list_sort( list, compare )
    node_type *list;
    int (*compare)();
{
    list_pair_type old, new; /* front of old stuff, back of new */
    int n;              /* current new list */
    int o;              /* current old list */
    long m, hm;         /* m is merge length, hm is half of that */
    long items_merged;  /* counts how many items have been merged */
    long count[2];      /* counts entries drawn from "old" lists */
    node_type front[2];  /* artificial front-of-list */

    front[0].next = 0;
    front[1].next = list;

/*  The basic notion of this sort is to make sorted sublists longer and
    longer by merging.  On the Nth pass through the list, sorted
    sublists of length 2^(N-1) are produced.  Eventually, the entire
    list is sorted, in log2(N)+1 passes through the list.  There is
    extra bookkeeping overhead, but minimal extra storage space needed.
    Counts and clever pointer management substitute for extra "glue"
    nodes.

    while more than one list
       while not at end of composite lists
             for each merge_length(m) block
                   merge first items in lists onto current output list
             toggle current output list
*/

    m = 1;
    hm = 0;
    while( front[1].next ){
        /* log2(N)+1 times through (since m doubles every time) */
        new[0] = &front[0];
        new[1] = &front[1];
        old[0] = front[0].next;
        old[1] = front[1].next;
        count[0] = count[1] = 0;
        n = 0;
        for( items_merged=0; old[0] || old[1]; ++items_merged ){
            /* N times through (each time through consumes one item
               from one or the other of the old lists)
            */
            if( items_merged>=m ){
                items_merged = 0;
                n = 1-n;
                count[0] = count[1] = 0;
            }
            if( old[0] && old[1] && (count[0]<hm) && (count[1]<hm) ){
                /*  Compare the two items at the heads of the input
                    lists, and put the smaller on the current output
                    list
                */
                o = (*compare)( old[0], old[1] );
                new[n]->next = old[o];
                old[o] = old[o]->next;
                ++count[o];
            }
            else{
                /*  One of two input lists exhausted... either because
                    of end of a composite list, or because of end of
                    logical list (too many gotten from one branch or
                    another).
                */
                o = ((old[0] && (count[0]<hm)) ?0:1);
                new[n]->next = old[o];
                old[o] = old[o]->next;
            }

            new[n] = new[n]->next;
        }
        new[0]->next = new[1]->next = 0;

        hm = m;
        m *= 2;
    }
    return( front[0].next );

}
---------- test_list_sort.c ----------
/*  This is a test for the list_sort routine.  It takes a list of
    integers from the command line, puts these integers into a list, and
    then hands this list to list_sort.
*/

int atoi();
char *malloc();
int printf();

typedef struct node_struct {
    struct node_struct  *next;
    int                 value; } node_type;

static int compare( a, b )
    node_type *a, *b;
{
    return( a->value > b->value );
}

node_type *list_sort();

void main( n, a )
    int n; char *(a[]);
{
    node_type *list = 0, *p;
    int i;

    for( i=1; a[i]; ++i ){
        p=(node_type *)malloc(sizeof(node_type));
        p->value = atoi( a[i] );
        p->next = list;
        list = p;
    }

    list = list_sort( list, compare );

    for( p=list; p; p=p->next ){
        printf( " %d", p->value );
    }
    printf( "\n" );

}
