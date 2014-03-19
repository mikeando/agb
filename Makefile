all : dummy.x

ANBGitBridge.o : ANBGitBridge.c ANBGitBridge.h
	g++ -g -c -Wall ANBGitBridge.c -I../../libgit2/include

dummy.o : dummy.c ANBGitBridge.h
	g++ -g -c -Wall dummy.c -I../../libgit2/include

dummy.x : dummy.o ANBGitBridge.o
	g++ -g -o dummy.x dummy.o ANBGitBridge.o -lgit2 -L../../libgit2/build/
