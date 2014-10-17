#include "git2/threads.h"
#include "git2/errors.h"
#include "clar/clar.h"

int main(int argc, char* argv[]) {

	git_threads_init();
//	anb_git_bridge_init();

	int res = clar_test(argc,argv);

	giterr_clear();
	git_threads_shutdown();

	return res;
}



