CC = /usr/bin/g++
CFLAGS = -Wall
LDFLAGS = -m64 -lc -arch x86_64

tests: test1 test2 test3 test4 test5 test6 test7

pager:
	$(CC) $(CFLAGS) -o pager pager.cc libvm_pager.a -ldl

test1:
	$(CC) $(CFLAGS) -o test1 test1.5.cc libvm_app.a -ldl
test2:
	$(CC) $(CFLAGS) -o test2 test2.2.cc libvm_app.a -ldl
test3:
	$(CC) $(CFLAGS) -o test3 test3.10.cc libvm_app.a -ldl
test4:
	$(CC) $(CFLAGS) -o test4 test4.10.cc libvm_app.a -ldl
test5:
	$(CC) $(CFLAGS) -o test5 test5.128.cc libvm_app.a -ldl
test6:
	$(CC) $(CFLAGS) -o test6 test6.2.cc libvm_app.a -ldl
test7:
	$(CC) $(CFLAGS) -o test7 test7.2.cc libvm_app.a -ldl

cleanPager:
	$(RM) pager
cleanTests:
	$(RM) test1 test2 test3 test4 test5 test6 test7
