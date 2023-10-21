
/* fontbanner 21/3/1985 */

/*
 * fontbanner: print banners using UNIX fonts
 * 
 * fontbanner [-vh] [-12] [-f font] [-d dir] [strings] ...
 *
 * Author: Marco Mercinelli (CSELT - IS) mcvax!i2unix!cselt!marco
 */

#define COPYRIGHT  "fontbanner 1985 - by Marco Mercinelli, CSELT-IS (Italy)"

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <ctype.h>
#include <vfont.h>

#ifdef VFONT_MAGIC
short	fontmagic = VFONT_MAGIC;
#else
short	fontmagic = 0436;
#endif

#define DEFAULTDIR   "/usr/lib/vfont"
#define DEFAULT_FONT "bocklin.14"

#define	WHITE	    ' '
#define	BLACK	    '#'
#define	BLACK2	    'O'
#define	HALFRIGHT   ')'
#define	HALFLEFT    '('
#define	NEXT	    '\n'

#define MAXUNPRINTABLE 10 /* Max number of unprintable char (> \177) allowed */

char *USAGE="usage:fontbanner [-vh] [-12] [-f font] [-d dir] [strings] ...\n";

char fontsdir[256];

struct header   FontHeader;
struct dispatch disptable[256] ;

int     fontfd      = -1;    /* File descriptor del file font                */
long	fbase;               /* Puntatore all'inizio delle bit map nel file  */
short 	vertical    = 0;     /* Indica banner verticale o orizzontale        */
short	nochange    = 1;     /* Indica se e' cambiato il font corrente       */
short	fontchanged = 0;     /* Indica se non e' stata data risposta a Font: */
char    style       = '1';   /* Stile di plottaggio ('1' o '2')              */
short	istty;		     /* Indica se stdout e' una tty		     */
char    fontname[80];	     /* Nome del font corrente                       */
short	error       = 0;     /* Indica un errore nel caricamento di un font  */
short	defaultfdir = 1;     /* Indica che la dir dei fonts e' DEFAULTFDIR   */
short	unprintable = 0;     /* Caratteri non stampabili (> \177) incontrati */
char	newline	    = '\n';
int     interactive_mode = 1; 

extern int errno;

char *gets(), *strcpy(), *strncpy(), *strcat(), *strncat(), *sprintf(),
     *malloc(), *realloc(),
     *listfonts();

long lseek();


main(argc,argv)

int    argc;
char **argv;

{
    fbase = sizeof(FontHeader)+sizeof(disptable);

    *fontname = *fontsdir = '\0';

    while ( --argc  && (**++argv == '-') ) /* tratta argomenti */
        getoptions(&argc, &argv);

    interactive_mode = !argc; /* =1 se non ci sono stringhe da stampare */
    istty = isatty(1);

    if ( defaultfdir )
        if ( accessdir(DEFAULTDIR) )
            (void)strncpy(fontsdir, DEFAULTDIR, sizeof(fontsdir));
        else {
            fprintf(stderr,
	            ".... Cannot open default fonts directory \"%s\"\n",
		    DEFAULTDIR);
            if ( !interactive_mode )
		exit(2);
            *fontsdir = '\0';
            defaultfdir = 0;
        }

    if ( interactive_mode )
        interactive();
    else
        not_interactive(argc, argv);

    if ( !istty )
        printf("\n\n%s\f\n", COPYRIGHT);

} /* main */

/*- NOT INTERACTIVE COMMAND HANDLER -------------- ------------------------*/

not_interactive(argc, argv)

int    argc;
char **argv;

{
    register char *c;

    if ( fontfd < 0 ){
        if ( defaultfdir ){ 
            /*
             * CARICA FONT DI DEFAULT
             */
            if ( (fontfd = load_font(DEFAULT_FONT)) < 0 ){
                fprintf(stderr,
		       ".... Cannot load default font \"%s\"\n", DEFAULT_FONT);
                exit(4);
            }
	} else {
	    fprintf(stderr, ".... No font specified\n");
	    goto bad;
	}
    }


    /*
     * TRATTA ARGOMENTI RIMANENTI
     */
    do {
        if ( error ) /* errore nel caricamento del font specificato */
            goto bad;

	if ( *(c = *argv) != '-' ) {
            /*
             * E' UNA STRINGA DA PLOTTARE
             */
            if ( *c == '\\' && ( c[1] == '-' || c[1] == '\\' ) )
                /*
                 * '\' elimina il significato di '-' o '\'
                 */
                c++;

            plot_string(c);

        } else 
    	    getoptions(&argc, &argv);

	argv++;

    } while ( --argc );

bad:
    fputs(USAGE, stderr);
    exit(7);

} /* not_interactive */

