all : test.x

INCLUDES=-I../../../libgit2/include -I../include -I.
LIBS=-L../../../libgit2/build -lgit2
CFLAGS=-Wall -Wimplicit-function-declaration -Wimplicit-int -Wimplicit

#TODO: Move this

../src/core/sync_files.o : ../src/core/sync_files.c ../include/anbgitbridge.h ../include/anbgitbridge/internal/types.h
	gcc -g -c $(CFLAGS) ../src/core/sync_files.c -o ../src/core/sync_files.o $(INCLUDES)

utils/clarx.o : utils/clarx.c utils/clarx.h
	gcc -g -c $(CFLAGS) utils/clarx.c -o utils/clarx.o $(INCLUDES)

utils/test_utils.o : utils/test_utils.c utils/test_utils.h
	gcc -g -c $(CFLAGS) utils/test_utils.c -o utils/test_utils.o $(INCLUDES)

core/sync.o : core/sync.c
	gcc -g -c $(CFLAGS) core/sync.c -o core/sync.o $(INCLUDES)

clar.o : clar.c clar.h clar.suite
	gcc -g -c clar.c

test.x : test.c ../include/anbgitbridge.h clar.o core/sync.o utils/test_utils.o utils/clarx.o ../src/core/sync_files.o
	gcc -g $(CFLAGS) test.c -o test.x $(INCLUDES) $(LIBS) external/popen3.o clar.o core/sync.o utils/test_utils.o utils/clarx.o ../src/core/sync_files.o

clean :
	rm test.x
	rm *.o
	rm utils/*.o
