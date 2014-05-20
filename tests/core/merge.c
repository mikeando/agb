#include "agb.h"
#include "clar/clar.h"
#include "utils/test_utils.h"
#include "utils/clarx.h"
#include <stdio.h>

#include "agb/internal/eh.h"

static const git_commit * branch_commit = NULL;
static const git_commit * head_commit = NULL;
static const git_commit * base_commit = NULL;

static const git_tree * head_tree = NULL; 
static const git_tree * branch_tree = NULL; 
static const git_tree * base_tree = NULL;
static git_repository * repo = NULL;

#define REPODIR "/Users/michaelanderson/Code/ANB/testDocRepos/test_branching/"

//TODO: I think this leaks...
const git_commit * commit_from_ref(const char * refname) {
	cl_assert(repo);
	git_reference * ref;
	git_reference_dwim(&ref, repo, refname);
	git_commit * retval;
	git_reference_peel( (git_object**)&retval, ref, GIT_OBJ_COMMIT);
	return retval;
}

void test_core_merge__initialize(void) {
	//TODO: Change this path and IDs - can use the code in
	//      utils/src/get_commit_info to get the tree ids.
	git_repository_open(&repo, REPODIR);

	head_commit = commit_from_ref("branch_a");
	branch_commit = commit_from_ref("branch_b");
	base_commit = commit_from_ref("master");

	git_commit_tree((git_tree**)&head_tree, head_commit);
	git_commit_tree((git_tree**)&branch_tree, branch_commit);
	git_commit_tree((git_tree**)&base_tree, base_commit);
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

	// Validate the repo... (We were/are getting bad git fsck output and corrupted directories etc .. which is v. bad.)
	//
	/*
	int old_g_debug = g_debug;
	g_debug=1;
	int ok = system_fmt("( cd %s ; git fsck --full --strict )", REPODIR);
	g_debug = old_g_debug;
	printf("git fsck returned %d\n",ok);
	*/
}

void test_core_merge__basic_iterator(void) {
	AGBMergeIterator * it = agb_merge__create_iterator(head_tree, branch_tree, base_tree, agb_merge_iterator_options_NONE);
	cl_assert(it!=NULL);

	// Since everything should be sorted alphabetically our first entry in the iterator should be
	// created_in_a.txt, which should only occur in the head side.
	//
	char hexid[GIT_OID_HEXSZ+1] = {0};
	char hexid2[GIT_OID_HEXSZ+1] = {0};
	
	cl_assert_equal_s(git_oid_tostr(hexid2, GIT_OID_HEXSZ+1, git_tree_id(head_tree)), git_oid_tostr(hexid,GIT_OID_HEXSZ+1,agb_merge_iterator_tree_id(it,0)));
	cl_assert_equal_s(git_oid_tostr(hexid2, GIT_OID_HEXSZ+1, git_tree_id(branch_tree)), git_oid_tostr(hexid,GIT_OID_HEXSZ+1,agb_merge_iterator_tree_id(it,1)));
	cl_assert_equal_s(git_oid_tostr(hexid2, GIT_OID_HEXSZ+1, git_tree_id(base_tree)), git_oid_tostr(hexid,GIT_OID_HEXSZ+1,agb_merge_iterator_tree_id(it,2)));

	//TODO: How do we handle the case with zero entries.
	//We probably need a agb_merge_iterator_is_valid(it)
	//We can then use it like:
	//
	//
	cl_assert_equal_s("created_in_a.txt", agb_merge_iterator_entry_name(it));
	cl_assert_equal_s("6a8f9dc8fbbc0a9c632ff7f58b419ab09f7d49d9", git_oid_tostr(hexid,GIT_OID_HEXSZ+1,agb_merge_iterator_entry_id(it,0)));
	cl_assert_equal_p(NULL, agb_merge_iterator_entry_id(it,1));
	cl_assert_equal_p(NULL, agb_merge_iterator_entry_id(it,2));

	cl_assert_equal_i(0, agb_merge_iterator_next(it) );
	cl_assert_equal_s("created_in_b.txt", agb_merge_iterator_entry_name(it));
	cl_assert_equal_p(NULL, agb_merge_iterator_entry_id(it,0));
	cl_assert_equal_s("21a97ca7942380e581d314d80aed559be2150219", git_oid_tostr(hexid,GIT_OID_HEXSZ+1,agb_merge_iterator_entry_id(it,1)));
	cl_assert_equal_p(NULL, agb_merge_iterator_entry_id(it,2));

	cl_assert_equal_i(1, agb_merge_iterator_next(it) );

	agb_merge_iterator_free(it);
}
void test_core_merge__basic_iterator_looping(void) {
	AGBMergeIterator * it = agb_merge__create_iterator(head_tree, branch_tree, base_tree, agb_merge_iterator_options_NONE);
	cl_assert(it!=NULL);

	const char * filenames[]={"created_in_a.txt","created_in_b.txt","SENTINEL"};
	int i=0;
	for( ; agb_merge_iterator_is_valid(it); agb_merge_iterator_next(it) ) {
		cl_assert_equal_s(filenames[i], agb_merge_iterator_entry_name(it));
		++i;
	}
	agb_merge_iterator_free(it);
}