/*-- INTERACTIVE COMMAND HANDLER --------------------------------------------*/

interactive()

{
    register char *s;

    char  string[256];
    int   newfontfd;
    short notify = 1;


    if ( !*fontsdir )
        (void)strcpy(fontsdir, ".");

    fprintf(stderr, ".... Current fonts directory is \"%s\"\n", fontsdir);

    if ( defaultfdir && !error && fontfd < 0 ){
        /*
         * Se non ho specificato nessun font come argomento
         * cerca di caricare il font di default
         */
        if ( (fontfd = load_font(DEFAULT_FONT)) < 0 ) {
            fprintf(stderr, ".... Cannot load default font \"%s\"\n", DEFAULT_FONT);
            *fontname = '\0';
        } else {
            (void)strcpy(fontname, DEFAULT_FONT);
        }
    }

    for(;;){

        if ( fontfd < 0 ){
            fputs(".... Please choose a font file\n", stderr);
        } else do {

            /*
             * NOTIFICA IL FONT CORRENTE E IL MODO DI STAMPA
             * SE C'E' STATA UNA MODIFICA
             */

            if ( notify ){
                /* (void)fputc(newline, stderr); */
                fputs("\n[ Current font is ", stderr);
                if ( fontchanged )
                    fputs("now ", stderr);
                if ( nochange ){
                    fputs("still ", stderr);
		    nochange = 0;
		}
                fprintf(stderr, "\"%s\",", fontname);
                fprintf(stderr, " plot style %c ", style);
                if ( style == '1' )
                    fputs("(compact),", stderr);
                else
                    fputs("(accurate),", stderr);
                if ( vertical )
                    fputs(" vertical", stderr);
                else
                    fputs(" horizontal", stderr);
                (void)fputs(" ]\n", stderr);
                notify = 0;
            }
    
            /*
             * CHIEDE MESSAGGIO
             */
            fputs("Message : ", stderr);
            if ( (s = gets(string)) == NULL ){
                /*
                 * EOF: fine del lavoro
                 */
                (void)fputc(newline, stderr);
                return;
            }
            if ( *s )
                plot_string(s);
        } while ( *s );
    
        /*
         * CHIEDE FONT
         */

askfont:
        error = 0;

        /* (void)fputc(newline, stderr); */
        fputs("Font    : ", stderr);
        if ( (s = gets(string)) == NULL ){
            /*
             * EOF: fine del lavoro
             */
            (void)fputc(newline, stderr);
            return;
        }

        while ( isspace(*s) ) s++;
            
        /*
         * TRATTA LE OPZIONI
         */
        while ( *s == '-' ) {
            while ( *++s && !isspace(*s) ){ /* gruppo di opzioni */

                switch ( *s ){

                case 'v': /* VERTICAL */
                    if ( !vertical ){
                        vertical = 1;
                        notify = 1;
                    }
                break;

                case 'h': /* HORIZONTAL */
                    if ( vertical ){
                        vertical = 0;
                        notify = 1;
                    }
                break;

                case '1': /* PLOTTING STYLE 1/2 */
                case '2':
                    if ( style != *s ){
                        style = *s;
                        notify = 1;
                    }
                break;

                case 'f': /* -f default */
                break;

                case 'd': /* CHANGE FONTS DIRECTORY */
                    
                    /* elimina blanks */
                    for ( s++ ; *s && isspace(*s) ; s++ );

                    if ( !*s )
                        fputs(".... Missing fonts directory name\n", stderr);
                    else {

                        register char *d;
                        register int   i;
                        char pathbuf[100];

                        d = pathbuf;
                        i = 1;
                        while ( *s && !isspace(*s) && ++i < sizeof(fontsdir))
                            *d++ = *s++;
                        *d = '\0';
                        s--;
                        if ( !accessdir(pathbuf) ){
                            fprintf(stderr,
				    ".... Cannot access fonts directory %s \n",
				    pathbuf);
                            fprintf(stderr,
				    ".... Fonts directory is still \"%s\"\n",
				    fontsdir);
                        } else {
                            (void)strncpy(fontsdir, pathbuf, sizeof(fontsdir));
                            fprintf(stderr,
				".... Current fonts directory is now \"%s\"\n",
				fontsdir);
                        }
                    }

		    goto askfont;
                break;

                case 'l': /* LIST FONTS */
                    
                    /* elimina blanks fino al pattern */
                    for ( s++ ; *s && isspace(*s) ; s++ );

                    s = listfonts(s);

                    /* s deve puntare all'ultimo dei */
                    /* caratteri considerati         */
                    s--;

                    /* Si presuppone che se l'utente ha  */
                    /* chiesto la lista dei font voglia  */
                    /* cambiare il font corrente per cui */
                    /* viene ripetuta la richiesta del   */
                    /* nome di un font                   */

                    goto askfont;
                break;

                default:
                    fprintf(stderr, ".... Bad option \"%c\"\n", *s);
                    error = 1;
		    goto askfont;
                }
            }
            while ( *s && isspace(*s) ) s++;
        }

        newfontfd   = -1;
        fontchanged = 0;
        if ( *s && !error ){
            /*
             * E' stato specificato il nome di un nuovo font
             */
            while ( isspace(*s) ) s++;
            if ( strcmp(fontname, s) ){
                /* E' il nome di un nuovo font */
                if ( (newfontfd = load_font(s)) >= 0){
                    fontfd = newfontfd;
                    (void)strcpy(fontname, s);
                    fontchanged = 1;
                    notify = 1;
                } else {
		    /* errore nel caricamento del font. Ripete domanda */
                    error = 1;
		    goto askfont;
		}
            }
        } else {
	    if ( !notify ){
		/* non ci sono cambiamenti di nessun tipo segnala errore */
		/* per la notify                                         */
	        notify = 1;
		nochange = 1;
	    }
	}
    } /* for ever */

    /* NOT REACHED */

} /* interactive */


