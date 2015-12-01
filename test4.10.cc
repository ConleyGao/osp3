//Test4.10.cc: 
//10 pages in memory

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
	b[1] = 'd';
	c[2] = 'e';
	d[3] = 'l';
	e[4] = 'a';
	f[5] = '!';
	//a[0] = 'H';

	a[1] = 'v';
	a[2] = 'e';
	a[3] = 'n';
	a[4] = 'e';
	a[5] = 'c';
	a[6] = 'i';
	a[7] = 'a';
	vm_syslog(b, 8);

	cout << "Adela is awesome" << endl;
}