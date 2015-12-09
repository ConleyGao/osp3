/*****************************************************************************
 File:   test6.2.cc
 Author: Adela Yang, Venecia Xu & Son Ngo
 Date:   Dec 2015

 Memory pages: 2
 Description: 
 Page b should be written out only 1 time (optimization)
 Similar to test9.2.cc but with arbitrary yield statements (test 10)

 Pages pointed to by e should never be written out 
 				(thus never read from disk) (test 12)
 				
 Simple program checking the updates of state (1,0,1,0,0) when 
 	evicted by the clock algorithm and the functionality of zeroFilledBit  (test 13)
******************************************************************************/

#include <iostream>
#include <stdint.h>
#include "vm_app.h"

using namespace std;

int main(){

/*****************************************************************************/
/* test 10 */
	char *a, *b, *c;
	a = (char *) vm_extend();
	b = (char *) vm_extend();
	c = (char *) vm_extend();

	a[0] = 'a';
	b[1] = 'b';
	c[2] = 'c';

	vm_syslog(b,2);
	vm_yield();

	a[1] = 'v';
	vm_yield();
	c[1] = 'c';

	vm_syslog(b,2);
	vm_yield();

	a[2] = 'e';
	c[2] = 'c';

	vm_syslog(b,2);

	a[3] = 'n';
	c[3] = 'c';

	vm_syslog(b,2);

	a[4] = 'e';
	vm_yield();
	c[4] = 'c';

	vm_syslog(b,2);

	a[5] = 'c';
	vm_yield();
	a[6] = 'i';

	vm_syslog(b,2);

	a[7] = 'a';

	cout << "c2(2): " << c[2] << endl;
	
	cout << "Chuc mung ban da dat 100 diem!" << endl;
	cout << "!(Son shi hen cong ming)" << endl;
	cout << "!(Son es muy intelligente)" << endl;
	cout << "!(Son est tres intelligent)" << endl;

	cout << "test 10" << endl;

/*****************************************************************************/
/* test 12 */
	char *d, *e, *f;
	d = (char *) vm_extend();
	e = (char *) vm_extend();
	f = (char *) vm_extend();

	d[0] = 'b';
	f[2] = 'e';

	vm_syslog(e,2);

	d[1] = 'v';
	f[1] = 'c';

	vm_syslog(e,2);

	d[2] = 'e';
	f[2] = 'c';

	vm_syslog(e,2);

	d[3] = 'n';
	f[3] = 'c';

	vm_syslog(e,2);

	d[4] = 'e';
	f[4] = 'c';

	vm_syslog(e,2);
	d[5] = 'c';
	d[6] = 'i';

	vm_syslog(e,2);

	d[7] = 'a';

	vm_syslog(d, 8);
	vm_syslog(e, 15);
	vm_syslog(f, 15);
	vm_syslog(d, 15);

	cout << "f2(2): " << f[2] << endl;

	cout << "test 12" << endl;

/*****************************************************************************/
/* test 13 */
	char *g, *h, *i;
	g = (char *) vm_extend();
	h = (char *) vm_extend();
	i = (char *) vm_extend();

	vm_syslog(g, 5);

	h[1] = 'b';
	i[1] = 'c';

	vm_syslog(g, 5);

	cout << "test 13" << endl;


}