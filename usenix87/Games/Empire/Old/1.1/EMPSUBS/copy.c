char *
copy(from, to)
char    *from, *to; 
{
        register char   *f, *t;

        f = from;
        t = to;
        while( *t++ = *f++ ) continue;
        t--;
        return(t);
}
