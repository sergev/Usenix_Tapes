/**  send a file  **/

/*
 * Operation of this routine depends on on MDM7BAT and YMDMBAT flags.
 *
 * If "name" is NULL; close out the BATCH send.
 */

#include "xmodem.h"

sfile(name)
char *name;
	{

	char *sectdisp();
	time_t time();
	char *strcpy();
	char *unix_cpm();

	extern unsigned short crctab[1<<B];	/* CRC-16 constant values, see getput.c */

	register int bufctr, 		/* array index for data buffer */
	sectnum;			/* packet number for packet header */

	register unsigned short checksum; 	/* checksum/crc */

	char blockbuf[BBUFSIZ+6];	/* holds packet as it is constructed */

	struct stat filestatbuf;	/* file status info */

	int fd, 		/* file descriptor for file being transmitted */
	errors,			/* cumulative count of errors */
	attempts,		/* number of attempts made to transmit a packet */
	nlflag, 		/* flag that we have to send a LF in next packet */
	sendfin, 		/* flag that we are sending the last packet */
	closeout,		/* flag that we are closing out batch send */
	tmode,			/* TRUE for text mode; FALSE for binary mode */
	bbufcnt,		/* array index for packet */
	firstchar,		/* first character in protocol transaction */
	bufsize,		/* packet size (128 or 1024) */
	sendresp,  		/* response char to sent block received from remote*/
	extrachar;		/* count of extra LF characters added */
	long sentsect;		/* count of 128 byte sectors actually sent */
	long expsect;		/* count of 128 byte sectors expected to be sent */
	time_t start;		/* starting time of transfer */
	char c;

	nbchr = 0;  /* clear buffered read char count */

	CRCMODE = FALSE;	/* Receiver determines use of crc or checksum */

	closeout = FALSE;	/* Check on NULL file name */
	if (strcmp(name,"") == 0)
		{
		if (BATCH)
			closeout = TRUE;
		else
			{
			sendbyte(CAN); sendbyte(CAN); sendbyte(CAN);
			error("NULL file name in send", TRUE);
			}
		}

	if (!closeout)		/* Are we closing down batch? */
		{			/* no; let's send a file */
		if ((fd = open(name, 0)) < 0)	
			{  
			sendbyte(CAN); sendbyte(CAN); sendbyte(CAN);
     	   		error("Can't open file for send", TRUE);
			}
	
		stat(name, &filestatbuf);  /* get file status bytes */
		expsect = (filestatbuf.st_size/128) + 1;
	
		if (LOGFLAG)
			{   
			fprintf(LOGFP, "----\nXMODEM Send Function\n");
		    	fprintf(LOGFP, "File Name: %s\n", name);
		  	fprintf(LOGFP,"Estimated File Size %ldK, %ld Records, %ld Bytes\n",
		  	  (filestatbuf.st_size/1024)+1, expsect, filestatbuf.st_size);
			projtime(expsect, LOGFP);
			}
		}
	else
		{
		logit("----\nXMODEM Send Function\n");
		logit("Closing down Batch Transmission\n");
		}


	tmode = (XMITTYPE == 't') ? TRUE : FALSE;	/* set text mode */

	bufsize = LONGPACK ? 1024 : 128;		/* set sector size */
	if (LONGPACK && !closeout)
		logit("1K packet mode chosen\n");

        sendfin = nlflag = FALSE;
  	attempts = 0;

	/* wait for and read startup character */
	do
		{
		while (((firstchar=readbyte(30)) != NAK) && (firstchar != CRCCHR) && (firstchar != CAN))
			if (++attempts > NAKMAX)
				error("Remote System Not Responding", TRUE);

		if ((firstchar & 0x7f) == CAN)
			if (readbyte(3) == CAN)
				error("Send cancelled at user's request",TRUE);

		if (firstchar == CRCCHR)
			{
			CRCMODE = TRUE;
			if (!closeout)
				logit("CRC mode requested\n");
			}
		}
	while (firstchar != NAK && firstchar != CRCCHR);

	if (closeout && MDM7BAT)	/* close out MODEM7 batch */
		{
		sendbyte(ACK); sendbyte (EOT);
		readbyte(2);			/* flush junk */
		return;
		}

	if (MDM7BAT)		/* send MODEM7 file name and resync for data packets */
		{
		if (send_name(unix_cpm(name)) == -1)		/* should do better job here!! */
			error("MODEM7-batch filename transfer botch", TRUE);

		firstchar = readbyte(5);
		if (firstchar != CRCCHR  && firstchar != NAK)	/* Should do some better error handling!!! */
			error("MODEM7 protocol botch, NAK/C expected", TRUE);

		CRCMODE = FALSE;
		if (firstchar == CRCCHR)
			{
			CRCMODE = TRUE;
			logit("CRC mode requested for MODEM7 batch transfer\n");
			}
		}

	sectnum = 1;

	if (YMDMBAT)	/* Fudge for YMODEM transfer (to send name packet) */
		{
		sectnum = 0;
		bufsize = 128;
		}

	attempts = errors = sentsect = extrachar = 0;
	start = time((time_t *) 0);

        do 			/* outer packet building/sending loop; loop till whole file is sent */
		{   

		if (closeout && YMDMBAT && sectnum == 1)	/* close out YMODEM */
			return;

		if (YMDMBAT && sectnum == 1)			/* get set to send YMODEM data packets */
			{
			bufsize = LONGPACK ? 1024 : 128;
			do
				{
				while (((firstchar=readbyte(3)) != CRCCHR) && (firstchar != CAN))
					if (++attempts > NAKMAX)
						error("YMODEM protocol botch, C expected", TRUE);
				if ((firstchar&0x7f) == CAN)
					if (readbyte(3) == CAN)
						error("Send cancelled at User's request", TRUE);
				}
			while (firstchar != CRCCHR);
			attempts = 0;
			}

		if (extrachar >= 128)	/* update expected sector count */
			{
			extrachar = 0;
			expsect++;
			}

		if ((bufsize == 1024) && (errors > KSWMAX))
			{
			logit("Reducing packet size to 128 due to excessive errors\n");
			bufsize = 128;
			}

		if ((bufsize == 1024) && ((expsect - sentsect) < 8))
			{
			logit("Reducing packet size to 128 for tail end of file\n");
			bufsize = 128;
			}

		if (sectnum > 0)	/* data packet */
			{
			for (bufctr=0; bufctr < bufsize;)
	    			{
				if (nlflag)
	       	 			{  
					buff[bufctr++] = LF;  /* leftover newline */
	       	    			nlflag = FALSE;
	        			}
				if (getbyte(fd, &c) == EOF)
					{ 
					sendfin = TRUE;  /* this is the last sector */
		   			if (!bufctr)  /* if EOF on sector boundary */
		      				break;  /* avoid sending extra sector */
		      			buff[bufctr++] = CTRLZ;  /* Control-Z for CP/M EOF (even do it for binary file) */
		   			continue;
		      			}
	
				if (tmode && c == LF)  /* text mode & Unix newline? */
		    			{
					extrachar++;
					buff[bufctr++] = CR;  /* insert carriage return */
			     		if (bufctr < bufsize)
		                		buff[bufctr++] = LF;  /* insert LF */
		 	      		else
			        		nlflag = TRUE;  /* insert on next sector */
		   			}	
				else
					buff[bufctr++] = c;  /* copy the char without change */
		    		}

	    		if (!bufctr)  /* if EOF on sector boundary */
   	       			break;  /* avoid sending empty sector */
			}	

		else		/* YMODEM filename packet */
			{
			for (bufctr=0; bufctr<bufsize; bufctr++)
				buff[bufctr]=0;
			if (!closeout)
				{
				cpmify(name);
				strcpy((char *)buff, name);
				buff[bufsize-2]	= (expsect & 0xff);
				buff[bufsize-1] = ((expsect >> 8) & 0xff);
				}
			}

		bbufcnt = 0;		/* start building block to be sent */
		blockbuf[bbufcnt++] = (bufsize == 1024) ? STX : SOH;    /* start of packet char */
		blockbuf[bbufcnt++] = sectnum;	    /* current sector # */
		blockbuf[bbufcnt++] = ~sectnum;   /* and its complement */

               	checksum = 0;  /* initialize checksum */
               	for (bufctr=0; bufctr < bufsize; bufctr++)
       			{
			blockbuf[bbufcnt++] = buff[bufctr];

			if (CRCMODE)
				checksum = (checksum<<B) ^ crctab[(checksum>>(W-B)) ^ buff[bufctr]];

			else
               			checksum = ((checksum+buff[bufctr]) & 0xff);
         		}

		if (CRCMODE)		/* put in CRC */
			{
			checksum &= 0xffff;
			blockbuf[bbufcnt++] = ((checksum >> 8) & 0xff);
			blockbuf[bbufcnt++] = (checksum & 0xff);
			}
		else			/* put in checksum */
			blockbuf[bbufcnt++] = checksum;

            	attempts = 0;
	
            	do				/* inner packet loop */
            		{

			writebuf(blockbuf, bbufcnt);	/* write the block */

			if (DEBUG)
				fprintf (LOGFP, "DEBUG: %d byte Packet %02xh (%02xh) sent, checksum %02xh %02xh\n", 
				bbufcnt, blockbuf[1]&0xff, blockbuf[2]&0xff, blockbuf[bufsize+3]&0xff, blockbuf[bufsize+4]&0xff);

                	attempts++;
			sendresp = readbyte(10);  /* get response from remote */

			if (sendresp != ACK)
		   		{
				errors++;

				if ((sendresp & 0x7f) == CAN)
					if ((readbyte(3) & 0x7f) == CAN)
						error("Send cancelled at user's request\n",TRUE);

				if (sendresp == TIMEOUT)
					{
		   			logitarg("Timeout on sector %s\n",sectdisp(sentsect,bufsize,1));
					}
				else
					{
		   			logitarg("Non-ACK on sector %s\n",sectdisp(sentsect,bufsize,1));
					}
		   		}
            		}
			while((sendresp != ACK) && (attempts < RETRYMAX) && (errors < ERRORMAX)); /* close of inner loop */

       		sectnum++;  /* increment to next sector number */
		sentsect += (bufsize == 128) ? 1 : 8;
    		}
		while (!sendfin && (attempts < RETRYMAX) && ( errors < ERRORMAX));  /* end of outer loop */

    	if (attempts >= RETRYMAX)
		{
		sendbyte(CAN); sendbyte(CAN); sendbyte(CAN);
		error("Remote System Not Responding", TRUE);
		}

	if (attempts > ERRORMAX)
		{
		sendbyte(CAN); sendbyte(CAN); sendbyte(CAN);
		error ("Too many errors in transmission", TRUE);
		}

    	attempts = 0;
    	sendbyte(EOT);  /* send 1st EOT to close down transfer */
	
    	while ((readbyte(15) != ACK) && (attempts++ < RETRYMAX)) 	/* wait for ACK of EOT */
		{
		logit("EOT not ACKed\n");
	   	sendbyte(EOT);
		}

    	if (attempts >= RETRYMAX)
	   	error("Remote System Not Responding on Completion", TRUE);

    	close(fd);

    	logit("Send Complete\n");
	prtime(sentsect, time((time_t *) 0) - start);
	}
