#include "clar/clar.h"
#include "anbgitbridge.h"
/* TODO: Remove this include and switch to using pointers.. */
#include "anbgitbridge/internal/types.h"
#include "git2.h"

#include "utils/test_utils.h"
#include "utils/clarx.h"
#include <stdio.h>


void check_git_ok(int ok) {
	if(ok!=0) {
		cl_fail(giterr_last()->message);
	}
}


int debug_me(const char * path, const char * refspec, void * payload) {
//	printf("==> %s debug_me : %s : %s\n",(char*) payload, path, refspec);
	return 0;
}


void test_core_sync__add_simple(void) {
	create_simple_repo();
	simple_repo_create_file("dummy.dat","hello world");

	ANBGitBridge anbGitBridge;
	ANBGitBridgeError anbGitBridgeError;
	init_bridge(&anbGitBridge);

	anb_git_bridge_sync_files(&anbGitBridge, &anbGitBridgeError);

	cl_assert(simple_repo_contains("dummy.dat"));
	cl_assert_equal_c('A', status_in_commit("simple_repo", "HEAD", "dummy.dat"));	
}

void test_core_sync__add_in_subdir(void) {
	create_simple_repo();
	system_fmt("( cd %s/simple_repo ; mkdir somedir ; )", temp_dir);
	simple_repo_create_file("somedir/dummy.dat","hello world");

	ANBGitBridge anbGitBridge;
	ANBGitBridgeError anbGitBridgeError;
	init_bridge(&anbGitBridge);

	anb_git_bridge_sync_files(&anbGitBridge, &anbGitBridgeError);

	cl_assert(simple_repo_contains("somedir/dummy.dat"));
	cl_assert_equal_c('A', status_in_commit("simple_repo", "HEAD", "somedir/dummy.dat"));	
}

void test_core_sync__removes_deleted_files(void) {
	create_simple_repo();
	simple_repo_delete_file("README.txt");

	ANBGitBridge anbGitBridge;
	ANBGitBridgeError anbGitBridgeError;
	init_bridge(&anbGitBridge);

	anb_git_bridge_sync_files(&anbGitBridge, &anbGitBridgeError);

	cl_assert(!simple_repo_contains("README.txt"));
	cl_assert_equal_c('D', status_in_commit("simple_repo", "HEAD", "README.txt"));	
}

void test_core_sync__saves_modified_files(void) {
	create_simple_repo();
	ANBGitBridge anbGitBridge;
	ANBGitBridgeError anbGitBridgeError;
	init_bridge(&anbGitBridge);
	system_fmt("echo 'HelloAgain' >> %s/simple_repo/README.txt", temp_dir);
	anb_git_bridge_sync_files(&anbGitBridge, &anbGitBridgeError);
	cl_assert_equal_c('M', status_in_commit("simple_repo", "HEAD", "README.txt"));	
}

void test_core_sync__gets_repo_from_bridge(void) {
	create_repo("tofu");
	repo_create_file("tofu","dummy.dat","hello world");

	ANBGitBridge anbGitBridge;
	ANBGitBridgeError anbGitBridgeError;
	init_bridge_with_repo(&anbGitBridge,"tofu");

	anb_git_bridge_sync_files(&anbGitBridge, &anbGitBridgeError);

	cl_assert(repo_contains("tofu", "dummy.dat"));
	cl_assert_equal_c('A', status_in_commit("tofu", "HEAD", "dummy.dat"));	
}