/*-- UTILITY ROUTINES -------------------------------------------------------*/

/*
 * GETOPTIONS tratta un insieme di opzioni in modo non interattivo
 *            esce appena trova un argomento che non e' una opzione
 */
getoptions(pargc, pargv)

int   *pargc;
char **pargv[];
{
    register int    argc;
    register char **argv;
    register char  *c;

    argc = *pargc;
    argv = *pargv;
    c    = *argv;

    /*
     * TRATTA OPZIONI
     */
    while ( *++c ){

        switch ( *c ){
    
        case 'v':
            vertical = 1;
        break;

        case 'h':
            vertical = 0;
        break;

        case '1':
        case '2':
            style = *c;
        break;

        case 'd': /* change default fonts directory */

            if ( ! --argc ){
                fputs(".... Missing fonts directory name\n", stderr);
		goto bad;
            }
            (void)strncpy(fontsdir, *++argv, sizeof(fontsdir));
            if ( !accessdir(fontsdir) ){
                fprintf(stderr,
		      ".... Cannot access fonts directory \"%s\"\n", fontsdir);
		goto bad;
            }
            defaultfdir = 0;
        break;

        case 'f':
            /*
             * carica un nuovo font
             */
            if ( ! --argc ){
                fputs(".... Missing font name\n", stderr);
		goto bad;
            }
            if ((fontfd = load_font(*++argv)) < 0){
                    error = 1; 
            } else {
                (void)strncpy(fontname, *argv, sizeof(fontsdir));
		fontchanged = 1;
	    }
        break;

        default:
            fprintf(stderr, ".... Bad option \"%c\"\n", *c);
	    goto bad;

        } /* switch */
    } /* while */

    *pargv = argv;
    *pargc = argc;

    return;

bad:
    fputs(USAGE, stderr);
    exit(4);
} /* getoptions */

