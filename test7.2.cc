/*****************************************************************************
 File:   test7.2.cc
 Author: Adela Yang, Venecia Xu & Son Ngo
 Date:   Dec 2015

 Memory pages: 2
 Description: syslog before extend
******************************************************************************/

#include <iostream>
#include <stdint.h>
#include "vm_app.h"
#include <stdio.h>

using namespace std;

int main(){
	char *a;
    char *b;
    char *c;

	a = (char *) vm_extend();

	a[0] = 'd';
	a[1] = 'v';
	a[2] = 'e';
	a[3] = 'n';
	a[4] = 'e';
	a[5] = 'c';
	a[6] = 'i';
	a[7] = 'a';
    a[8192-1] = 'o';

    vm_syslog(a, 8192);
    
    cout << "switched" << endl; //should print out 'dv'
    b = a;
    
    vm_syslog(a, 1);    //should print out 'd'
    vm_syslog(b, 2);    //should print out 'd'
    
    a = (char *) vm_extend();
    
    cout << "a is renewed" << endl;
    
    a[0] = 's';
    b[1] = 'e';
    b[2] = 'n';
    
	vm_syslog(a, 1);    //should print out 's'
    vm_syslog(b, 10);    //should print out 'den'
    
    c = (char *) vm_extend();
    c[0] = 'm';
    c[1] = 'e';
    c[2] = 'e';
    c[3] = 'p';

    if (vm_syslog(b, 3*8192) == 0){
        cout << "succeeds" << endl;
    }
    else {
        cout << "failure -> pager buggy!" << endl;
    }

    if (vm_syslog(b, 3*8192 + 1) == -1){
        cout << "failure" << endl;
    }
    else {
        cout << "should not succeed --> pager buggy!" << endl;
    }

    if (vm_syslog(c-8192, 8192) == 0){
        cout << "succeeds" << endl;
    }
    else {
        cout << "failure -> pager buggy!" << endl;
    }

    cout << "test 7" << endl;
}