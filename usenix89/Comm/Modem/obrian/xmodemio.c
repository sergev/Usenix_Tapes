#include <stdio.h>

unsigned char mk_chksum();

#define	NUL	0x00
#define	SOH	0x01
#define	EOT	0x04
#define	ACK	0x06
#define	NAK	0x15
#define	CAN	0x18

#define	RETRY_MAX	10
#define	BUFF_SIZE	128




/*	Send_file()							*/
/*	Routine to send a file from Unix to another computer via 	*/
/*	/dev/tty using the xmodem protocol in checksum mode.		*/
/*									*/
/*	On Entry: terminal file id, and file to send file id
/*	On Exit: returns 0 for success or -1 for failure		*/

send_file(tty_fid, file_id)
int	tty_fid,
	file_id;
{
int	ret_code,
	read_status,
	tty_status,
	ack_status,
	retry_counter;
	

unsigned char	checksum,
		sector_num,
		temp_byte,
		start_bad;

char	cpm_buff[BUFF_SIZE]; /* CP/M disk buffers are ALWAYS 128 bytes	*/


	ret_code = 0; /* assume no errors */
	cls();
	mov_cur(12, 10);
	printf("You have 1 minute to start receiving (control X to quit)....\n");

	uncook_tty(tty_fid);

	retry_counter = 0;	/* reset counter */
	do	/* wait for NAK from receiver so we can get in sync */
	{
		tty_status = timed_read(tty_fid, &temp_byte, 1, 6);

		if ( (tty_status == 0) &&
		     ((temp_byte == NAK) || (temp_byte == CAN)) )
		{
			if (temp_byte == NAK)
			{
				start_bad = 0;
				break;
			}
			else
			{
				start_bad = 1;
				break;
			}
		}
		else
		{
			start_bad = 1;
			retry_counter++;
		}
	}
	while (retry_counter < RETRY_MAX);

	if (start_bad)
	{
		ret_code = -1;	/* say error */
				/* We timed out or we got too much noise */
	}
	else
	{
		sector_num = 0;
		do
		{
			read_status = read(file_id, cpm_buff, BUFF_SIZE);
			sector_num++;
			if (read_status > 0)
			{
				for(; read_status < BUFF_SIZE; read_status++)
				{
					cpm_buff[read_status] = 0x1a; /* CP/M eof */
				}

				retry_counter = 0;	/* reset to send actual packet */
				do
				{
					temp_byte = SOH;
					write(tty_fid, &temp_byte, 1);	/* send SOH */
					temp_byte = ~sector_num;	/* get ones complement of sector number */
					write(tty_fid, &sector_num, 1);	/* sector count */
					write(tty_fid, &temp_byte, 1);	/* and its complement */
					checksum = mk_chksum(cpm_buff, BUFF_SIZE);
					write(tty_fid, cpm_buff, BUFF_SIZE);
					write(tty_fid, &checksum, 1);	/* send checksum */
					if ((ack_status = getack(tty_fid)) == -1)
					{
						retry_counter++;
					}
				}
				while ( (ack_status == -1) && (retry_counter < RETRY_MAX) );
			}
		}
		while ( (read_status > 0) && (retry_counter < RETRY_MAX) );

		retry_counter = 0;
		do
		{
			temp_byte = EOT;
			write(tty_fid, &temp_byte, 1);	/* send EOT */
		}
		while ( (getack(tty_fid) != 0) && (retry_counter++ < RETRY_MAX) );
	}

	if (retry_counter >= RETRY_MAX)
	{
		ret_code = -1;
	}

	close(file_id);
	return(ret_code);
}



/*	Recv_file()							*/
/*	Routine to receive  a file to Unix from another computer via 	*/
/*	/dev/tty using the xmodem protocol in checksum mode.		*/
/*									*/
/*	On Entry: terminal file id, file to recv file id
/*	On Exit: returns 0 for success or -1 for failure		*/

