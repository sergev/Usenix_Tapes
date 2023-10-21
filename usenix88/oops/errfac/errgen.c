/*
 *  Author:     S.M. Orlow
 *              Systex,Inc.
 *              Beltsville, MD 20705
 *              301-474-0111
 *
 *  Contractor: K.E. Gorlen
 *              Computer Systems Laboratory
 *              Rm. 2017, Bld 12A
 *              National Institutes of Health
 *              Bethesda, MD 20205
 *              (301)-496-5361
 *
 * Modification History:
 */
#include <stream.h>
#include <string.h>
#include <osfcn.h>
#include <fcntl.h>
#include "ErrFac.h"
#include "gettok.h"
#include "dofile.h"
#include "errors.h"

static istream *cmdstrm;
static ostream *hstrm, *cstrm;
static char *cmdfile, *hfile, *cfile;
static ErrorFacility* errfacility;
static char* basename;
static char* longname;
static char* shortname;
static ErrorSpecs* errfac;
static int severity;
static int errorcount = 0;
static new_severity = 0;
static new_facility = 0;

/* make_filenames -- make errgen data file names */
void make_filenames(char* argv) {
     char* ext;

     basename = base_filename(argv);
     ext = ext_filename(argv);
     if ( strlen(ext) == 0 ) {
       ext = new char[5];
       strcpy(ext,".err");
       };
     cmdfile = new char[strlen(basename)+strlen(ext)+1];
     strcpy(cmdfile,basename); strcat(cmdfile,ext);
     hfile = new char[strlen(basename)+2+1];
     strcpy(hfile,basename); strcat(hfile,".h");
     cfile = new char[strlen(basename)+2+1];
     strcpy(cfile,basename); strcat(cfile,".c");
     delete ext;
}/* end make_filenames */

void clean_filenames() {
	delete basename;
	delete cmdfile;
	delete hfile;
	delete cfile;
	}

int fac_code(istream& istrm,char* facname) {
	char buf[132];
	while ( !istrm.eof() ) {
	  istrm >> buf;
	  if ( strcmp(buf,".define") == 0 ) { // check for facility name
	    istrm >> buf;
	    if ( strcmp(buf,facname) == 0 ) { // get facility code
		istrm >> buf;
		return atoi(buf);
		};
	    };
	  };
	return -1;
	}
int facility_code(char* facname) {
	istream* istrm = open_istream("/etc/errgen_tab");
        int retval = fac_code(*istrm,facname);
	delete istrm;
	if ( retval != -1 ) return retval;

	istrm = open_istream("/etc/errgen_tab.loc");
        retval = fac_code(*istrm,facname);
	delete istrm;
	if ( retval != -1 ) return retval;
	cerr << "facility: " << facname << " not found\n";
	exit(1);
	return -1;
	}

int severity_code(char* codename) {
   if ( strcmp(codename,"FATAL")   == 0 ) return FATAL;
   if ( strcmp(codename,"ERROR")   == 0 ) return ERROR;
   if ( strcmp(codename,"WARNING") == 0 ) return WARNING;
   if ( strcmp(codename,"INFO")    == 0 ) return INFO;
   if ( strcmp(codename,"INFORMATION") == 0  ) return INFORMATION;
   if ( strcmp(codename,"SUCCESS") == 0 ) return SUCCESS;
   }

#define BUF_SIZE 132

/* process_commands -- process commands in *.err file */
void process_commands() {
char* buf = new char[BUF_SIZE];
char *token, *token1;

   while ( !( cmdstrm->eof() ) ) {
     cmdstrm->get(buf,BUF_SIZE,'\n');
     eatwhite(*cmdstrm);
//     cerr << buf << "\n";

     switch(buf[0]) {
       case ';':
         break;			// do nothing for comment 
       case '.':
           cerr << buf << "\n";
       	   token = gettok(buf,white_space);
	   if ( strcmp(token,".facility") == 0 ) { // start new facility 
	      new_facility = 1;					     
              new_severity = 0;						   
	      token = 0;
	      token = gettok(0,white_space);
	      if ( token ) {
	        longname = new char[strlen(token)+1]; 
	        strcpy(longname,token);
		}
	       else {
	        cerr << " Missing longname for .facility\n";
	        exit(1);
	        };
	      token = gettok(0,white_space);
	      if ( token ) {
	        shortname = new char[strlen(token)+1]; 
	        strcpy(shortname,token);
		}
	       else {
	        cerr << " Missing shortname for .facility\n";
	        exit(1);
	        };

	      errfacility =
	      	new ErrorFacility(longname,shortname,
				  facility_code(longname),basename);
	      errfacility->h_init(*hstrm);
	      errfacility->h_define(*hstrm,"FACILITY",errfacility->code());
	      errorcount = (errfacility->code())<<FAC_SHIFT;
	       errfacility->h_define(*hstrm,"FIRSTERROR",errorcount);
	      };// end .facility

           if ( strcmp(token,".severity") == 0 ) { // start new severity list 
	      if ( !new_facility ) {
		cerr << "errgen> missing facility definition\n";
		exit(1);
	        };
	      new_severity = 1;					   

	      token = gettok(0,white_space);
	      severity = severity_code(token);
	      };// end .severity 
         break;
       default:	// process error line 
	 if ( !new_severity ) {
	   cerr << "Missing severity definition\n";
	   exit(1);
	   };
         token = gettok(buf,white_space);	// get error name token
	 cerr << token << " ";
	  errfacility->h_define(*hstrm,token,errorcount++);
	 token1 = gettok(0,"\n");		// get rest of line
	 if ( token1[0] == '"' ) {	// skip args field
	   token = "";
	   }
	  else {		
	   token = gettok(token1,white_space); 	// get args field
	   token1 = gettok(0,"\n");		// get text field
	   };

         errfac = new ErrorSpecs(severity,token,token1);
	 errfacility->add(*errfac);
         break;
       };// end switch 
     };// end while 

     cerr << " End of Stream.\n";
     errfacility->h_define(*hstrm,"LAST_ERROR",--errorcount);
     errfacility->printOn(*cstrm); *cstrm << "\n";
}/* end process_commands */

main(int argc,char* argv[]) {
   if ( argc == 1 ) {
    cerr << "usage: errgen errfiles\n";
    exit(1);
    };
   for (int iarg=1; iarg < argc; iarg++ ) {
     make_filenames(argv[iarg]);
     cmdstrm = open_istream(cmdfile);
     hstrm = open_ostream(hfile);
     cstrm = open_ostream(cfile);
     process_commands();
     delete cmdstrm;
     delete hstrm;
     delete cstrm;
     clean_filenames();
     }; /* end for(iarg) */
exit(0);
}/* end main */
