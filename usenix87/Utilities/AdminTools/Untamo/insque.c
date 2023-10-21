/* Queue insertion and deletion routines for non-Vaxen */
struct qelem { 
    struct qelem *q_forw , *q_back;
#ifdef LINT
    char q_data[];
#endif LINT
};

#ifndef VAX
insque( elem , pred )
    register struct qelem *elem , *pred;
{
    elem->q_forw = pred->q_forw;
    pred->q_forw = elem;
    elem->q_forw->q_back = elem;
    elem->q_back = pred;
}

#endif VAX
