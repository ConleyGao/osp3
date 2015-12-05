/*****************************************************************************
 File:   test6.2.cc
 Author: Adela Yang, Venecia Xu & Son Ngo
 Date:   Dec 2015

 Memory pages: 2
 Description: Trying to syslog at invalid address
******************************************************************************/

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