recv_file(tty_fid, file_id)
int	tty_fid;
{
int	ret_code,
	read_status,
	tty_status,
	ack_status,
	retry_counter;
	

unsigned char	checksum,
		sector_num,
		temp_byte,
		dummys[5],
		start_bad;


char	cpm_buff[BUFF_SIZE]; /* CP/M disk buffers are ALWAYS 128 bytes	*/

	ret_code = 0; /* assume no errors */
	cls();
	mov_cur(12, 10);
	printf("You have 1 minute to start sending (control X to quit)....\n");

	retry_counter = 0;	/* reset counter */
	do	/* send a NAK to sender so we can get in sync */
	{
		temp_byte = NAK;
		write(tty_fid, &temp_byte, 1);

		tty_status = timed_read(tty_fid, &temp_byte, 1, 6);

		if ( (tty_status == 0) &&
		     ((temp_byte == SOH) || (temp_byte == CAN)) )
		{
			if (temp_byte == SOH)
			{
				start_bad = 0;
				break;
			}
			else
			{
				start_bad = 1;
				break;
			}
		}
		else
		{
			start_bad = 1;
			retry_counter++;
		}
	}
	while (retry_counter < RETRY_MAX);

	if (start_bad)
	{
		ret_code = -1;	/* say error */
				/* We timed out or we got too much noise */
	}
	else
	{
		retry_counter = 0;
		sector_num = 1;		/* to compare with sent sector number */

/* First SOH was caught in NAK, CAN, code above. */

		while ( (temp_byte == SOH) && (retry_counter < RETRY_MAX) )
		{
			tty_status = stimed_read(tty_fid, &dummys[0], 2);
			tty_status += stimed_read(tty_fid, &dummys[1], 2);

			for (ack_status = 0; ack_status < BUFF_SIZE; ack_status++)
			{
				tty_status += stimed_read(tty_fid, &cpm_buff[ack_status], 2);
			}

			tty_status += stimed_read(tty_fid, &dummys[2], 2);

			checksum = mk_chksum(cpm_buff, BUFF_SIZE);

			if ( (tty_status == 0) &&
			     (sector_num == dummys[0]) && ((~sector_num & 0xff) == dummys[1]) &&
			     (checksum == dummys[2]) )
			{
				ack_status == write(file_id, cpm_buff, BUFF_SIZE);
				if (ack_status == BUFF_SIZE)
				{
					temp_byte = ACK;
					write(tty_fid, &temp_byte, 1);
					retry_counter = 0;
					ret_code = 0;
					sector_num++;
				}
				else
				{
					temp_byte = NAK;
					write(tty_fid, &temp_byte, 1);
					retry_counter++;
					ret_code = -1;
				}
			}
			else
			{
				temp_byte = NAK;
				write(tty_fid, &temp_byte, 1);
				retry_counter++;
				ret_code = -1;
			}

			tty_status = stimed_read(tty_fid, &temp_byte, 2);

			while (	(temp_byte != SOH) && (temp_byte != EOT) &&
				(retry_counter < RETRY_MAX) )
			{
				retry_counter++;
				tty_status = stimed_read(tty_fid, &temp_byte, 2);
			}

			if (retry_counter >= RETRY_MAX)
			{
				ret_code = -1;
				break;
			}

			retry_counter = 0;
			if (temp_byte == EOT)
			{
				temp_byte = ACK;
				write(tty_fid, &temp_byte, 1);
				break;
			}
		}
	}

	close(file_id);
	return(ret_code);
}


/*	getack: Will do a timed read on a pre-selected tty and 
	will return the status.

	On Entry: is passed the file id of the tty to use
	On Exit: returns 0 if got an ACK else returns -1 
*/

getack(tty_fid)
int	tty_fid;
{
char	temp_byte;

	if ( (timed_read(tty_fid, &temp_byte, 1, 5) == 0) &&
	     (temp_byte == ACK) )
	{
		return(0);	/* got ACK ok */
	}
	else
	{
		return(-1);	/* timed out or wrong character */
	}
}


/*	mk_chksum: Will generate a checksum on array of specified size and return
		   that checksum.

	On Entry: is passed the array and size to use
	On Exit: returns checksum
*/

unsigned char mk_chksum(array, size)
char	*array;
int	size;
{
int	counter;
unsigned char	checksum;

	checksum = 0;
	for (counter = 0; counter < size; counter++)
	{
		checksum += array[counter];	/* build up checksum */
	}
	return(checksum);
}
