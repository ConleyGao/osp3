/*****************************************************************************
 File:   test4.2.cc
 Author: Adela Yang, Venecia Xu & Son Ngo
 Date:   Dec 2015

 Memory pages: 2
 Description: Illegal write to an invalid page (fault should return -1)
******************************************************************************/

#include <iostream>
#include <stdint.h>
#include "vm_app.h"

using namespace std;

int main(){
	char *a = (char*) vm_extend();
	a[8193] = 't';
	cout << "This should not be printed out" << endl;
}