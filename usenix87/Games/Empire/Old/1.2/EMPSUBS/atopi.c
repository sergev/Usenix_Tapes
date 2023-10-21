atopi(cp)
char *cp;
{
        register int n;

        if ((n = atoi(cp)) < 0)
                return(-n);
        else
                return(n);
}
