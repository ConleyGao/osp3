//Test5.128.cc: 
//1 page in memory
//length of 0

#include <iostream>
#include <stdint.h>
#include "vm_app.h"

using namespace std;

int main(){
	char *a;
	a = (char *) vm_extend();
	a = (char *) vm_extend();


	a[0] = 'm';
	a[1] = 'e';
	a[2] = 'r';
	a[3] = 'p';
	a[4] = 'a';
	a[5] = 'd';
	a[6] = 'e';
	a[7] = 'r';
	a[8] = 'p';
	vm_syslog(a, 0);

	cout << "Last statement" << endl;
}