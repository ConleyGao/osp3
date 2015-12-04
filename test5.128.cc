//Test5.128.cc: 
//1 page in memory
//length of 0

#include <iostream>
#include <stdint.h>
#include "vm_app.h"

using namespace std;

int main(){
	char *a, *b;
	a = (char *) vm_extend();
	b = a;
	a[0] = 's';
	//a = (char *) vm_extend();


	//a[0] = 'm';
	a[1] = 'e';
	a[2] = 'r';
	a[3] = 'p';
	a[4] = 'a';
	a[5] = 'd';
	a[6] = 'e';
	a[7] = 'r';
	a[8] = 'p';

	cout << "what the hell" << endl;
	if (vm_syslog(b, 1) == -1){
		cout << "syslog at b fails" << endl;
	}
	else {
		cout << "syslog at b succeeds" << endl;
	}	

	if (vm_syslog(a, 0) == -1){
		cout << "adela is crazy" << endl;
	}
	else {
		cout << "i'm awesome" << endl;
	}
	
	cout << "Last statement" << endl;
}