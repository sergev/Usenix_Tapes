
/* possible improvements:
    ation
    ize
    prefixes: non, sub, un
    */
ends (tp, word)
int    *tp;
char   *word;
{
        register char  *p;

        wword (tp, word, "", 0);
        for (p = word; *p != 0; ++p);
                                                        /* now, p points to the end of the word */
        if (p - word <= 2)
                return;
        switch (*--p)
        {
                case '\'': 
                        {
                                if ((*--p | ' ') == 's')
                                {
                                        p[1] = '\0';
                                        wword (tp, word, "+'", 2);
                                        *p = '\0';
                                        wword (tp, word, "+s'", 3);
                                        if((*--p|' ')=='e')
                                        {

                                                *p = '\0';
                                                wword(tp,word,"+es'", 4);
                                                if((*--p|' ')=='i')
                                                {
                                                        *p = 'y';
                                                        wword(tp,word,"-y+ies'", 5);
                                                }
                                        }
                                        return;
                                }
                        }
                case 'S': 
                case 's': 
                        *p = 0;
                        if ((*--p | ' ') == '\'')
                        {
                                *p = '\0';
                                wword (tp, word, "+'s", 3);
                                return;
                        }
                        if(((*p|' ')=='u'&&(p[-1]|' ')=='o'))
                                return;
                        if((*p|' ')!='s')
                        {
                                wword (tp, word, "+s", 1);
                        if ((*p | ' ') == 'g')
                        {
                                if ((*--p | ' ') == 'n' && (*--p | ' ') == 'i')
                                {
                                        *p = '\0';
                                        wword (tp, word, "+ings", 1);
                                }
                                return;
                        }
                        }
                        if ((*p | ' ') != 'e')
                                return;
                        if ((*(p - 1) | ' ') == 'i')
                        {
                                *p = 0;
                                *--p = 'y';
                                wword (tp, word, "-y+ies", 3);
                                return;
                        }
                        *p = 0;
                        wword (tp, word, "+es", 3);
                        return;
                case 'D': 
                case 'd': 
                        *p = 0;
                        if ((*--p | ' ') != 'e')
                                return;
                        if ((*(p - 1) | ' ') == 'i')
                        {
                                *p = 0;
                                *--p = 'y';
                                wword (tp, word, "-y+ied", 2);
                                return;
                        }
                        wword (tp, word, "+d", 1);
                        *p = 0;
                        wword (tp, word, "+ed", 2);
                        return;
                case 'G': 
                case 'g': 
                        if ((*--p | ' ') != 'n')
                                return;
                        if ((*--p | ' ') != 'i')
                                return;
                        *p = 0;
                        wword (tp, word, "+ing", 1);
                        *p = 'e';
                        *++p = 0;
                        wword (tp, word, "-e+ing", 2);
                        return;
                case 'R': 
                case 'r': 
                        if ((*--p | ' ') != 'e')
                                return;
                        if ((*(p - 1) | ' ') == 'i')
                        {
                                *p = 0;
                                *--p = 'y';
                                wword (tp, word, "-y+ier", 2);
                                return;
                        }
                        *p = 0;
                        wword (tp, word, "+er", 1);
                        return;

                case 'Y': 
                case 'y': 
                        if ((*--p | ' ') != 'l')
                                return;
                        *p = 0;
                        wword (tp, word, "+ly", 1);
                        return;

                case 'T': 
                case 't': 
                        if ((*--p | ' ') != 's')
                                return;
                        if ((*--p | ' ') != 'e')
                                return;
                        if ((*(p - 1) | ' ') == 'i')
                        {
                                *p = 0;
                                *--p = 'y';
                                wword (tp, word, "-y+iest", 1);
                                return;
                        }
                        *p = 0;
                        wword (tp, word, "+est", 1);
                        return;
        }
}
wword (tp, w, s, l)
int    *tp;
char   *w, *s;
int     l;
{
        if (l != 0)
                xword (tp, w, s, l);
        switch (*w)
        {
                case 'R': 
                case 'r': 
                        if ((w[1] | ' ') == 'e')
                                xword (tp, &w[2], s, l + 10);
                        break;
                case 'U': 
                case 'u': 
                        if ((w[1] | ' ') == 'n')
                                xword (tp, &w[2], s, l + 20);
                        break;
                case 'N': 
                case 'n': 
                        if ((w[1] | ' ') == 'o' && (w[2] | ' ') == 'n')
                                xword (tp, &w[3], s, l + 30);
                        break;
                case 'M': 
                case 'm': 
                        if ((w[1] | ' ') == 'u' && (w[2] | ' ') == 'l'
                                        && (w[3] | ' ') == 't' && (w[4] | ' ') == 'i')
                                xword (tp, &w[5], s, l + 40);
        }
}

xword (tp, word, s, l)
int    *tp;
char   *s, *word;
int     l;
{
        register char  *p;
        register char   q;

        for (p = word; *p != 0;)
        {
                q = *p++ | ' ';
                if (q == 'a' || q == 'e' || q == 'i' || q == 'o' || q == 'u' || q == 'y')
                        goto prt;
        }
        if (*s == '.' && word[0] != '\0')
                goto prt;
        return;


prt: 
        wadd (tp, word, l);
}
