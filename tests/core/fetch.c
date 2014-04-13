#include "clar/clar.h"
#include "anbgitbridge.h"
#include "git2.h"

#include "utils/test_utils.h"
#include "utils/clarx.h"
#include <stdio.h>
#include <signal.h>


static int port=8090;
static pid_t server_pid = 0;

void test_core_fetch__initialize(void) {
	create_temp_dir();
	server_pid = start_serving_repos(port);
}

void kill_server(void) {
	if(server_pid>0) {
		kill(server_pid,SIGINT);

		int status;
		waitpid(server_pid, &status, 0);
		server_pid=0;
	}
}

void  test_core_fetch__cleanup(void) {
	kill_server();
}




void test_core_fetch__works(void) {
	int ok;

	create_repo("banana_remote");

	repo_create_file("banana_remote","banana_fetched.txt","This is a banana,");
	system_fmt("( cd %s/banana_remote ; git add banana_fetched.txt ; git commit -m'Adding banana' ; )", temp_dir );

	create_repo("banana");
	system_fmt("( cd %s/banana ; git remote add origin git://localhost:8090/banana_remote ; git commit -m'Adding banana' ; )", temp_dir );

	ANBGitBridge * anbGitBridge;
	ANBGitBridgeError * error ;
	cl_assert_equal_i(0, anb_gitbridge_error_new(&error));
	cl_assert_equal_i(0, anb_gitbridge_bridge_new(&anbGitBridge));

	init_bridge_with_repo(anbGitBridge,"banana");

	cl_assert( !repo_contains("banana", "banana_fetched.txt"));
	ok = anb_git_bridge_fetch(anbGitBridge, error);
	cl_assert_equal_i(0, ok);
	cl_assert( !repo_contains("banana", "banana_fetched.txt"));
	cl_assert_equal_c('A', status_in_commit("banana","origin/master","banana_fetched.txt"));

	anb_gitbridge_bridge_delete(anbGitBridge);
	anb_gitbridge_error_delete(error);
}

void test_core_fetch__reports_errors(void) {
	int ok;

	kill_server();

	ANBGitBridge * anbGitBridge;
	ANBGitBridgeError * error ;
	cl_assert_equal_i(0, anb_gitbridge_error_new(&error));
	cl_assert_equal_i(0, anb_gitbridge_bridge_new(&anbGitBridge));
	init_bridge_with_repo(anbGitBridge,"banana");

	ok = anb_git_bridge_fetch(anbGitBridge, error);
	cl_assert_equal_i(1, ok);
	//TODO: Better error message
	cl_assert_equal_s("git_remote_connect failed : Failed to connect to localhost: Connection refused [git:-1]", anb_gitbridge_error_message(error) );
	anb_gitbridge_bridge_delete(anbGitBridge);
	anb_gitbridge_error_delete(error);

}
