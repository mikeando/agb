#include "agb.h"
#include "agb/internal/types.h"
#include "agb/internal/eh.h"

#include <string.h>
#include <stdio.h>

int agb_branch_delete( AGBBranch * branch ) {
	if(branch==NULL) return 0;
	git_reference_free(branch->ref);
	if(branch->oid) free(branch->oid);
	return 0;
}

int agb_branch_find( AGBCore * core , const char * name, AGBBranch ** branch, AGBError * e) {

	git_reference * ref=NULL;
	git_reference * resolved=NULL;
	const git_oid * oid;
	int ok;

	if(core==NULL) {
		agb__error_from_string(e, "agb_branch_find : passed NULL core", 2, 0 );
		return 2;
	}

	
	if((ok=git_reference_dwim(&ref, core->repository, name))!=0) {
		agb__error_translate(e,"git_reference_lookup failed",ok);
		goto cleanup_error;
	}

	if((ok=git_reference_resolve(&resolved, ref))!=0) {
		agb__error_translate(e,"git_reference_resolve failed",ok);
		goto cleanup_error;
	}
	oid=git_reference_target(resolved);

	AGBBranch * retval = (AGBBranch*)malloc(sizeof(AGBBranch));
	memset(retval,0,sizeof(AGBBranch));
	retval->ref = ref;
	retval->oid = malloc(GIT_OID_RAWSZ);
	retval->core = core;
	git_oid_cpy(retval->oid, oid);
	*branch = retval;


	return 0;

cleanup_error:
	if(ref) git_reference_free(ref);
	if(resolved) git_reference_free(resolved);
	return e->error_code;
}

int agb_branch_compare(const AGBBranch * branch_a, const AGBBranch * branch_b, AGBBranchCompare * result, AGBError * error ) {

	if(branch_a->core!=branch_b->core) {
		agb__error_from_string(error,"Both branches to agb_branch_compare must be from the same AGBCore.",2,0);
		return 2;
	}


	git_repository * repo = branch_a->core->repository;

	result->extra_commits_on_a = 0;
	result->extra_commits_on_b = 0;
	int ok = git_graph_ahead_behind(
			&result->extra_commits_on_a,
			&result->extra_commits_on_b,
			repo,
			branch_a->oid,
			branch_b->oid
			);
	if(ok!=0) {
		agb__error_translate(error, "git_graph_ahead_behind failed", ok);
		return error->error_code;
	}

	return 1;
}

