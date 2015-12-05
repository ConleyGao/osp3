/*****************************************************************************
 File:   test8.128.cc
 Author: Adela Yang, Venecia Xu & Son Ngo
 Date:   Dec 2015

 Memory pages: 128
 Description: Trying to extend more than available disk blocks
******************************************************************************/

#include <iostream>
#include <stdint.h>
#include "vm_app.h"

using namespace std;

int main(){
	char *a = (char*) vm_extend();
	a[0] = 's';
	unsigned int i;
	for (i = 0; i <= 8192; i++){
		a = (char *) vm_extend();
		if (a == NULL){
			cout << "not enough arena size for " << i << endl;
			break;
		}
	}
	
	if (i >= 1024){
		cout << "pager is buggy" << endl;
	}

	cout << "Last statement" << endl;
}