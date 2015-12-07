/*****************************************************************************
 File:   test14.2.cc
 Author: Adela Yang, Venecia Xu & Son Ngo
 Date:   Dec 2015

 Memory pages: 2
 Description: Simple program checking the updates of state (1,0,1,0,0) when 
 	evicted by the clock algorithm and the functionality of zeroFilledBit 
******************************************************************************/

#include <iostream>
#include <stdint.h>
#include "vm_app.h"

using namespace std;

int main(){
	char *a;
	a = (char *) vm_extend();

	for (unsigned int i = 0; i < VM_PAGESIZE; i++){
		a[i] = '1';
	}

	if (vm_syslog(a, 8192) == -1){
		cout << "syslog at a fails, buggy" << endl;
	}
	else {
		cout << "syslog at a succeeds." << endl;
	}

	if (vm_syslog(a, 8193) == -1){
		cout << "syslog at a fails." << endl;
	}
	else {
		cout << "syslog at a succeeds. Buggy pager!" << endl;
	}	

	cout << "End of test 14" << endl;
}