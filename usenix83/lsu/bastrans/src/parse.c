#include <stdio.h>
#include "bastrans.h"

/*
Nothing elegant - brute force stuff here
*/

parse(string,bword,prefix,suffix,type,pref)
int type;
int *pref;
char *bword, *prefix, *suffix;
char string[CLEN];
{

	char dummy[128];
	char temp[128];
	char varspec[10],var[128];
	int ind,i,j,k,l,v,q;
	int lf=1;
	int gtflag;
	int tind;
	*varspec = '\0';
	 while ((ind=index(string,bword,1))>=0) {
		switch (type) {

			case 0:  /* IF */

				if ((tind=index(string,"THEN",1))<0) error(IFTH,string);
				*dummy='\0';
				for (i=strlen(dummy),j=0;j < tind;i++,j++)
				{
					if (string[j] == '=' && (string[j-1] != '<' && string[j-1] != '>'))
						dummy[i++] = '=';
					dummy[i]=string[j];
				}
				dummy[i++]='\0';
				while (string[j++]!='N');
				while (string[j++]==' ');
				if (issdigit(string[j-1])) {
					cat(dummy,") goto l");
					for (k=j,l=0;issdigit(temp[l++]=string[k++]););
					temp[l]='\0';
					*pref=atoi(temp);
					gtflag=1;
				}
				else if (string[j]=='P') {
					gtflag=0;
				}
				else {
					cat(dummy,") ");
					gtflag=0;
				}
				i=0;
				while (dummy[i++] != '\0');
				--i;
				--j;
				while ((dummy[i++]=string[j++]) != '\0');
				dummy[i++]='\0';
				j=tind;
				replace(dummy,"<>"," != ",0,1);
				replace(dummy,"><"," != ",0,1);
				cpy(string,dummy);
				if (gtflag) {
					cat(string,";");
					squeeze(string,'\n',1,0);
				}
				replace(string,bword,prefix,1,1);
				break;

			case 1:  /* RETURN */

				break;

			case 2:  /* PRINT */

				if (index(string,"PRINT\\0",1)>=0 || index(string,"PRINT:",1)>=0) {
					replace(string,bword,"printf(\"\\n\");",1,1);
				}
				else {
					for (*temp= *var='\0',i=ind+5,j=k=l=v=q=0;string[i]!=':'&&string[i]!='\0';l++,i++) {
						temp[l]=string[i];
						if (string[i]=='\"') q++;
						if (q) {
							if (string[i]!='"')
								dummy[j++]=string[i];
							if (q==2) 
								q=0;
						}
						else if (string[i]==';')
							lf=0;
						else if (string[i]!=',' && !q) {
							if (*(string+i) != '\n')
								*(var+k++)= *(string+i);
							if (issupper(string[i]) || string[i]=='$') {
								if (!v++) {
									dummy[j++]='%';
									dummy[j++]='f';
									dummy[j++]=' ';
								}
							}
						}
						else if (string[i]==',') {
							if (var[k-1]=='%')
								var[k++]='f';
							var[k++]=',';
							v=0;
						}
						else error(PPF,string);
					}
					if (lf) {
						dummy[j++]='\\';
						dummy[j++]='n';
					}
					dummy[j++]='"';
					dummy[j++]=',';
					temp[l]=dummy[j]=var[k]='\0';
					cat(dummy,var);
					squeeze(dummy,'\015');
					cat(dummy,");");
					replace(string,temp,dummy,1,0);
					replace(string,bword,prefix,1,0);
					replace(string,",)",")",0,0);
				}
				break;

			case 3:  /* REM */

				replace(string,bword,prefix,1,1);
				replace(string,"\n",suffix,1,1);
				break;

			case 4:  /* INPUT */

				if ((index(string,"$",1))>0) {
					replace(string,bword,"strinp(",1,0);
					replace(string,";",",",1,0);
					if ((index(string,":",1))>=0)
						replace(string,":",",128);",1,0);
					else cat(string,",128)");
				}
				else {
					replace(string,bword,"numinp(",1,0);
					replace(string,";",",&",1,0);
					cat(string,",16)");
				}
				break;

			case 5:  /* GOSUB */


				replace(string,bword,prefix,0,1);
				replace(string,"f ","f",1,1);
				cat(string,suffix);
				break;

			case  7:  /* GOTO  */

				for (i=ind+4,j=0;issdigit(temp[j++]=string[i++]););
				temp[j]='\0';
				*pref=atoi(temp);
				replace(string,bword,prefix,1,1);
				cat(string,suffix);
				break;

			case 87:  /* TAB */

				for (i=ind,j=0;(dummy[j++]=string[i++])!=')';);
				dummy[j]='\0';
				replace(string,dummy,prefix,1,0);
				break;

			case 99:  /*  ^  */

				for (i=ind-1,j=0;issalpha(var[j++]=string[i--]););
				var[j-1]='\0';
				reverse(var);
				for (i=ind+1,j=0;issalpha(temp[j++]=string[i++]););
				temp[j-1]='\0';
				cpy(dummy,"pow(");
				cat(dummy,var);
				cat(dummy,",(double)(");
				cat(dummy,temp);
				cat(dummy,"))");
				cat(var,"^");
				cat(var,temp);
				replace(string,var,dummy,0,1);
				break;

			case  6:  /* END */
			case  8:  /* LET */
			case  9:  /* ABS */
			case 10:  /* AND */
			case 12:  /* ATN */
			case 19:  /* COS */
			case 25:  /* EXP */
			case 41:  /* INT */
			case 47:  /* LOG */
			case 53:  /* NOT */
			case 57:  /*  OR */
			case 79:  /* SIN */
			case 82:  /* SQR */
			case 88:  /* TAN */

				replace(string,bword,prefix,0,1);
				break;


			/* All the currently unparsed words below */

			case 13:  /*  AT */
			case 37:  /* HOME*/
			case 61:  /* POKE*/
			case 64:  /* PR# */
			case 84:  /* STOP*/

				cpy(dummy,"/* ?");
				cat(dummy,bword);
				for (i=ind+strlen(bword),j=0;(temp[j++]=string[i++])!=':' && string[i-1]!='\0';);
				temp[j]='\0';
				cat(dummy,temp);
				cat(dummy," */");
				for (i=0;(*(dummy+i)=lower(*(dummy+i)))!='\0';i++);
				replace(string,temp,"",1,0);
				replace(string,bword,dummy,0,1);
				break;
			default:
				break;
		}
	}
	squeeze(string,'\015');
}