void test_core_merge__basic_iterator_all(void) {
	AGBMergeIterator * it = agb_merge__create_iterator(head_tree, branch_tree, base_tree, agb_merge_iterator_options_ALL_ENTRIES);
	cl_assert(it!=NULL);

	// Since everything should be sorted alphabetically our first entry in the iterator should be
	// created_in_a.txt, which should only occur in the head side.
	//
	char hexid[GIT_OID_HEXSZ+1] = {0};
	char hexid2[GIT_OID_HEXSZ+1] = {0};
	
	cl_assert_equal_s(git_oid_tostr(hexid2, GIT_OID_HEXSZ+1, git_tree_id(head_tree)), git_oid_tostr(hexid,GIT_OID_HEXSZ+1,agb_merge_iterator_tree_id(it,0)));
	cl_assert_equal_s(git_oid_tostr(hexid2, GIT_OID_HEXSZ+1, git_tree_id(branch_tree)), git_oid_tostr(hexid,GIT_OID_HEXSZ+1,agb_merge_iterator_tree_id(it,1)));
	cl_assert_equal_s(git_oid_tostr(hexid2, GIT_OID_HEXSZ+1, git_tree_id(base_tree)), git_oid_tostr(hexid,GIT_OID_HEXSZ+1,agb_merge_iterator_tree_id(it,2)));

	cl_assert_equal_s("README.txt", agb_merge_iterator_entry_name(it));
	cl_assert_equal_s("e69de29bb2d1d6434b8b29ae775ad8c2e48c5391", git_oid_tostr(hexid,GIT_OID_HEXSZ+1,agb_merge_iterator_entry_id(it,0)));
	cl_assert_equal_s("e69de29bb2d1d6434b8b29ae775ad8c2e48c5391", git_oid_tostr(hexid,GIT_OID_HEXSZ+1,agb_merge_iterator_entry_id(it,1)));
	cl_assert_equal_s("e69de29bb2d1d6434b8b29ae775ad8c2e48c5391", git_oid_tostr(hexid,GIT_OID_HEXSZ+1,agb_merge_iterator_entry_id(it,2)));

	cl_assert_equal_i(0, agb_merge_iterator_next(it) );
	cl_assert_equal_s("created_in_a.txt", agb_merge_iterator_entry_name(it));
	cl_assert_equal_s("6a8f9dc8fbbc0a9c632ff7f58b419ab09f7d49d9", git_oid_tostr(hexid,GIT_OID_HEXSZ+1,agb_merge_iterator_entry_id(it,0)));
	cl_assert_equal_p(NULL, agb_merge_iterator_entry_id(it,1));
	cl_assert_equal_p(NULL, agb_merge_iterator_entry_id(it,2));

	cl_assert_equal_i(0, agb_merge_iterator_next(it) );
	cl_assert_equal_s("created_in_b.txt", agb_merge_iterator_entry_name(it));
	cl_assert_equal_p(NULL, agb_merge_iterator_entry_id(it,0));
	cl_assert_equal_s("21a97ca7942380e581d314d80aed559be2150219", git_oid_tostr(hexid,GIT_OID_HEXSZ+1,agb_merge_iterator_entry_id(it,1)));
	cl_assert_equal_p(NULL, agb_merge_iterator_entry_id(it,2));

	cl_assert_equal_i(0, agb_merge_iterator_next(it) );
	cl_assert_equal_s("created_in_root.txt", agb_merge_iterator_entry_name(it));
	cl_assert_equal_s("0867712ecec02144b5bf4ee2a59deb0f42b49a53", git_oid_tostr(hexid,GIT_OID_HEXSZ+1,agb_merge_iterator_entry_id(it,0)));
	cl_assert_equal_s("0867712ecec02144b5bf4ee2a59deb0f42b49a53", git_oid_tostr(hexid,GIT_OID_HEXSZ+1,agb_merge_iterator_entry_id(it,1)));
	cl_assert_equal_s("0867712ecec02144b5bf4ee2a59deb0f42b49a53", git_oid_tostr(hexid,GIT_OID_HEXSZ+1,agb_merge_iterator_entry_id(it,2)));

	cl_assert_equal_i(0, agb_merge_iterator_next(it) );
	cl_assert_equal_s("created_in_root_2.txt", agb_merge_iterator_entry_name(it));
	cl_assert_equal_s("0867712ecec02144b5bf4ee2a59deb0f42b49a53", git_oid_tostr(hexid,GIT_OID_HEXSZ+1,agb_merge_iterator_entry_id(it,0)));
	cl_assert_equal_s("0867712ecec02144b5bf4ee2a59deb0f42b49a53", git_oid_tostr(hexid,GIT_OID_HEXSZ+1,agb_merge_iterator_entry_id(it,1)));
	cl_assert_equal_s("0867712ecec02144b5bf4ee2a59deb0f42b49a53", git_oid_tostr(hexid,GIT_OID_HEXSZ+1,agb_merge_iterator_entry_id(it,2)));

	cl_assert_equal_i(1, agb_merge_iterator_next(it) );

	agb_merge_iterator_free(it);
}

