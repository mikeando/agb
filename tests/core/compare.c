#include "clar/clar.h"
#include "agb.h"
#include "agb/internal/types.h"
#include "utils/clarx.h"


__attribute__((unused))
static void check_git_ok(int ok) {
	if(ok!=0) {
		cl_fail(giterr_last()->message);
	}
}

#define REPODIR "/Users/michaelanderson/Code/ANB/testDocRepos/test_compare/"


static AGBCore  * anbGitBridge = NULL;
static AGBError * anbGitBridgeError;
static AGBBranch * branch_a;
static AGBBranch * branch_b;

void test_core_compare__initialize(void) {
	agb_error_new(&anbGitBridgeError);
	branch_a = NULL;
	branch_b = NULL;
	anbGitBridge = NULL;
	//TODO: This way of opening a repo is _SUPER_ ugly
	agb_bridge_new(&anbGitBridge);
	int ok = git_repository_open(&anbGitBridge->repository,REPODIR);
	if(ok!=0) {
		cl_fail_v(__FILE__, __LINE__, "Unable to open repo %s : %s ", REPODIR, giterr_last()->message );
	}
	anbGitBridge->origin_name = "origin";
}

void test_core_compare__cleanup(void) {
	agb_branch_delete(branch_a);
	agb_branch_delete(branch_b);
	agb_bridge_delete(anbGitBridge);
	agb_error_delete(anbGitBridgeError);
}

__attribute__((unused))
static void setupDefault() {
}

void test_core_compare__same_gives_zero(void) {

	if(agb_branch_find(anbGitBridge, "A", &branch_a, anbGitBridgeError )!=0) {
		cl_failx( "Error getting branch \"refs/heads/A\" : %s", agb_error_message(anbGitBridgeError) );
	}
	if(agb_branch_find(anbGitBridge, "A", &branch_b, anbGitBridgeError )!=0) {
		cl_failx( "Error getting branch \"refs/heads/A\" : %s", agb_error_message(anbGitBridgeError) );
	}
	
	AGBBranchCompare compare;

	int ok = agb_branch_compare(branch_a, branch_b, &compare, anbGitBridgeError);

	cl_assert_equal_i(0, compare.extra_commits_on_a);
	cl_assert_equal_i(0, compare.extra_commits_on_b);
}

void test_core_compare__parent_gives_correct(void) {
	if(agb_branch_find(anbGitBridge, "A", &branch_a, anbGitBridgeError )!=0) {
		cl_failx( "Error getting branch \"A\" : %s", agb_error_message(anbGitBridgeError) );
	}
	if(agb_branch_find(anbGitBridge, "A_parent", &branch_b, anbGitBridgeError )!=0) {
		cl_failx( "Error getting branch \"A_parent\" : %s", agb_error_message(anbGitBridgeError) );
	}
	
	AGBBranchCompare compare;

	int ok = agb_branch_compare(branch_a, branch_b, &compare, anbGitBridgeError);

	cl_assert_equal_i(1, compare.extra_commits_on_a);
	cl_assert_equal_i(0, compare.extra_commits_on_b);

}

void test_core_compare__child_gives_correct(void) {
	if(agb_branch_find(anbGitBridge, "A_parent", &branch_a, anbGitBridgeError )!=0) {
		cl_failx( "Error getting branch \"A_parent\" : %s", agb_error_message(anbGitBridgeError) );
	}
	if(agb_branch_find(anbGitBridge, "A", &branch_b, anbGitBridgeError )!=0) {
		cl_failx( "Error getting branch \"A\" : %s", agb_error_message(anbGitBridgeError) );
	}
	
	AGBBranchCompare compare;

	int ok = agb_branch_compare(branch_a, branch_b, &compare, anbGitBridgeError);

	cl_assert_equal_i(0, compare.extra_commits_on_a);
	cl_assert_equal_i(1, compare.extra_commits_on_b);
}
