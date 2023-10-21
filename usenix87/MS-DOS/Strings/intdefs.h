#ifdef MSC
#include <dos.h>
#define STRUCTURE union REGS r;
#define RAX r.x.ax 
#define RBX r.x.bx
#define RCX r.x.cx 
#define RDX r.x.dx 
#define RSI r.x.si 
#define RDI r.x.di
#define CALLDOS intdos(&r, &r)
#define RESULT r.x.cflag
#define PUTC(i) (tsc) ? putch(i) : fputc(i,stdout)
#define PUTS(i) (tsc) ? cputs(i) : fputs(i,stdout)
#define GETC getch()
#define KBCHECK kbhit()
#endif

#ifdef CIC86
#define STRUCTURE struct{int ax, bx, cx, dx, si, di, ds, es;}r;\
struct{int cs,ss,dz,ez;}s;int x_z;
#define RAX r.ax 
#define RBX r.bx
#define RCX r.cx 
#define RDX r.dx 
#define RSI r.si 
#define RDI r.di
#define CALLDOS  segread(&s); r.ds = s.dz; x_z = sysint21(&r, &r) & 1
#define RESULT x_z
#define PUTC(i) fputc(i, stdout)
#define PUTS(i) fputs(i, stdout)
#define GETC (bdos(7) & 0xFF)
#define KBCHECK (bdos(0x0B) & 1)
#endif

#ifdef DESMET
#define STRUCTURE extern int _rax, _rbx, _rcx, _rdx, _rsi, _rdi, _rds, _carryf;
#define RAX _rax
#define RBX _rbx
#define RCX _rcx
#define RDX _rdx
#define RSI _rsi
#define RDI _rdi
#define RSI _rsi
#define CALLDOS _rds = -1;_doint(0x21)
#define RESULT (_carryf & 1)
#define PUTC(i) fputc(i, stdout)
#define PUTS(i) fputs(i, stdout)
#define GETC ci()
#define KBCHECK csts()
#endif

