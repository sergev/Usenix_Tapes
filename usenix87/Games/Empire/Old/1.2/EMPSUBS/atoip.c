atoip(ptrptr)
char    **ptrptr;
{
        register        num, base;
        register char   *cp;
        short   neg;

        cp = *ptrptr;
        num = 0;
        base = 10;
        neg = 0;
        goto X46;
X26:
        if( *cp != '0' ) goto X124;
        base = 8;
        cp++;
        goto X124;
X44:
        cp++;
X46:
        if( *cp == ' ' ) goto X44;
        if( *cp == '\t' ) goto X44;
        if( *cp != '-' ) goto X26;
        neg++;
        goto X44;
X76:
        if( *cp > '9' ) goto X132;
        num = num * base + *cp++;
        num -= '0';
X124:
        if( *cp >= '0' ) goto X76;
X132:
        *ptrptr = cp;
        return( (neg != 0) ? -num : num );
}
