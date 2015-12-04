//Test7.2.cc: 
//2 page in memory
//length of 9
//initalizing before extend


#include <iostream>
#include <stdint.h>
#include "vm_app.h"

using namespace std;

int main(){
	char *a = new (nothrow) char;
	a[0] = 's';

	a = (char *) vm_extend();

	//a[0] = 'm';
	a[1] = 'e';
	a[2] = 'r';
	a[3] = 'p';
	a[4] = 'a';
	a[5] = 'd';
	a[6] = 'e';
	a[7] = 'r';
	a[8] = 'p';
	if (vm_syslog(a, 9) == -1){
		cout << "syslog for a fails. Buggy pager!" << endl;
	}
	else {
		cout << "syslog for a succeeds" << endl;
	}

	cout << "Last statement" << endl;
}