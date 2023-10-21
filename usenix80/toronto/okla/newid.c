/*
 copyright June 17, 1979

  This is a data gathering program.  It is intended to be used by
  semi-skilled persons to obtain the information necessary to create
  several user-ids at one time.  It is usually used for a course where
  everyone is expected to have an account on the system.
      Each user is represented by one line in the file in the form:
  
  	<user-id>TAB<real name>TAB<student-id & course>TAB<password>
  
  The initial password will be the same as the user-id.   The file will
  have two lines, the first two, that must be stripped before it can
  be used as input to the add-users program.

 Author: Suzanne P. O'Dell
 University of Oklahoma Engineering Computer Network
 */

char line [90];
char file [16];
char buff [512];
char ibuf [512];
char course [9];
char section [4];
char name [31];
char yorn [2];
char sid [12];
char uid [9];
int fd;

main() {

int num;

putchar('\014');
printf("Enter information as required.  Do not put blanks in any of \
the replies\nexcept real names.  Only one section of a course can \
be entered at one\ntime.  When finished with a section type cntl-d \
(hold CONTROL and push d)\nwhen asked for the student name.\n \
Recall the program to process a new section.\n");
printf("\nCourse identifier : ");
for((num = getans(course,8));num == 0;(num = getans(course,8)))
	printf("\ntry course number again : ");
printf("\nsection number : ");
for((num = getans(section,3));num = 0;(num = getans(section,3)))
	printf("\ntry section again : ");
line[0] = '\0';
cat(line,course,"-",section);
file[0] = '\0';
cat(file,line);
for((num=stat(file,buff));num != -1;(num = stat(file,buff))) {
	if(size(file) >= 14) err("file name");
	cat(file,file,"a");
	}
if((fd = creat(file,0707)) < 0) err("creat");
printf("\nInstructor's real name: ");
num = read(0,ibuf,512);
write(fd,ibuf,num);
printf("\nInstructor's user name : ");
num = read(0,ibuf,512);
write(fd,ibuf,num);

for(;;)	{
	putchar('\014');
	printf("\nstudent's real name (Last, First) : ");
	for((num = getans(name,30)); num == 0 || num == -1;)  {
		printf("\nThis name was truncated to :\n%s\nIs that satisfactory? (y or n) :\  ",name);
		num = getans(yorn,1);
		if(yorn[0] != 'y')  {
			printf("\nName : ");
			num = getans(name,30);
			}
		}
	printf("\nstudent id number in the form\n000-00-0000 : ");
	for((num = getans(sid,11));num != 11;(num = getans(sid,11)))
		printf("\ntry id number again : ");
	printf("\nconstruct a user name that is made up of initals and/or students last name\nmust be 8 or fewer characters	lower-case letters only\nex: jefagen		odell\n\nuser name : ");
	for((num = getans(uid,8));num == 0;(num = getans(uid,8)))
		printf("\ntry user name again : ");
	cat(line,uid,"	",name,"	",sid," ",course,"-",section,"\n");
	if((write(fd,line,size(line))) < 0) err("write line");
/*  debug stuff   
	printf("%s",line);
	getans(yorn,1);
	*/
	}
}
getans(cptr,size) char *cptr; int size;	{
	int len, i;
	char *bptr;
	len = (read(0,buff,512)) - 1;
	bptr = buff;
	if(len < 0) exit();
	if(len < size) size = len;
	else if(len > size) len = -1;
	for(i = 0;i < size;i++)
		*cptr++ = *bptr++;
	*cptr++ = '\0';
	return(len);
	}

size(string) char *string; {
	int len;
	char *sptr;
	len = 0;
	for(sptr = string ;*sptr++ != '\0';++len) ;
	return(len);
	}

err(string) char *string; {
	printf("ERROR   report following message to ECN staff\n%s\n",string);
	exit();
	}
