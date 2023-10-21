/* Test Object I/O

	THIS SOFTWARE FITS THE DESCRIPTION IN THE U.S. COPYRIGHT ACT OF A
	"UNITED STATES GOVERNMENT WORK".  IT WAS WRITTEN AS A PART OF THE
	AUTHOR'S OFFICIAL DUTIES AS A GOVERNMENT EMPLOYEE.  THIS MEANS IT
	CANNOT BE COPYRIGHTED.  THIS SOFTWARE IS FREELY AVAILABLE TO THE
	PUBLIC FOR USE WITHOUT A COPYRIGHT NOTICE, AND THERE ARE NO
	RESTRICTIONS ON ITS USE, NOW OR SUBSEQUENTLY.

Author:
	K. E. Gorlen
	Bg. 12A, Rm. 2017
	Computer Systems Laboratory
	Division of Computer Research and Technology
	National Institutes of Health
	Bethesda, Maryland 20892
	Phone: (301) 496-5363
	uucp: uunet!ncifcrf.gov!nih-csl!keith
	Internet: keith%nih-csl@ncifcrf.gov

Function:
	
Modification History:
	
$Log:	oio.c,v $
 * Revision 1.1  88/01/17  22:24:53  keith
 * Initial revision
 * 

*/
static char rcsid[] = "$Header: oio.c,v 1.1 88/01/17 22:24:53 keith Exp $";

#include "oopsconfig.h"
#include "ArrayOb.h"
#include "Assoc.h"
#include "Bag.h"
#include "Bitset.h"
#include "Date.h"
#include "Dictionary.h"
#include "Float.h"
#include "Fraction.h"
#include "Heap.h"
#include "LinkedList.h"
#include "LinkOb.h"
#include "OrderedCltn.h"
#include "Point.h"
#include "Range.h"
#include "Rectangle.h"
#include "Set.h"
#include "SortedCltn.h"
#include "Stack.h"
#include "String.h"
#include "Time.h"

Object* testStore()
{
	ArrayOb& arrayobid = *new ArrayOb(4);
	Bag& bag = *new Bag(100);
	Bitset& bitset = *new Bitset(4,6);
	Date& date = *new Date(11,"Nov",48);
	Dictionary& dict = *new Dictionary(100);
	Float& flt = *new Float(3.1415);
	Fraction& fract = *new Fraction(22,7);
	Heap& heap = *new Heap;
	LinkedList& linkedlist = *new LinkedList;
	OrderedCltn& orderedcltn = *new OrderedCltn;
	Point& point = *new Point(1,2);
	Range& range = *new Range(-1,1);
	Rectangle& rect = *new Rectangle(0,1,2,3);
	Set& set = *new Set(100);
	SortedCltn& sortedcltn = *new SortedCltn;
	Stack& stack = *new Stack;
	String& str = *new String("Hello, world!");
	Time& time = *new Time(date,1,2,3);
	
	arrayobid[0] = &bitset; arrayobid[1] = &point; arrayobid[2] = &str;
	orderedcltn.add(point); orderedcltn.add(rect); orderedcltn.add(str);
	orderedcltn.add(flt); orderedcltn.add(fract); orderedcltn.add(range);
	orderedcltn.add(date); orderedcltn.add(time);
	stack.push(bitset); stack.push(orderedcltn);
	bag.addWithOccurrences(point,1);
	bag.addWithOccurrences(rect,2);
	bag.addWithOccurrences(str,3);
	bag.add(orderedcltn);
	sortedcltn.add(*new String("Huey"));
	sortedcltn.add(*new String("Dewey"));
	sortedcltn.add(*new String("Louie"));
	heap.add(*new String("what's"));
	heap.add(*new String("going"));
	heap.add(*new String("on"));
	heap.add(*new String("hey"));
	heap.add(*new String("there"));
	heap.add(*new String("spirit"));
	heap.add(*new String("of"));
	heap.add(*new String("saint"));
	heap.add(*new String("louis"));
	heap.add(*new String("yes"));
	dict.add(*new Assoc(orderedcltn,point));
	dict.add(*new Assoc(bag,rect));
	dict.add(*new Assoc(sortedcltn,stack));
	linkedlist.add(*new LinkOb(point));
	linkedlist.add(*new LinkOb(rect));
	linkedlist.add(*new LinkOb(bitset));
	linkedlist.add(*new LinkOb(str));
	
	set.add(arrayobid);
	set.add(bag);
	set.add(dict);
	set.add(heap);
	set.add(linkedlist);
	set.add(orderedcltn);
	set.add(sortedcltn);
	set.add(stack);
	
	OrderedCltn& monster = *new OrderedCltn;
	monster.add(set); monster.add(*set.deepCopy());
	return &monster;
}