//TODO: Check error message
void test_core_merge__null_head(void) {

	head_tree = NULL; 

	AGBMergeIterator * it = agb_merge__create_iterator(head_tree, branch_tree, base_tree, agb_merge_iterator_options_NONE);
	cl_assert(it==NULL);
}

//TODO: Check error message
void test_core_merge__null_branch(void) {

	branch_tree = NULL; 

	AGBMergeIterator * it = agb_merge__create_iterator(head_tree, branch_tree, base_tree, agb_merge_iterator_options_NONE);
	cl_assert(it==NULL);
}

//TODO: Check error message
void test_core_merge__null_base(void) {

	base_tree = NULL; 

	AGBMergeIterator * it = agb_merge__create_iterator(head_tree, branch_tree, base_tree, agb_merge_iterator_options_NONE);
	cl_assert(it==NULL);
}

//A NULL safe git_oid_equal
//TODO: Copied from merge.c - probably could make this common
static inline int agb_git_oid_equal(const git_oid * oid_a, const git_oid * oid_b ) {
	if(oid_a==NULL && oid_b==NULL) return 1;
	if(oid_a==NULL || oid_b==NULL) return 0;
	return git_oid_equal(oid_a,oid_b);
}

void test_core_merge__demo_create_merge_commit(void) {

	AGBError * error;
	agb_error_new(&error);


	// Here we're going to try to use the merge iterator to create a merge commit that contains the right information.
	// Note - this version does not recurse into directories. (subtrees)
	git_treebuilder * builder;
	git_treebuilder_create(&builder, base_tree);
	AGBMergeIterator * it = agb_merge__create_iterator(head_tree, branch_tree, base_tree, agb_merge_iterator_options_NONE);

	for( ; agb_merge_iterator_is_valid(it) ; agb_merge_iterator_next(it) ) {
		int hasHeadChanged = !agb_git_oid_equal( agb_merge_iterator_entry_id(it,2), agb_merge_iterator_entry_id(it,0) );
		int hasBranchChanged = !agb_git_oid_equal( agb_merge_iterator_entry_id(it,2), agb_merge_iterator_entry_id(it,1) );
		printf("-- %s %d %d\n", agb_merge_iterator_entry_name(it), hasHeadChanged, hasBranchChanged);

		if( !hasHeadChanged && !hasBranchChanged ) {
			continue;
		}

		if( hasHeadChanged && !hasBranchChanged ) { 
			//We want the head version.
			if(agb_merge_iterator_entry_id(it,0) == NULL) {
				//TODO: Check error message.
				printf("REMOVING %s from tree\n", agb_merge_iterator_entry_name(it) );
				git_treebuilder_remove(builder, agb_merge_iterator_entry_name(it) );
				continue;
			}

			//TODO: Check for error
			printf("ADDING OR UPDATING %s to tree\n", agb_merge_iterator_entry_name(it) );
			git_treebuilder_insert(NULL, builder, agb_merge_iterator_entry_name(it), agb_merge_iterator_entry_id(it,0), agb_merge_iterator_entry_filemode(it,0) );
			continue;
		}

		if( !hasHeadChanged && hasBranchChanged ) { 
			//We want the head version.
			if(agb_merge_iterator_entry_id(it,1) == NULL) {
				//TODO: Check error message.
				printf("REMOVING %s from tree\n", agb_merge_iterator_entry_name(it) );
				git_treebuilder_remove(builder, agb_merge_iterator_entry_name(it) );
				continue;
			}

			//TODO: Check for error
			printf("ADDING OR UPDATING %s to tree\n", agb_merge_iterator_entry_name(it) );
			git_treebuilder_insert(NULL, builder, agb_merge_iterator_entry_name(it), agb_merge_iterator_entry_id(it,1), agb_merge_iterator_entry_filemode(it,1) );
			continue;
		}

			printf("CONFLICT %s in tree\n", agb_merge_iterator_entry_name(it) );
		//TODO: CONFLICT - Handle it!

	}

	// Our tree builder should now be full...
	// Lets write it out to a tree
	//
	//TODO: Check for errors
	git_oid new_tree_oid = {};
	git_treebuilder_write(&new_tree_oid, repo, builder);

	char hexid[GIT_OID_HEXSZ+1];
	printf("Tree SHA is %s\n", git_oid_tostr(hexid,GIT_OID_HEXSZ+1, &new_tree_oid));

	git_tree * new_tree = NULL;
	git_tree_lookup(&new_tree, repo, &new_tree_oid);

	// Now we need to create the commit.
    const git_commit** parents = (const git_commit**)malloc(sizeof(git_commit*)*2);

	parents[0] = head_commit;
	parents[1] = branch_commit;



		
	
	git_signature * author_signature = NULL;

	// Time since epoch
	// TODO: Get these correctly - 
	// Could use git_signature_now instead...
	{	
		git_time_t author_time = 1396222461ull;
		int timezone_offset = 0;
		int ok;

		if((ok=git_signature_new(&author_signature,"Someone","someone@somewhere.com", author_time, timezone_offset))!=0) {
			agb__error_translate(error,"git_signature_new failed",ok);
			goto cleanup_error;
		}
	}



	git_oid commit_id;
	int ok = git_commit_create(
				&commit_id, 
				repo,
				"refs/heads/branch_c",
				author_signature,
				author_signature,
				"UTF-8",
				"An exciting commit",
				new_tree,
			       	2, //Two parents
				parents
				);
	if(ok!=0) {
		agb__error_translate(error,"git_commit_create failed",ok);
		goto cleanup_error;
	}
	
	git_signature_free(author_signature);

	// Then update the refs. 

	cl_fail("IMPLEMENT ME!");

cleanup_error:

	printf("ERROR: %s\n",error->message);

	if(author_signature) {
		git_signature_free(author_signature);
	}
	

}