/*
 * LISTFONTS esegue il comando:
 *           ls -F fontsdir/pattern >&2     se e' passato un pattern valido
 * altrimanti esegue
 *           ls -F -d fontsdir/* >&2
 * Ritorna il puntatore al primo carattere dopo il pattern
 */

char *
listfonts(paddr)

register char *paddr;
{
    register char *p;
    char  shcmd[100], buf[100];
    short slashs = 0; /* indica che il pattern contiene degli '/' */
    char  cbuf;

    fputs(".... List \"", stderr);

    p = paddr;
    if ( *p && !isspace(*p) ){ /* c'e' un pattern specificato */

	/* cerca fine pattern e gli mette temporaneamente \0 e */
	/* intanto controlla se il pattern contiene degli '/'  */
        for ( p; *p && !isspace(*p) ; p++){
            if ( *p == '/' )
	        slashs = 1;
	    if ( *p == '&' || !isprint(*p) ) /* puo' confondere lo shell */
               	*p = '?';
	}
	cbuf = *p;
	*p = '\0';

        /* controlla se il pattern contiene un path */
        /* se non lo contiene premette  cd fontsdir */

        if ( !slashs && *paddr != '.' ){
            /* il pattern non contiene un path     */
            /* esegue cd fontsdir;ls -F -d pattern */
            (void)sprintf(shcmd,"cd %s;ls -F -d %s", fontsdir, paddr);
	    (void)sprintf(buf, "%s/%s", fontsdir, paddr);
	    if ( accessdir(buf) )
		(void)strcat(shcmd, "/*");
            fprintf(stderr, "%s/", fontsdir);
        } else {
            /* il pattern contiene un path  */
            /* esegue ls -F -d pattern      */
            (void)sprintf(shcmd, "ls -F -d %s", paddr);
	    if ( accessdir(paddr) )
		(void)strcat(shcmd, "/*");
	    if ( *paddr != '.' && *paddr != '/' )
            	fputs("./", stderr);
        }

        (void)fputs(paddr, stderr);
	/* ripristina carattere di fine pattern */
	*p = cbuf;

    } else { /* non c'e' un pattern specificato */

        /* esegue cd fontsdir;ls -F */
        (void)sprintf(shcmd, "cd %s;ls -F", fontsdir);
        fprintf(stderr, fontsdir);
    }
    (void)strcat(shcmd, " >&2");
    (void)fputs("\"\n", stderr);

    if ( system(shcmd) )
        fputs( ".... Shell error\n", stderr);

    return(--p);

} /* listfonts */

/*
 * ACCESSDIR retorna 1 se il path si riferisce a una directory
 */
accessdir(path)

char *path;
{
	struct stat statbuf;

	if ( stat(path, &statbuf) < 0 )
		return(0);
	return( ((statbuf.st_mode & S_IFMT) == S_IFDIR) && 
		(statbuf.st_mode & S_IEXEC) );
}

/*-- PLOTTING ROUTINES ------------------------------------------------------*/

static short backslash;
static short controlchar;

plot_string(string)

char * string;
{
	register char *s;
	char  ch;

	backslash = 0;
	controlchar = 0;
	s = string;
	while ( (ch = *s++) != '\0' ){

		if ( ch < ' ' ){
			controlchar++;
			ch += '\100';
			plot_char(ch);
			continue;
		}

		if ( ch > '\177' ){
			fprintf(stderr,
				"unprintable char '\\%o' skipped\n", ch); 
			if ( ++unprintable > MAXUNPRINTABLE ){
				fputs(".... Too many unprintable chars\n", 
				      stderr);
				exit(1);
			}
			continue;
		}

		/* '\' toglie al carattere successivo l'eventuale */
		/* significato speciale ( \\ \^ )                 */

	   	if ( backslash ){
			backslash = 0;
			plot_char(ch);
			continue;
	   	}

		switch ( ch ){

		case '\\':
				if ( *s )
					backslash = 1;
				else
					plot_char('\\');
		break;

		/* se un carattere e' preceduto da '^' stampa i caratteri   */
		/* corrispondenti da '\000' a '\037' (da cntrl-@ a cntrl-_) */

		case '^':
			if ( controlchar || !*s || !(*s >= '@' && *s <= '_' ) )

			/* Se il carattere precedente era '^': control ^   */
			/* oppure sono a fine stringa                      */
			/* oppure il carattere successivo non e' compreso  */
			/* fra '@' e '_'  (non sarebbe un control...)      */

				plot_char('^');
			else
				controlchar = 1;
		break;

		default:
			plot_char(ch);

		}/*switch*/
	}/*while*/

	if ( !istty )
		/*
		 * se la stampa non e' su tty separa le stringhe con blank
		 */
		plot_char(' ');

} /* plot_string */




