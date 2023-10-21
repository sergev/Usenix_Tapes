// This may look like C code, but it is really -*- C++ -*-
 
/*
 A test file for Strings
*/

#include <std.h>
#include <stream.h>
#include <String.h>

main()
{
  cout << "Simple string demo from documentation examples\n";

  String dec1;
  cout << "an empty String:" << dec1 << "\n";

  String dec2 = "Hello";
  cout << "A string initialized to Hello:" << dec2 << "\n";

  String dec2a = dec2;
  cout << "A string initialized to previous string:" << dec2a << "\n";

  String dec2b = dec2a.at(1, 2);
  cout << "A string initialized to previous string.at(1, 2):" << dec2b << "\n";

  String dec3 = '@';
  cout << "A string initialized to @:" << dec3 << "\n";

  String dec4 = dec(20);
  cout << "A string initialized to dec(20):" << dec4 << "\n";

  String X = "Hello";
  String Y = "world";
  String N = "123";
  char*  s = ",";
  Regex  r = "e[a-z]*o";
  char   c;
  int    i;
  double f;

  String x = X;
  String y = Y;
  String n = N;
  String z;

  cout << "x (reset for each test) = " << x << "\n";
  cout << "y (reset for each test) = " << y << "\n";

  x = X;
  z = x + y;
  cout << "z = x + y = " << z << "\n";

  x += y;
  cout << "x += y; x = " << x << "\n";

  x = X;
  cout << "(char*)x = " << (char*)(x) << "\n";

  c = x[0];
  cout << "c = x[0] = " << c << "\n";


  n = N;
  i = atoi(n);
  f = atof(n);
  cout << "n = " << n << " atoi(n) = " << i << " atof(n) = " << f << "\n";


  x = X;
  z = x.at(2, 3);
  cout << "z = x.at(2, 3) = " << z << "\n";

  x.at(2, 2) = "r";
  cout << "x.at(2, 2) = r; x = " << x << "\n";

  x = X;
  x.at(0, 1) = "j";
  cout << "x.at(0, 1) = j; x = " << x << "\n";

  x = X;
  x.at("He") = "je";
  cout << "x.at(He) = je; x = " << x << "\n";

  x = X;
  x.at("l", -1) = "i";
  cout << "x.at(l, -1) = i; x = " << x << "\n";

  x = X;
  z = x.at(r);
  cout << "z = x.at(r) = " << z << "\n";

  z = x.before("o");
  cout << "z = x.before(o) = " << z << "\n";

  x.before("ll") = "Bri";
  cout << "x.before(ll) = Bri; x = " << x << "\n";

  x = X;
  z = x.before(2);
  cout << "z = x.before(2) = " << z << "\n";

  z = x.after("Hel");
  cout << "z = x.after(Hel) = " << z << "\n";

  x.after("Hel") = "p";  
  cout << "x.after(Hel) = p; x = " << x << "\n";

  x = X;
  z = x.after(3);
  cout << "z = x.after(3) = " << z << "\n";

  z = "  a bc";
  z  = z.after(RXwhite);
  cout << "z =   a bc; z = z.after(RXwhite); z =" << z << "\n";

  x = X;
  y = Y;
  z = x + s + ' ' + y.at("w") + y.after("w") + ".";
  cout << "z = x + s +  + y.at(w) + y.after(w) + . = " << z << "\n";

  z = x + y;

  cout << "relationals: x = " << x << " y = " << y << " z = " << z << "\n";
  cout << "(Regex r = e[a-z]*o) \n";

  if (x == y)
  {
    cout << x << " == " << y << "\n";
  }
  else
  {
    cout << x << " != " << y << "\n";
  }

  if (x == "Hello")
  {
    cout << x << " == " << "Hello" << "\n";
  }
  else
  {
    cout << x << " != " << "Hello" << "\n";
  }

  if (x == z.at(0, 4))
  {
    cout << x << " == " << z.at(0, 4) << "\n";
  }
  else
  {
    cout << x << " != " << z.at(0, 4) << "\n";
  }

  if (x < y)
  {
    cout << x << " < " << y << "\n";
  }
  else
  {
    cout << x << " not < " << y << "\n";
  }

  if (x >= z.at(0, 6))
  {
    cout << x << " >= " << z.at(0, 6) << "\n";
  }
  else
  {
    cout << x << " not >= " << z.at(0, 6) << "\n";
  }


  if (x.contains("He"))
  {
    cout << "x contains He" << "\n";
  }
  else
  {
    cout << "x does not contain He" << "\n";
  }

  if (z.contains(x))
  {
    cout << "z contains x" << "\n";
  }
  else
  {
    cout << "z does not contain x" << "\n";
  }

  if (x.contains(r))
  {
    cout << "x contains r" << "\n";
  }
  else
  {
    cout << "x does not contain r" << "\n";
  }

  cout << "x.matches(r) = " << x.matches(r) << "\n";
  cout << "x.matches(RXalpha) = " << x.matches(RXalpha) << "\n";
  cout << "n.matches(RXalpha) = " << n.matches(RXalpha) << "\n";
  cout << "n.matches(RXint) = " << n.matches(RXint) << "\n";
  cout << "n.matches(RXdouble) = " << n.matches(RXdouble) << "\n";

  cout << "x.index(lo) = " << x.index("lo") << "\n";
  cout << "x.index(l, 2) = " << x.index("l", 2) << "\n";
  cout << "x.index(l, -1) = " << x.index("l", -1) << "\n";
  cout << "x.index(r) = " << x.index(r) << "\n";
  cout << "x.index(r, -2) = " << x.index(r, -2) << "\n";

  x = X;
  x.gsub("l", "ll");
  cout << "x.gsub(l, ll); x = " << x << "\n";

  x = X;
  x.gsub(r, "ello should have been replaced by this string");
  cout << "x.gsub(r, ...); x = " << x << "\n";

  z = X + Y;
  z.del("loworl");
  cout << "z = x+y; z.del(loworl); z = " << z << "\n";

  x = X;
  z = reverse(x);
  cout << "z = reverse(x) = " << z << "\n";

  z = replicate('*', 10);
  cout << "z = replicate(*, 10) = " << z << "\n";
  cout << "z.length() = " << z.length() << "\n";

  z = "left/middle/right";
  Regex r2("/[a-z]*/");
  String lft, mid, rgt;
  decompose(z, lft, mid, rgt, r2);
  cout << "decompose test: z = " << z << "\n";
  cout << "decompose(z, lft, mid, rgt, r2=Regex(/[a-z]*/)) :\n";
  cout << "lft =" << lft << "\n";
  cout << "mid =" << mid << "\n";
  cout << "rgt =" << rgt << "\n";

  z = "This string\thas\nfive words";
  cout << "split test: z = " << z << "\n";

  String w[10];
  int nw = split(z, w, 10, RXwhite);
  cout << "from split(z, RXwhite, w, 10), n words = " << nw << ":\n";
  for (i = 0; i < nw; ++i)
    cout << w[i] << "\n";

  z = join(w, nw, "/");
  cout << "z = join(w, nw, /); z =" << z << "\n";

  cout << "enter a word:";
  cin >> z;
  cout << "\nz =" << z << "\n";
  cout << "z.length = " << z.length() << "\n";

  cout << "\nEnd of test\n";
}
