diag(s,p1,p2,p3,p4,p5,p6)
{
  register f;
  extern fout;

  flush();
  f = fout;
  fout = 1;
  printf("diag: ");
  printf(s,p1,p2,p3,p4,p5,p6);
  flush();
  fout = f;
}
