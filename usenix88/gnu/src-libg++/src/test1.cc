// This may look like C code, but it is really -*- C++ -*-

/*
 * a few tests for new stream package
 *
 * creation: Sat Jan 30 07:12:30 1988  Doug Lea  (dl at rocky.oswego.edu)
 *
 */

#include <std.h>
#include <stream.h>
#include <SFile.h>
#include <PlotFile.h>


class record
{
public:
  char c; int i; double d;
};

ostream& operator<<(ostream& s, record& r)
{
  return(s << "(i = " << r.i << " c = " << r.c << " d = " << r.d << ")");
}

main()
{
  char ch;
  int i;
  short h;
  long l;
  float f;
  double d;
  char s[100];
  record r;

  cout << "Hello, world via cout\n";
  cerr << "Hello, world via cerr\n";

  cout << "enter a char:";  cin >> ch;
  cout.put('c');  cout.put(' ');  cout.put('=');  cout.put(' ');
  cout.put('"');  cout.put(ch);    cout << '"';  cout << char('\n');

  cout << "enter three integers (short, int, long):";  
  cin >> h; cin >> i;   cin.scan("%ld", &l);
  cout << "first  = " << h << " via dec = " << dec(h, 8) << "\n";
  cout << "second = " << i << form(" via form = %d = 0%o", i, i);
  cout.form(" via File::form = %d = 0x%x\n", i, i);
  cout << "third  = " << l  << " via hex = " << hex(l) << "\n";

  cout << "enter a float then a double:";  cin >> f; cin >> d;
  cout << "first  = " << f << "\n";
  cout << "second = " << d << "\n";

  cout << "enter 5 characters separated with spaces:";  cin >> s;
  cout << "first  = " << s << "\n";
  cin.get(s, 100);
  cout << "rest   = " << s << "\n";



  cout << "\nMaking streams sout and sin...";
  ostream sout("streamfile", io_writeonly, a_create);
  sout << "This file has one line testing output streams.\n";
  sout.close();
  istream sin("streamfile", io_readonly, a_useonly);
  cout << "contents of file:\n";
//  while(sin.get(c)) cout << c;
  while(sin >> ch) cout << ch;
  sin.remove();

  
  cout << "\nMaking File tf ... ";
  File tf("tempfile", io_readwrite, a_create);
  strcpy(s, "This is the first and only line of this file.\n");
  tf.put(s);
  tf.seek(0);
  tf.get(s, 100);
  cout << "first line of file:\n" << s << "\n";
  cout << "next char = ";
  tf.get(ch);
  cout << (int)ch;
  cout << " (should be 10 = int value of newline char)\n";
  strcpy(s, "Now there is a second line.\n");
  cout << "reopening tempfile, appending: " << s;
  tf.open(tf.name(), io_appendonly, a_use);
  tf.put(s);
  tf.open(tf.name(), io_readonly, a_use);
  tf.raw();
  cout << "First 10 chars via raw system read after reopen for input:\n";
  read(tf.filedesc(), s, 10);
  for (i = 0; i < 10; ++ i)
    cout.put(s[i]);
  lseek(tf.filedesc(), 5, 0);
  cout << "\nContents after raw lseek to pos 5:\n";
  while ( (tf.get(ch)) && (cout.put(ch)) );
  tf.remove();


  cout << "\nMaking SFile rf...";
  SFile rf("recfile", sizeof(record), io_readwrite, a_create);
  for (i = 0; i < 10; ++i)
  {
    r.c = i + 'a';
    r.i = i;
    r.d = (double)(i) / 1000.0;
    rf.put(&r);
  }
  cout << "odd elements of file in reverse order:\n";
  for (i = 9; i >= 0; i -= 2)
  {
    rf[i].get(&r);
    cout << r << "\n";
  }
  rf.remove();


  cout << "\nMaking PlotFile pf ..."; 
  PlotFile pf("plot.out", io_writeonly, a_create);
  pf.move(10,10);
  pf.label("Test");
  pf.circle(300,300,200);
  pf.line(100, 100, 500, 500);
  cout << "(You may delete or attempt to plot " << pf.name() << ")\n";


  cout << "\nThe following file open should generate error message:";
  File ef("shouldnotexist", io_readonly, a_useonly);


  cout << "\nFinal names & states:\n";
  cout << "cin:      " << cin.name()  << "\t" << cin.rdstate() << "\n";
  cout << "cout:     " << cout.name() << "\t" << cout.rdstate() << "\n";
  cout << "cerr:     " << cerr.name() << "\t" << cerr.rdstate() << "\n";
  cout << "sout:     " << sout.name() << "\t" << sout.rdstate() << "\n";
  cout << "sin:      " << sin.name()  << "\t" << sin.rdstate() << "\n";
  cout << "tf:       " << tf.name()   << "\t" << tf.rdstate() << "\n";
  cout << "rf:       " << rf.name()   << "\t" << rf.rdstate() << "\n";
  cout << "pf:       " << pf.name()   << "\t" << pf.rdstate() << "\n";
  cout << "ef:       " << ef.name()   << "\t" << ef.rdstate() << "\n";

  cout << "\nend of test.\n";
}
