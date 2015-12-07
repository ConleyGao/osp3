CC = /usr/bin/g++
CFLAGS = -Wall
LDFLAGS = -m64 -lc -arch x86_64

tests: test1 test2 test3 test4 test5 test6 test7 test8 test9 test10 test11 test12 test13

pager:
	$(CC) $(CFLAGS) -o pager pager.cc libvm_pager.a -ldl

print:
	$(CC) $(CFLAGS) -D PRINT -o pager pager.cc libvm_pager.a -ldl

test1:
	$(CC) $(CFLAGS) -o test1 test1.4.cc libvm_app.a -ldl
test2:
	$(CC) $(CFLAGS) -o test2 test2.2.cc libvm_app.a -ldl
test3:
	$(CC) $(CFLAGS) -o test3 test3.4.cc libvm_app.a -ldl
test4:
	$(CC) $(CFLAGS) -o test4 test4.2.cc libvm_app.a -ldl
test5:
	$(CC) $(CFLAGS) -o test5 test5.128.cc libvm_app.a -ldl
test6:
	$(CC) $(CFLAGS) -o test6 test6.2.cc libvm_app.a -ldl
test7:
	$(CC) $(CFLAGS) -o test7 test7.2.cc libvm_app.a -ldl
test8:
	$(CC) $(CFLAGS) -o test8 test8.128.cc libvm_app.a -ldl
test9:
	$(CC) $(CFLAGS) -o test9 test9.2.cc libvm_app.a -ldl
test10:
	$(CC) $(CFLAGS) -o test10 test10.2.cc libvm_app.a -ldl
test11:
	$(CC) $(CFLAGS) -o test11 test11.2.cc libvm_app.a -ldl
test12:
	$(CC) $(CFLAGS) -o test12 test12.2.cc libvm_app.a -ldl
test13:
	$(CC) $(CFLAGS) -o test13 test13.2.cc libvm_app.a -ldl

cleanPager:
	$(RM) pager
cleanTests:
	$(RM) test1 test2 test3 test4 test5 test6 test7 test8 test9 test10 test11 test12 test13
