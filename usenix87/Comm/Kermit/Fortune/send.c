#include "kermit.h"
/*
 *      s e n d s w
 *
 *      Sendsw is the state table switcher for sending
 *      files.  It loops until either it finishes, or
 *      an error is encountered.  The routines called by
 *      sendsw are responsible for changing the state.
 */
sendsw()
{
        gnxtfl ();
        state = 'S';       /* Send initiate is the start state */
        n = 0;             /* Initialize message number */
        numtry = 0;        /* Say no tries yet */
        while(TRUE)        /* Do this as long as necessary */
        {
                switch(state)
                {
                case 'D':  
                        state = sdata(); 
                        break; /* Data-Send state */

                case 'F':  
                        state = sfile(); 
                        break; /* File-Send */

                case 'Z':  
                        state = seof(); 
                        time (&tstop);
                        tstart = tstop - tstart;
                        tstart = filch / tstart;
                        filch = 0;
                        if ( !host ) 
                             fprintf ( stderr,"%d bps\n",tstart );
                        break;  /* End-of-File */
                case 'S':  
                        state = sinit(); 
                        break; /* Send-Init */
                case 'B':  
                        state = sbreak(); 
                        break; /* Break-Send */
                case 'C':  
                        return (TRUE);          /* Complete */
                case 'A':  
                        return (FALSE);         /* Abort */
                default:   
                        return (FALSE);         /* Unknown, abort */
                }
        }
}
/*
 *      s i n i t
 */
sinit()
{
        int num, len;
        if (debug) fprintf(stderr,"sinit\n");
        if (numtry++ > MAXTRY) return('A');   /* If too many tries, abort */
        spar(packet);                         /* Fill up with init info */
        if (debug) fprintf(stderr,"n = %d\n",n);
        spack('S',n,6,packet);                /* Send an 'S' packet */
        switch(rpack(&len,&num,recpkt))       /* What was the reply? */
        {
        case 'N':                           /* NAK */
                if (n != num--) return(state);    
                /* If NAK for next one, like ACK */
        case 'Y':                           /* ACK */
                {
                        if (n != num)       /* If wrong ACK, fail */
                                return(state);
                        rpar(recpkt);       /* Get the init info */
                        if (eol == 0) /* check and set defaults */
                                eol = MYEOL;
                        if (quote == 0) quote = '#';
                        numtry = 0;/* reset try counter */
                        n = (n+1)%64;/* and bump packet count */
                        if (debug) fprintf(stderr,"Opening %s\n",filnam);
                        fd = open(filnam,0);/* Open the file to read */
                        if (fd < 0) return('A');
                        /* if bad file descriptor, abort */
                        filch = 0;
                        time (&tstart);
                        if (!host) fprintf(stderr,"\t\tSending %s\r",filnam);
                        return('F');        /* switch state to F */
                }
        case FALSE: 
                return(state);          /* Receive failure, fail */
        default: 
                return('A');               /* Just abort */
        }
}
/*
 *      s f i l e
 */
sfile()
{
        int num, len;
        if (debug) fprintf(stderr,"sfile\n");
        if (numtry++ > MAXTRY) return('A');/* If too many tries, abort */
        for (len=0; filnam[len] != '\0'; len++); /* count the length */
        len++;
        spack('F',n,len,filnam);              /* send an 'F' packet */
        switch(rpack(&len,&num,recpkt))
        {                                    /* What was the reply? */
        case 'N':                           /* NAK */
                if (n != num--) return(state);
                /* If NAK for next pack its like ACK */
        case 'Y':                           /* ACK */
                {
                        if (n != num) return(state);      
                        /* If wrong ACK, fail */
                        numtry = 0;           /* reset try counter */
                        n = (n+1)%64;       /* and bump packet count */
                        size = bufill(packet);
                        /* get packet of data from file */
                        return('D');      /* switch state to D */
                }
        case FALSE: 
                return(state);          /* receive failure, fail */
        default: 
                return('A');               /* just abort */
        }
}
/*
 *      s d a t a
 */
sdata()
{
        int num, len;
        if (numtry++ > MAXTRY)            /* If too many tries, abort */
                return('A');
        spack('D',n,size,packet);         /* send a 'D' packet */
        switch(rpack(&len,&num,recpkt))
        {                                 /* What was the reply */
        case 'N':                         /* NAK */
                if (n != num--) return(state);
                /* If NAK for next pack, like ACK */
        case 'Y':                           /* ACK */
                {
                        if (n != num) return(state);
                        /* If wrong ACK, fail */
                        numtry = 0;  /* reset try counter */
                        n = (n+1)%64;/* and bump packet count */
                        if ((size = bufill(packet)) == EOF) 
                                /* get the data from file */
                                return('Z');/* if EOF set state to it */
                        return('D');        /* switch state to 'D' */
                }
        case FALSE: 
                return(state);          /* receive failure, fail */
        default: 
                return('A');               /* just abort */
        }
}
/*
 *      s e o f
 */
seof()
{
        int num, len;
        if (debug) fprintf(stderr,"seof\n");
        if (numtry++ > MAXTRY) return('A');   /* If too many tries, abort */
        spack('Z',n,0,packet);                /* Send a 'Z' packet */
        if (debug) fprintf(stderr,"seof1 ");
        switch(rpack(&len,&num,recpkt))
        {                                    /* What was the reply? */
        case 'N':                           /* NAK */
                if (n != num--) return(state);    /* If NAK for next pkt, like ACK */
        case 'Y':                           /* ACK */
                {
                        if (debug) fprintf(stderr,"seof2 ");
                        if (n != num)                     /* if wrong ACK, fail */
                                return(state);
                        numtry = 0;                       /* reset try counter */
                        /* n = (n+1)%64;                  /* and bump packet count */
                        if (debug) fprintf(stderr,"closing %s, ",filnam);
                        close(fd);                        /* close the input file */
                        if (debug) fprintf(stderr,"ok, getting next file\n");
                        if (gnxtfl() == FALSE)            /* No more files go? */
                        {
                                n = (n+1)%64;
                                return('B');                    /* if not, break */
                        }
                        if (debug) fprintf(stderr,"new file is %s\n",filnam);
                        return('S');                      /* switch state to F */
                }
        case FALSE: 
                return(state);          /* Receive failure, fail */
        default: 
                return('A');               /* just abort */
        }
}
/*
 *      s b r e a k
 */
sbreak()
{
        int num, len;
        if (debug) fprintf(stderr,"sbreak\n");
        if (numtry++ > MAXTRY)                /* If too many tries abort */
                return('A');
        spack('B',n,0,packet);                /* Send a B packet */
        switch (rpack(&len,&num,recpkt))
        {                                    /* What was the reply? */
        case 'N':                           /* NAK */
                if (n != num--) return(state);    /* If NAK for next pack, */
                /*  it's like ACK */
        case 'Y':                           /* ACK */
                {
                        if (n != num) return(state);      /* If wrong ACK, fail */
                        numtry = 0;                       /* reset try counter */
                        n = (n+1)%64;                     /* and bump packet count */
                        return('C');                      /* switch state to C */
                }
        case FALSE: 
                return(state);          /* receive failure, fail */
        default: 
                return ('A');              /* just abort */
        }
}
