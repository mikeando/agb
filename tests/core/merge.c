#include "agb.h"
#include "clar/clar.h"
#include "utils/test_utils.h"
#include "utils/clarx.h"
#include <stdio.h>
#include <git2/errors.h>

#include "agb/internal/eh.h"
#include "external/popen3.h"

static const git_commit * branch_commit = NULL;
static const git_commit * head_commit = NULL;
static const git_commit * base_commit = NULL;

static const git_tree * head_tree = NULL;
static const git_tree * branch_tree = NULL;
static const git_tree * base_tree = NULL;
static git_repository * repo = NULL;

#define REPODIR "/Users/michaelanderson/Code/ANB/testDocRepos/test_branching/"

//TODO: This is derived from test_utils.c status_in_commit

char test_core_merge__compare_with_parents_merge_base(const char * commit_id, const char * filename) {
	char * cmd;
	int readfd;
	asprintf(&cmd,"(cd " REPODIR " ; git diff --name-status $(git merge-base $(git show --pretty=format:%%p %s))..%s | ( grep '\\<%s\\>' || true ) )", commit_id, commit_id, filename);
	if(g_debug) printf("repo_contains: running %s\n", cmd);
	pid_t pid = popen3(cmd, NULL, &readfd, NULL);


	char result=0;
	ssize_t bytes_read = read(readfd,&result,1);

	int status=0;
	waitpid(pid,&status,0);
	free(cmd);

	if(!WIFEXITED(status) || WEXITSTATUS(status)!=0 ) {
		cl_fail("git diff failed");
	}

	if( bytes_read==0 )
		return 0;
	return result;
}


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
	base_commit = commit_from_ref("a_b_merge_base");

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

    AGBMergeEntry * mergeEntry = agb_merge_entry_from_iterator(it);

	cl_assert_equal_s(git_oid_tostr(hexid2, GIT_OID_HEXSZ+1, git_tree_id(head_tree)), git_oid_tostr(hexid,GIT_OID_HEXSZ+1,agb_merge_iterator_tree_id(it,AGB_MERGE_LOCAL)));
	cl_assert_equal_s(git_oid_tostr(hexid2, GIT_OID_HEXSZ+1, git_tree_id(branch_tree)), git_oid_tostr(hexid,GIT_OID_HEXSZ+1,agb_merge_iterator_tree_id(it,AGB_MERGE_REMOTE)));
	cl_assert_equal_s(git_oid_tostr(hexid2, GIT_OID_HEXSZ+1, git_tree_id(base_tree)), git_oid_tostr(hexid,GIT_OID_HEXSZ+1,agb_merge_iterator_tree_id(it,AGB_MERGE_BASE)));

	//TODO: How do we handle the case with zero entries.
	//We probably need a agb_merge_iterator_is_valid(it)
	//We can then use it like:
	//
	//
	cl_assert_equal_s("created_in_a.txt", agb_merge_entry_name(mergeEntry));
	cl_assert_equal_s("6a8f9dc8fbbc0a9c632ff7f58b419ab09f7d49d9", git_oid_tostr(hexid,GIT_OID_HEXSZ+1,agb_merge_entry_id(mergeEntry,AGB_MERGE_LOCAL)));
	cl_assert_equal_p(NULL, agb_merge_entry_id(mergeEntry,AGB_MERGE_REMOTE));
	cl_assert_equal_p(NULL, agb_merge_entry_id(mergeEntry,AGB_MERGE_BASE));

	cl_assert_equal_i(0, agb_merge_iterator_next(it) );
    mergeEntry = agb_merge_entry_from_iterator(it);

    cl_assert_equal_s("created_in_b.txt", agb_merge_entry_name(mergeEntry));
	cl_assert_equal_p(NULL, agb_merge_entry_id(mergeEntry,AGB_MERGE_LOCAL));
	cl_assert_equal_s("21a97ca7942380e581d314d80aed559be2150219", git_oid_tostr(hexid,GIT_OID_HEXSZ+1,agb_merge_entry_id(mergeEntry,AGB_MERGE_REMOTE)));
	cl_assert_equal_p(NULL, agb_merge_entry_id(mergeEntry,AGB_MERGE_BASE));

	cl_assert_equal_i(1, agb_merge_iterator_next(it) );

	agb_merge_iterator_free(it);
}
void test_core_merge__basic_iterator_looping(void) {
	AGBMergeIterator * it = agb_merge__create_iterator(head_tree, branch_tree, base_tree, agb_merge_iterator_options_NONE);
	cl_assert(it!=NULL);

	const char * filenames[]={"created_in_a.txt","created_in_b.txt","SENTINEL"};
	int i=0;
	for( ; agb_merge_iterator_is_valid(it); agb_merge_iterator_next(it) ) {
        AGBMergeEntry * mergeEntry = agb_merge_entry_from_iterator(it);
        cl_assert_equal_s(filenames[i], agb_merge_entry_name(mergeEntry));
		++i;
	}
	agb_merge_iterator_free(it);
}


