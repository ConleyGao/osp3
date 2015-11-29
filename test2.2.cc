#include <iostream>
#include <stdint.h>
#include "vm_app.h"

using namespace std;

int main(){
	char *p; 
	p = (char *) vm_extend();
	//a = (char *) vm_extend();
	//cout << "test2: get a = " << hex << a << endl;
	//cout << "test1: get p = " << p << endl;

	p[0] = 'h';
	p[1] = 'e';
	p[2] = 'l';
	p[3] = 'l';
	p[4] = 'o';
	vm_syslog(p, 5);

	cout << "Adela is awesome" << endl;
}