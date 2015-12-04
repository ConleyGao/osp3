//Test14.2.cc: 
//Should never write out pages of b except for the 1st time
//since b is never modified after that
//Check the functionality of zeroFilledBit

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


	cout << "c2(2): " << c[2] << endl;

	cout << "Good bye world." << endl;
}