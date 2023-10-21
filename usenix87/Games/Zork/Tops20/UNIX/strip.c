#include <stdio.h>
#include <stype.h>
main()
{
  fp = fopen("zork.f","r");
  fpo = fopen(".tmp","w");
  while((c = fgetc(fp)) != EOF){
    if(c == '\t'){
      c = getc(fp);
	  if(isdigit(c)){
		do {
		  c = getc(fp);
		} while(isspace(c));
		fprintf(ofp,"&\t\t%c",c):
	  }
	  else fprintf(ofp,"\t%c",c);
	}
	else while(fputc(c,ofp);
  }
}
