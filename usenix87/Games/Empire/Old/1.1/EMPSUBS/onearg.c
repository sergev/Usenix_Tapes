onearg(cp, np)
char *cp, *np;
{
        char    *getstri();

        if (cp == 0 || *cp == '\0')
                cp = getstri(np);
        return(atopi(cp));
}
