#include "clar/clar.h"
#include "anbgitbridge.h"
#include "anbgitbridge/internal/types.h"
#include "git2.h"

#include "utils/test_utils.h"
#include "utils/clarx.h"
#include <stdio.h>
#include <signal.h>

void anb_git_bridge_fetch(ANBGitBridge * anbGitBridge, ANBGitBridgeError * anbGitBridgeError) {
	// TODO: Do this using libgit2!
	int ok = system_fmt("( cd %s/banana ; git fetch origin )", temp_dir );
	cl_assert_equal_i(ok,0);
}

static int port=8090;
static pid_t server_pid = 0;

void test_core_fetch__initialize(void) {
	create_temp_dir();
	server_pid = start_serving_repos(port);
}

void  test_core_fetch__cleanup(void) {
	kill(server_pid,SIGINT);
	
	int status;
	waitpid(server_pid, &status, 0);
}


void test_core_fetch__fail(void) {

	printf("Working in temp_dir=%s\n",temp_dir);
	create_repo("banana_remote");

	repo_create_file("banana_remote","banana_fetched.txt","This is a banana,");
	system_fmt("( cd %s/banana_remote ; git add banana_fetched.txt ; git commit -m'Adding banana' ; )", temp_dir );

	create_repo("banana");
	system_fmt("( cd %s/banana ; git remote add origin git://localhost:8090/banana_remote ; git commit -m'Adding banana' ; )", temp_dir );

	ANBGitBridge anbGitBridge;
	ANBGitBridgeError anbGitBridgeError;
	init_bridge_with_repo(&anbGitBridge,"banana");

	cl_assert( !repo_contains("banana", "banana_fetched.txt"));
	anb_git_bridge_fetch(&anbGitBridge, &anbGitBridgeError);
	cl_assert( !repo_contains("banana", "banana_fetched.txt"));
	cl_assert_equal_c('A', status_in_commit("banana","origin/master","banana_fetched.txt"));
}
