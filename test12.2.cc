/*****************************************************************************
 File:   test12.2.cc
 Author: Adela Yang, Venecia Xu & Son Ngo
 Date:   Dec 2015

 Memory pages: 2
 Description: Pages pointed to by b should never be written out 
 				(thus never read from disk)
******************************************************************************/

#include <iostream>
#include <stdint.h>
#include "vm_app.h"

using namespace std;

int main(){
	char *a, *b, *c;
	a = (char *) vm_extend();
	b = (char *) vm_extend();
	c = (char *) vm_extend();

	a[0] = 'b';
	c[2] = 'e';

	vm_syslog(b,2);

	a[1] = 'v';
	c[1] = 'c';

	vm_syslog(b,2);

	a[2] = 'e';
	c[2] = 'c';

	vm_syslog(b,2);

	a[3] = 'n';
	c[3] = 'c';

	vm_syslog(b,2);

	a[4] = 'e';
	c[4] = 'c';

	vm_syslog(b,2);
	a[5] = 'c';
	a[6] = 'i';

	vm_syslog(b,2);

	a[7] = 'a';

	vm_syslog(a, 8);
	vm_syslog(b, 15);
	vm_syslog(c, 15);
	vm_syslog(a, 15);

	cout << "c2(2): " << c[2] << endl;
}