all : dummy.x

INCLUDES=-I../../libgit2/include

ANBGitBridge.o : ANBGitBridge.c ANBGitBridge.h
	g++ -g -c -Wall ANBGitBridge.c $(INCLUDES)

dummy.o : dummy.c ANBGitBridge.h
	g++ -g -c -Wall dummy.c $(INCLUDES)

dummy.x : dummy.o ANBGitBridge.o
	g++ -g -o dummy.x dummy.o ANBGitBridge.o -lgit2 -L../../libgit2/build/
