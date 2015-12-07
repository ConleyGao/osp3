/*****************************************************************************
 File:   test17.2.cc
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
    
    vm_syslog(a, 2);
    
    cout << "switched" << endl; //should print out 'dv'
    b = a;
    
    vm_syslog(a, 1);    //should print out 'd'
    vm_syslog(b, 1);    //should print out 'd'
    
    a = (char *) vm_extend();
    
    cout << "a is renewed" << endl;
    
    a[0] = 's';
    b[1] = 'e';
    b[2] = 'n';
    
	vm_syslog(a, 1);    //should print out 's'
    vm_syslog(b, 3);    //should print out 'den'
    
    b = (char *) vm_extend();
    
    c = (char *) vm_extend();
    c = "meep";
    
    //b = c;
    //b[0] = '3';
    vm_syslog(c, 4);
}