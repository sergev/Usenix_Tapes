# include "stdio.h"
# include "stat.h"

struct inode _Dstatb;

descend(name, goal, function, arg)
int (*function)();
char *name, goal;
{
int	dir	0,	/* open directory */
	offset,		/* in directory */
	dsize,		/* size of current directory */
	dirsize,
	entries;
struct dir_entry{
	int	d_inode;
	char	d_name[14];
} dentry[32];
register struct dir_entry *dp;
register char *c1, *c2;
int i;
char aname[128];

	if(stat(name, &_Dstatb) < 0){
		fprintf(stderr, "--bad status: %s\n", name);
		return(0);
	}
	if((_Dstatb.i_mode&IFMT) != IFDIR){
		if(goal == 'f' || goal == 'b')	/* search goal for files */
			(*function)(arg, name);
		return(1);
	}else
		if(goal == 'd' || goal == 'b')	/* search goal is directories */
			(*function)(arg, name);
	dirsize = _Dstatb.i_size1;

	for(offset = 0; offset < dirsize; offset += 512){	/* each block */
		dsize = 512 < (dirsize - offset) ? 512 : (dirsize-offset);
		if(!dir){
			if((dir = open(name, 0)) < 0){
				fprintf(stderr, "--cannot open %s\n", name);
				return(0);
			}
			if(offset)
				seek(dir,offset,0);
			if(read(dir, &dentry, dsize) < 0){
				fprintf(stderr, "--cannot read %s\n", name);
				return(0);
			}
			if(dir > 8){
				close(dir);
				dir = 0;
			}
		}else
			if(read(dir, &dentry, dsize) < 0){
				fprintf(stderr, "--cannot read %s\n", name);
				return(0);
			}
		entries = dsize >> 4;
		dp = &dentry;
		do{	/* each directory entry */
			if(dp -> d_inode == 0 || strcmp(dp -> d_name, ".") == 0 ||
				strcmp(dp -> d_name, "..") == 0)
				goto Continue;
			if(dp -> d_inode == -1) break;
			c1 = aname;
			c2 = name;
			while(*c1++ = *c2++);
			--c1;
			*c1++ = '/';
			c2 = dp -> d_name;
			for(i=0; i<14; i++)
				if(*c2)
					*c1++ = *c2++;
				else
					break;
			*c1 = '\0';
			if(descend(aname, goal, function, arg) == 0)
				fprintf(stderr, "--%s\n", name);
		Continue:
			++dp;
		}while(--entries);
	}
	if(dir)
		close(dir);
	return(1);
}
