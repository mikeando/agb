#include "agb/status.h"
#include "agb/internal/types.h"

#include <string.h>

//TODO: Get rid of this include
#include <assert.h>

//TODO: Get rid of this include
#include <stdio.h>

struct AGBStatus {
	git_status_list *status;
};

int agb_get_status_new(AGBStatus ** status, AGBCore * repo ) {
	assert(repo);
	assert(repo->repository);
	*status = (AGBStatus*) malloc(sizeof(AGBStatus));
	(*status)->status = NULL;

	git_status_options statusopt;
	git_status_init_options(&statusopt, GIT_STATUS_OPTIONS_VERSION);
	statusopt.show  = GIT_STATUS_SHOW_INDEX_AND_WORKDIR;
	statusopt.flags = GIT_STATUS_OPT_INCLUDE_UNTRACKED |
		GIT_STATUS_OPT_RENAMES_HEAD_TO_INDEX |
		GIT_STATUS_OPT_SORT_CASE_SENSITIVELY;


	//TODO: Handle errors.
	int ok = git_status_list_new(&((*status)->status), repo->repository, &statusopt);
	if(ok!=0) {
		//TODO: Better error handling
		printf("ERROR %s\n", giterr_last()->message);
		return ok;
	}
	return 0;
}

int agb_status_for_filename(const char * filename, AGBStatus * status) {

	size_t  nStatusEntries = git_status_list_entrycount(status->status);
	for(size_t i=0; i<nStatusEntries; ++i) {
		const git_status_entry * entry = git_status_byindex(status->status, i);


		//Does this diff touch the required file
		int thisEntry = 0;
		if( entry->head_to_index ) {
			{
				const char * eName = entry->head_to_index->old_file.path;
				if(eName && strcmp(eName,filename)==0) {
					thisEntry = 1;
				}
			}
			{
				const char * eName = entry->head_to_index->new_file.path;
				if(eName && strcmp(eName,filename)==0) {
					thisEntry = 1;
				}
			}
		}
		if( entry->index_to_workdir ) {
			{
				const char * eName = entry->index_to_workdir->old_file.path;
				if(eName && strcmp(eName,filename)==0) {
					thisEntry = 1;
				}
			}
			{
				const char * eName = entry->index_to_workdir->new_file.path;
				if(eName && strcmp(eName,filename)==0) {
					thisEntry = 1;
				}
			}
		}
		if(thisEntry==0)
			continue;
		return entry->status;
	}

	return -1;
}

