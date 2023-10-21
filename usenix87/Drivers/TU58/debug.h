#ifdef DEBUG

extern FILE *dbfd;
#undef stderr
#define stderr dbfd
#define debugs(st) \
	do { fprintf(dbfd,"%s\n",(st)); fflush(dbfd); } while (0)
#define debugd(st,dec) \
	do { fprintf(dbfd,"%s %d\n",(st),(dec)); fflush(dbfd); } while (0)
#define debugo(st,oct) \
	do { fprintf(dbfd,"%s 0%o\n",(st),(oct)); fflush(dbfd); } while (0)
#define debugx(st,hex) \
	do { fprintf(dbfd,"%s 0x%x\n",(st),(hex)); fflush(dbfd); } while (0)

#else

#define debugs(st);
#define debugd(st,dec); 
#define debugo(st,oct); 
#define debugx(st,hex); 

#endif
