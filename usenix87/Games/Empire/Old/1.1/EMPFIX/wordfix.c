/*
        n = ptr to name
        vp= ptr to var with current value
        wv= default value
*/
wordfix(n, vp, wv)
char    *n;
short   *vp;
int     wv;
{
        *vp = (short)fixup(n, (int)*vp, wv);
        return;
}
