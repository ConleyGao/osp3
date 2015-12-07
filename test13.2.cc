/*****************************************************************************
 File:   test13.2.cc
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
	char *a, *b, *c;
	a = (char *) vm_extend();
	b = (char *) vm_extend();
	c = (char *) vm_extend();

	vm_syslog(a, 5);

	b[1] = 'b';
	c[1] = 'c';

	vm_syslog(a, 5);

	cout << "End of test 13" << endl;
}