plot_char(c)

char c;
{

register struct dispatch *pdisp; /* Dispatch table del carattere              */
register int      len, col, bit;
	 int      ch;	         /* Carattere da plottare (ch = c)            */
static   int      prevch = -1;   /* Ricorda l'ultimo carattere plottato       */
static   char *charbits;         /* Buffer per la bitmap del carattere        */
static   unsigned buflen = 0;    /* Attuale dimensione del buffer             */
	 unsigned bmlen;	 /* Numero di byte componenti la bitmap       */
static	 int      H;	         /* Altezza della bitmap del carattere in bit */
static   int      W;	         /* Larghezza della bitmap carattere in bit   */
static   int      WB;	         /* Numero di byte in ogni riga della bit map */
static   int      HB;	         /* Indice dell'ultimo byte della prima col.  */
			         /* (primo byte dell'ultima linea)            */
	 char    *byteind;	 /* Punta al byte della bit map da plottare   */
register short    thisbyte;      /* Contiene il byte puntato da byteind       */
register short    thisbit;       /* Maschera per il bit corrente in thisbyte  */
	 int      vspace;        /* Spazio fra 2 caratteri in verticale       */



	ch = c;

	if ( controlchar )
		ch -= 0100;

	pdisp = &(disptable[ch]);

	if ( (ch != ' ') && ((bmlen = pdisp->nbytes) == 0) ){
	   	fputs("character  \"", stderr);
		if ( controlchar )
			(void)fputc('^', stderr);
		fprintf(stderr,"%c\" is not provided by font\n",c);
		if ( istty )
			goto out; 
		ch = ' ';
	}

   	if ( ch == ' ' ){
		/* 
		 * TRATTA CARATTERE BLANK
		 */
		if ( vertical ){
			/* Somma l'altezza del carattere '|' se */
			/* questo non e' previsto prova con il  */
			/* carattere 'a'. Se anche questo non   */
			/* e' previsto somma la massima altezza */

	      		if ( disptable['|'].nbytes )
		 		bmlen = disptable['|'].up;
			else {
	      			if ( disptable['a'].nbytes )
		 			bmlen = disptable['a'].up;
	      			else
		 			bmlen = FontHeader.maxx;
			}
		} else {
   			/* Se il carattere blank non e' previsto allora   */
			/* sommo solo la larghezza del carattere 'a' e se */
			/* anche questo non e' previsto prendo la massima */
			/* larghezza di un carattere                      */

			if ( pdisp->nbytes == 0 ){
	      			if ( disptable['a'].nbytes )
		 			bmlen = disptable['a'].width;
	      			else
		 			bmlen = FontHeader.maxx;
			}
			else
				bmlen = pdisp->width;
		}
		for ( col=0 ; col < bmlen ; col++ )
			plot_dot(NEXT);
	} else {
		/*
		 * TRATTA I CARATTERI STAMPABILI
		 */

		if ( prevch != ch || fontchanged ){

			/* se il carattere non e' uguale a quello precedente */
			/* (o non ho cambiato font)                          */

			/* LEGGE LA BITMAP DEL CARATTERE */

			if (lseek(fontfd,(long)(fbase+pdisp->addr),0)<(long)0){
				perror("lseek");
				exit(errno);
			}
			if ( buflen ){
				if ( bmlen > buflen ){
					/*
					 * l'attuale dimensione del buffer
					 * non e' sufficiente quindi questo
					 * viene espanso
					 */
					charbits = realloc(charbits, bmlen);
					if ( charbits == NULL){
						perror("realloc");
						exit(errno);
					}
					buflen = bmlen;
				}
			} else {
				/* non esiste ancora un buffer allocato */
				if ((charbits = malloc(buflen=bmlen)) == NULL){
					perror("malloc");
					exit(errno);
				}
			}
			if ( read(fontfd, charbits, (int)bmlen) != bmlen ){
				fputs(".... Error reading bit maps\n", stderr);
				perror("read");
				exit(2);
			}

			/* ALTEZZA DELLA BITMAP (in bit) */

			H = (pdisp->up) + (pdisp->down);

			/* LARGHEZZA DELLA BITMAP (in bit) */

			W = (pdisp->left) + (pdisp->right);

			/* NUMERO DI BYTES CHE FORMANO UNA RIGA DELLA BITMAP */

			WB = (W+7)>>3;

			/* INDICE DELL'ULTIMO BYTE DELLA PRIMA COLONNA */

			HB = (H-1)*WB;
		}

		/*
		 * PLOTTA CARATTERE
		 */

		thisbit  = 0x80;    /* primo bit in un byte     */

		if ( vertical ){

			/*
			 * CARATTERE PLOTTATO IN VERTICALE
			 */
			byteind = charbits; /* primo byte della bit map */
			thisbyte = *byteind & 0xff;

			for ( len=0 ; len < H ; len++ ){
				/* per tutte le linee */

				/* GLI SPAZI A SINISTRA DEL CARATTERE NON  */
				/* SONO COMPRESI NELLA BIT MAP SE left < 0 */

				for ( col=pdisp->left ; col < 0 ; col++ )
					plot_dot(WHITE);

				for ( bit = 0 ; bit < W  ; bit++ ){
					/* per tutti i bit di una linea */

					if ( thisbyte & thisbit )
						/* se il bit e' settato */
						plot_dot(BLACK);
					else
						plot_dot(WHITE);

					thisbit >>= 1; /* bit successivo */
					if ( !thisbit ){
						/*
						 * Passa al byte successivo
						 */
						thisbyte = *++byteind & 0xff;
						thisbit = 0x80;
					}
				}

				if ( thisbit != 0x80 ){
					/*
					 * L'ultimo byte della linea non era
					 * totalmente utilizzato:  forza il
					 * il passaggio al byte successivo 
					 */
					thisbyte = *++byteind & 0xff;
					thisbit = 0x80;
				}
					
				plot_dot(NEXT);
			}

			/*
			 * SE down < 0 GLI SPAZI SOTTO IL CARATTERE NON SONO
			 * COMPRESI NELLA BITMAP:  QUINDI DEVO AGGIUNGERLI ...
			 */

			/* OLD: lascia  troppo o troppo poco spazio !!!
			 *
			 * for ( len=pdisp->down ; len < 0 ; len++ )
			 *     plot_dot(NEXT);
			 */

			/* NEW:
			 * lascio sempre un ottavo di maxy (se down < 0)
			 * e comunque al minimo 1 carattere
			 */
			if ( pdisp->down < 0 ) {
				if ( fontchanged )
					/* ricalcola vspace */
			       		if ( (vspace=FontHeader.maxy>>3) == 0)
						vspace = 1;
				for ( len=0 ; len < vspace ;  len++ )
				    	plot_dot(NEXT);
			}
		} else {
			/*
			 * CARATTERE PLOTTATO IN ORIZZONTALE
			 */

			/* GLI SPAZI A SINISTRA DEL CARATTERE NON SONO */
			/* COMPRESI NELLA BIT MAP SE left < 0          */

			for ( col=pdisp->left ; col < 0 ; col++ )
				plot_dot(NEXT);

			byteind = &charbits[HB]; /* 1o byte ultima linea */

			for ( col=0 ; col < W ; col++ ){
				/* per tutte le colonne */

				/* GLI SPAZI SOTTO IL CARATTERE NON SONO */
				/* COMPRESI NELLA BIT MAP SE down < 0    */

				for ( bit=pdisp->down ; bit < 0 ; bit++ )
					plot_dot(WHITE);

				thisbyte = *byteind & 0xff;

				for ( bit = H-1 ; bit >= 0  ; bit-- ){
					/* per tutti i bit di una colonna */

					if ( thisbyte & thisbit )
						/* il bit e' settato */
						plot_dot(BLACK);
					else
						plot_dot(WHITE);

					byteind -= WB; /* linea precedente */
					thisbyte = *byteind & 0xff;
				}
				byteind += HB+WB; /* Torna a ultima linea    */
						  /* NB non cambia colonna   */

				thisbit >>= 1;    /* bit successivo nel byte */

				if ( !thisbit ){
					/*
					 * Passa al byte successivo stessa linea
					 * (ultimo byte colonna successiva)
					 */
					++byteind;
					thisbit = 0x80;
				}

				plot_dot(NEXT);
			}
			/* LASCIA LO SPAZIO FINO AL PROSSIMO CARATTERE */
			for( col=pdisp->width - W ; col > 0  ; col-- )
				plot_dot(NEXT);
		}
	}
out:
	prevch = ch;
	if ( controlchar )
		controlchar = 0;

} /* plot_char */




