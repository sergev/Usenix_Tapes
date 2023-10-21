/* Copyright 1983 - University of Maryland */

#define ADDMONTH(var) month1[monthmax++]=var
#define ADDDAY(var) day1[daymax++]=var
#define ADDNUM(var) tnum[cnum][tnummax[cnum]++]=var

allocnum()
{
    if (tnummax[0] > 0) cnum = 1;
    else cnum = 0;
}

addnumseq(st,fin)
int st,fin;
{
    int i;
    for (i=st; i <= fin; i++) ADDNUM(i);
}

addmonseq(st,fin)
int st,fin;
{
    int i;

    fin++;
    if (fin > 12) fin = 1;

    ADDMONTH(st);

    for (i = st + 1; i != fin; i = (i >= 12) ? 1 : i+1) ADDMONTH(i);
}

adddayseq(st,fin)
int st,fin;
{
    int i;

    fin++;
    if (fin > 7) fin = 1;

    ADDDAY(st);

    for (i = st + 1; i != fin; i = (i >= 7) ? 1 : i+1) ADDDAY(i);
}

copydays(ar)
int ar;
{
    int i;

    for (i = 0; i < tnummax[ar]; i++) day1[i] = tnum[ar][i];
    daymax = i;
    tnummax[ar] = 0;
}

copymonths(ar)
int ar;
{
    int i;

    for (i = 0; i < tnummax[ar]; i++) month1[i] = tnum[ar][i];
    monthmax = i;
    tnummax[ar] = 0;
}
