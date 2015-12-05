/*****************************************************************************
 File:   test1.4.cc
 Author: Adela Yang, Venecia Xu & Son Ngo
 Date:   Dec 2015

 Memory pages: 128
 Description: Simple program to check the functionality of syslog, and pages
 	should see 0 for unmodified addresses
 		Also syslog should return -1 when len = 0
******************************************************************************/

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

	if (vm_syslog(b, 1) == -1){
		cout << "syslog at b fails" << endl;
	}
	else {
		cout << "syslog at b succeeds" << endl;
	}	

	if (vm_syslog(a, 0) == -1){
		cout << "syslog at a fails" << endl;
	}
	else {
		cout << "syslog at a succeeds" << endl;
	}	
	
	cout << "Last statement" << endl;
}