#define PSIZE	(float)1.66 /* rapporto fra altezza e larghezza di un */
			    /* carattere sulla stampante              */

plot_dot(newpoint)

char newpoint;

/* Prepara la linea di stampa in base al valore di newpoint:
 *
 * WHITE aggiunge spazi bianchi;
 * BLACK aggiunge un punto nero
 * NEXT  stampa la linea corrente e passa alla successiva
 *
 * Se si deve tracciare un segmento (bianco o nero) la sua lunghezza in
 * caratteri della stampante viene calcolata approssimando la dimensione
 * di ogni punto che la compone a 1.66 caratteri 
 */

{
	static char  line[256];		/* Buffer di stampa                */
	static char *firstfree;		/* Prima posizione libera in line  */
	static char  previous  = NEXT;  /* Carattere precedente            */
	static int   pointnum;		/* Lunghezza del segmento in punti */
	static float reminder = 0.0;	/* resto dell'arrotondamento prec. */

	int 		numchar;        /* lunghezza segmento in caratteri */
	double 		vnumchar;       /* lunghezza virtuale segmento     */
	short		halfleft  = 0;
	short		halfright = 0;
	char		newchar;

	/* Se il carattere attuale e' diverso dal precedente vuol */
	/* dire che e' cambiato il tipo di segmento: calcolo la   */
	/* lunghezza del segmento in caratteri della stampante e  */
	/* riempio opportunamente line.                           */
	/* Se il carattere attuale e' uguale al precedente        */
	/* incremento il conto della lunghezza del segmento       */
	/* Se lo stile e' il secondo: usa BLACK2 invece di BLACK  */
	/* se la lunghezza del segmento e' compresa fra 0.33333 e */
	/* 0.66666 viene stampato un carattere intermedio che e'  */
	/* HALFLEFT se inizia un segmento BLACK e HALFLEFT se lo  */
	/* termina.                                               */

	if ( newpoint == previous ){
		/*
		 * PERMANENZA
		 */
		if ( newpoint == NEXT ) { /* LINEA VUOTA */
			if ( write(1, &newline, 1) < 0 ){
				perror("write");
				exit(errno);
			}
		} else
			pointnum++; /* incrementa la lunghezza del segmento */
	} else { 
		/*
		 * VARIAZIONE
		 */
		if ( previous == NEXT ){
			/*
			 * INIZIO LINEA NON VUOTA O PRIMISSIMA ESECUZIONE
			 */
			firstfree = line;
			pointnum = 1;
			reminder = 0.0;
		} else {
			/*
			 * inserisco segmento precedente in line
			 */

			register int 	i;
			register char  *f; /* firstfree */

			f = firstfree;

			vnumchar = PSIZE*pointnum + reminder;
			numchar  = vnumchar;
			reminder = vnumchar - numchar;

			if ( style == '2' ){
				if ( reminder >= 0.66666 ){
					/*
					 * approssima per eccesso
					 */
					numchar++;
					reminder -= 1.0;
				} else {
					if ( reminder >= 0.33333 ){
						/*
						 * approssimazione intermedia
						 */
						if ( previous == BLACK )
							halfright++;
						else
							if ( newpoint == BLACK )
								halfleft++;
						reminder -= 1.0;
					}
				} /* altrimenti approssima per eccesso */

				if ( previous == BLACK )
					newchar = BLACK2;
				else
					newchar = previous;

				for ( i=0 ; i < numchar ; i++ )
					*f++ = newchar;

				if ( halfright ){
					*f++ = HALFRIGHT;
				} else
					if ( halfleft ){
						*f++ = HALFLEFT;
					}

			} else { /* style 1 */
				if ( reminder >= 0.5 ){
					/*
					 * approssima per eccesso
					 */
					numchar++;
					reminder -= 1.0;
				}
				for ( i=0 ; i < numchar ; i++ )
					*f++ = previous;
			}
					
			if ( newpoint == NEXT ){ /* FINE LINEA */
				/*
				 * STAMPA LINEA
				 */

				*f++ = newline;
				*f = '\0';
				if ( write(1, line, strlen(line)) < 0 ){
					perror("write");
					exit(errno);
				}
				pointnum = 0;
			} else {
				/*
				 * TRANSIZIONE DA WHITE A BLACK O VICEVERSA
				 */
				pointnum = 1;
			}
			firstfree = f;
		}
		previous = newpoint;
	}
} /* plot_dot */


