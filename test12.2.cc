//Test12.2.cc: 
//Only 2 memory pages --> a lot of page fault

#include <iostream>
#include <stdint.h>
#include "vm_app.h"

using namespace std;

int main(){
	char *a, *b, *c, *d, *e, *f; 
	a = (char *) vm_extend();
	b = (char *) vm_extend();
	c = (char *) vm_extend();
	d = (char *) vm_extend();
	e = (char *) vm_extend();
	f = (char *) vm_extend();
	//a = (char *) vm_extend();
	//cout << "test2: get a = " << hex << a << endl;
	//cout << "test1: get p = " << p << endl;

	a[0] = 'b';
	//b[1] = 'd';
	c[2] = 'e';
	d[3] = 'l';
	e[4] = 'a';
	f[5] = '!';
	//a[0] = 'H';

	//cout << "b: " << b[2] << endl;
	vm_syslog(b,2);

	a[1] = 'v';
	c[1] = 'c';

	vm_syslog(b,2);

	a[2] = 'e';
	c[2] = 'c';

	//cout << "b: " << b[2] << endl;
	vm_syslog(b,2);

	a[3] = 'n';
	c[3] = 'c';

	//cout << "b: " << b[2] << endl;
	vm_syslog(b,2);

	a[4] = 'e';
	c[4] = 'c';

	vm_syslog(b,2);
	a[5] = 'c';
	a[6] = 'i';

	vm_syslog(b,2);

	a[7] = 'a';

	vm_syslog(a, 8);
	vm_syslog(d, 8);
	vm_syslog(b, 15);
	vm_syslog(c, 15);
	vm_syslog(d, 15);
	vm_syslog(e, 15);
	vm_syslog(f, 15);
	vm_syslog(a, 15);

	cout << "c2(2): " << c[2] << endl;

	cout << "Adele(a) is awesome" << endl;
}