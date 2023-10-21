/* "DCP" a uucp clone. Copyright Richard H. Lamb 1985,1986,1987 */
/* file send routines */
#include "dcp.h"
static unsigned char spacket[MAXPACK];
/*<FF>*/
/***************SEND PROTOCOL***************************/
/*
 *  s d a t a
 *
 *  Send File Data
 */
sdata()
{
        if((*sendpkt)(spacket,size,0)) return(0);     /* send data */
        if ((size = bufill(spacket)) == -1) /* Get data from file */
             return('Z');/* If EOF set state to that */
        return('D');/* Got data, stay in state D */

}

/*<FF>*/
/*
 *  b u f i l l
 *
 *  Get a bufferful of data from the file that's being sent.
 *  Only control-quoting is done; 8-bit & repeat count prefixes are
 *  not handled.
 */
bufill(buffer)
char buffer[];/* Buffer */
{
    int i;/* Loop index */
    char t;/* Char read from file */
    i = 0;/* Init data buffer pointer */
        while(read(fp,&t,1) ==1)
        {
            buffer[i++] = t;/* Deposit the character itself */
            if(i >= pktsize) return(i);
        }
        if(i==0) return(-1);
    return(i);/* Handle partial buffer */
}


/*<FF>*/
/*
 *  s b r e a k
 *
 *  Send Break (EOT)
 */
sbreak()
{
        int len,i;
        sprintf(spacket,"H");
        if((*sendpkt)(spacket,0,1)) return(0);
        if((*getpkt)(spacket,&len)) return(0);
		if(debug) printmsg("Switch modes");
        if(spacket[1] == 'N') return('G');
        return('Y');
}

/*<FF>*/
/*
 *  s e o f
 *
 *  Send End-Of-File.
 */
seof()
{
        int len,i;
		if((*sendpkt)(spacket,0,0)) return(0);
		if((*getpkt)(spacket,&len)) return(0); /* rec CY or CN */
	    if(strncmp(spacket,"CY",2)) return(0); /* cant send file */
		close(fp);
		fp = (-1);
		unlink(fromfile);
		if(debug) printmsg("transfer %s complete,%d",fromfile,fp);
        return('F');                    /* go get the next file to send */
}

/*<FF>*/
/*
 *  s f i l e
 *
 *  Send File Header.
 */
sfile()
{
int i,len;
if (fp == -1) {/* If not already open, */
       if (debug > 1) printmsg("looking for next file...");
       if(getfile()) {/* get next file from current work*/
              close(fw);
              unlink(cfile);/* close and delete completed workfile */
              fw = -1;
              return('B'); /* end sending session */
       } 
#ifdef DG /* brain damaged MV/UX on DG */
       if((i=mindex(fromfile,'-')) != -1) fromfile[i] = '?';
#endif DG
       if (debug > 1) printmsg("  New file is %s", fromfile);
       if (debug) printmsg("   Opening %s for sending.", fromfile);
       fp = open(fromfile,0);/* open the file to be sent */
       if (fp == -1) {/* If bad file pointer, give up */
       	 if(debug) printmsg("Cannot open file %s", fromfile);
         return('A');
       }
} else return('A'); /* If somethings already open. were in trouble*/
if(debug > 1) printmsg("Sending %s as %s",fromfile,tofile);
strcpy(spacket,tofile);
if((*sendpkt)(spacket,0,1)) return(0);       /* send S fromfile tofile */
if((*getpkt)(spacket,&len)) return(0);       /* user - tofile 0666. */
if(spacket[1] != 'Y') return('A'); 			/* If otherside says no-quit*/
size = bufill(spacket);
return('D');
}

/*<FF>*/
/*
 *  s i n i t
 *
 *  Send Initiate: send this host's parameters and get other side's back.
 */
sinit()
{
        if((*openpk)()) return('A');
        return('B');
}

/*<FF>*/
/*
 *
 *
 *  getfile
 *
 *  getfile reads the next line from the presently open workfile
 *  (cfile) and determines from this the next file to be sent
 *  (file). If there are no more TRUE is returned.
 *  --A fix for "R from to 0666" should be done here to recieve files
 *  --in addition to sending them. The appropriate "state letter"
 *  --i.e. "R" should be returned to the send "master" or "slave"
 *  --state switching table in "dcp.c"
 *  --I did not implement this since the majority of uucp transactions
 *  --appear to be "S from to 0666" type. RHLamb 1/87
 *
 */
getfile()
{
int i;
char line[132];
if(getline(fw,line)) return(TRUE);
sscanf(&line[2],"%s ",fromfile);
for(i=0;line[i];i++) {if(strncmp(&line[i],"0666",4) == 0) break;}
line[i+4] = '\0';
strcpy(tofile,line);
return(FALSE);
}

