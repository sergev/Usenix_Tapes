      subroutine gnumb(x,y,h,val,angle,ifmt)
      character*20 iar,ifmt
      write(iar,"(ifmt)") val
      call symbol(x,y,h,iar,angle,20)
      return
      end
