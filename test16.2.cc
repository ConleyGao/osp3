/*****************************************************************************
 File:   test16.2.cc
 Author: Adela Yang, Venecia Xu & Son Ngo
 Date:   Dec 2015

 Memory pages: 2
 Description: syslog before extend
******************************************************************************/

#include <iostream>
#include <stdint.h>
#include "vm_app.h"

using namespace std;

int main(){
	char *a;

	a = (char *) vm_extend();

	a[0] = 'd';
	a[1] = 'v';
	a[2] = 'e';
	a[3] = 'n';
	a[4] = 'e';
	a[5] = 'c';
	a[6] = 'i';
	a[7] = 'a';
	
	vm_syslog((void*)-1, 8);
}