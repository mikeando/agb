#include "clar/clar.h"
#include "agb.h"

#include "utils/test_utils.h"
#include "utils/clarx.h"
#include <signal.h>
#include <sys/wait.h>


static int port=8090;
static pid_t server_pid = 0;

static AGBCore * anbGitBridge;
static AGBError * error ;

void test_core_fetch__initialize(void) {
	create_temp_dir();
	server_pid = start_serving_repos(port);

	agb_error_new(&error);
}

void kill_server(void) {
	if(server_pid>0) {
		kill(server_pid,SIGINT);

		int status;
		waitpid(server_pid, &status, 0);
		server_pid=0;
	}
}

void test_core_fetch__cleanup(void) {
	kill_server();
	agb_bridge_delete(anbGitBridge);
	agb_error_delete(error);
}

void create_remote_repo(void) {
	create_repo("banana_remote");

	repo_create_file("banana_remote","banana_fetched.txt","This is a banana,");
	system_fmt("( cd %s/banana_remote ; git add banana_fetched.txt ; git commit -m'Adding banana' ; )", temp_dir );
}

void create_local_repo(void) {
	create_repo("banana");
	system_fmt("( cd %s/banana ; git remote add origin git://localhost:8090/banana_remote ; git commit -m'Adding banana' ; )", temp_dir );
	agb_bridge_new(&anbGitBridge);
	init_bridge_with_repo(anbGitBridge,"banana");
}




void test_core_fetch__works(void) {
	int ok;

	create_remote_repo();
	create_local_repo();

	cl_assert( !repo_contains("banana", "banana_fetched.txt"));
    size_t ahead, behind;
	ok = agb_fetch(anbGitBridge, error, &ahead, &behind);
	cl_assert_equal_i(0, ok);
	cl_assert( !repo_contains("banana", "banana_fetched.txt"));
	cl_assert_equal_c('A', status_in_commit("banana","origin/master","banana_fetched.txt"));
    cl_assert_equal_i(0, (int)ahead);
    cl_assert_equal_i(1, (int)behind);

}

void test_core_fetch__reports_errors(void) {
	int ok;
	create_remote_repo();
	create_local_repo();

	kill_server();
    size_t ahead, behind;

	ok = agb_fetch(anbGitBridge, error, &ahead, &behind);
	cl_assert_equal_i(1, ok);
	cl_assert_equal_s("git_remote_connect failed : Failed to connect to localhost: Connection refused [git:-1]", agb_error_message(error) );
}


//TODO: This should have more arguments.
static void * g_userdata;
void fetch_callback(void * userdata) {
	g_userdata = userdata;
}

void test_core_fetch__calls_callbacks(void) {
	int ok;
	create_remote_repo();
	create_local_repo();

	void * userdata = &ok;
	ok = agb_set_fetch_callback(anbGitBridge, fetch_callback, userdata,error);
	cl_assert_equal_i(0, ok);

	g_userdata = NULL;
    size_t ahead, behind;
	ok = agb_fetch(anbGitBridge, error, &ahead, &behind);
	cl_assert_equal_i(0, ok);
	cl_assert_equal_p(userdata, g_userdata);
}

