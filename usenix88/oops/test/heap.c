/* Test class Heap

	THIS SOFTWARE FITS THE DESCRIPTION IN THE U.S. COPYRIGHT ACT OF A
	"UNITED STATES GOVERNMENT WORK".  IT WAS WRITTEN AS A PART OF THE
	AUTHOR'S OFFICIAL DUTIES AS A GOVERNMENT EMPLOYEE.  THIS MEANS IT
	CANNOT BE COPYRIGHTED.  THIS SOFTWARE IS FREELY AVAILABLE TO THE
	PUBLIC FOR USE WITHOUT A COPYRIGHT NOTICE, AND THERE ARE NO
	RESTRICTIONS ON ITS USE, NOW OR SUBSEQUENTLY.

Modification History:
	
$Log:	heap.c,v $
 * Revision 1.1  88/01/17  22:24:43  keith
 * Initial revision
 * 

*/
static char rcsid[] = "$Header: heap.c,v 1.1 88/01/17 22:24:43 keith Exp $";

#include "Heap.h"
#include "Integer.h"
#include "OrderedCltn.h"
#include "Set.h"
#include "Point.h"

main()
{
 	cout << "Testing Heap\n";
	Integer a=100;
	Integer x=70;
	Integer y=76;
	Integer d=22;
	Integer e=101;
	Integer f=54;
	Integer g=60;
	Integer h=2;
	Integer i=5;
	Integer j=601;
	OrderedCltn oc(10);
	oc.add(a);
	oc.add(x);
	oc.add(y);
	oc.add(d);
	oc.add(e);
	oc.add(f);
	oc.add(g);
	oc.add(h);
	oc.add(i);
	oc.add(j);
	cout << oc << "\n";

	cout << "Test asHeap function\n";
	cout << "OrderedCltn as Heap: \n" << oc.asHeap();
	Heap heap1 = oc.asHeap();

	Heap heap2(1);		//Check reSize function
	heap2.add(e);
	heap2.add(h);
	heap2.add(a);
	heap2.add(i);
	heap2.add(y);
	heap2.add(x);
	heap2.add(d);
	heap2.add(f);
	heap2.add(g);
	heap2.add(j);

	cout << "Test isEqual function\n";
	if (heap1.isEqual(heap2))   cout << "SUCCESS\n";
	else cout << "FAILURE\n";
	heap2.add(g);
	if (heap1!=heap2)   cout << "SUCCESS\n";
	else cout << "FAILURE\n";
	heap2.removeFirst();
	if (heap1==heap2)   cout << "FAILURE\n";
	else cout << "SUCCESS\n";

	Point A(1,1);
	Point B(1,2);
	Point C(1,3);
	Point D(1,3);
	Heap b(16);
	b.add(A);
	b.add(B);
	b.add(C);
	b.add(D);
	Heap c=b;
	cout << "c = " << c << "\n";
	cout << "b = " << b << "\n";
	cout << "b.first(): " << *(b.first()) << "\n";
	cout << "b.last(): " << *(b.last()) << "\n";
	Point E(4,5);
	b.add(E);
	cout << "b = " << b << "\n";
	cout << "remove min from b " << *b.removeFirst() << "\n";
	cout << "remove max from b " << *b.removeLast() << "\n";
	Point F(3,2);
	Point G(1,4);
	Point H(5,6);
	Point I(0,1);
	Point J(9,8);
	b.add(F);
	b.add(G);
	b.add(H);
	b.add(I);
	b.add(J);
	b.add(I);
	b.add(B);
	b.add(A);
	b.add(B);
	cout << "b= " << b << "\n"; 
	cout << "\noccurrencesOf((1,2)): " << b.occurrencesOf(B) << "\n";
	cout << "b.sort(): " << b.sort() << "\n";
	cout << "b.asSet(): " << (b.asSet()) << "\n\n";

	cout << "Testing remove(Object&)\n";
	Integer aa = 57;
	Integer bb = 36;
	Integer cc = 45;
	Integer dd = 59;
	Integer ee = 20;
	Integer ff = 14;
	Integer gg = 32;
	Integer hh = 18;
	Integer ii = 5;
	Integer jj = 28;
	Integer kk = 17;
	Integer ll = 30;
	Integer mm = 16;
	Integer nn = 65;
	Integer oo = 34;
	Integer pp = 27;
	Integer qq = 30;
	Integer rr = 31;
	Integer ss = 80;
	Integer tt = 8;
	Integer uu = 37;
	Integer vv = 39;
	Integer ww = 38;
	Integer xx = 45;
	Integer yy = 50;
	Integer zz = 15;
	Integer aaa = 12;
	Integer bbb = 13;
	Integer ccc = 10;
	Integer ddd = 25;
	Integer eee = 15;
	Heap aheap(15);
	aheap.add(aa);
	aheap.add(bb);
	aheap.add(cc);
	aheap.add(dd);
	aheap.add(ee);
	aheap.add(ff);
	aheap.add(gg);
	aheap.add(hh);
	aheap.add(ii);
	aheap.add(jj);
	aheap.add(kk);
	aheap.add(ll);
	aheap.add(mm);
	aheap.add(nn);
	aheap.add(oo);
	aheap.add(pp);
	aheap.add(qq);
	aheap.add(rr);
	aheap.add(ss);
	aheap.add(tt);
	aheap.add(uu);
	aheap.add(vv);
	aheap.add(ww);
	aheap.add(xx);
	aheap.add(yy);
	aheap.add(zz);
	aheap.add(aaa);
	aheap.add(bbb);
	aheap.add(ccc);
	aheap.add(ddd);
	aheap.add(eee);
	cout << aheap << "\n\n";
	aheap.remove(ccc);
	cout << aheap << "\n\n";
	aheap.remove(ss);	
	cout << aheap << "\n\n";
	ff.value(16);
	cout << aheap << "\n\n";
	aheap.remove(hh);
	cout << aheap << "\n\n";
	
}
