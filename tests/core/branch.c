#include "clar/clar.h"
#include "agb.h"

#include "utils/test_utils.h"


__attribute__((unused))
static void check_git_ok(int ok) {
	if(ok!=0) {
		cl_fail(giterr_last()->message);
	}
}


static AGBCore  * anbGitBridge = NULL;
static AGBError * anbGitBridgeError = NULL ;
static AGBBranch * branch = NULL;

void test_core_branch__initialize(void) {
	agb_error_new(&anbGitBridgeError);
	branch = NULL;
}

void test_core_branch__cleanup(void) {
	agb_branch_delete(branch);
	agb_bridge_delete(anbGitBridge);
	agb_error_delete(anbGitBridgeError);
}

static void setupDefault(void) {
	create_simple_repo();
	agb_bridge_new(&anbGitBridge);
	init_bridge(anbGitBridge);
}

void test_core_branch__find_fails_correctly(void) {

	setupDefault();	

	cl_assert( branch == NULL );
	int ok = agb_branch_find(anbGitBridge, "refs/heads/no_such_branch", &branch, anbGitBridgeError );
	cl_assert( ok!=0 );
	cl_assert( branch == NULL );
	cl_assert_equal_s("git_reference_lookup failed : not found. [git:-3]", agb_error_message(anbGitBridgeError));
}
