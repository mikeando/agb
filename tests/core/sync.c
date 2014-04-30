#include "clar/clar.h"
#include "agb.h"
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

AGBCore  * anbGitBridge;
AGBError * anbGitBridgeError;

void test_core_sync__initialize(void) {
	agb_error_new(&anbGitBridgeError);
}

void test_core_sync__cleanup(void) {
	agb_bridge_delete(anbGitBridge);
	agb_error_delete(anbGitBridgeError);
}

void setupDefault() {
	create_simple_repo();
	agb_bridge_new(&anbGitBridge);
	init_bridge(anbGitBridge);
}

void setupTofu() {
	create_repo("tofu");
	agb_bridge_new(&anbGitBridge);
	init_bridge_with_repo(anbGitBridge,"tofu");
}


void test_core_sync__add_simple(void) {
	setupDefault();
	simple_repo_create_file("dummy.dat","hello world");

	agb_sync_files(anbGitBridge, anbGitBridgeError);

	cl_assert(simple_repo_contains("dummy.dat"));
	cl_assert_equal_c('A', status_in_commit("simple_repo", "HEAD", "dummy.dat"));	
}

void test_core_sync__add_in_subdir(void) {
	setupDefault();
	system_fmt("( cd %s/simple_repo ; mkdir somedir ; )", temp_dir);
	simple_repo_create_file("somedir/dummy.dat","hello world");


	agb_sync_files(anbGitBridge, anbGitBridgeError);

	cl_assert(simple_repo_contains("somedir/dummy.dat"));
	cl_assert_equal_c('A', status_in_commit("simple_repo", "HEAD", "somedir/dummy.dat"));	
}

void test_core_sync__removes_deleted_files(void) {
	setupDefault();
	simple_repo_delete_file("README.txt");


	agb_sync_files(anbGitBridge, anbGitBridgeError);

	cl_assert(!simple_repo_contains("README.txt"));
	cl_assert_equal_c('D', status_in_commit("simple_repo", "HEAD", "README.txt"));	
}

void test_core_sync__saves_modified_files(void) {
	setupDefault();
	system_fmt("echo 'HelloAgain' >> %s/simple_repo/README.txt", temp_dir);
	agb_sync_files(anbGitBridge, anbGitBridgeError);
	cl_assert_equal_c('M', status_in_commit("simple_repo", "HEAD", "README.txt"));	
}

void test_core_sync__gets_repo_from_bridge(void) {
	setupTofu();
	repo_create_file("tofu","dummy.dat","hello world");

	agb_sync_files(anbGitBridge, anbGitBridgeError);

	cl_assert(repo_contains("tofu", "dummy.dat"));
	cl_assert_equal_c('A', status_in_commit("tofu", "HEAD", "dummy.dat"));	
}
