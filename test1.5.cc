#include <iostream>
#include <stdint.h>
#include "vm_app.h"

using namespace std;

int main(){
	char *p;
	int q; 
	int* a;   
	p = (char *) vm_extend();
	q = (intptr_t) vm_extend();
	a = (int *) vm_extend();
	cout << "test1: get q = " << hex << q << endl;
	cout << "test1: get a = " << hex << a << endl;
	//cout << "test1: get p = " << hex << p << endl;
	
	//a[0] = 1;
	a[1] = 2;

	vm_syslog(a,2);

	// p[0] = 'h';
	// p[1] = 'e';
	// p[2] = 'l';
	// p[3] = 'l';
	// p[4] = 'o';
	// vm_syslog(p, 5);
}