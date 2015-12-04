//Test8.4.cc: 
//128 pages in memory
//test message with length of 0
//test whether syslog bring things to memory
//and make sure b saves the address in a


#include <iostream>
#include <stdint.h>
#include "vm_app.h"

using namespace std;

int main(){
	char *a, *b;
	a = (char *) vm_extend();
	b = a;
	a[0] = 's';

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

	if (vm_syslog(b, 10) == -1){
		cout << "syslog at b fails" << endl;
	}
	else {
		cout << "syslog at b succeeds" << endl;
	}	

	if (vm_syslog(a, 10) == -1){
		cout << "syslog at a fails" << endl;
	}
	else {
		cout << "syslog at a succeeds" << endl;
	}	
	
	cout << "Last statement" << endl;
}