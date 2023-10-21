/*
 *      These defines contain a list of a major magic
 *      numbers for data files.  Used by lpr.c
 *      and perhaps file.c
 */
#define ARMAGIC  0177555 /* archive (old format) */
#define NARMAGIC 0177554 /* source archive (narc) */
#define NETMAGIC 0177553 /* netarc archve (nethelp) */
#define ARCMAGIC 0177545 /* new archiver format */
#define VMAGIC   0405    /* for overlays etc. */
#define FMAGIC   0407    /* standard executable */
#define NMAGIC   0410    /* pure, shareable executable */
#define IMAGIC   0411    /* separate I/D executable */
