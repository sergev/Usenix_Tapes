#include <stdio.h>
#include <sys/file.h>

#define	BUFF_SIZE	128

main()
{
int	tty_fid,
	file_fid,
	cnt0,
	cnt1,
	t_tmp;

char	ch_reply,
	tmp_filename[24],
	file_name[50],		/* arbitrary size */
	f_buff[BUFF_SIZE],
	file_type;

FILE	*f_tmp;

	if (term_setup() != 0)
	{
		printf("Warning: This program needs to have the\n");
		printf("TERM environment variable setup before\n");
		printf("being run. Make sure this is done before\n");
		printf("executing xumodem again (set yourself up like\n");
		printf("you were going to use vi).\n");
		exit(1);
	}

	tty_fid = raw_tty("/dev/tty");
	if (tty_fid == -1)
	{
		printf("Error opening terminal for raw i/o\n");
		exit(1);
	}

	ch_reply = 0;
	file_type = 0;

	cls();
	mov_cur(22, 1);
	printf("XUMODEM - Xmodem for Unix Systems");
	mov_cur(9, 3);
        printf("NOTICE: This version of XUMODEM supports checksum transfers only.");
	mov_cur(7,10);
	printf("Send a file, Receive a file, set Text file xfer, set Binary file xfer,");
	mov_cur(25, 12);
	printf("or Quit (s, r, t, b, q): ");
	clr_eol();
	mov_cur(24, 7);
	printf("File transfer mode = ");

	while (ch_reply != 'q')
	{
		mov_cur(45, 7);
		if (file_type == 0)
		{
			printf("text files");
		}
		else
		{
			printf("binary files");
		}
		clr_eol();

		mov_cur(50, 12);
		ch_reply = tolower(0x7f & getchar());

		switch(ch_reply)
		{
			case 's' :
				file_fid = get_fileid(tty_fid, file_name, ch_reply);
				if (file_fid >= 0)
				{
					if (file_type == 0)
					{
						sprintf(tmp_filename, "/tmp/xum%d", getpid());
						f_tmp = fopen(tmp_filename, "w+");
						while( (cnt0 = read(file_fid, f_buff, BUFF_SIZE)) > 0 )
						{
							for (cnt1 = 0; cnt1 < cnt0; cnt1++)
							{
								if (f_buff[cnt1] == '\n')
								{
									fputc('\r', f_tmp);
								}
								fputc(f_buff[cnt1], f_tmp);
							}
						}
						fclose(f_tmp);
						close(file_fid);
						file_fid = open(tmp_filename, O_RDONLY);
					}
					send_file(tty_fid, file_fid);
					if (file_type == 0)
					{
						unlink(tmp_filename);
					}
				}
				ch_reply = 'q';
				break;
			case 'r' :
				file_fid = get_fileid(tty_fid, file_name, ch_reply);
				if (file_fid >= 0)
				{
					cnt0 = recv_file(tty_fid, file_fid);
					if ( (file_type == 0) &&
					     (cnt0 == 0) )
					{
						sprintf(tmp_filename, "/tmp/xum%d", getpid());
						f_tmp = fopen(tmp_filename, "w+");
						file_fid = open(file_name, O_RDONLY);
						while( (cnt0 = read(file_fid, f_buff, BUFF_SIZE)) > 0 )
						{
							for (cnt1 = 0; cnt1 < cnt0; cnt1++)
							{
								if ( (f_buff[cnt1] != '\r') &&
								     (f_buff[cnt1] != 0x1a) )
								{
									fputc(f_buff[cnt1], f_tmp);
								}
							}
						}
						close(file_fid);
						fclose(f_tmp);
						t_tmp = open(tmp_filename, O_RDONLY);
						file_fid = open(file_name, O_CREAT | O_TRUNC | O_WRONLY, 0600);
						while( (cnt0 = read(t_tmp, f_buff, BUFF_SIZE)) > 0 )
						{
							write(file_fid, f_buff, cnt0);
						}
						close(file_fid);
						close(t_tmp);
						unlink(tmp_filename);
					}
				}
				ch_reply = 'q';
				break;
			case 't' :
				file_type = 0;
				break;
			case 'b' :
				file_type = 1;
				break;
			default :
				break;
		}
	}

	mov_cur(50, 12);
	clr_eol();
	printf("Quit");
	reset_tty(tty_fid);
	mov_cur(1, 24);
}



/*	Get_fileid()							*/
/*	Routine to ask for a filename and do processing based on send	*/
/*	or receive mode.						*/
/*									*/
/*	On Entry: terminal file id, array for file name, and		*/
/*		  type of file function (send, recv).			*/
/*	On Exit: returns opened or created file id or -1 for failure	*/
/*		 and file_name is the file name used.			*/


get_fileid(tty_fid, file_name, file_func)
int	tty_fid;
char	*file_name,
	file_func;
{
int	file_id;
char	temp_byte;

	if (file_func == 'r')
	{

		printf("receive a file");

		mov_cur(1, 14);
		clr_eol();

		mov_cur(10, 14);

		printf("Input filename to receive: ");
		clr_eol();
		cook_tty(tty_fid);
		scanf("%s", file_name);
		uncook_tty(tty_fid);

		file_id = open(file_name, O_CREAT | O_TRUNC | O_EXCL | O_WRONLY, 0600);
		if (-1 == file_id)
		{
			printf("\nError creating %s. File may exist. Overwrite (y, n) ? ", file_name);

			temp_byte = tolower(0x7f & getchar());
			while ( (temp_byte != 'y') &&
				(temp_byte != 'n') )
			{
				temp_byte = tolower(0x7f & getchar());
			}

			if (temp_byte == 'y')
			{
				file_id = open(file_name, O_CREAT | O_TRUNC | O_WRONLY, 0600);
			}
		}

		if (file_id  < 0)
		{
			printf("\r\nError opening file; can not create %s\r\n", file_name);
		}
	}
	else
	{
		printf("send a file");


		mov_cur(10, 14);

		printf("Input filename to send: ");
		clr_eol();
		cook_tty(tty_fid);
		scanf("%s", file_name);
		uncook_tty(tty_fid);

		if (-1 == (file_id = open(file_name, O_RDONLY)))
		{
			printf("Error opening %s; file does not exist\n", file_name);
		}
	}
	return(file_id);
}
