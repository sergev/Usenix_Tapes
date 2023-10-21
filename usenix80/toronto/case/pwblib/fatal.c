int	Fflags	0;
char	*Ffile;
int	Fcnt	0;
int	(*Ffunc)();
int	Fjmp[3];

# define FTLMSG		0100000
# define FTLCLN		 040000
# define FTLFUNC	 020000
# define FTLACT		    077
# define FTLJMP		     02
# define FTLEXIT	     01
# define FTLRET		      0

# define FSAVE( val )	SAVE(Fflags, old_Fflags); Fflags = val;
# define FRSTR()	RSTR(Fflags, old_Fflags);

fatal(msg)
char *msg;
{
char space[132];

	if(Fflags & FTLMSG){
		write(2,"ERROR",5);
		if(Ffile){
			sprintf(space," [%s]",Ffile);
			write(2,space,length(space));
		}
		sprintf(space,": %s\n",msg);
		write(2,space,length(space));
	}

	if(Fflags & FTLCLN)
		clean_up();
	if(Fflags & FTLFUNC)
		(*Ffunc)(msg);
	switch(Fflags & FTLACT){

	case FTLJMP:
		longjmp(Fjmp);

	case FTLEXIT:
		exit(userexit(1));

	default:
		return(Fcnt);
	}
}
