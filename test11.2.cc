//Test11.2.cc: 
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

	a[0] = 'b';
	b[1] = 'd';
	c[2] = 'e';
	d[3] = 'l';
	e[4] = 'a';
	f[5] = '!';

	cout << "c2: " << c[2] << endl;

	a[1] = 'v';
	a[2] = 'e';
	a[3] = 'n';
	a[4] = 'e';
	a[5] = 'c';
	a[6] = 'i';
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

	cout << "Good bye world." << endl;
}