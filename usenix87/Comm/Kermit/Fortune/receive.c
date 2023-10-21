#include "kermit.h"

/*
 *      receive - routines for receiving data
 */
/*
 *      r e c s w
 */
recsw()
{
        state = 'R';          /* receive is the start state */
        n = 0;                /* initialize message number */
        numtry = 0;           /* say no tries yet */
        while(TRUE) switch(state) /* do this for as long as necessary */
        {
        case 'D':   
                state = rdata(); 
                break; /* data receive state */
        case 'F':   
                state = rfile(); 
                break; /* file receive state */
        case 'R':   
                state = rinit(); 
                break; /* send initiate state */
        case 'C':   
                return(TRUE);           /* complete state */
        case 'A':   
                return(FALSE);          /* unknown state, abort */
        }
}

/*
 *       r i n i t
 */

rinit()
{
        int len, num;
        if (numtry++ > MAXTRY) return('A');/* abort if to many tries */
        switch(rpack(&len,&num,packet))       /* get a packet */
        {
        case 'S':                            /* send-init */
                {
                        rpar(packet);        /* get the init data */
                        spar(packet); /* fill up packet w/ init info */
                        spack('Y',n,6,packet);/* ACK w/ my parameters */
                        oldtry = numtry;      /* save old try count */
                        numtry = 0;        /* and start a new counter */
                        n = (n+1)%64;      /* bump packet number */
                        return('F');       /* enter file-send state */
                }
        case FALSE:  
                return (state);         /* we didn't receive a packet */
        default:     
                return('A');            /* unknown type, abort */
        }
}
/*
 *      r f i l e
 */
rfile()
{
        int num, len;
        if (numtry++ > MAXTRY) return('A');/* abort if too many tries */
        switch(rpack(&len,&num,packet))        /* get packet */
        {
        case 'S':                            /* send-init */
                {
                        if (oldtry++ > MAXTRY)
                                return('A');/* if 2 many tries, abort */
                        if (num == n-1)     /* check packet number */
                        {
                                spar(packet);/* get the init params */
                                spack('Y',num,6,packet);/* it's right, ACK it */
                                numtry = 0;      /* reset try counter */
                                return(state);
                        }
                        else return('A');      /* sorry, wrong number */
                }
        case 'Z':                            /* end-of-file */
                {
                        if (oldtry++ > MAXTRY) return('A');
                        if (num == n-1)   /* acknowledge good packet */
                        {
                                spack('Y',num,0,0);
                                numtry = 0;
                                return(state);
                        }
                        else return('A'); /* or abort on bad one */
                }
        case 'F':                            /* file-header */
                {
                        if (num != n) return('A');
                        if (!getfil(packet))
                        {
                                fprintf(stderr,"Could not create %s\n");
                                return('A');
                        }
                        else
                            if (!host){
                                filch = 0;
                                time ( &tstart );
                                fprintf(stderr,"\t\tReceiving %s\r",packet);
                                }
                        spack('Y',n,0,0);   /* acknowledge */
                        oldtry = numtry;
                        numtry = 0;
                        n = (n+1)%64;
                        return('D');        /* switch to data state */
                }
        case 'B':                            /* break transmission */
                {
                        if (num != n) return ('A');
                        spack('Y',n,0,0);   /* say OK */
                        return('C');      /* and go to complete state */
                }
        case FALSE: 
                return(state);
        default: 
                return ('A');
        }
}
/*
 *      r d a t a
 */
rdata()
{
        int num, len;
        if (numtry++ > MAXTRY) return('A');/* abort if too many tries */
        switch(rpack(&len,&num,packet))        /* get packet */
        {
        case 'D':                            /* data */
                {
                        if (num != n)       /* right packet? */
                        {                         /* no */
                                if (oldtry++ > MAXTRY) return('A'); 
                                /* if too many tries */
                                if (num == n-1)/* check packet number */
                                {
                                        spar(packet);
                                        /* get the init params */
                                        spack('Y',num,6,packet);
                                        /* ACK it */
                                        numtry = 0;
                                        /* reset try counter */
                                        return(state);
                                }
                                else return('A');
                                /* sorry wrong number */
                        }
                        /* data with right packet number */
                        bufemp(packet,fd,len);/* write out the packet */
                        spack('Y',n,0,0);/* right packet, acknowledge */
                        oldtry = numtry;
                        numtry = 0;
                        n = (n+1)%64;
                        return('D');     /* remain in data state */
                }
        case 'F':                        /* file-send */
                {
                        if (oldtry++ > MAXTRY) return('A');
                        if (num == n-1)  /* acknowledge good packet */
                        {
                                spack('Y',num,0,0);
                                numtry = 0;
                                return(state);
                        }
                        else return('A');/* or abort on bad one */
                }
        case 'Z':                        /* end-of-file */
                {
                        if (num != n) return('A');
                        spack('Y',n,0,0);/* say OK */
                        time (&tstop);
                        tstart = tstop - tstart;
                        tstart = filch / tstart;
                        filch = 0;
                        if ( !host ) 
                             fprintf ( stderr,"%d bps\n",tstart );
                        close(fd);       /* close up the file */
                        n = (n+1)%64;
                        return('F');     /* and go to file state */
                }
        case FALSE:  
                return(state);
        default:     
                return('A');
        }
}
