#define checkbits(N,CP,L) { \
    N = 1; \
    L = 0; \
    while (N != 0) { \
	N <<= 1; \
	L++; \
    } \
    printf("%s   %4d   %6d    ", CP, L, sizeof(N)); \
    N = 1; \
    N = ~N; \
    switch (N) { \
        case -1 : printf("1's c.\n"); break; \
	case -2 : printf("2's c.\n"); break; \
	default : if (N < 0) { \
	    printf("  mag.\n"); \
	} else { \
	    printf("unsign\n"); \
	} \
    } \
}

#define printbytes(CP,N,L) { \
    long temp = (long) N; \
    printf("%s ",CP); \
    for (i = 0; i < ((L) / bits); i++) { \
        printf("%3d",(int) (temp & 0xFFL)); \
	temp >>= bits; \
    } \
    printf("\n"); \
}

char C;
short S;
int I;
long L;
int icl, isl, iil, ill;

union u {
    char C;
    short S;
    int I;
    long L;
    char FILLER[sizeof(L)];
} U;


main()
{
    int i;
    int bits;
    for (i = 0; i < sizeof(U.L); i++) U.FILLER[i] = (char) (i + 1);
    printf(" Type   Bits   Sizeof   Format\n");
    checkbits(C," Char",icl);
    bits = icl;
    checkbits(S,"Short",isl);
    checkbits(I,"  Int",iil);
    checkbits(L," Long",ill);
    printf("\nPosition ");
    for (i = 0; i < sizeof(U.L); i++) printf("%3d",i);
    printf("\n");
    printbytes("    Char",U.C,icl);
    printbytes("   Short",U.S,isl);
    printbytes("     Int",U.I,iil);
    printbytes("    Long",U.L,ill);
    printf("\n");
    if ( (char) U.S == U.S ) printf("Char == Short\n");
    if ( (short) U.I == U.I ) printf("Short == Int\n");
    if ( (long)((int) (U.L)) == U.L ) printf("Int == Long\n");
}
