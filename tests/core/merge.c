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

void test_core_merge__initialize(void) {
	//TODO: Change this path and IDs - can use the code in
	//      utils/src/get_commit_info to get the tree ids.
	git_repository_open(&repo,"/Users/michaelanderson/Code/ANB/testDocRepos/test_branching/");

	head_tree = tree_from_oid_string("f9628f14dcb57c750c09d61034f275150eaa5985");
	branch_tree = tree_from_oid_string("851524411b2b19053a77189c19954a669f14efe6");
	base_tree = tree_from_oid_string("895811d2b7899de2597a43a5afb2768db42e1961");
}

/*
 * $ ( cd ../testDocRepos/test_branching/ ; git ls-tree branch_a )
 * 100644 blob e69de29bb2 README.txt
 * 100644 blob 6a8f9dc8fb created_in_a.txt
 * 100644 blob 0867712ece created_in_root.txt
 * 100644 blob 0867712ece created_in_root_2.txt
 *
 *
 * $ ( cd ../testDocRepos/test_branching/ ; git ls-tree branch_b )
 * 100644 blob e69de29bb2 README.txt
 * 100644 blob 21a97ca794 created_in_b.txt
 * 100644 blob 0867712ece created_in_root.txt
 * 100644 blob 0867712ece created_in_root_2.txt
 *
 *
 * $ ( cd ../testDocRepos/test_branching/ ; git ls-tree $(git merge-base branch_a branch_b) )
 * 100644 blob e69de29bb2 README.txt
 * 100644 blob 0867712ece created_in_root.txt
 * 100644 blob 0867712ece created_in_root_2.txt
*/


void test_core_merge__cleanup(void) {
	git_repository_free(repo);
}

void test_core_merge__basic_iterator(void) {
	ANBGitBridgeMergeIterator * it = create_merge_iterator(head_tree, branch_tree, base_tree, anb_gitbridge_merge_iterator_options_NONE);
	cl_assert(it!=NULL);

	// Since everything should be sorted alphabetically our first entry in the iterator should be
	// created_in_a.txt, which should only occur in the head side.
	//
	char hexid[GIT_OID_HEXSZ+1] = {0};
	
	cl_assert_equal_s("f9628f14dcb57c750c09d61034f275150eaa5985", git_oid_tostr(hexid,GIT_OID_HEXSZ+1,anb_gitbridge_merge_iterator_head_id(it)));
	cl_assert_equal_s("851524411b2b19053a77189c19954a669f14efe6", git_oid_tostr(hexid,GIT_OID_HEXSZ+1,anb_gitbridge_merge_iterator_branch_id(it)));
	cl_assert_equal_s("895811d2b7899de2597a43a5afb2768db42e1961", git_oid_tostr(hexid,GIT_OID_HEXSZ+1,anb_gitbridge_merge_iterator_base_id(it)));

	//TODO: How do we handle the case with zero entries.
	//We probably need a anb_gitbridge_merge_iterator_is_valid(it)
	//We can then use it like:
	//
	//
	cl_assert_equal_s("created_in_a.txt", anb_gitbridge_merge_iterator_entry_name(it));
	cl_assert_equal_p(NULL, anb_gitbridge_merge_iterator_head_entry_id(it));
	cl_assert_equal_s("6a8f9dc8fbbc0a9c632ff7f58b419ab09f7d49d9", git_oid_tostr(hexid,GIT_OID_HEXSZ+1,anb_gitbridge_merge_iterator_branch_entry_id(it)));
	cl_assert_equal_p(NULL, anb_gitbridge_merge_iterator_base_entry_id(it));

	cl_assert_equal_i(0, anb_gitbridge_merge_iterator_next(it) );
	cl_assert_equal_s("created_in_b.txt", anb_gitbridge_merge_iterator_entry_name(it));
	cl_assert_equal_s("21a97ca7942380e581d314d80aed559be2150219", git_oid_tostr(hexid,GIT_OID_HEXSZ+1,anb_gitbridge_merge_iterator_head_entry_id(it)));
	cl_assert_equal_p(NULL, anb_gitbridge_merge_iterator_branch_entry_id(it));
	cl_assert_equal_p(NULL, anb_gitbridge_merge_iterator_base_entry_id(it));

	cl_assert_equal_i(1, anb_gitbridge_merge_iterator_next(it) );

	anb_gitbridge_merge_iterator_free(it);
}
void test_core_merge__basic_iterator_looping(void) {
	ANBGitBridgeMergeIterator * it = create_merge_iterator(head_tree, branch_tree, base_tree, anb_gitbridge_merge_iterator_options_NONE);
	cl_assert(it!=NULL);

	const char * filenames[]={"created_in_a.txt","created_in_b.txt","SENTINEL"};
	int i=0;
	for( ; anb_gitbridge_merge_iterator_is_valid(it); anb_gitbridge_merge_iterator_next(it) ) {
		cl_assert_equal_s(filenames[i], anb_gitbridge_merge_iterator_entry_name(it));
		++i;
	}
	anb_gitbridge_merge_iterator_free(it);
}


