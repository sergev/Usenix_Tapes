      subroutine line(x1,y1,x2,y2,thick,iclip)
c
c    routine line draws a line from coordinates (x1,y1) to (x2,y2)
c    are as in routine box.
c
      dimension x(3),y(3)
c
      x(1) = x1
      y(1) = y1
      x(3) = x2
      y(3) = y2
      x(2) = (x1+x2)/2.
      y(2) = (y1+y2)/2.
      call nplot(3,x,y,0,iclip,thick,' ')
c
      return
      end
c
c