#include <fcntl.h>
#include <osfcn.h>

char* optarg;
int optind = 1;

static char* optchar = "?";

int getopt(int argc,char** argv,char* optstr) {
  int optlen = strlen(optstr);
  int inx;

  if ( argv[optind]==0 || strlen(argv[optind]) < 2 || *argv[optind] != '-' ) { /* not a switch */
     return EOF; 
     };
   *optchar = argv[optind][1];
     
//  inx = strcspn(optstr,optchar);
  for(inx=0; inx<= strlen(optstr); inx++)
    if ( optstr[inx] == *optchar ) break;
     
  if ( inx == optlen ) {			/* illegal switch */
    ++optind;
    fprintf(stderr,"getopt> illegal switch\n");
    return '?';
    };

  if ( (inx+1 <= optlen-1) && (optstr[inx+1] == ':') ) {/*switch has argument*/
    if (strlen(argv[optind]) == 2)
      optarg = argv[++optind];	       /* optarg is next argv */
    else 
      optarg = &argv[optind][2];	       /* optarg in this argv */
    };
  ++optind;
    return *optchar;
}


main(int argc, char** argv)
{
	extern int optind;		// used by getopt()
	extern char* optarg;		// used by getopt()
	char* filea = "oio.obj";
	char* fileb;
	bool binaryIO = NO;		// test binary storeOn/readFrom flag
	bool dumpOpt = NO;		// dump read object on cout
	bool filebOpt = NO;		// file to store object read specified

// parse command options	

	int c;
	while ((c = getopt (argc, argv, "bd")) != EOF)
		switch	(c) {
			case 'b': binaryIO = YES; break;
			case 'd': dumpOpt = YES; break;
			case '?':
				cerr << "usage: [ -bd ] [ filea [ fileb ] ]\n ";
				exit(2);
		}
	if (optind < argc) filea = argv[optind++];
	if (optind < argc) {
		fileb = argv[optind++];
		filebOpt = YES;
	}
				
	Object* a = testStore();
	Object* b;
	
	if (binaryIO) {
		cout << "Test storeOn(fileDescTy)\n";
		int fd = creat(filea,0664);
		a->storeOn(fd);
		close(fd);
		cout << "Test readFrom(fileDescTy)\n";
		fd = open(filea,O_RDONLY);
		b = readFrom(fd);
		close(fd);
	}
	else {
		cout << "Test storeOn(ostream&)\n";
		ostream* out = new ostream(creat(filea,0664));
		a->storeOn(*out);
		delete out;
		cout << "Test readFrom(istream&)\n";
		istream* in = new istream(open(filea,O_RDONLY));
		b = readFrom(*in);
		delete in;
	}

	
// compare object stored with object read	
	
	if (a->isEqual(*b)) {
		cout << "SUCCESS! -- stored object equals object read\n";
		if (dumpOpt) cout << *b;
	}
	else {
		cout << "FAILURE! -- stored object does not equal object read\n";
		cout << "*** object stored:\n" << *a << "\n";
		cout << "*** object read:\n" << *b << "\n";
	}
	
// store object read	

	if (filebOpt) {
		if (binaryIO) {
			int fd =creat(fileb,0664);
			b->storeOn(fd);
			close(fd);
		}
		else {
			ostream out(creat(fileb,0664));
			b->storeOn(out);
		}
	}
}

