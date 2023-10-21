# include "stat.h"
# include "errnos.h"

# define MODE		0444
# define INTERVAL	10
# define READ		0

lockit(file,count,pid)
char *file;
int count, pid;
{
register i;
long c_time;
struct inode buf;
int	fd;

	for(i = 0; i < count && (fd = creat(file,MODE)) < 0; i++)
		sleep(INTERVAL);
	if(fd >= 0){
wfile:
		write(fd,&pid,2);
		close(fd);
		return(0);
	}
	if(stat(file,&buf) >= 0){
		time(&c_time);
		if(c_time - buf.i_mtime > 60 && buf.i_size1 == 0)
			unlink(file);
	}
	for(i = 0; i < count && (fd = creat(file,MODE)) < 0; i++);
	if(fd >= 0) goto wfile;
	return(-1);
}

unlockit(file, pid)
char *file;
int pid;
{
int data, fd;

	if((fd = open(file,READ)) < 0)
		return(-1);
	if(read(fd,&data,2) != 2)
		return(-1);
	if(data != pid)
		return(-1);
	close(fd);
	return(unlink(file));
}
