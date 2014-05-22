#include "clar/clar.h"
#include "agb.h"
#include "git2.h"

#include "utils/test_utils.h"
#include "utils/clarx.h"
#include <stdio.h>


__attribute__((unused))
static void check_git_ok(int ok) {
	if(ok!=0) {
		cl_fail(giterr_last()->message);
	}
}


static AGBCore  * anbGitBridge;
static AGBError * anbGitBridgeError;
static AGBBranch * branch_a;
static AGBBranch * branch_b;

void test_core_compare__initialize(void) {
	agb_error_new(&anbGitBridgeError);
	branch_a = NULL;
	branch_b = NULL;
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

	agb_branch_find(anbGitBridge, "A", &branch_a, anbGitBridgeError );
	agb_branch_find(anbGitBridge, "A", &branch_b, anbGitBridgeError );
	
	AGBBranchCompare compare;

	agb_branch_compare(branch_a, branch_b, &compare);

	cl_assert_equal_i(0, compare.extra_commits_on_a);
	cl_assert_equal_i(0, compare.extra_commits_on_b);
}

void test_core_compare__parent_gives_correct(void) {
	agb_branch_find(anbGitBridge, "A", &branch_a, anbGitBridgeError );
	agb_branch_find(anbGitBridge, "A_parent", &branch_b, anbGitBridgeError );
	
	AGBBranchCompare compare;

	agb_branch_compare(branch_a, branch_b, &compare);

	cl_assert_equal_i(1, compare.extra_commits_on_a);
	cl_assert_equal_i(0, compare.extra_commits_on_b);

}

void test_core_compare__child_gives_correct(void) {
	agb_branch_find(anbGitBridge, "A_parent", &branch_a, anbGitBridgeError );
	agb_branch_find(anbGitBridge, "A", &branch_b, anbGitBridgeError );
	
	AGBBranchCompare compare;

	agb_branch_compare(branch_a, branch_b, &compare);

	cl_assert_equal_i(0, compare.extra_commits_on_a);
	cl_assert_equal_i(1, compare.extra_commits_on_b);
}
