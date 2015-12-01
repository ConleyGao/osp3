//Test3.10.cc: 
//Requesting and using 6 pages while there are only 
//4 pages in memory

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

	a[0] = 'h';
	b[1] = 'e';
	c[2] = 'l';
	d[3] = 'l';
	e[4] = 'o';
	f[5] = '!';
	//a[0] = 'H';
	vm_syslog(b, 6);

	cout << "Adela is awesome" << endl;
}