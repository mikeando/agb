#include "ANBGitBridge.h"
#include "git2.h"
#include <stdio.h>

void print_tree(git_repository * repo, const git_oid* tree_id, int indent);
void print_history(const git_commit* commit);
void find_merge_base_demo(git_repository * repo);
int remote_test(ANBGitBridge* anbGitBridge, ANBGitBridgeError * e);
void example();

int main() {

	anb_git_bridge_init();

	// Lets open ourselves a repo
	//
	git_repository * repo = NULL;
	int ok =  git_repository_open(&repo, "/tmp/git/test_repo");
	printf("git_repository_open returned %d\n", ok);

	git_reference * head_ref = NULL;
	ok = git_repository_head(&head_ref, repo);

	printf("REPO head is %s\n", git_reference_name(head_ref));

	git_oid head_commit;
	ok = git_reference_name_to_id(&head_commit, repo, "HEAD");

	printf("git_reference_name_to_id returned  %d\n",ok);

	//40 for SHA, +1 for trailing null
	char hex_sha[41] = {0};
	git_oid_fmt(hex_sha,&head_commit);
	printf("HEAD = %s\n",hex_sha);

	git_commit * commit;
	ok = git_commit_lookup(&commit, repo, &head_commit);
	printf("git_commit_lookup returned %d\n", ok);

	print_history(commit);

	const git_oid * tree_id = git_commit_tree_id(commit);
	git_oid_fmt(hex_sha,tree_id);
	printf("HEAD TREE ID = %s\n",hex_sha);

	print_tree(repo,tree_id,0);

	git_commit_free(commit);
	git_reference_free(head_ref);

	find_merge_base_demo(repo);

	ANBGitBridge anbGitBridge;
	anbGitBridge.repository = repo;
	anbGitBridge.origin_name = "origin";

	ANBGitBridgeError error;
	anb_git_bridge_error_init(&error);
	ok = remote_test(&anbGitBridge, &error);
	if(ok!=0) {
		printf("Error: %s\n", error.message);
	}
	anb_git_bridge_error_free(&error);

	git_repository_free(repo);

	example();
}

void print_indent(int indent) {
	for(int i=0;i<indent;++i) {
		printf(" >  ");
	}
}

void print_tree(git_repository * repo, const git_oid* tree_id, int indent) {
	git_tree * head_tree = NULL;
	git_tree_lookup(&head_tree, repo, tree_id);

	int nEntries = (int)git_tree_entrycount(head_tree) ;

	//40 for SHA, +1 for trailing null
	char hex_sha[41] = {0};

	for(int i=0; i<nEntries; ++i) {	
		const git_tree_entry * e = git_tree_entry_byindex(head_tree,i);
		
		git_otype entry_type = git_tree_entry_type(e);
		const git_oid * entry_id = git_tree_entry_id(e);

		git_oid_fmt(hex_sha,entry_id);
		print_indent(indent);
		printf("%d %s %s %s\n",i, hex_sha, git_object_type2string(entry_type), git_tree_entry_name(e));
		if( entry_type == GIT_OBJ_TREE ) {
			print_tree( repo, git_tree_entry_id(e), indent+1 );
		}
	}

	git_tree_free(head_tree);
}


void print_history(const git_commit* commit) {

	const git_commit * c = commit;
	char hex_sha[41] = {0};

	while( git_commit_parentcount(c) > 0 ) {
		git_oid_fmt(hex_sha, git_commit_id(c));
		printf("commit : %s %s\n", hex_sha, git_commit_message(c) ); 
		git_commit * next_commit = NULL;
		git_commit_parent(&next_commit, c, 0);
		c = next_commit;
	}
	git_oid_fmt(hex_sha, git_commit_id(c));
	printf("commit : %s %s\n", hex_sha, git_commit_message(c) ); 
	
}

void find_merge_base_demo(git_repository * repo) {
	printf("Testing finding a merge base\n");
	git_oid branch_a_id;
	git_oid branch_b_id;
	int ok;
	ok = git_reference_name_to_id(&branch_a_id,repo,"refs/heads/branch_a");
	if(ok!=0) {
		
		printf("git_reference_name_to_id for branch_a returned %d : %s \n",ok, giterr_last()->message);
		return;
	}
	ok = git_reference_name_to_id(&branch_b_id,repo,"refs/heads/branch_b");
	if(ok!=0) {
		printf("git_reference_name_to_id for branch_b returned %d : %s \n",ok, giterr_last()->message);
		return;
	}

	char hex_sha[41] = {0};
	git_oid_fmt(hex_sha, &branch_a_id);
	printf("branch_a = %s\n", hex_sha);
	git_oid_fmt(hex_sha, &branch_b_id);
	printf("branch_b = %s\n", hex_sha);

	//Get the merge base
	git_oid merge_base_id;
	ok = git_merge_base(&merge_base_id, repo, &branch_a_id, &branch_b_id);
	if(ok!=0) {
		printf("git_merge_base returned %d\n",ok);
		return;
	}
	git_oid_fmt(hex_sha, &merge_base_id);
	printf("merge_base = %s\n", hex_sha);
}

/**
 * This function gets called for each remote-tracking branch that gets
 * updated. The message we output depends on whether it's a new one or
 * an update.
 */
static int update_cb(const char *refname, const git_oid *a, const git_oid *b, void *data)
{
	printf("In update_cb....\n");
	char a_str[GIT_OID_HEXSZ+1], b_str[GIT_OID_HEXSZ+1];
	(void)data;

	git_oid_fmt(b_str, b);
	b_str[GIT_OID_HEXSZ] = '\0';

	if (git_oid_iszero(a)) {
		printf("[new]     %.20s %s\n", b_str, refname);
	} else {
		git_oid_fmt(a_str, a);
		a_str[GIT_OID_HEXSZ] = '\0';
		printf("[updated] %.10s..%.10s %s\n", a_str, b_str, refname);
	}

	return 0;
}
static int progress_cb(const char *str, int len, void *data)
{
	(void)data;
	printf("remote: %.*s", len, str);
	fflush(stdout); /* We don't have the \n to force the flush */
	return 0;
}

