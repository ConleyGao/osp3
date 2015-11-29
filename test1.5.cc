#include <iostream>
#include <stdint.h>
#include "vm_app.h"

using namespace std;

int main(){
	//char *p;
	// int q; 
	char* a;   
	//p = (char *) vm_extend();
	// q = (intptr_t) vm_extend();
	a = (char *) vm_extend();
	// // cout << "test1: get q = " << hex << q << endl;
	//cout << "test1: get a = " << hex << a << endl;
	// cout << "test1: get p = " << p << endl;
	
	for (int i = 0; i < 100; i++){
		a[i] = 'a';
	}

	vm_yield();

	vm_syslog(a,100);

	cout << "Adela is crazy" << endl;
}