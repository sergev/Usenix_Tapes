#include <stdio.h>
#include <stat.h>
#ifdef MAIN
main
#else
ch_mod
#endif
(argc,argv)
	char *argv[];
{
	int or_mask,and_mask,file_stat;
	struct { int ax,bx,cx,dx,si,di,ds,es; } regs;
	extern int _dsval;
	char *current;
	regs.ds = _dsval;
	if (argc==1)
	{
		fprintf(stderr,"Usage : chmod +|-[ahw] file [file ...]\n");
	}
	/* set attributes to default */
	or_mask = 0; and_mask = 0xFFFF;
	while(--argc)
	{
		current = *(++argv);
		switch (*current)
		{
		case '-':
			while (*++current)
			{
				switch(*current)
				{
				case 'w':
				case 'W':
					or_mask |= ST_RDONLY;
					break;
				case 'h':
				case 'H':
					and_mask &= (ST_HIDDEN ^ 0xFFFF);
					break;
				case 'r':
				case 'R':
					or_mask |= ST_HIDDEN;
					break;
				case 'a':
				case 'A':
					and_mask &= (ST_ARCHIV ^ 0xFFFF);
					break;
				case 's':
				case 'S':
					and_mask &= (ST_SYSTEM ^ 0xFFFF);
					break;
				default:
					write(2,"invalid attribute\r\n",19);
				return -1;
				}
			}
			break;
		case '+':
			while(*++current)
			{
				switch(*current)
				{
				case 'w':
				case 'W':
					and_mask &= (ST_RDONLY ^ 0xFFFF);
					break;
				case 'h':
				case 'H':
					or_mask |= ST_HIDDEN;
					break;
				case 's':
				case 'S':
					or_mask |= ST_SYSTEM;
					break;
				case 'r':
				case 'R':
					and_mask &= (ST_HIDDEN ^ 0xFFFF);
					break;
				case 'a':
				case 'A':
					or_mask |= ST_ARCHIV;
					break;
				default:
					write(2,"invalid attribute\r\n",19);
					return -1;

				}
			}
			break;
		default:
			/* get current attribute */
			regs.ax = 0x4300;
			regs.dx = (int)current;
			regs.ds = _dsval;
			sysint(0x21,&regs,&regs);
			file_stat = regs.cx;
			fprintf(stderr,"current attribute for %s = %x\n",
				current,file_stat);
			/* set new attribute */
			file_stat |= or_mask;
			file_stat &= and_mask;
			regs.ax = 0x4301;
			regs.dx = (int)current;
			regs.cx = file_stat;
			regs.ds = _dsval;
			sysint(0x21,&regs,&regs);
			/* get attribute to see if it changed */
			regs.ax = 0x4300;
			regs.dx = (int)current;
			regs.ds = _dsval;
			sysint(0x21,&regs,&regs);
			file_stat = regs.cx;
			fprintf(stderr,"new attribute for %s = %x\n",
				current,file_stat);
			break;
		}
	}
}

