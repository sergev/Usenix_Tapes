#include descrip
#include iodef
extern short chan;
char getchartt()
        {
        int status;
        short iosb[4];
        char buf[1];
        status = sys$qiow(0,chan,IO$_READLBLK | IO$M_NOECHO,
                iosb,0,0,buf,1,32767,0,0,0,0);
        if (status != 1)
                LIB$STOP(status);
        if (iosb[0] != 1)
                LIB$STOP(iosb[0]);
        if (buf[0] == '\r')
                buf[0] = '\n';  /* emulate UNIX */
        return(buf[0]);
        }

/*
 * returns a channel
 */

 ttopen()
        {
        register int status;
        struct dsc$descriptor dev_name;

        dev_name.dsc$w_length = 4;
        dev_name.dsc$a_pointer = "TT:";
        dev_name.dsc$b_class = DSC$K_CLASS_S;
        dev_name.dsc$b_dtype = DSC$K_DTYPE_T;
        status = sys$assign(&dev_name,&chan,0,0,0);
        if (status != 1)
                LIB$STOP(status);
        return;
        }


spawn_command(command)
   char *command;
          {

#include jpidef
#include descrip
#include ssdef
        struct dsc$descriptor descr_command;
               
               /*
                * make the character strings
                */
               
      char the_command[40];

	if((!strcmp("sh",command)) || command[0]=='!' || command[0]=='\0'){

        lib$spawn(0,
                  0,
                  0, 
                  0,
                  0,
                  0,
                  0,
                  0,
                  0,
                  0,
                  0,
                  0);
	return;
       }


         sprintf(the_command,"%s",command);
                
               /*
                * Give descriptors the number of printable ascii  
                * characters
                */
                
                 descr_command.dsc$w_length=strlen(the_command);
            
                /*
                 * Give descriptors the addresses of the strings
               	 */
            
                 descr_command.dsc$a_pointer=the_command;
                    
                /*
                 * Define the string descriptor class
                 */
             
                 descr_command.dsc$b_class=DSC$K_CLASS_S;
                    
                /*
                 * Explain that it is an ascii string.
                 */
             
                 descr_command.dsc$b_dtype=DSC$K_DTYPE_T;




        lib$spawn(&descr_command,
                          0,
                          0, 
                          0,
                          0,
                          0,
                          0,
                          0,
                          0,
                          0,
                          0,
                          0);

}

/*
 * this is here because vms curses sucks.
 * the curser was following hidden monsters about.
 */

#include curses
refresh_vms(){
static int old_x,old_y;
static int odd=0;

getyx(stdscr,old_y,old_x);
move(LINES -2,COLS-1);
 if(odd==0){
odd=1;

addch(' ');
 }else{
odd=0;
addch('.');
} 
refresh();
move(old_y,old_x);
}
