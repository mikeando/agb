all : test.x

INCLUDES=-I../../../libgit2/include -I..

test.x : test.c ../ANBGitBridge.h
	g++ -g test.c -o test.x $(INCLUDES)