int cred_acquire_cb(git_cred **out,
		const char * url,
		const char * username_from_url,
		unsigned int allowed_types,
		void * payload)
{
	char username[128] = {0};
	char password[128] = {0};

	printf("Username: ");
	scanf("%s", username);

	/* Yup. Right there on your terminal. Careful where you copy/paste output. */
	printf("Password: ");
	scanf("%s", password);

	return git_cred_userpass_plaintext_new(out, username, password);
}

#include <stdarg.h>

int error_message(ANBGitBridgeError * e, int code, const char * fmt, ...) {
	if(e==NULL) return code;
	if(e->message) free((void*)e->message);
	va_list ap;
	va_start(ap, fmt);
	vasprintf((char**)&e->message,fmt, ap);
	va_end(ap);
	e->error_code = code;
	return code;
}

void my_anb_git_bridge_callback( ANBGitBridgeError * e, void * userdata ) {
	printf("In anb_git_bridge_callback( e = %p, userdata = %p )\n",e,userdata);
}

int remote_test(ANBGitBridge* anbGitBridge, ANBGitBridgeError * e) {

	git_repository * repo = anbGitBridge->repository;
	const char * origin_name = anbGitBridge->origin_name;

	//Get the remote
	git_remote * remote = NULL;
	int ok = git_remote_load(&remote, repo, origin_name);
	if(ok==GIT_ENOTFOUND) {
		return error_message(e, ok, "'%s' remote not found.", origin_name);
	}
 	if(ok==GIT_EINVALIDSPEC) {
		return error_message(e, ok, "'%s' remote has invalid spec.", origin_name);
	}
	if (ok < 0) {
		return error_message(e, ok, "Failed to load '%s': '%s'", origin_name, giterr_last()->message);
	}

	//Check what we're going to fetch
	int n_ref_specs = git_remote_refspec_count(remote);
	printf("We've got %d refspecs..\n",n_ref_specs);

	for(int i=0; i<n_ref_specs; ++i) {
		const git_refspec * refspec = git_remote_get_refspec( remote, i);
		printf("Refspec: %s\n",git_refspec_string(refspec) );
		
	}

	// Set up the callbacks (only update_tips for now)
	git_remote_callbacks callbacks = GIT_REMOTE_CALLBACKS_INIT;
	callbacks.update_tips = &update_cb;
	callbacks.progress = &progress_cb;
	callbacks.credentials = cred_acquire_cb;
	git_remote_set_callbacks(remote, &callbacks);

	const git_transfer_progress *stats = git_remote_stats(remote);
	printf("stats = %p\n",stats);

	//TODO: Run this on a worker thread? Like in the fetch.c example

	printf("Downloading....\n");

	// Connect to the remote end specifying that we want to fetch
	// information from it.
	if ((ok = git_remote_connect(remote, GIT_DIRECTION_FETCH)) < 0) {
		return error_message(e, ok, "Connection error: %s\n", giterr_last()->message );
	}

	// Download the packfile and index it. This function updates the
	// amount of received data and the indexer stats which lets you
	// inform the user about progress.
	if (git_remote_download(remote) < 0) {
		return error_message(e, ok, "Download error: %s\n", giterr_last()->message );
	}
	printf("Done downloading....\n");

	printf("\rReceived %d/%d objects in %zu bytes (used %d local objects)\n",
			stats->indexed_objects, stats->total_objects, stats->received_bytes, stats->local_objects);

	// Disconnect the underlying connection to prevent from idling.
	git_remote_disconnect(remote);

	// Update the references in the remote's namespace to point to the
	// right commits. This may be needed even if there was no packfile
	// to download, which can happen e.g. when the branches have been
	// changed but all the neede objects are available locally.
	if (git_remote_update_tips(remote, NULL, NULL) < 0) {
		printf("Error updating tips\n");
	}

	git_remote_free(remote);
	return 0;
}

void example() {

	git_repository * repo = NULL;
	int ok =  git_repository_open(&repo, "/tmp/git/test_repo");
	printf("git_repository_open returned %d\n", ok);

	ANBGitBridge anbGitBridge;
	anbGitBridge.repository = repo;
	anbGitBridge.origin_name = "origin";

	// Check if working dir is clean
	git_status_options statusopt = GIT_STATUS_OPTIONS_INIT;
	statusopt.show  = GIT_STATUS_SHOW_INDEX_AND_WORKDIR;
	statusopt.flags = GIT_STATUS_OPT_INCLUDE_UNTRACKED |
		GIT_STATUS_OPT_RENAMES_HEAD_TO_INDEX |
		GIT_STATUS_OPT_SORT_CASE_SENSITIVELY;

	git_status_list *status;
	ok = git_status_list_new(&status, repo, &statusopt);
	if(ok<0) {
		printf("status list failed\n");
		return;
	}

	const git_status_entry *s;
	size_t i, maxi = git_status_list_entrycount(status);
	for (i = 0; i < maxi; ++i) {
		s = git_status_byindex(status, i);
		printf("STATUS : %p\n",s);
	}

	git_status_list_free(status);

	//   + key seems to be git_status_list_new
	//   +   further details via  git_status_list_entrycount and git_status_byindex
	

	//
	//  - Commit if needed
	//
	// Fetch
	//
	// See if we're a fast-forward, behind, or merge
	//
	// -- FF => update origin and push
	// -- Behind => update head
	// -- merge => merge
}

