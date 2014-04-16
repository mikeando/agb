#include "anbgitbridge.h"
#include "clar/clar.h"
#include "utils/test_utils.h"
#include "utils/clarx.h"
#include <stdio.h>


git_tree * head_tree = NULL; 
git_tree * branch_tree = NULL; 
git_tree * base_tree = NULL;
git_repository * repo = NULL;

//TODO: Add error handling!
//TODO: Get repo from somewhere!
git_tree * tree_from_oid_string(const char * oidstr) {
	cl_assert(repo);
	git_oid oid;
	git_oid_fromstr(&oid,oidstr);
	git_tree * tree;
	git_tree_lookup(&tree, repo, &oid);
	return tree;
}

//TODO: Use real tree IDs
void test_core_merge__initialize(void) {
	//TODO: Change this path.
	git_repository_open(&repo,"/Users/michaelanderson/Code/ANB/AuthorsNotebook/");

	head_tree = tree_from_oid_string("012345");
	branch_tree = tree_from_oid_string("012345");
	base_tree = tree_from_oid_string("012345");
}


void test_core_merge__cleanup(void) {
	git_repository_free(repo);
}

void test_core_merge__basic_iterator(void) {
	ANBGitBridgeMergeIterator * it = create_merge_iterator(head_tree, branch_tree, base_tree);
	cl_assert(it!=NULL);
	anb_gitbridge_merge_iterator_free(it);
}

//TODO: Check error message
void test_core_merge__null_head(void) {

	head_tree = NULL; 

	ANBGitBridgeMergeIterator * it = create_merge_iterator(head_tree, branch_tree, base_tree);
	cl_assert(it==NULL);
}

//TODO: Check error message
void test_core_merge__null_branch(void) {

	branch_tree = NULL; 

	ANBGitBridgeMergeIterator * it = create_merge_iterator(head_tree, branch_tree, base_tree);
	cl_assert(it==NULL);
}

//TODO: Check error message
void test_core_merge__null_base(void) {

	base_tree = NULL; 

	ANBGitBridgeMergeIterator * it = create_merge_iterator(head_tree, branch_tree, base_tree);
	cl_assert(it==NULL);
}

