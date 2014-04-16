#include "anbgitbridge.h"
#include "anbgitbridge/internal/types.h"
#include "git2.h"

#include <memory.h>

ANBGitBridgeMergeIterator * create_merge_iterator(git_tree * head_tree, git_tree * branch_tree, git_tree * base_tree) {

	if(!head_tree) return NULL;	
	if(!branch_tree) return NULL;	
	if(!base_tree) return NULL;	

	ANBGitBridgeMergeIterator * retval = (ANBGitBridgeMergeIterator*)malloc(sizeof(ANBGitBridgeMergeIterator));
	memset(retval,0, sizeof(ANBGitBridgeMergeIterator));
	return retval;
}


//TODO: Add error handling.
int commit_oid_to_tree(git_tree ** tree, git_repository * repo, git_oid * oid ) {
	if(!repo) return 1;
	git_commit * commit;
	git_commit_lookup(&commit, repo, oid);
	const git_oid * tree_oid = git_commit_tree_id(commit);
	git_tree_lookup(tree, repo, tree_oid);
	git_commit_free(commit);
	return 0;
}

//TODO: Add error handling.
/*
 * Get the SHA of the two trees we're going to merge and create an iterator from them.
 */
ANBGitBridgeMergeIterator * merge( ANBGitBridge * anbGitBridge, ANBGitBridgeError * error ) {

	git_oid head_oid;
	git_oid branch_oid;
	git_oid base_oid;
	git_tree * head_tree = NULL;
	git_tree * branch_tree = NULL;
	git_tree * base_tree = NULL;

	/* TODO: FIX THIS */
	const char * branch_name = "refs/heads/branch_a";
	git_repository * repo = anbGitBridge->repository;
	if(!repo) return NULL;
	int ok;


	git_reference_name_to_id(&head_oid, repo, "HEAD");
	git_reference_name_to_id(&branch_oid, repo, branch_name);

	ok = git_merge_base(&base_oid, repo, &head_oid, &branch_oid);

	commit_oid_to_tree(&head_tree, repo, &head_oid);
	commit_oid_to_tree(&branch_tree, repo, &branch_oid);
	commit_oid_to_tree(&base_tree, repo, &base_oid);


	return create_merge_iterator(head_tree, branch_tree, base_tree);
}	
