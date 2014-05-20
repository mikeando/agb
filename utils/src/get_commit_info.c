#include <stdio.h>
#include <setjmp.h>
#include "git2.h"


static jmp_buf eh;

static void handle_git_error( int ok ) {
	if(ok==0) return;
	printf("Error encountered.... %d : %s\n",ok, giterr_last()->message);
	longjmp(eh,ok);
}

int main(int argc, const char** argv) {

	if(argc!=3) {
		printf("Usage: %s path ref\n", argv[0]);
		return 1;
	}


	git_repository * repo = NULL;
	git_object * obj = NULL; 
	git_object * peeled_obj = NULL; 

	int ok;
	if((ok=setjmp(eh))!=0)
		goto handle_error;

	handle_git_error( git_repository_open_ext(&repo, argv[1], 0, NULL) );


	printf("REPOSITORY INFO:\n");
	printf("  Repo path = %s\n", git_repository_path(repo));
	printf("  Repo workdir = %s\n", git_repository_workdir(repo));
	handle_git_error( git_revparse_single(&obj, repo, argv[2]) );
	handle_git_error( git_object_peel(&peeled_obj,obj,GIT_OBJ_COMMIT) );

	git_commit * commit = (git_commit*)peeled_obj;

	printf("COMMIT INFO:\n");
	char hexid[GIT_OID_HEXSZ+1] = {0};

	printf("  Commit id   : %s\n", git_oid_tostr(hexid, GIT_OID_HEXSZ+1, git_commit_id(commit)));
	printf("  Commit tree : %s\n", git_oid_tostr(hexid, GIT_OID_HEXSZ+1, git_commit_tree_id(commit)));



	return 0;

handle_error:

	printf("Got me an error...\n");
	return 1;
}