void test_core_merge__basic_iterator_all(void) {
	AGBMergeIterator * it = agb_merge__create_iterator(head_tree, branch_tree, base_tree, agb_merge_iterator_options_ALL_ENTRIES);
	cl_assert(it!=NULL);
    AGBMergeEntry * mergeEntry = agb_merge_entry_from_iterator(it);


    // Since everything should be sorted alphabetically our first entry in the iterator should be
	// created_in_a.txt, which should only occur in the head side.
	//
	char hexid[GIT_OID_HEXSZ+1] = {0};
	char hexid2[GIT_OID_HEXSZ+1] = {0};

	cl_assert_equal_s(git_oid_tostr(hexid2, GIT_OID_HEXSZ+1, git_tree_id(head_tree)), git_oid_tostr(hexid,GIT_OID_HEXSZ+1,agb_merge_iterator_tree_id(it,AGB_MERGE_LOCAL)));
	cl_assert_equal_s(git_oid_tostr(hexid2, GIT_OID_HEXSZ+1, git_tree_id(branch_tree)), git_oid_tostr(hexid,GIT_OID_HEXSZ+1,agb_merge_iterator_tree_id(it,AGB_MERGE_REMOTE)));
	cl_assert_equal_s(git_oid_tostr(hexid2, GIT_OID_HEXSZ+1, git_tree_id(base_tree)), git_oid_tostr(hexid,GIT_OID_HEXSZ+1,agb_merge_iterator_tree_id(it,AGB_MERGE_BASE)));

	cl_assert_equal_s("README.txt", agb_merge_entry_name(mergeEntry));
	cl_assert_equal_s("e69de29bb2d1d6434b8b29ae775ad8c2e48c5391", git_oid_tostr(hexid,GIT_OID_HEXSZ+1,agb_merge_entry_id(mergeEntry,AGB_MERGE_LOCAL)));
	cl_assert_equal_s("e69de29bb2d1d6434b8b29ae775ad8c2e48c5391", git_oid_tostr(hexid,GIT_OID_HEXSZ+1,agb_merge_entry_id(mergeEntry,AGB_MERGE_REMOTE)));
	cl_assert_equal_s("e69de29bb2d1d6434b8b29ae775ad8c2e48c5391", git_oid_tostr(hexid,GIT_OID_HEXSZ+1,agb_merge_entry_id(mergeEntry,AGB_MERGE_BASE)));

	cl_assert_equal_i(0, agb_merge_iterator_next(it) );
    mergeEntry = agb_merge_entry_from_iterator(it);
    cl_assert_equal_s("created_in_a.txt", agb_merge_entry_name(mergeEntry));
	cl_assert_equal_s("6a8f9dc8fbbc0a9c632ff7f58b419ab09f7d49d9", git_oid_tostr(hexid,GIT_OID_HEXSZ+1,agb_merge_entry_id(mergeEntry,AGB_MERGE_LOCAL)));
	cl_assert_equal_p(NULL, agb_merge_entry_id(mergeEntry,AGB_MERGE_REMOTE));
	cl_assert_equal_p(NULL, agb_merge_entry_id(mergeEntry,AGB_MERGE_BASE));

	cl_assert_equal_i(0, agb_merge_iterator_next(it) );
    mergeEntry = agb_merge_entry_from_iterator(it);

    cl_assert_equal_s("created_in_b.txt", agb_merge_entry_name(mergeEntry));
	cl_assert_equal_p(NULL, agb_merge_entry_id(mergeEntry,AGB_MERGE_LOCAL));
	cl_assert_equal_s("21a97ca7942380e581d314d80aed559be2150219", git_oid_tostr(hexid,GIT_OID_HEXSZ+1,agb_merge_entry_id(mergeEntry,AGB_MERGE_REMOTE)));
	cl_assert_equal_p(NULL, agb_merge_entry_id(mergeEntry,AGB_MERGE_BASE));

	cl_assert_equal_i(0, agb_merge_iterator_next(it) );
    mergeEntry = agb_merge_entry_from_iterator(it);
    cl_assert_equal_s("created_in_root.txt", agb_merge_entry_name(mergeEntry));
	cl_assert_equal_s("0867712ecec02144b5bf4ee2a59deb0f42b49a53", git_oid_tostr(hexid,GIT_OID_HEXSZ+1,agb_merge_entry_id(mergeEntry,AGB_MERGE_LOCAL)));
	cl_assert_equal_s("0867712ecec02144b5bf4ee2a59deb0f42b49a53", git_oid_tostr(hexid,GIT_OID_HEXSZ+1,agb_merge_entry_id(mergeEntry,AGB_MERGE_REMOTE)));
	cl_assert_equal_s("0867712ecec02144b5bf4ee2a59deb0f42b49a53", git_oid_tostr(hexid,GIT_OID_HEXSZ+1,agb_merge_entry_id(mergeEntry,AGB_MERGE_BASE)));

	cl_assert_equal_i(0, agb_merge_iterator_next(it) );
    mergeEntry = agb_merge_entry_from_iterator(it);
    cl_assert_equal_s("created_in_root_2.txt", agb_merge_entry_name(mergeEntry));
	cl_assert_equal_s("0867712ecec02144b5bf4ee2a59deb0f42b49a53", git_oid_tostr(hexid,GIT_OID_HEXSZ+1,agb_merge_entry_id(mergeEntry,AGB_MERGE_LOCAL)));
	cl_assert_equal_s("0867712ecec02144b5bf4ee2a59deb0f42b49a53", git_oid_tostr(hexid,GIT_OID_HEXSZ+1,agb_merge_entry_id(mergeEntry,AGB_MERGE_REMOTE)));
	cl_assert_equal_s("0867712ecec02144b5bf4ee2a59deb0f42b49a53", git_oid_tostr(hexid,GIT_OID_HEXSZ+1,agb_merge_entry_id(mergeEntry,AGB_MERGE_BASE)));

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




void test_core_merge__demo_create_merge_commit(void) {

	AGBError * error;
	agb_error_new(&error);


	// Here we're going to try to use the merge iterator to create a merge commit that contains the right information.
	// Note - this version does not recurse into directories. (subtrees)
	git_treebuilder * builder;
	git_treebuilder_create(&builder, base_tree);
	AGBMergeIterator * it = agb_merge__create_iterator(head_tree, branch_tree, base_tree, agb_merge_iterator_options_NONE);

	for( ; agb_merge_iterator_is_valid(it) ; agb_merge_iterator_next(it) ) {
        AGBMergeEntry * entry = agb_merge_entry_from_iterator(it);
		int hasLocalChanged = !agb_git_oid_equal( agb_merge_entry_id(entry,AGB_MERGE_BASE), agb_merge_entry_id(entry,AGB_MERGE_LOCAL) );
		int hasRemoteChanged = !agb_git_oid_equal( agb_merge_entry_id(entry,AGB_MERGE_BASE), agb_merge_entry_id(entry,AGB_MERGE_REMOTE) );
		printf("-- %s %d %d\n", agb_merge_entry_name(entry), hasLocalChanged, hasRemoteChanged);

		if( !hasLocalChanged && !hasRemoteChanged) {
			continue;
		}

		if( hasLocalChanged && !hasRemoteChanged) {
			//We want the head version.
			if(agb_merge_entry_id(entry,AGB_MERGE_LOCAL) == NULL) {
				//TODO: Check error message.
				printf("REMOVING %s from tree\n", agb_merge_entry_name(entry) );
				git_treebuilder_remove(builder, agb_merge_entry_name(entry) );
				continue;
			}

			//TODO: Check for error
			printf("ADDING OR UPDATING %s to tree\n", agb_merge_entry_name(entry) );
			int ok = git_treebuilder_insert(NULL, builder, agb_merge_entry_name(entry), agb_merge_entry_id(entry,AGB_MERGE_LOCAL), agb_merge_entry_filemode(entry,AGB_MERGE_LOCAL) );
            if(ok!=0) {
                printf("Error duting add/update of tree builder: %s\n", giterr_last()->message);
                abort();
            }
            continue;
		}

		if( !hasLocalChanged && hasRemoteChanged) {
			//We want the head version.
			if(agb_merge_entry_id(entry,AGB_MERGE_REMOTE) == NULL) {
				//TODO: Check error message.
				printf("REMOVING %s from tree\n", agb_merge_entry_name(entry) );
				git_treebuilder_remove(builder, agb_merge_entry_name(entry) );
				continue;
			}

			//TODO: Check for error
			printf("ADDING OR UPDATING %s to tree\n", agb_merge_entry_name(entry) );
			int ok = git_treebuilder_insert(NULL, builder, agb_merge_entry_name(entry), agb_merge_entry_id(entry,AGB_MERGE_REMOTE), agb_merge_entry_filemode(entry,AGB_MERGE_REMOTE) );
			if(ok!=0) {
                printf("Error duting add/update of tree builder: %s\n", giterr_last()->message);
                abort();
            }
            continue;
		}

			printf("CONFLICT %s in tree\n", agb_merge_entry_name(entry) );
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

		git_time_t author_time = time(NULL);
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
				NULL,
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

	// Then update the refs.
	//TODO: Do we need to release this ref?
	git_reference * ref;
	ok = git_reference_create_matching(
			&ref,
			repo,
		   	"refs/heads/branch_c",
			&commit_id,
			1,
			NULL,
			author_signature,
			"merged by libagb");

	if(ok!=0) {
		agb__error_translate(error,"git_reference_create failed",ok);
		goto cleanup_error;
	}

	git_signature_free(author_signature);

	// Now check we got the expected files
	cl_assert_equal_c('A', test_core_merge__compare_with_parents_merge_base("branch_c", "created_in_a.txt"));
	cl_assert_equal_c('A', test_core_merge__compare_with_parents_merge_base("branch_c", "created_in_b.txt"));

	return;

cleanup_error:

	printf("ERROR: %s\n",error->message);

	if(author_signature) {
		git_signature_free(author_signature);
	}
	cl_fail(error->message);


}

static const char * index_to_string(enum AGBMergeIndex idx) {
    if(idx==AGB_MERGE_BASE) { return "merge_base"; }
    if(idx==AGB_MERGE_LOCAL) { return "local"; }
    if(idx==AGB_MERGE_REMOTE) { return "remote"; }
    return "invalid";
}

static int merge_conflict(AGBMerger * self, AGBMergeEntry * entry) {
	printf("Got a conflict : self = %p, entry = %p\n", self, entry);
  return 0;
}

static int merge_every(AGBMerger * self, AGBMergeEntry * entry) {
    const git_oid * base_id = agb_merge_entry_id(entry,AGB_MERGE_BASE);
    const git_oid * local_id = agb_merge_entry_id(entry,AGB_MERGE_LOCAL);
    const git_oid * remote_id = agb_merge_entry_id(entry,AGB_MERGE_REMOTE);
    int hasLocalChanged = !agb_git_oid_equal(base_id, local_id );
    int hasRemoteChanged = !agb_git_oid_equal(base_id, remote_id );
    printf("-- %s localChanged = %d  remote changed = %d\n", agb_merge_entry_name(entry), hasLocalChanged, hasRemoteChanged);
    return 0;
}

static int merge_changed(AGBMerger * self, AGBMergeEntry * entry) {
	printf("Got a change : self = %p, entry = %p\n", self, entry);
  return 0;
}

static int merge_add(AGBMerger * self, AGBMergeEntry * entry, enum AGBMergeIndex idx) {
    printf("ADDING  %s to tree from %s\n", agb_merge_entry_name(entry), index_to_string(idx));
    //git_treebuilder_insert(NULL, builder, agb_merge_entry_name(mergeEntry), agb_merge_entry_id(mergeEntry,AGB_MERGE_LOCAL), agb_merge_entry_filemode(mergeEntry,AGB_MERGE_LOCAL) );
    return 0;
}



static int merge_delete(AGBMerger * self, AGBMergeEntry * entry, enum AGBMergeIndex idx) {
    printf("REMOVING %s from tree - deleted in %s\n", agb_merge_entry_name(entry), index_to_string(idx) );
    //git_treebuilder_remove(builder, agb_merge_entry_name(entry) );
    return 0;
}

static int merge_update(AGBMerger * self, AGBMergeEntry * entry, enum AGBMergeIndex idx) {
    printf("UPDATING %s from tree - deleted in %s\n", agb_merge_entry_name(entry), index_to_string(idx) );
    //git_treebuilder_insert(NULL, builder, agb_merge_entry_name(mergeEntry), agb_merge_entry_id(mergeEntry,AGB_MERGE_LOCAL), agb_merge_entry_filemode(mergeEntry,AGB_MERGE_LOCAL) );
    return 0;
}

static int merge_done(AGBMerger * self) {
	printf("Merge completed : self = %p\n", self);
  return 0;
}

static int merge_failed(AGBMerger * self) {
	printf("Merge failed : self = %p\n", self);
  return 0;
}

/* Create a merge commit using the higher level API. */
void test_core_merge__demo_create_merge_commit2(void) {

    printf("RUNNING DEMO MERGE COMMIT2\n==================\n");

	AGBError * error;
	agb_error_new(&error);

	AGBMerger_vtable merge_callbacks = {
		.onConflict = merge_conflict,
		.onEveryEntry = merge_every,
            .onEveryChange = merge_changed,
            .onAdd = merge_add,
            .onRemove = merge_delete,
            .onModify = merge_update,

		.done = merge_done,
		.failed = merge_failed
	};



	AGBCore * core = NULL;
	if(agb_core_create(&core, REPODIR)!=0) {
		cl_fail("agb_core_create failed");
	}

	core->local_branch_name = "refs/heads/branch_a_better";
	core->remote_branch_name = "refs/heads/branch_b_better";

    AGBMerger merger;
    merger.vtable = & merge_callbacks;
    merger.core =core;
    merger.user_data = NULL;

    int ok = agb_merge(&merger, error);

	printf("agb_merge returned....\n");

    printf("DONE WITH DEMO MERGE COMMIT2\n==================\n");


}
