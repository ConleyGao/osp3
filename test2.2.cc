/*****************************************************************************
 File:   test2.2.cc
 Author: Adela Yang, Venecia Xu & Son Ngo
 Date:   Dec 2015

 Memory pages: 2
 Description: 
 Syslog should not read other pages' data (test 3)

 Trying to syslog at invalid address (test 6)
 Simple program checking the updates of state (1,0,1,0,0) when 
 	evicted by the clock algorithm and the functionality of zeroFilledBit  (test 14)

 syslog before extend (test 15)
 
 syslog before extend (test 16)
******************************************************************************/

#include <iostream>
#include <stdint.h>
#include "vm_app.h"

using namespace std;

int main(){

/*****************************************************************************/
/* test 3 */ 
	char *a, *b, *c, *d, *e, *f; 
	a = (char *) vm_extend();
	b = (char *) vm_extend();
	c = (char *) vm_extend();
	d = (char *) vm_extend();
	e = (char *) vm_extend();
	f = (char *) vm_extend();

	a[0] = 'a';
	b[1] = 'b';
	c[2] = 'c';
	d[3] = 'd';
	e[4] = 'e';
	f[5] = 'f';

	cout << "c2: " << c[2] << endl;

	a[1] = 'v';
	a[2] = 'e';
	a[3] = 'n';
	a[4] = 'e';
	a[5] = 'c';
	a[6] = 'i';
	a[7] = 'a';
	vm_syslog(a, 8);
	vm_syslog(d, 8);
	vm_syslog(b, 15);
	vm_syslog(c, 15);
	vm_syslog(d, 15);
	vm_syslog(e, 15);
	vm_syslog(f, 15);
	vm_syslog(a, 15);

	cout << "c2(2): " << c[2] << endl;

	cout << "test 3" << endl;

/*****************************************************************************/
/* test 6 */ 
	char *g;
	g = (char *) vm_extend();
	
	if (vm_syslog(g, 8192) == -1){
		cout << "syslog at g fails" << endl;
	}
	else {
		cout << "syslog at g succeeds" << endl;
	}

	if (vm_syslog(g, 8193) == -1){
		cout << "syslog at g fails" << endl;
	}
	else {
		cout << "syslog at g succeeds" << endl;
	}

	cout << "test 6" << endl;

/*****************************************************************************/
/* test 14 */ 
	char *h;
	h = (char *) vm_extend();

	for (unsigned int i = 0; i < VM_PAGESIZE; i++){
		h[i] = '1';
	}

	if (vm_syslog(h, 8192) == -1){
		cout << "syslog at h fails, buggy" << endl;
	}
	else {
		cout << "syslog at h succeeds." << endl;
	}

	if (vm_syslog(h, 8193) == -1){
		cout << "syslog at h fails." << endl;
	}
	else {
		cout << "syslog at h succeeds. Buggy pager!" << endl;
	}	

	cout << "test 14" << endl;

/*****************************************************************************/
/* test 15 */ 
	char *i;

	vm_syslog(i,8);

	i = (char *) vm_extend();

	i[0] = 'i';
	i[1] = 'v';
	i[2] = 'e';
	i[3] = 'n';
	i[4] = 'e';
	i[5] = 'c';
	i[6] = 'i';
	i[7] = 'a';
	
	vm_syslog(i, 8);

	cout << "test 15" << endl;

/*****************************************************************************/
/* test 16 */ 
	char *j;

	j = (char *) vm_extend();

	j[0] = 'j';
	j[1] = 'v';
	j[2] = 'e';
	j[3] = 'n';
	j[4] = 'e';
	j[5] = 'c';
	j[6] = 'i';
	j[7] = 'a';
	
	vm_syslog((void*)-1, 8);

	cout <<  "test 16" << endl;

}