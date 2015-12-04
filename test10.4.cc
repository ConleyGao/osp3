//Test10.4.cc: 
//4 pages in memory
//Description: check for enough disk blocks condition


#include <iostream>
#include <stdint.h>
#include "vm_app.h"

using namespace std;

int main(){
	char *a = (char*) vm_extend();
	a[0] = 's';
	unsigned int i;
	for (i = 0; i <= 5; i++){
		a = (char *) vm_extend();
		if (a == NULL){
			cout << "not enough arena size for " << i << endl;
			break;
		}
	}
	
	if (i >= 5){
		cout << "pager is buggy" << endl;
	}

	cout << "Last statement" << endl;
}