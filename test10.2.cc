/*****************************************************************************
 File:   test10.128.cc
 Author: Adela Yang, Venecia Xu & Son Ngo
 Date:   Dec 2015

 Memory pages: 128
 Description: Similar to test9.2.cc but with arbitrary yield statements
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
	b[1] = 'd';
	c[2] = 'e';

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
}
