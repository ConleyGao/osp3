//Test5.128.cc: 
//1 page in memory
//no initializing

#include <iostream>
#include <stdint.h>
#include "vm_app.h"

using namespace std;

int main(){
	char *a;
	a = (char *) vm_extend();
	vm_syslog(a, 1);

	cout << "Last statement" << endl;
}