//Test5.128.cc: 
//1 page in memory
//page should see values of 0 (within valid addresses)
//make sure to catch invalid addresses

#include <iostream>
#include <stdint.h>
#include "vm_app.h"

using namespace std;

int main(){
	char *a;
	a = (char *) vm_extend();
	
	if (vm_syslog(a, 8192) == -1){
		cout << "syslog at a fails" << endl;
	}
	else {
		cout << "syslog at a succeeds" << endl;
	}

	if (vm_syslog(a, 8193) == -1){
		cout << "syslog at a fails" << endl;
	}
	else {
		cout << "syslog at a succeeds" << endl;
	}

	cout << "Last statement" << endl;
}