/*-- LOAD FONT ROUTINES -----------------------------------------------*/

load_font(fontfile)

char *fontfile;

/* This routine reads in the appropriate font file */


{
    register char *pf;
    char           fontpath[100];
    int            f;
    short	   doinvert=0;


    fontpath[0] = '\0';

    /* vede se il nome del font contiene gia' un pathname */
    pf = fontfile;
    while(*pf && *pf != '/' ) pf++;

    if ( *pf != '/' ){ /* aggiungo il path */
    	(void)strcpy(fontpath, fontsdir);
	(void)strcat(fontpath, "/");
    }

    (void)strcat(fontpath,fontfile);

    if((f = open(fontpath,0)) < 0 ){
	fprintf(stderr,".... Cannot open font file \"%s\"\n",fontpath);
	return(-1);
    }

    /* Get the FontHeader and check magic number */

    if(read(f,(char *)&FontHeader,sizeof(FontHeader)) != sizeof(FontHeader)){
	fprintf(stderr,".... Bad FontHeader in font file %s\n",fontpath);
	return(-1);
    }

    if(FontHeader.magic != fontmagic){
	/*
	 * L'header e' formato da short che su vax sono di 2 byte.
	 * Su alcuni processori (es. sun) i byte in uno short sono invertiti.
	 * Prova quindi a invertire prima di dichiarare bad magic
	 */
	
    	invert((u_short *)&FontHeader.magic);

    	if(FontHeader.magic != fontmagic){
	    fprintf(stderr,".... Bad magic number in font file %s\n", fontpath);
	    return(-1);
	} else {
    	    invert(&FontHeader.size);
	    invert((u_short *)&FontHeader.maxx);
	    invert((u_short *)&FontHeader.maxy);
	    invert((u_short *)&FontHeader.xtend);
	    doinvert = 1;
	}
    }

    /* Get disptablees */
    if(read(f,(char *)disptable,sizeof(disptable)) != sizeof(disptable)){
	fprintf(stderr,".... Bad disptable array in font file %s\n",fontpath);
	return(-1);
    }

    if ( doinvert ) {
        register int i;
	register struct dispatch *pd;

	pd = disptable;
 
        for ( i = 0 ; i<256 ; i++ ){
            invert((u_short *)&(pd->nbytes));
            invert((u_short *)&(pd->addr));
            invert((u_short *)&(pd->width));
            pd++;
        }
    }
    return(f);
} /* load_font */

/*
 * Inverte i due byte in uno short (16 bit)
 */
invert(s)

u_short *s;
{
        char tmp;
        register char *c1,*c2;

        c1 = c2 = (char *)s;
        c2++;
        tmp = *c1;
        *c1 = *c2;
        *c2 = tmp;
}
