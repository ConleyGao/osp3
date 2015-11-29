#include <iostream>
#include "vm_app.h"

using namespace std;

int main(){
	char *p;
	p = (char *) vm_extend();
	cout << "test1: p = " << p << endl;
	p[0] = 'h';
	p[1] = 'e';
	p[2] = 'l';
	p[3] = 'l';
	p[4] = 'o';
	vm_syslog(p, 5);
}