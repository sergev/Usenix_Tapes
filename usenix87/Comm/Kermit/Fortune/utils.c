#include "kermit.h"
/*
 *      Various KERMIT utilities.
 */
clkint()                    /* timer interrupt handler */
{
        longjmp(env,TRUE);  /* tell rpack to give up */
}
quit()                      /* panic interrupt handler */
{
        ioctl(0,TIOCSETP,&cookedmode);
        printf("\n");
        exit();
}
/* tochar converts a control character 
to a printable one by adding a space */
char tochar(ch)
char ch;
{
        return(ch + ' ');             /* make sure not a control char */
}
/* unchar undoes tochar */
char unchar(ch)
char ch;
{
        return(ch - ' ');             /* restore char */
}
/*
 * ctl turns a control character into a printable character by toggling the
 * control bit (ie. ^A becomes A and A becomes ^A).
 */
char ctl(ch)
char ch;
{
        return(ch ^ 64);               /* toggle the control bit */
}
/*
 *      s p a c k
 */
spack(type,num,len,data)
char type, *data;
int num, len;
{
        int i;
        char chksum, buffer[100];
        register char *bufp;

        /* initialize some parameters */

        bufp = buffer;
        /* issue necessary padding */
        for (i=1; i<=pad; i++) write(remfd,&padchar,1);
        *bufp++ = SOH;           /* packet marker, ASCII 1 (SOH) */
        chksum = tochar(len+3);  /* initialize the checksum */
        *bufp++ = tochar(len+3); /* send the character count */
        chksum = chksum + tochar(num);        /* init checksum */
        *bufp++ = tochar(num);                /* packet number */
        chksum = chksum + type;
        *bufp++ = type;                       /* packet type */
        /* loop for all data characters */
        for (i=0; i<len; i++)
        {
                *bufp++ = data[i];
                chksum = chksum+data[i];
        }
        chksum = (chksum + (chksum & 192) / 64) & 63;
        *bufp++ = tochar(chksum);             /* checksum */
        *bufp = eol;                  /* extra packet line terminator */
        write(remfd, buffer,bufp-buffer+1);   /* send it out */
        if (debug) putc('.',stderr);
        if ( !host ) fprintf ( stderr , "%d\r" , filch );
}
/*
 *      r p a c k
 */
rpack(len,num,data)
int *len, *num;
char *data;
{
        int i, fld;
        char chksum, t, type;

        chksum = 0;                   /* initialize checksum */
        t = 0;                        /* initialize input char value */
        type=i=fld=0;
        /* set up clock interrupt */
        if (setjmp(env))  
                /* if clock interrupt happens, print message and fail */
        {
                if (debug) if (!host)
                        fprintf(stderr,"Read timed out\n");
                return FALSE;  /* return false if we time out */
        }
        signal(SIGALRM,clkint);
        if ((timint > MAXTIM) || (timint < MINTIM)) timint = MYTIME;
        alarm(timint);
        while (t != SOH)
        {
                read(remfd,&t,1);  /* wait for packet header */
                t = t & 0177;
        }
        for (fld=1; fld<=5; fld++)    /* got one, loop for each field */
        {
                if (fld != 5 || i != 0)
                        /* don't get char on no data packets */
                {
                        read(remfd,&t,1);
                        t = t & 0177;
                        if (t == SOH)
                                fld = 0;          /* resynch if SOH */
                }
                if (fld <= 3) chksum = chksum + t;/* accumulate checksum */
                switch (fld)
                {
                case 0:    
                        chksum = 0; 
                        break;      /* restart loop */
                case 1:    
                        *len = unchar(t)-3; 
                        break; /* character count */
                case 2:    
                        *num = unchar(t); 
                        break; /* packet number */
                case 3:    
                        type = t; 
                        break;        /* packet type */
                case 4:    
                        for (i=0; i<*len; i++)
                        {
                                if (i != 0)
                                {
                                        read(remfd,&t,1);
                                        t = t & 0177;
                                        if (t == SOH)  /* get a char */
                                        {
                                                fld = -1;
                                                break;
                                        }
                                }
                                chksum = chksum + t;/* add it to checksum */
                                data[i] = t;        /* normal character */
                        }
                        data[*len] = 0;

                case 5:    
                        chksum = (chksum + (chksum & 192) / 64) & 63;
                        break;
                }
        }

        alarm(0);                  /* disable the timer interrupt */
        /* check the checksum */
        if (chksum != unchar(t))
        {
                printf("Kermit: bad checksum %c, should be %c\n",
                t,tochar(chksum));
                return(FALSE);
        }
        return(type); /* return packet type */
}
/*
 *      b u f i l l
 */
bufill(buffer)
char buffer[];
{
        int i;
        char t;
        i = 0;                   /* init data buffer pointer */
        while(read(fd,&t,1) > 0) /* get the next character */
        {
                t = t & 0177;
                filch++;
                if (t<SP || t==DEL || t==quote)
                {
                        if (t=='\n') /* if newline, squeeze CR in first */
                        {
                                buffer[i++] = quote;
                                buffer[i++] = ctl('\r');
                        }
                        buffer[i++] = quote;
                        if (t != quote) t=ctl(t);
                }
                buffer[i++] = t;
                if (i >= spsiz-8) return(i);
        }
        if (i==0) return(EOF);  /* wind up here only on EOF */
        return(i);              /*  so the partial buffer isn't lost */
}
/*
 *      b u f e m p
 */
bufemp(buffer,fd,len)
char buffer[];
int fd, len;
{
        int i;
        char t;
        for (i=0; i<len; i++)
        {
                t = buffer[i];
                if (t == MYQUOTE)    /* handle quoted characters */
                {
                        t = buffer[++i];/* get the quoted character */
                        if (t != MYQUOTE) t = ctl(t);/* decontrolify it */
                }
                if (t != 015)           /* don't pass CR */
                        {
                        filch++;
                        write(fd,&t,1); /* put the char in the file */
                        }
        }
}
/*
 *      g e t f i l
 */
getfil(filenm)
char *filenm;
{
        if (filenm[0] == '\0')
                fd = creat(packet,0644);/* if filename known, use it */
        else
            fd = creat(filenm,0644);/* else use sourcefile name */
        return (fd > 0);           /* return false if file won't open */
}
/*
 *      g n x t f l
 */
gnxtfl()
{
        if (debug) fprintf(stderr, "gnxtfl\n");
        filnam = *(filelist++);
        if (filnam == 0) return FALSE;
        else return TRUE;
}
/* spar fills the data array with appropriate send-init paramaters */
spar(data)
char data[];
{
        data[0] = tochar(MAXPACK);  /* biggest packet I can receive */
        data[1] = tochar(MYTIME);   /* when I want to be timed out */
        data[2] = tochar(MYPAD);    /* how much padding I need */
        data[3] = ctl(MYPCHAR);     /* padding character I want */
        data[4] = tochar(MYEOL);    /* end-of-line character I want */
        data[5] = MYQUOTE;          /* quote character I send */
}
/* rpar gets the other host's send-init parameters */
rpar(data)
char data[];
{
        spsiz = unchar(data[0]);  /* maximum send packet size */
        timint = unchar(data[1]); /* when I should time out */
        pad = unchar(data[2]);    /* number of pads to send */
        padchar = ctl(data[3]);   /* padding character to send */
        eol = unchar(data[4]);    /* EOL character I must send */
        quote = data[5];          /* incoming data quote char */
}
