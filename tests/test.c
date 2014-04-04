#include "ANBGitBridge.h"
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>


#include "clar/clar.h"
#include "git2.h"

int main(int argc, char* argv[]) {

	git_threads_init();
//	anb_git_bridge_init();

	int res = clar_test(argc,argv);

	giterr_clear();
	git_threads_shutdown();

	return res;
}



