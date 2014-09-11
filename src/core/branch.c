#include "agb.h"
#include "agb/internal/types.h"
#include "agb/internal/eh.h"

#include <string.h>
#include <stdio.h>

int agb_branch_delete( AGBBranch * branch ) {
	if(branch==NULL) return 0;
	git_reference_free(branch->ref);
	return 0;
}

int agb_branch_find( AGBCore * core , const char * name, AGBBranch ** branch, AGBError * e) {

	if(core==NULL) {
		e->error_code = 2;
		e->error_code_git = 0;
		free((char*)e->message);
		asprintf((char**)&e->message, "agb_branch_find : passed NULL core");
		return 1;
	}

	git_reference * ref=NULL;
	int ok;
	
	if((ok=git_reference_dwim(&ref, core->repository, name))!=0) {
		agb__error_translate(e,"git_reference_lookup failed",ok);
		goto cleanup_error;
	}

	AGBBranch * retval = (AGBBranch*)malloc(sizeof(AGBBranch));
	memset(retval,0,sizeof(AGBBranch));
	retval->ref = ref;
	*branch = retval;


	return 0;

cleanup_error:
	if(ref) git_reference_free(ref);
	return e->error_code;
}

int agb_branch_compare( const AGBBranch * branch_a, const AGBBranch * branch_b, AGBBranchCompare * result) {
	result->extra_commits_on_a = 0;
	result->extra_commits_on_b = 0;
	return 1;
}