void test_core_merge__basic_iterator_all(void) {
	ANBGitBridgeMergeIterator * it = create_merge_iterator(head_tree, branch_tree, base_tree, anb_gitbridge_merge_iterator_options_ALL_ENTRIES);
	cl_assert(it!=NULL);

	// Since everything should be sorted alphabetically our first entry in the iterator should be
	// created_in_a.txt, which should only occur in the head side.
	//
	char hexid[GIT_OID_HEXSZ+1] = {0};
	
	cl_assert_equal_s("f9628f14dcb57c750c09d61034f275150eaa5985", git_oid_tostr(hexid,GIT_OID_HEXSZ+1,anb_gitbridge_merge_iterator_head_id(it)));
	cl_assert_equal_s("851524411b2b19053a77189c19954a669f14efe6", git_oid_tostr(hexid,GIT_OID_HEXSZ+1,anb_gitbridge_merge_iterator_branch_id(it)));
	cl_assert_equal_s("895811d2b7899de2597a43a5afb2768db42e1961", git_oid_tostr(hexid,GIT_OID_HEXSZ+1,anb_gitbridge_merge_iterator_base_id(it)));

	cl_assert_equal_s("README.txt", anb_gitbridge_merge_iterator_entry_name(it));
	cl_assert_equal_s("e69de29bb2d1d6434b8b29ae775ad8c2e48c5391", git_oid_tostr(hexid,GIT_OID_HEXSZ+1,anb_gitbridge_merge_iterator_head_entry_id(it)));
	cl_assert_equal_s("e69de29bb2d1d6434b8b29ae775ad8c2e48c5391", git_oid_tostr(hexid,GIT_OID_HEXSZ+1,anb_gitbridge_merge_iterator_branch_entry_id(it)));
	cl_assert_equal_s("e69de29bb2d1d6434b8b29ae775ad8c2e48c5391", git_oid_tostr(hexid,GIT_OID_HEXSZ+1,anb_gitbridge_merge_iterator_base_entry_id(it)));

	cl_assert_equal_i(0, anb_gitbridge_merge_iterator_next(it) );
	cl_assert_equal_s("created_in_a.txt", anb_gitbridge_merge_iterator_entry_name(it));
	cl_assert_equal_p(NULL, anb_gitbridge_merge_iterator_head_entry_id(it));
	cl_assert_equal_s("6a8f9dc8fbbc0a9c632ff7f58b419ab09f7d49d9", git_oid_tostr(hexid,GIT_OID_HEXSZ+1,anb_gitbridge_merge_iterator_branch_entry_id(it)));
	cl_assert_equal_p(NULL, anb_gitbridge_merge_iterator_base_entry_id(it));

	cl_assert_equal_i(0, anb_gitbridge_merge_iterator_next(it) );
	cl_assert_equal_s("created_in_b.txt", anb_gitbridge_merge_iterator_entry_name(it));
	cl_assert_equal_s("21a97ca7942380e581d314d80aed559be2150219", git_oid_tostr(hexid,GIT_OID_HEXSZ+1,anb_gitbridge_merge_iterator_head_entry_id(it)));
	cl_assert_equal_p(NULL, anb_gitbridge_merge_iterator_branch_entry_id(it));
	cl_assert_equal_p(NULL, anb_gitbridge_merge_iterator_base_entry_id(it));

	cl_assert_equal_i(0, anb_gitbridge_merge_iterator_next(it) );
	cl_assert_equal_s("created_in_root.txt", anb_gitbridge_merge_iterator_entry_name(it));
	cl_assert_equal_s("0867712ecec02144b5bf4ee2a59deb0f42b49a53", git_oid_tostr(hexid,GIT_OID_HEXSZ+1,anb_gitbridge_merge_iterator_head_entry_id(it)));
	cl_assert_equal_s("0867712ecec02144b5bf4ee2a59deb0f42b49a53", git_oid_tostr(hexid,GIT_OID_HEXSZ+1,anb_gitbridge_merge_iterator_branch_entry_id(it)));
	cl_assert_equal_s("0867712ecec02144b5bf4ee2a59deb0f42b49a53", git_oid_tostr(hexid,GIT_OID_HEXSZ+1,anb_gitbridge_merge_iterator_base_entry_id(it)));

	cl_assert_equal_i(0, anb_gitbridge_merge_iterator_next(it) );
	cl_assert_equal_s("created_in_root_2.txt", anb_gitbridge_merge_iterator_entry_name(it));
	cl_assert_equal_s("0867712ecec02144b5bf4ee2a59deb0f42b49a53", git_oid_tostr(hexid,GIT_OID_HEXSZ+1,anb_gitbridge_merge_iterator_head_entry_id(it)));
	cl_assert_equal_s("0867712ecec02144b5bf4ee2a59deb0f42b49a53", git_oid_tostr(hexid,GIT_OID_HEXSZ+1,anb_gitbridge_merge_iterator_branch_entry_id(it)));
	cl_assert_equal_s("0867712ecec02144b5bf4ee2a59deb0f42b49a53", git_oid_tostr(hexid,GIT_OID_HEXSZ+1,anb_gitbridge_merge_iterator_base_entry_id(it)));

	cl_assert_equal_i(1, anb_gitbridge_merge_iterator_next(it) );

	anb_gitbridge_merge_iterator_free(it);
}

//TODO: Check error message
void test_core_merge__null_head(void) {

	head_tree = NULL; 

	ANBGitBridgeMergeIterator * it = create_merge_iterator(head_tree, branch_tree, base_tree, anb_gitbridge_merge_iterator_options_NONE);
	cl_assert(it==NULL);
}

//TODO: Check error message
void test_core_merge__null_branch(void) {

	branch_tree = NULL; 

	ANBGitBridgeMergeIterator * it = create_merge_iterator(head_tree, branch_tree, base_tree, anb_gitbridge_merge_iterator_options_NONE);
	cl_assert(it==NULL);
}

//TODO: Check error message
void test_core_merge__null_base(void) {

	base_tree = NULL; 

	ANBGitBridgeMergeIterator * it = create_merge_iterator(head_tree, branch_tree, base_tree, anb_gitbridge_merge_iterator_options_NONE);
	cl_assert(it==NULL);
}

