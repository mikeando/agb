#include "clar/clar.h"
#include "agb.h"
#include "agb/status.h"

#include "utils/test_utils.h"
#include "utils/clarx.h"
#include <stdio.h>


__attribute__((unused))
static void check_git_ok(int ok) {
	if(ok!=0) {
		cl_fail(giterr_last()->message);
	}
}


static AGBCore  * repo;
static AGBError * e;

void test_core_status__initialize(void) {
	agb_error_new(&e);
}


#include "agb/internal/types.h"
#include <assert.h>


#include <string.h>



void test_core_status__simple_status_report(void) {
	create_simple_repo();
	simple_repo_create_file("dummy.txt","Nothing exciting");

	agb_bridge_new(&repo);
	init_bridge(repo);

	AGBStatus * status=NULL;
	agb_get_status_new(&status, repo);
	cl_assert(status!=NULL);

	int istatus = agb_status_for_filename("dummy.txt", status);

	cl_assert_equal_i(istatus, GIT_STATUS_WT_NEW);
}

void test_core_status__simple_status_report2(void) {
	create_simple_repo();
	simple_repo_create_file("README.txt","Nothing exciting");

	agb_bridge_new(&repo);
	init_bridge(repo);

	AGBStatus * status=NULL;
	agb_get_status_new(&status, repo);
	cl_assert(status!=NULL);

	{
		int istatus = agb_status_for_filename("dummy.txt", status);
		cl_assert_equal_i(istatus, -1);
	}

	{
		int istatus = agb_status_for_filename("README.txt", status);
		cl_assert_equal_i(istatus, GIT_STATUS_WT_MODIFIED);
	}
}

