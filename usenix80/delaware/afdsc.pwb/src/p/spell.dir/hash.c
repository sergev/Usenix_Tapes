
int hash (p)
char *p;
{
        register int h;

        h = hchr(&p) * 26;
        h =+ hchr(&p);
        return h;
}

int hchr (pp)
char **pp;
{
        register char **p;

        for (p = pp;; (*p)++ ) {
                if (**p <= 'z' && **p >= 'a') return *(*p)++ - 'a';
                if (**p <= 'Z' && **p >= 'A') return *(*p)++ - 'A';
                if (**p == 0) return 0;
        }
}
