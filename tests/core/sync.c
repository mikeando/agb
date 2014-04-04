#include "clar.h"
#include "ANBGitBridge.h"

#include "utils/test_utils.h"
#include "utils/clarx.h"
#include <stdio.h>

void init_bridge(ANBGitBridge* anbGitBridge) {

	char * repo_path;
	asprintf(&repo_path, "%s/simple_repo", temp_dir ); 
	git_repository_open(&anbGitBridge->repository,repo_path);
	anbGitBridge->origin_name = "origin";
}

void init_bridge_with_repo(ANBGitBridge* anbGitBridge, const char * repo_name) {

	char * repo_path;
	asprintf(&repo_path, "%s/%s", temp_dir, repo_name ); 
	git_repository_open(&anbGitBridge->repository,repo_path);
	anbGitBridge->origin_name = "origin";
}

void uninit_bridge(ANBGitBridge* anbGitBridge) {
	git_repository_free(anbGitBridge->repository);
}

void check_git_ok(int ok) {
	if(ok!=0) {
		cl_fail(giterr_last()->message);
	}
}

int anb_git_bridge__translate(int errcode, const char * context, ANBGitBridgeError * error ) {
	//TODO: Do something with the git error info and plug it into the error object
	return 1;
}

int debug_me(const char * path, const char * refspec, void * payload) {
//	printf("==> %s debug_me : %s : %s\n",(char*) payload, path, refspec);
	return 0;
}

//TODO: Move this into ANBGitBridge.h
//TODO: Clean up resources correctly on failures
int anb_git_bridge_sync_files(ANBGitBridge* anbGitBridge, ANBGitBridgeError * error) {
	git_index *index=NULL;
	git_strarray array = {0};
	array.count = 1;
	array.strings = malloc(sizeof(char*) * array.count);
	array.strings[0]= "*";
	git_tree * tree = NULL;;
	//Author and Committer signature
	git_signature * author_signature = NULL;
       	const git_commit ** parents = NULL;

	int ok;
	const char * error_context = NULL;

	if((ok=git_repository_index(&index, anbGitBridge->repository ))!=0) {
		error_context = "git_repository_index";
		goto cleanup_error;
	}
	if((ok=git_index_add_all(index, &array, 0, NULL, NULL))!=0) {
		error_context = "git_index_add_all";
		goto cleanup_error;
	}
	if((ok=git_index_update_all(index, &array, NULL, NULL))!=0) {
		error_context = "git_index_update_all";
		goto cleanup_error;
	}

	if((ok=git_index_write(index))!=0) {
		error_context = "git_index_write";
		goto cleanup_error;
	}

	git_oid tree_oid;
	if((ok=git_index_write_tree(&tree_oid, index))!=0) {
		error_context = "git_index_write_tree";
		goto cleanup_error;
	}

	if((ok=git_tree_lookup(&tree, anbGitBridge->repository, &tree_oid))!=0) {
		error_context = "git_tree_lookup";
		goto cleanup_error;
	}



	// Time since epoch
	// TODO: Get these correctly - 
	// Could use git_signature_now instead...
	{	
		git_time_t author_time = 1396222461ull;
		int timezone_offset = 0;

		if((ok=git_signature_new(&author_signature,"Someone","someone@somewhere.com", author_time, timezone_offset))!=0) {
			error_context = "git_signature_new";
			goto cleanup_error;
		}
	}

      
       	parents = (const git_commit**)malloc(sizeof(git_commit*)*1);	

	git_oid head_commit;
	if((ok=git_reference_name_to_id(&head_commit, anbGitBridge->repository, "HEAD"))!=0) {
		error_context = "git_reference_name_to_id";
		goto cleanup_error;
	}

	if((ok=git_commit_lookup((git_commit**)&parents[0], anbGitBridge->repository, &head_commit))!=0) {
		error_context = "git_commit_lookup";
		goto cleanup_error;
	}
	
	

	//Create the commit
	git_oid commit_id;
	ok = git_commit_create(
				&commit_id, 
				anbGitBridge->repository,
				"HEAD",
				author_signature,
				author_signature,
				"UTF-8",
				"An exciting commit",
				tree,
			       	1,
				parents
				);
	if(ok!=0) {
		error_context = "git_commit_create";
		goto cleanup_error;
	}

	

	git_commit_free((git_commit*)parents[0]);
	free(parents);
	git_signature_free(author_signature);
	git_index_free(index);
	free(array.strings);
	return 0;
cleanup_error:
	if(parents) {
		git_commit_free((git_commit*)parents[0]);
		free(parents);
	}
	if(author_signature) {
		git_signature_free(author_signature);
	}
	if(index) {
		git_index_free(index);
	}
	if(array.strings){
		free(array.strings);
	}
	return anb_git_bridge__translate(ok,error_context,error);
}

void test_core_sync__add_simple(void) {
	create_simple_repo();
	simple_repo_create_file("dummy.dat","hello world");

	ANBGitBridge anbGitBridge;
	ANBGitBridgeError anbGitBridgeError;
	init_bridge(&anbGitBridge);

	anb_git_bridge_sync_files(&anbGitBridge, &anbGitBridgeError);

	cl_assert(simple_repo_contains("dummy.dat"));
	cl_assert_equal_c('A', status_in_commit("simple_repo", "HEAD", "dummy.dat"));	
}

void test_core_sync__add_in_subdir(void) {
	create_simple_repo();
	system_fmt("( cd %s/simple_repo ; mkdir somedir ; )", temp_dir);
	simple_repo_create_file("somedir/dummy.dat","hello world");

	ANBGitBridge anbGitBridge;
	ANBGitBridgeError anbGitBridgeError;
	init_bridge(&anbGitBridge);

	anb_git_bridge_sync_files(&anbGitBridge, &anbGitBridgeError);

	cl_assert(simple_repo_contains("somedir/dummy.dat"));
	cl_assert_equal_c('A', status_in_commit("simple_repo", "HEAD", "somedir/dummy.dat"));	
}

void test_core_sync__removes_deleted_files(void) {
	create_simple_repo();
	simple_repo_delete_file("README.txt");

	ANBGitBridge anbGitBridge;
	ANBGitBridgeError anbGitBridgeError;
	init_bridge(&anbGitBridge);

	anb_git_bridge_sync_files(&anbGitBridge, &anbGitBridgeError);

	cl_assert(!simple_repo_contains("README.txt"));
	cl_assert_equal_c('D', status_in_commit("simple_repo", "HEAD", "README.txt"));	
}

void test_core_sync__saves_modified_files(void) {
	create_simple_repo();
	ANBGitBridge anbGitBridge;
	ANBGitBridgeError anbGitBridgeError;
	init_bridge(&anbGitBridge);
	system_fmt("echo 'HelloAgain' >> %s/simple_repo/README.txt", temp_dir);
	anb_git_bridge_sync_files(&anbGitBridge, &anbGitBridgeError);
	cl_assert_equal_c('M', status_in_commit("simple_repo", "HEAD", "README.txt"));	
}

void test_core_sync__gets_repo_from_bridge(void) {
	create_repo("tofu");
	repo_create_file("tofu","dummy.dat","hello world");

	ANBGitBridge anbGitBridge;
	ANBGitBridgeError anbGitBridgeError;
	init_bridge_with_repo(&anbGitBridge,"tofu");

	anb_git_bridge_sync_files(&anbGitBridge, &anbGitBridgeError);

	cl_assert(repo_contains("tofu", "dummy.dat"));
	cl_assert_equal_c('A', status_in_commit("tofu", "HEAD", "dummy.dat"));	
}
